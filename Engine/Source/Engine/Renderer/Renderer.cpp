#include <memory>

#include "Engine/Renderer/Device.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Platform/Window.hpp"
#include "Engine/Utility/Logger.hpp"

#include "Engine/Renderer/Renderer.hpp"

namespace Engine::Renderer
{
  const std::vector<const char *> Renderer::s_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation" };

  Renderer::Renderer( Platform::Window & window )
    : m_Window( window )
    , m_Device( nullptr )
    , m_SwapChain( nullptr )
    , m_CurrentFrame( 0 )
    , m_ImageIndex( 0 )
    , m_IsFrameStarted( false )
    , m_IsFramebufferResized( false )
  {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();

    m_Device    = std::make_unique<Device>( m_Instance, m_Surface );
    m_SwapChain = std::make_unique<SwapChain>(
      *m_Device, m_Surface, m_Window.GetWidth(), m_Window.GetHeight() );

    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
  }

  Renderer::~Renderer()
  {
    if ( m_Device )
    {
      m_Device->Wait();
    }

    for ( std::size_t i = 0; i < s_MaxFramesInFlight; ++i )
    {
      if ( m_ImageSemaphores.at( i ) )
      {
        m_Device->Get().destroySemaphore( m_ImageSemaphores.at( i ) );
      }

      if ( m_FencesInFlight.at( i ) )
      {
        m_Device->Get().destroyFence( m_FencesInFlight.at( i ) );
      }
    }

    for ( auto semaphore : m_RenderSemaphores )
    {
      if ( semaphore )
      {
        m_Device->Get().destroySemaphore( semaphore );
      }
    }

    if ( m_CommandPool )
    {
      m_Device->Get().destroyCommandPool( m_CommandPool );
    }

    m_SwapChain.reset();
    m_Device.reset();

    if ( m_Surface )
    {
      m_Instance.destroySurfaceKHR( m_Surface );
    }

    if ( m_DebugMessenger )
    {
      m_Instance.destroyDebugUtilsMessengerEXT( m_DebugMessenger, nullptr,
                                                m_Dispatch );
    }

    if ( m_Instance )
    {
      m_Instance.destroy();
    }
  }

  void Renderer::BeginDraw()
  {
    if ( m_IsFrameStarted )
    {
      LOG_ERROR( "BeginDraw invalid while draw is in progress!" );
      return;
    }

    if ( m_CurrentFrame >= s_MaxFramesInFlight )
    {
      LOG_FATAL( "Current frame index out of bounds: {}", m_CurrentFrame );
      return;
    }

    const auto Wait = m_Device->Get().waitForFences(
      1, &m_FencesInFlight.at( m_CurrentFrame ), vk::True,
      std::numeric_limits<u64>::max() );

    if ( Wait != vk::Result::eSuccess )
    {
      LOG_ERROR( "Failed to wait for fence!" );
      return;
    }

    const auto Acquired = m_Device->Get().acquireNextImageKHR(
      m_SwapChain->Get(), std::numeric_limits<u64>::max(),
      m_ImageSemaphores.at( m_CurrentFrame ), nullptr );

    if ( Acquired.result == vk::Result::eErrorOutOfDateKHR )
    {
      RecreateSwapChain();
      return;
    }

    if ( Acquired.result != vk::Result::eSuccess &&
         Acquired.result != vk::Result::eSuboptimalKHR )
    {
      LOG_FATAL( "Failed to acquire swap chain image!" );
      return;
    }

    m_ImageIndex = Acquired.value;

    if ( m_ImageIndex >= m_ImagesInFlight.size() )
    {
      LOG_FATAL( "Image index out of bounds: {} >= {}", m_ImageIndex,
                 m_ImagesInFlight.size() );
      return;
    }

    // If image is currently in flight, wait for it to be available
    if ( m_ImagesInFlight.at( m_ImageIndex ) != nullptr )
    {
      const auto WaitResult = m_Device->Get().waitForFences(
        1, &m_ImagesInFlight.at( m_ImageIndex ), vk::True,
        std::numeric_limits<u64>::max() );

      if ( WaitResult != vk::Result::eSuccess )
      {
        LOG_ERROR( "Failed to wait for image fence!" );
        return;
      }
    }

    m_ImagesInFlight.at( m_ImageIndex ) = m_FencesInFlight.at( m_CurrentFrame );

    const auto ResetResult =
      m_Device->Get().resetFences( 1, &m_FencesInFlight.at( m_CurrentFrame ) );

    if ( ResetResult != vk::Result::eSuccess )
    {
      LOG_FATAL( "Failed to reset fence: {}", vk::to_string( ResetResult ) );
      return;
    }

    m_CommandBuffers.at( m_CurrentFrame ).reset();

    constexpr vk::CommandBufferBeginInfo CommandBuffer = {};
    m_CommandBuffers.at( m_CurrentFrame ).begin( CommandBuffer );

    vk::RenderPassBeginInfo renderPass = {};
    renderPass.renderPass              = m_SwapChain->GetRenderPass();
    renderPass.framebuffer = m_SwapChain->GetFramebuffers().at( m_ImageIndex );
    renderPass.renderArea.offset = vk::Offset2D { 0, 0 };
    renderPass.renderArea.extent = m_SwapChain->GetExtent();

    vk::ClearValue clear       = {};
    clear.color                = m_ClearColor;
    renderPass.clearValueCount = 1;
    renderPass.pClearValues    = &clear;

    m_CommandBuffers.at( m_CurrentFrame )
      .beginRenderPass( renderPass, vk::SubpassContents::eInline );

    m_IsFrameStarted = true;
  }

