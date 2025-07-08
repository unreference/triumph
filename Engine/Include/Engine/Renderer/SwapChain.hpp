#pragma once

#include <vulkan/vulkan.hpp>

#include "Engine/Core/Macro.hpp"
#include "Engine/Core/Types.hpp"

namespace Engine::Renderer
{
  class Device;
}

namespace Engine::Renderer
{
  class SwapChain
  {
    DISALLOW_COPY( SwapChain );
    DISALLOW_MOVE( SwapChain );

  public:
    SwapChain( const Device & device, vk::SurfaceKHR surface, u32 width,
               u32 height );
    ~SwapChain() = default;

    void Recreate( u32 width, u32 height );

    vk::SwapchainKHR                     Get() const;
    vk::Format                           GetImageFormat() const;
    vk::Extent2D                         GetExtent() const;
    const std::vector<vk::Image> &       GetImages() const;
    const std::vector<vk::ImageView> &   GetImageViews() const;
    const std::vector<vk::Framebuffer> & GetFramebuffers() const;
    vk::RenderPass                       GetRenderPass() const;
    std::size_t                          GetImageCount() const;

  private:
    struct SwapChainSupportDetails
    {
      vk::SurfaceCapabilitiesKHR        m_Capabilities = {};
      std::vector<vk::SurfaceFormatKHR> m_Formats      = {};
      std::vector<vk::PresentModeKHR>   m_PresentModes = {};
    };

    void Create();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateFramebuffers();
    void Cleanup();

    SwapChainSupportDetails
    QuerySwapChainSupport( vk::PhysicalDevice device ) const;

    static vk::SurfaceFormatKHR ChooseSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> & availableFormats );
    static vk::PresentModeKHR ChoosePresentMode(

      const std::vector<vk::PresentModeKHR> & availableModes );
    vk::Extent2D
    ChooseExtent( const vk::SurfaceCapabilitiesKHR & capabilities ) const;

    const Device & m_Device;
    vk::SurfaceKHR m_Surface;
    u32            m_Width;
    u32            m_Height;

    vk::SwapchainKHR             m_SwapChain;
    std::vector<vk::Image>       m_Images;
    vk::Format                   m_ImageFormat;
    vk::Extent2D                 m_Extent;
    std::vector<vk::ImageView>   m_ImageViews;
    std::vector<vk::Framebuffer> m_Framebuffers;
    vk::RenderPass               m_RenderPass;
  };
} // namespace Engine::Renderer
