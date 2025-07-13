/*--------------------------------------------------------------------------------*
  Copyright Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain proprietary
  information of Nintendo and/or its licensed developers and are protected by
  national and international copyright laws. They may not be disclosed to third
  parties or copied or duplicated in any form, in whole or in part, without the
  prior written consent of Nintendo.

  The content herein is highly confidential and should be handled accordingly.
 *--------------------------------------------------------------------------------*/

#include "Engine/Renderer/Device.hpp"
#include "Engine/Utility/Logger.hpp"

#include "Engine/Renderer/SwapChain.hpp"

namespace Engine::Renderer
{
  SwapChain::SwapChain( const Device & device, const vk::raii::SurfaceKHR & surface,
                        const u32 width, const u32 height )
    : m_Device( device )
    , m_pSurface( surface )
    , m_Width( width )
    , m_Height( height )
    , m_pSwapChain( nullptr )
    , m_ImageFormat()
    , m_pRenderPass( nullptr )
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

    m_pFramebuffers.clear();
    m_pImageViews.clear();
    m_Images.clear();

    m_pRenderPass = nullptr;
    m_pSwapChain  = nullptr;

    Create();
    CreateImageViews();
    CreateRenderPass();
    CreateFramebuffers();
  }

  const vk::raii::SwapchainKHR & SwapChain::Get() const
  {
    return m_pSwapChain;
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

  const std::vector<vk::raii::ImageView> & SwapChain::GetImageViews() const
  {
    return m_pImageViews;
  }

  const std::vector<vk::raii::Framebuffer> & SwapChain::GetFramebuffers() const
  {
    return m_pFramebuffers;
  }

  const vk::raii::RenderPass & SwapChain::GetRenderPass() const
  {
    return m_pRenderPass;
  }

  std::size_t SwapChain::GetImageCount() const
  {
    return m_Images.size();
  }

  void SwapChain::Create()
  {
    auto [ capabilities, formats, presentModes ] =
      QuerySwapChainSupport( m_Device.GetPhysicalDevice() );

    const auto Surface     = ChooseSurfaceFormat( formats );
    const auto PresentMode = ChoosePresentMode( presentModes );
    const auto Extent      = ChooseExtent( capabilities );

    auto imageCount = capabilities.minImageCount + 1;
    if ( capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount )
    {
      imageCount = capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChain = {};
    swapChain.surface                    = *m_pSurface;
    swapChain.minImageCount              = imageCount;
    swapChain.imageFormat                = Surface.format;
    swapChain.imageColorSpace            = Surface.colorSpace;
    swapChain.imageExtent                = Extent;
    swapChain.imageArrayLayers           = 1;
    swapChain.imageUsage                 = vk::ImageUsageFlagBits::eColorAttachment;

    const auto & [ Graphics, Present ] = m_Device.GetQueueFamilyIndices();
    const std::array Indices           = { Graphics.value(), Present.value() };

    if ( Graphics != Present )
    {
      swapChain.imageSharingMode      = vk::SharingMode::eConcurrent;
      swapChain.queueFamilyIndexCount = 2;
      swapChain.pQueueFamilyIndices   = Indices.data();
    }
    else
    {
      swapChain.imageSharingMode = vk::SharingMode::eExclusive;
    }

    swapChain.preTransform   = capabilities.currentTransform;
    swapChain.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChain.presentMode    = PresentMode;
    swapChain.clipped        = vk::True;

    if ( *m_pSwapChain )
    {
      swapChain.oldSwapchain = *m_pSwapChain;
    }

    try
    {
      m_pSwapChain = m_Device.Get().createSwapchainKHR( swapChain );
    }
    catch ( const vk::SystemError & E )
    {
      LOG_FATAL( "Failed to create swap chain: {}", E.what() );
    }

    m_Images      = m_pSwapChain.getImages();
    m_ImageFormat = Surface.format;
    m_Extent      = Extent;
  }

  void SwapChain::CreateImageViews()
  {
    m_pImageViews.clear();
    m_pImageViews.reserve( m_Images.size() );

    for ( const auto & Image : m_Images )
    {
      vk::ImageViewCreateInfo imageView         = {};
      imageView.image                           = Image;
      imageView.viewType                        = vk::ImageViewType::e2D;
      imageView.format                          = m_ImageFormat;
      imageView.components.r                    = vk::ComponentSwizzle::eIdentity;
      imageView.components.g                    = vk::ComponentSwizzle::eIdentity;
      imageView.components.b                    = vk::ComponentSwizzle::eIdentity;
      imageView.components.a                    = vk::ComponentSwizzle::eIdentity;
      imageView.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
      imageView.subresourceRange.baseMipLevel   = 0;
      imageView.subresourceRange.levelCount     = 1;
      imageView.subresourceRange.baseArrayLayer = 0;
      imageView.subresourceRange.layerCount     = 1;

      try
      {
        m_pImageViews.emplace_back( m_Device.Get().createImageView( imageView ) );
      }
      catch ( const vk::SystemError & E )
      {
        LOG_FATAL( "Failed to create image views: {}", E.what() );
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
    dependency.srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.srcAccessMask = vk::AccessFlagBits::eNone;
    dependency.dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
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
      m_pRenderPass = m_Device.Get().createRenderPass( renderPass );
    }
    catch ( const vk::SystemError & E )
    {
      LOG_FATAL( "Failed to create render pass: {}", E.what() );
    }
  }

  void SwapChain::CreateFramebuffers()
  {
    m_pFramebuffers.clear();
    m_pFramebuffers.reserve( m_pImageViews.size() );

    for ( const auto & ImageView : m_pImageViews )
    {
      const std::array Attachments = { *ImageView };

      vk::FramebufferCreateInfo framebuffer = {};
      framebuffer.renderPass                = *m_pRenderPass;
      framebuffer.attachmentCount           = static_cast<u32>( Attachments.size() );
      framebuffer.pAttachments              = Attachments.data();
      framebuffer.width                     = m_Extent.width;
      framebuffer.height                    = m_Extent.height;
      framebuffer.layers                    = 1;

      try
      {
        m_pFramebuffers.emplace_back(
          m_Device.Get().createFramebuffer( framebuffer ) );
      }
      catch ( const vk::SystemError & E )
      {
        LOG_FATAL( "Failed to create framebuffer: {}", E.what() );
      }
    }
  }

  SwapChain::SwapChainSupportDetails
  SwapChain::QuerySwapChainSupport( const vk::raii::PhysicalDevice & device ) const
  {
    SwapChainSupportDetails details = {};
    details.m_Capabilities = device.getSurfaceCapabilitiesKHR( *m_pSurface );
    details.m_Formats      = device.getSurfaceFormatsKHR( *m_pSurface );
    details.m_PresentModes = device.getSurfacePresentModesKHR( *m_pSurface );
    return details;
  }

  vk::SurfaceFormatKHR SwapChain::ChooseSurfaceFormat(
    const std::vector<vk::SurfaceFormatKHR> & availableFormats )
  {
    const std::vector<vk::SurfaceFormatKHR> Prefer = {
      { vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear },
      { vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear },
      { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear },
      { vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear },
    };

    for ( const auto & Want : Prefer )
    {
      const auto I =
        std::ranges::find_if( availableFormats,
                              [ & ]( const vk::SurfaceFormatKHR & Available )
                              {
                                return Available.format == Want.format &&
                                       Available.colorSpace == Want.colorSpace;
                              } );

      if ( I != availableFormats.end() )
      {
        return *I;
      }
    }

    return availableFormats[ 0 ];
  }

  vk::PresentModeKHR SwapChain::ChoosePresentMode(
    const std::vector<vk::PresentModeKHR> & availableModes )
  {
    for ( const auto & Candidate : availableModes )
    {
      if ( Candidate == vk::PresentModeKHR::eMailbox )
      {
        return Candidate;
      }
    }

    return vk::PresentModeKHR::eFifo;
  }

  vk::Extent2D
  SwapChain::ChooseExtent( const vk::SurfaceCapabilitiesKHR & capabilities ) const
  {
    if ( capabilities.currentExtent.width != std::numeric_limits<u32>::max() )
    {
      return capabilities.currentExtent;
    }

    vk::Extent2D actual = { m_Width, m_Height };
    actual.width  = std::clamp( actual.width, capabilities.minImageExtent.width,
                                capabilities.maxImageExtent.width );
    actual.height = std::clamp( actual.height, capabilities.minImageExtent.height,
                                capabilities.maxImageExtent.height );

    return actual;
  }
} // namespace Engine::Renderer