  void Renderer::EndDraw()
  {
    if ( !m_IsFrameStarted )
    {
      LOG_ERROR( "EndDraw invalid when frame not in progress!" );
      return;
    }

    m_CommandBuffers.at( m_CurrentFrame ).endRenderPass();
    m_CommandBuffers.at( m_CurrentFrame ).end();

    const vk::Semaphore Waits[] = { m_ImageSemaphores.at( m_CurrentFrame ) };
    constexpr vk::PipelineStageFlags Stages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submission {};
    submission.waitSemaphoreCount   = 1;
    submission.pWaitSemaphores      = Waits;
    submission.pWaitDstStageMask    = Stages;
    submission.commandBufferCount   = 1;
    submission.pCommandBuffers      = &m_CommandBuffers.at( m_CurrentFrame );
    submission.pSignalSemaphores    = &m_RenderSemaphores.at( m_ImageIndex );
    submission.signalSemaphoreCount = 1;

    m_Device->GetGraphicsQueue().submit(
      submission, m_FencesInFlight.at( m_CurrentFrame ) );

    vk::PresentInfoKHR present = {};
    present.pWaitSemaphores    = &m_RenderSemaphores.at( m_ImageIndex );
    present.waitSemaphoreCount = 1;

    const vk::SwapchainKHR SwapChains[] = { m_SwapChain->Get() };
    present.swapchainCount              = 1;
    present.pSwapchains                 = SwapChains;
    present.pImageIndices               = &m_ImageIndex;

    if ( const auto Result = m_Device->GetPresentQueue().presentKHR( present );
         Result == vk::Result::eErrorOutOfDateKHR ||
         Result == vk::Result::eSuboptimalKHR || m_IsFramebufferResized )
    {
      m_IsFramebufferResized = false;
      RecreateSwapChain();
    }
    else if ( Result != vk::Result::eSuccess )
    {
      LOG_ERROR( "Failed to present swap chain image!" );
      return;
    }

    m_IsFrameStarted = false;
    m_CurrentFrame   = ( m_CurrentFrame + 1 ) % s_MaxFramesInFlight;
  }

  void Renderer::Clear( f32 r, f32 g, f32 b, f32 a /*= 1.0f */ )
  {
    m_ClearColor = { r, g, b, a };
  }

  void Renderer::Resize( u32 width, u32 height )
  {
    m_IsFramebufferResized = true;
  }

  bool Renderer::IsFrameInProgress() const
  {
    return m_IsFrameStarted;
  }

  void Renderer::CreateInstance()
  {
    if ( s_IsValidationLayerEnabled && !IsValidationLayerSupported() )
    {
      LOG_WARN( "No validation layers available!" );
    }

    vk::ApplicationInfo app = {};
    app.pApplicationName    = m_Window.GetTitle().c_str();
    app.applicationVersion  = vk::makeApiVersion( 0, 0, 1, 0 );
    app.pEngineName         = app.pApplicationName;
    app.engineVersion       = app.applicationVersion;
    app.apiVersion          = vk::ApiVersion10;

    vk::InstanceCreateInfo instance = {};
    instance.pApplicationInfo       = &app;

    const auto Extensions            = GetRequiredExtensions();
    instance.enabledExtensionCount   = static_cast<u32>( Extensions.size() );
    instance.ppEnabledExtensionNames = Extensions.data();

    vk::DebugUtilsMessengerCreateInfoEXT messenger = {};
    if ( s_IsValidationLayerEnabled )
    {
      instance.enabledLayerCount =
        static_cast<u32>( s_ValidationLayers.size() );
      instance.ppEnabledLayerNames = s_ValidationLayers.data();

      messenger.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
      messenger.messageType =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
      messenger.pfnUserCallback = DebugCallback;

      instance.pNext = &messenger;
    }
    else
    {
      instance.enabledLayerCount = 0;
      instance.pNext             = nullptr;
    }

    try
    {
      m_Instance = vk::createInstance( instance );
    }
    catch ( const vk::SystemError & E )
    {
      LOG_FATAL( "Failed to create instance: {}", E.what() );
    }
  }

