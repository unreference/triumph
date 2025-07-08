#include "Engine/Renderer/Device.hpp"

#include "Engine/Renderer/SwapChain.hpp"

namespace Engine::Renderer
{
  SwapChain::SwapChain( const Device & device, const vk::SurfaceKHR surface,
                        const u32 width, const u32 height )
    : m_Device( device )
    , m_Surface( surface )
    , m_Width( width )
    , m_Height( height )
  {
    Create();
    CreateImageViews();
    CreateRenderPass();
    CreateFramebuffers();
  }

  void SwapChain::Recreate( const u32 width, const u32 height )
  {
    m_Width  = width;
    m_Height = height;

    m_Device.Wait();

    Cleanup();

    Create();
    CreateImageViews();
    CreateFramebuffers();
  }

  vk::SwapchainKHR SwapChain::Get() const
  {
    return m_SwapChain;
  }

  vk::Format SwapChain::GetImageFormat() const
  {
    return m_ImageFormat;
  }

  vk::Extent2D SwapChain::GetExtent() const
  {
    return m_Extent;
  }

  const std::vector<vk::Image> & SwapChain::GetImages() const
  {
    return m_Images;
  }

  const std::vector<vk::ImageView> & SwapChain::GetImageViews() const
  {
    return m_ImageViews;
  }

  const std::vector<vk::Framebuffer> & SwapChain::GetFramebuffers() const
  {
    return m_Framebuffers;
  }

  vk::RenderPass SwapChain::GetRenderPass() const
  {
    return m_RenderPass;
  }

  std::size_t SwapChain::GetImageCount() const
  {
    return m_Images.size();
  }

