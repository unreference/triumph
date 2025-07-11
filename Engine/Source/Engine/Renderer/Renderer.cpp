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
    , m_pInstance( nullptr )
    , m_pSurface( nullptr )
    , m_pDevice( nullptr )
    , m_pSwapChain( nullptr )
    , m_pCommandPool( nullptr )
    , m_CurrentFrame( 0 )
    , m_ImageIndex( 0 )
    , m_IsFrameStarted( false )
    , m_IsFramebufferResized( false )
    , m_pDebugMessenger( nullptr )
  {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();

    m_pDevice    = std::make_unique<Device>( m_pInstance, m_pSurface );
    m_pSwapChain = std::make_unique<SwapChain>(
      *m_pDevice, m_pSurface, m_Window.GetWidth(), m_Window.GetHeight() );

    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncObjects();
  }

  Renderer::~Renderer()
  {
    if ( m_pDevice )
    {
      m_pDevice->Wait();
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

    const auto Wait = m_pDevice->Get().waitForFences(
      { *m_pFencesInFlight.at( m_CurrentFrame ) }, vk::True,
      std::numeric_limits<u64>::max() );

    if ( Wait != vk::Result::eSuccess )
    {
      LOG_ERROR( "Failed to wait for fence!" );
      return;
    }

    const auto Acquired = m_pSwapChain->Get().acquireNextImage(
      std::numeric_limits<u64>::max(),
      *m_pImageSemaphores.at( m_CurrentFrame ) );

    if ( Acquired.first == vk::Result::eErrorOutOfDateKHR )
    {
      RecreateSwapChain();
      return;
    }

    if ( Acquired.first != vk::Result::eSuccess &&
         Acquired.first != vk::Result::eSuboptimalKHR )
    {
      LOG_FATAL( "Failed to acquire swap chain image!" );
      return;
    }

    m_ImageIndex = Acquired.second;

    if ( m_ImageIndex >= m_pImagesInFlight.size() )
    {
      LOG_FATAL( "Image index out of bounds: {} >= {}", m_ImageIndex,
                 m_pImagesInFlight.size() );
      return;
    }

    if ( m_pImagesInFlight.at( m_ImageIndex ) != nullptr )
    {
      const vk::Result waitResult = m_pDevice->Get().waitForFences(
        { **m_pImagesInFlight.at( m_ImageIndex ) }, vk::True,
        std::numeric_limits<u64>::max() );

      if ( waitResult != vk::Result::eSuccess )
      {
        LOG_ERROR( "Failed to wait for image fence!" );
        return;
      }
    }

    m_pImagesInFlight.at( m_ImageIndex ) =
      &m_pFencesInFlight.at( m_CurrentFrame );

    m_pDevice->Get().resetFences( { *m_pFencesInFlight.at( m_CurrentFrame ) } );

    m_CommandBuffers.at( m_CurrentFrame ).reset();

    constexpr vk::CommandBufferBeginInfo CommandBuffer = {};
    m_CommandBuffers.at( m_CurrentFrame ).begin( CommandBuffer );

    vk::RenderPassBeginInfo renderPass = {};
    renderPass.renderPass              = m_pSwapChain->GetRenderPass();
    renderPass.framebuffer = m_pSwapChain->GetFramebuffers().at( m_ImageIndex );
    renderPass.renderArea.offset = vk::Offset2D { 0, 0 };
    renderPass.renderArea.extent = m_pSwapChain->GetExtent();

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

    const vk::Semaphore waits[] = { *m_pImageSemaphores.at( m_CurrentFrame ) };
    constexpr vk::PipelineStageFlags Stages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submission {};
    submission.waitSemaphoreCount   = 1;
    submission.pWaitSemaphores      = waits;
    submission.pWaitDstStageMask    = Stages;
    submission.commandBufferCount   = 1;
    submission.pCommandBuffers      = &*m_CommandBuffers.at( m_CurrentFrame );
    submission.pSignalSemaphores    = &*m_pRenderSemaphores.at( m_ImageIndex );
    submission.signalSemaphoreCount = 1;

    m_pDevice->GetGraphicsQueue().submit(
      submission, *m_pFencesInFlight.at( m_CurrentFrame ) );

    vk::PresentInfoKHR present = {};
    present.pWaitSemaphores    = &*m_pRenderSemaphores.at( m_ImageIndex );
    present.waitSemaphoreCount = 1;

    const vk::SwapchainKHR swapChains[] = { *m_pSwapChain->Get() };
    present.swapchainCount              = 1;
    present.pSwapchains                 = swapChains;
    present.pImageIndices               = &m_ImageIndex;

    if ( const auto Result = m_pDevice->GetPresentQueue().presentKHR( present );
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
      m_pInstance = m_pContext.createInstance( instance );
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
        m_pDebugMessenger =
          m_pInstance.createDebugUtilsMessengerEXT( messenger );
      }
      catch ( const vk::SystemError & E )
      {
        LOG_FATAL( "Failed to set up debug messenger: {}", E.what() );
      }
    }
  }

  void Renderer::CreateSurface()
  {
    auto result = m_Window.CreateSurface( *m_pInstance );
    if ( !result )
    {
      LOG_ERROR( "Failed to create surface: {}", result.GetError() );
      return;
    }

    m_pSurface = vk::raii::SurfaceKHR( m_pInstance, result.GetValue() );
  }

  void Renderer::CreateCommandPool()
  {
    const auto & [ graphicsFamily, presentFamily ] =
      m_pDevice->GetQueueFamilyIndices();

    vk::CommandPoolCreateInfo commandPool = {};
    commandPool.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    commandPool.queueFamilyIndex = graphicsFamily.value();

    try
    {
      m_pCommandPool = m_pDevice->Get().createCommandPool( commandPool );
    }
    catch ( const vk::SystemError & E )
    {
      LOG_FATAL( "Failed to create command pool: {}", E.what() );
    }
  }

  void Renderer::CreateCommandBuffers()
  {
    m_CommandBuffers.clear();
    m_CommandBuffers.reserve( s_MaxFramesInFlight );

    vk::CommandBufferAllocateInfo commandBuffer = {};
    commandBuffer.commandPool                   = *m_pCommandPool;
    commandBuffer.level              = vk::CommandBufferLevel::ePrimary;
    commandBuffer.commandBufferCount = s_MaxFramesInFlight;

    try
    {
      auto commandBuffers =
        m_pDevice->Get().allocateCommandBuffers( commandBuffer );
      for ( auto && buffer : commandBuffers )
      {
        m_CommandBuffers.emplace_back( std::move( buffer ) );
      }
    }
    catch ( const vk::SystemError & E )
    {
      LOG_FATAL( "Failed to allocate command buffers: {}", E.what() );
    }
  }

  void Renderer::CreateSyncObjects()
  {
    const std::size_t ImageCount = m_pSwapChain->GetImageCount();

    m_pImageSemaphores.clear();
    m_pRenderSemaphores.clear();
    m_pFencesInFlight.clear();
    m_pImagesInFlight.clear();

    m_pImageSemaphores.reserve( s_MaxFramesInFlight );
    m_pRenderSemaphores.reserve( ImageCount );
    m_pFencesInFlight.reserve( s_MaxFramesInFlight );
    m_pImagesInFlight.resize( ImageCount, nullptr );

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
        m_pImageSemaphores.emplace_back(
          m_pDevice->Get().createSemaphore( Semaphore ) );
        m_pFencesInFlight.emplace_back( m_pDevice->Get().createFence( fence ) );
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
        m_pRenderSemaphores.emplace_back(
          m_pDevice->Get().createSemaphore( Semaphore ) );
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

    m_pDevice->Wait();
    m_pSwapChain->Recreate( width, height );
  }

  bool Renderer::IsValidationLayerSupported()
  {
    const auto Available = vk::enumerateInstanceLayerProperties();

    for ( const char * Name : s_ValidationLayers )
    {
      bool isFound = false;

      for ( const auto & Properties : Available )
      {
        if ( std::strcmp( Name, Properties.layerName ) == 0 )
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

  VKAPI_ATTR VKAPI_ATTR vk::Bool32 Renderer::DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
    vk::DebugUtilsMessageTypeFlagsEXT              type,
    const vk::DebugUtilsMessengerCallbackDataEXT * callbackData,
    void *                                         userData )
  {
    if ( severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning )
    {
      LOG_WARN( "Validation layer: {}", callbackData->pMessage );
    }

    return vk::False;
  }

} // namespace Engine::Renderer
