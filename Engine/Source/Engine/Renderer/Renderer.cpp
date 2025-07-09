#include <memory>

#include "Engine/Renderer/Device.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Platform/IWindow.hpp"
#include "Engine/Utility/Logger.hpp"

#include "Engine/Renderer/Renderer.hpp"

namespace Engine::Renderer
{
  const std::vector<const char *> Renderer::s_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation" };

  Renderer::Renderer( Platform::IWindow & window )
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

    m_Window.SetEventCallback(
      [ this ]( const Platform::WindowEvent & event )
      {
        if ( std::holds_alternative<Platform::WindowResizeEvent>( event ) )
        {
          m_IsFramebufferResized = true;
        }
      } );
  }

  Renderer::~Renderer()
  {
    if ( m_Device )
    {
      m_Device->Wait();
    }

    for ( std::size_t i = 0; i < s_MaxFramesInFlight; ++i )
    {
      if ( m_RenderSemaphores.at( i ) )
      {
        m_Device->Get().destroySemaphore( m_RenderSemaphores.at( i ) );
      }

      if ( m_ImageSemaphores.at( i ) )
      {
        m_Device->Get().destroySemaphore( m_ImageSemaphores.at( i ) );
      }

      if ( m_FencesInFlight.at( i ) )
      {
        m_Device->Get().destroyFence( m_FencesInFlight.at( i ) );
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

    auto wait = m_Device->Get().waitForFences(
      1, &m_FencesInFlight.at( m_CurrentFrame ), vk::True,
      std::numeric_limits<u64>::max() );

    if ( wait != vk::Result::eSuccess )
    {
      LOG_ERROR( "Failed to wait for fence!" );
      return;
    }

    auto acquired = m_Device->Get().acquireNextImageKHR(
      m_SwapChain->Get(), std::numeric_limits<u64>::max(),
      m_ImageSemaphores.at( m_CurrentFrame ), nullptr );

    if ( acquired.result == vk::Result::eErrorOutOfDateKHR )
    {
      RecreateSwapChain();
      return;
    }

    if ( acquired.result != vk::Result::eSuccess &&
         acquired.result != vk::Result::eSuboptimalKHR )
    {
      LOG_FATAL( "Failed to acquire swap chain image!" );
    }

    m_ImageIndex = acquired.value;

    m_Device->Get().resetFences( 1, &m_FencesInFlight.at( m_CurrentFrame ) );
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

    vk::Semaphore          waits[] = { m_ImageSemaphores.at( m_CurrentFrame ) };
    vk::PipelineStageFlags stages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submission {};
    submission.waitSemaphoreCount = 1;
    submission.pWaitSemaphores    = waits;
    submission.pWaitDstStageMask  = stages;
    submission.commandBufferCount = 1;
    submission.pCommandBuffers    = &m_CommandBuffers.at( m_CurrentFrame );

    vk::Semaphore signals[] = { m_RenderSemaphores.at( m_CurrentFrame ) };
    submission.signalSemaphoreCount = 1;
    submission.pSignalSemaphores    = signals;

    m_Device->GetGraphicsQueue().submit(
      submission, m_FencesInFlight.at( m_CurrentFrame ) );

    vk::PresentInfoKHR present = {};
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores    = signals;

    const vk::SwapchainKHR SwapChains[] = { m_SwapChain->Get() };
    present.swapchainCount              = 1;
    present.pSwapchains                 = SwapChains;
    present.pImageIndices               = &m_ImageIndex;

    auto result = m_Device->GetPresentQueue().presentKHR( present );
    if ( result == vk::Result::eErrorOutOfDateKHR ||
         result == vk::Result::eSuboptimalKHR || m_IsFramebufferResized )
    {
      m_IsFramebufferResized = false;
      RecreateSwapChain();
    }
    else if ( result != vk::Result::eSuccess )
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

  bool Renderer::IsFrameInProgress() const
  {
    return m_IsFrameStarted;
  }

  void Renderer::CreateInstance()
  {
    if ( !IsValidationLayerSupported() )
    {
      LOG_ERROR( "No validation layers available!" );
      return;
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
    if constexpr ( s_IsValidationLayerEnabled )
    {
      auto vkGetInstanceProcAddr =
        m_Loader.getProcAddress<PFN_vkGetInstanceProcAddr>(
          "vkGetInstanceProcAddr" );
      m_Dispatch =
        vk::detail::DispatchLoaderDynamic { m_Instance, vkGetInstanceProcAddr };

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
    m_ImageSemaphores.resize( s_MaxFramesInFlight );
    m_RenderSemaphores.resize( s_MaxFramesInFlight );
    m_FencesInFlight.resize( s_MaxFramesInFlight );

    if ( s_MaxFramesInFlight == 0 )
    {
      LOG_FATAL( "s_MaxFramesInFlight cannot be zero!" );
    }

    vk::FenceCreateInfo fence = {};
    fence.flags               = vk::FenceCreateFlagBits::eSignaled;

    constexpr vk::SemaphoreCreateInfo Semaphore = {};
    for ( std::size_t i = 0; i < s_MaxFramesInFlight; i++ )
    {
      try
      {
        m_ImageSemaphores.at( i ) =
          m_Device->Get().createSemaphore( Semaphore );
        m_RenderSemaphores.at( i ) =
          m_Device->Get().createSemaphore( Semaphore );
        m_FencesInFlight.at( i ) = m_Device->Get().createFence( fence );
      }
      catch ( const vk::SystemError & E )
      {
        LOG_FATAL( "Failed to create synchronization objects: {}", E.what() );
      }
    }
  }

  void Renderer::RecreateSwapChain()
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

  bool Renderer::IsValidationLayerSupported() const
  {
    auto available = vk::enumerateInstanceLayerProperties();

    for ( const char * name : s_ValidationLayers )
    {
      bool isFound = false;

      for ( const auto & properties : available )
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
    vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
    vk::DebugUtilsMessageTypeFlagsEXT              type,
    const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
    void *                                         pUserData )
  {
    if ( severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning )
    {
      LOG_WARN( "Validation layer: {}", pCallbackData->pMessage );
    }

    return vk::False;
  }

} // namespace Engine::Renderer