  void SwapChain::Create()
  {
    auto [ m_Capabilities, m_Formats, m_PresentModes ] =
      QuerySwapChainSupport( m_Device.GetPhysicalDevice() );

    const vk::SurfaceFormatKHR Surface   = ChooseSurfaceFormat( m_Formats );
    const vk::PresentModeKHR PresentMode = ChoosePresentMode( m_PresentModes );
    const vk::Extent2D       Extent      = ChooseExtent( m_Capabilities );

    u32 imageCount = m_Capabilities.minImageCount + 1;
    if ( m_Capabilities.maxImageCount > 0 &&
         imageCount > m_Capabilities.maxImageCount )
    {
      imageCount = m_Capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChain = {};
    swapChain.surface                    = m_Surface;
    swapChain.minImageCount              = imageCount;
    swapChain.imageFormat                = Surface.format;
    swapChain.imageColorSpace            = Surface.colorSpace;
    swapChain.imageExtent                = Extent;
    swapChain.imageArrayLayers           = 1;
    swapChain.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    const auto & [ m_GraphicsFamily, m_PresentFamily ] =
      m_Device.GetQueueFamilyIndices();
    const u32 FamilyIndices[] = { m_GraphicsFamily.value(),
                                  m_PresentFamily.value() };

    if ( m_GraphicsFamily != m_PresentFamily )
    {
      swapChain.imageSharingMode      = vk::SharingMode::eConcurrent;
      swapChain.queueFamilyIndexCount = 2;
      swapChain.pQueueFamilyIndices   = FamilyIndices;
    }
    else
    {
      swapChain.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapChain.preTransform   = m_Capabilities.currentTransform;
    swapChain.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChain.presentMode    = PresentMode;
    swapChain.clipped        = vk::True;
    swapChain.oldSwapchain   = nullptr;

    try
    {
      m_SwapChain = m_Device.Get().createSwapchainKHR( swapChain );
    }
    catch ( const vk::SystemError & E )
    {
      throw std::runtime_error(
        std::format( "Failed to create swap chain: {}", E.what() ) );
    }

    m_Images      = m_Device.Get().getSwapchainImagesKHR( m_SwapChain );
    m_ImageFormat = Surface.format;
    m_Extent      = Extent;
  }

  void SwapChain::CreateImageViews()
  {
    m_ImageViews.resize( m_Images.size() );

    for ( std::size_t i = 0; i < m_Images.size(); ++i )
    {
      vk::ImageViewCreateInfo imageView       = {};
      imageView.image                         = m_Images.at( i );
      imageView.viewType                      = vk::ImageViewType::e2D;
      imageView.format                        = m_ImageFormat;
      imageView.components.r                  = vk::ComponentSwizzle::eIdentity;
      imageView.components.g                  = vk::ComponentSwizzle::eIdentity;
      imageView.components.b                  = vk::ComponentSwizzle::eIdentity;
      imageView.subresourceRange.aspectMask   = vk::ImageAspectFlagBits::eColor;
      imageView.subresourceRange.baseMipLevel = 0;
      imageView.subresourceRange.levelCount   = 1;
      imageView.subresourceRange.baseArrayLayer = 0;
      imageView.subresourceRange.layerCount     = 1;

      try
      {
        m_ImageViews.at( i ) = m_Device.Get().createImageView( imageView );
      }
      catch ( const vk::SystemError & E )
      {
        throw std::runtime_error(
          std::format( "Failed to create image views: {}", E.what() ) );
      }
    }
  }

  void SwapChain::CreateRenderPass()
  {
    vk::AttachmentDescription color = {};
    color.format                    = m_ImageFormat;
    color.samples                   = vk::SampleCountFlagBits::e1;
    color.loadOp                    = vk::AttachmentLoadOp::eClear;
    color.storeOp                   = vk::AttachmentStoreOp::eStore;
    color.stencilLoadOp             = vk::AttachmentLoadOp::eClear;
    color.stencilStoreOp            = vk::AttachmentStoreOp::eDontCare;
    color.initialLayout             = vk::ImageLayout::eUndefined;
    color.finalLayout               = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorRef = {};
    colorRef.attachment              = 0;
    colorRef.layout                  = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint      = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount   = 1;
    subpass.pColorAttachments      = &colorRef;

    vk::SubpassDependency dependency = {};
    dependency.srcSubpass            = vk::SubpassExternal;
    dependency.dstSubpass            = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    vk::RenderPassCreateInfo renderPass = {};
    renderPass.attachmentCount          = 1;
    renderPass.pAttachments             = &color;
    renderPass.subpassCount             = 1;
    renderPass.pSubpasses               = &subpass;
    renderPass.dependencyCount          = 1;
    renderPass.pDependencies            = &dependency;

    try
    {
      m_RenderPass = m_Device.Get().createRenderPass( renderPass );
    }
    catch ( const vk::SystemError & E )
    {
      throw std::runtime_error(
        std::format( "Failed to create render pass: {}", E.what() ) );
    }
  }

  void SwapChain::CreateFramebuffers()
  {
    m_Framebuffers.resize( m_ImageViews.size() );

    for ( std::size_t i = 0; i < m_ImageViews.size(); ++i )
    {
      std::array attachments = { m_ImageViews.at( i ) };

      vk::FramebufferCreateInfo framebuffer = {};
      framebuffer.renderPass                = m_RenderPass;
      framebuffer.attachmentCount = static_cast<u32>( attachments.size() );
      framebuffer.pAttachments    = attachments.data();
      framebuffer.width           = m_Extent.width;
      framebuffer.height          = m_Extent.height;
      framebuffer.layers          = 1;

      try
      {
        m_Framebuffers.at( i ) =
          m_Device.Get().createFramebuffer( framebuffer );
      }
      catch ( const vk::SystemError & E )
      {
        throw std::runtime_error(
          std::format( "Failed to create framebuffer: {}", E.what() ) );
      }
    }
  }

  void SwapChain::Cleanup()
  {
    for ( auto & framebuffer : m_Framebuffers )
    {
      if ( framebuffer )
      {
        m_Device.Get().destroyFramebuffer( framebuffer );
      }
    }

    for ( auto & imageView : m_ImageViews )
    {
      if ( imageView )
      {
        m_Device.Get().destroyImageView( imageView );
      }
    }

    if ( m_RenderPass )
    {
      m_Device.Get().destroyRenderPass( m_RenderPass );
      m_RenderPass = nullptr;
    }

    m_Framebuffers.clear();
    m_ImageViews.clear();
    m_Images.clear();
  }

  SwapChain::SwapChainSupportDetails
  SwapChain::QuerySwapChainSupport( const vk::PhysicalDevice device ) const
  {
    SwapChainSupportDetails details = {};
    details.m_Capabilities = device.getSurfaceCapabilitiesKHR( m_Surface );
    details.m_Formats      = device.getSurfaceFormatsKHR( m_Surface );
    details.m_PresentModes = device.getSurfacePresentModesKHR( m_Surface );

    return details;
  }

  vk::SurfaceFormatKHR SwapChain::ChooseSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> & availableFormats )
  {
    for ( const auto & candidate : availableFormats )
    {
      if ( candidate.format == vk::Format::eB8G8R8A8Srgb &&
           candidate.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
      {
        return candidate;
      }
    }

    return availableFormats.at( 0 );
  }

  vk::PresentModeKHR SwapChain::ChoosePresentMode(
    const std::vector<vk::PresentModeKHR> & availableModes )
  {
    for ( const auto & candidate : availableModes )
    {
      if ( candidate == vk::PresentModeKHR::eMailbox )
      {
        return candidate;
      }
    }

    return vk::PresentModeKHR::eFifo;
  }

  vk::Extent2D SwapChain::ChooseExtent(
    const vk::SurfaceCapabilitiesKHR & capabilities ) const
  {
    if ( capabilities.currentExtent.width != std::numeric_limits<u32>::max() )
    {
      return capabilities.currentExtent;
    }

    vk::Extent2D actual = { m_Width, m_Height };
    actual.width = std::clamp( actual.width, capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width );
    actual.height =
      std::clamp( actual.height, capabilities.minImageExtent.height,
                  capabilities.maxImageExtent.height );

    return actual;
  }
} // namespace Engine::Renderer