  void Renderer::SetupDebugMessenger()
  {
    if ( s_IsValidationLayerEnabled && IsValidationLayerSupported() )
    {
      const auto VkGetInstanceProcAddr =
        m_Loader.getProcAddress<PFN_vkGetInstanceProcAddr>(
          "vkGetInstanceProcAddr" );
      m_Dispatch =
        vk::detail::DispatchLoaderDynamic { m_Instance, VkGetInstanceProcAddr };

      vk::DebugUtilsMessengerCreateInfoEXT messenger = {};
      messenger.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
      messenger.messageType =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
      messenger.pfnUserCallback = DebugCallback;

      try
      {
        m_DebugMessenger = m_Instance.createDebugUtilsMessengerEXT(
          messenger, nullptr, m_Dispatch );
      }
      catch ( const vk::SystemError & E )
      {
        LOG_FATAL( "Failed to set up debug messenger: {}", E.what() );
      }
    }
  }

  void Renderer::CreateSurface()
  {
    auto result = m_Window.CreateSurface( m_Instance );
    if ( !result )
    {
      LOG_ERROR( "Failed to create surface!\n\t{}", result.GetError() );
      return;
    }

    m_Surface = result.GetValue();
  }

  void Renderer::CreateCommandPool()
  {
    const auto & [ m_GraphicsFamily, m_PresentFamily ] =
      m_Device->GetQueueFamilyIndices();

    vk::CommandPoolCreateInfo commandPool = {};
    commandPool.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    commandPool.queueFamilyIndex = m_GraphicsFamily.value();

    try
    {
      m_CommandPool = m_Device->Get().createCommandPool( commandPool );
    }
    catch ( const vk::SystemError & E )
    {
      LOG_FATAL( "Failed to create command pool: {}", E.what() );
    }
  }

  void Renderer::CreateCommandBuffers()
  {
    m_CommandBuffers.resize( s_MaxFramesInFlight );

    vk::CommandBufferAllocateInfo commandBuffer = {};
    commandBuffer.commandPool                   = m_CommandPool;
    commandBuffer.level = vk::CommandBufferLevel::ePrimary;
    commandBuffer.commandBufferCount =
      static_cast<u32>( m_CommandBuffers.size() );

    try
    {
      m_CommandBuffers =
        m_Device->Get().allocateCommandBuffers( commandBuffer );
    }
    catch ( const vk::SystemError & E )
    {
      LOG_FATAL( "Failed to allocate command buffers: {}", E.what() );
    }
  }

  void Renderer::CreateSyncObjects()
  {
    const std::size_t ImageCount = m_SwapChain->GetImageCount();

    m_ImageSemaphores.resize( s_MaxFramesInFlight );
    m_RenderSemaphores.resize( ImageCount );
    m_FencesInFlight.resize( s_MaxFramesInFlight );
    m_ImagesInFlight.resize( ImageCount, nullptr );

    if constexpr ( s_MaxFramesInFlight == 0 )
    {
      LOG_FATAL( "s_MaxFramesInFlight cannot be zero!" );
    }

    vk::FenceCreateInfo fence = {};
    fence.flags               = vk::FenceCreateFlagBits::eSignaled;

    constexpr vk::SemaphoreCreateInfo Semaphore = {};
    for ( std::size_t i = 0; i < s_MaxFramesInFlight; ++i )
    {
      try
      {
        m_ImageSemaphores.at( i ) =
          m_Device->Get().createSemaphore( Semaphore );
        m_FencesInFlight.at( i ) = m_Device->Get().createFence( fence );
      }
      catch ( const vk::SystemError & E )
      {
        LOG_FATAL( "Failed to create synchronization objects: {}", E.what() );
      }
    }

    for ( std::size_t i = 0; i < ImageCount; ++i )
    {
      try
      {
        m_RenderSemaphores.at( i ) =
          m_Device->Get().createSemaphore( Semaphore );
      }
      catch ( const vk::SystemError & E )
      {
        LOG_FATAL( "Failed to create render semaphores: {}", E.what() );
      }
    }
  }

  void Renderer::RecreateSwapChain() const
  {
    u32 width  = m_Window.GetWidth();
    u32 height = m_Window.GetHeight();

    while ( width == 0 || height == 0 )
    {
      m_Window.PollEvents();
      width  = m_Window.GetWidth();
      height = m_Window.GetHeight();
    }

    m_Device->Wait();
    m_SwapChain->Recreate( width, height );
  }

  bool Renderer::IsValidationLayerSupported()
  {
    const auto Available = vk::enumerateInstanceLayerProperties();

    for ( const char * name : s_ValidationLayers )
    {
      bool isFound = false;

      for ( const auto & properties : Available )
      {
        if ( std::strcmp( name, properties.layerName ) == 0 )
        {
          isFound = true;
          break;
        }
      }

      if ( !isFound )
      {
        return false;
      }
    }

    return true;
  }

  std::vector<const char *> Renderer::GetRequiredExtensions() const
  {
    auto extensions = m_Window.GetRequiredExtensions();

    if ( s_IsValidationLayerEnabled )
    {
      extensions.push_back( vk::EXTDebugUtilsExtensionName );
    }

    return extensions;
  }

  VKAPI_ATTR vk::Bool32 VKAPI_CALL Renderer::DebugCallback(
    const vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT,
    const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData, void * )
  {
    if ( severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning )
    {
      LOG_WARN( "Validation layer: {}", pCallbackData->pMessage );
    }

    return vk::False;
  }

} // namespace Engine::Renderer
