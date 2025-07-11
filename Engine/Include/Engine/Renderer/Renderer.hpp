#pragma once

#include <vector>
#include <memory>

#include <vulkan/vulkan_raii.hpp>

#include "Engine/Core/Macro.hpp"
#include "Engine/Core/Types.hpp"

namespace Engine
{
  namespace Platform
  {
    class Window;
  }

  namespace Renderer
  {
    class Device;
    class SwapChain;
  } // namespace Renderer
} // namespace Engine

namespace Engine::Renderer
{
  class Renderer
  {
    DISALLOW_COPY( Renderer );
    DISALLOW_MOVE( Renderer );

  public:
    explicit Renderer( Platform::Window & window );
    ~Renderer();

    void BeginDraw();
    void EndDraw();

    void Clear( f32 r, f32 g, f32 b, f32 a = 1.0f );
    void Resize( u32 width, u32 height );

    [[nodiscard]] bool IsFrameInProgress() const;

  private:
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void RecreateSwapChain() const;

    [[nodiscard]] static bool               IsValidationLayerSupported();
    [[nodiscard]] std::vector<const char *> GetRequiredExtensions() const;

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL
    DebugCallback( vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                   vk::DebugUtilsMessageTypeFlagsEXT              type,
                   const vk::DebugUtilsMessengerCallbackDataEXT * callbackData,
                   void *                                         userData );

    Platform::Window &   m_Window;
    vk::raii::Context    m_pContext;
    vk::raii::Instance   m_pInstance;
    vk::raii::SurfaceKHR m_pSurface;

    std::unique_ptr<Device>    m_pDevice;
    std::unique_ptr<SwapChain> m_pSwapChain;

    vk::raii::CommandPool                m_pCommandPool;
    std::vector<vk::raii::CommandBuffer> m_CommandBuffers;

    std::vector<vk::raii::Semaphore> m_pImageSemaphores;
    std::vector<vk::raii::Semaphore> m_pRenderSemaphores;
    std::vector<vk::raii::Fence>     m_pFencesInFlight;
    std::vector<vk::raii::Fence *>   m_pImagesInFlight;

    u32  m_CurrentFrame;
    u32  m_ImageIndex;
    bool m_IsFrameStarted;
    bool m_IsFramebufferResized;

    vk::ClearColorValue m_ClearColor;

    vk::raii::DebugUtilsMessengerEXT m_pDebugMessenger;

    static constexpr u32                   s_MaxFramesInFlight        = 3;
    static constexpr bool                  s_IsValidationLayerEnabled = true;
    static const std::vector<const char *> s_ValidationLayers;
  };
} // namespace Engine::Renderer
