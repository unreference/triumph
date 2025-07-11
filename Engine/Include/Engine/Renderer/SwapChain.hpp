#pragma once

#include <vulkan/vulkan_raii.hpp>

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
    SwapChain( const Device & device, const vk::raii::SurfaceKHR & surface,
               u32 width, u32 height );
    ~SwapChain() = default;

    void Recreate( u32 width, u32 height );

    [[nodiscard]] const vk::raii::SwapchainKHR & Get() const;
    [[nodiscard]] vk::Format                     GetImageFormat() const;
    [[nodiscard]] vk::Extent2D                   GetExtent() const;
    [[nodiscard]] const std::vector<vk::Image> & GetImages() const;
    [[nodiscard]] const std::vector<vk::raii::ImageView> &
    GetImageViews() const;
    [[nodiscard]] const std::vector<vk::raii::Framebuffer> &
                                               GetFramebuffers() const;
    [[nodiscard]] const vk::raii::RenderPass & GetRenderPass() const;
    [[nodiscard]] std::size_t                  GetImageCount() const;

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

    [[nodiscard]] SwapChainSupportDetails
    QuerySwapChainSupport( const vk::raii::PhysicalDevice & device ) const;

    static vk::SurfaceFormatKHR ChooseSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR> & availableFormats );
    static vk::PresentModeKHR
    ChoosePresentMode( const std::vector<vk::PresentModeKHR> & availableModes );

    [[nodiscard]] vk::Extent2D
    ChooseExtent( const vk::SurfaceCapabilitiesKHR & capabilities ) const;

    const Device &               m_Device;
    const vk::raii::SurfaceKHR & m_pSurface;
    u32                          m_Width;
    u32                          m_Height;

    vk::raii::SwapchainKHR             m_pSwapChain;
    std::vector<vk::Image>             m_Images;
    vk::Format                         m_ImageFormat;
    vk::Extent2D                       m_Extent;
    std::vector<vk::raii::ImageView>   m_pImageViews;
    std::vector<vk::raii::Framebuffer> m_pFramebuffers;
    vk::raii::RenderPass               m_pRenderPass;
  };
} // namespace Engine::Renderer
