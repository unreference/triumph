#pragma once

#include <vector>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "Engine/Core/Macro.hpp"
#include "Engine/Core/Types.hpp"

namespace Engine
{
  namespace Platform
  {
    class IWindow;
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
    explicit Renderer( Platform::IWindow & window );
    ~Renderer();

    void BeginDraw();
    void EndDraw();
    void Clear( f32 r, f32 g, f32 b, f32 a = 1.0f );

    [[nodiscard]] bool IsFrameInProgress() const;

  private:
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void RecreateSwapChain();

    [[nodiscard]] bool                      IsValidationLayerSupported() const;
    [[nodiscard]] std::vector<const char *> GetRequiredExtensions() const;

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL
    DebugCallback( vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
                   vk::DebugUtilsMessageTypeFlagsEXT              type,
                   const vk::DebugUtilsMessengerCallbackDataEXT * pCallbackData,
                   void *                                         pUserData );

    Platform::IWindow & m_Window;
    vk::Instance        m_Instance;
    vk::SurfaceKHR      m_Surface;

    std::unique_ptr<Device>    m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;

    vk::CommandPool                m_CommandPool;
    std::vector<vk::CommandBuffer> m_CommandBuffers;

    std::vector<vk::Semaphore> m_ImageSemaphores;
    std::vector<vk::Semaphore> m_RenderSemaphores;
    std::vector<vk::Fence>     m_FencesInFlight;

    u32  m_CurrentFrame;
    u32  m_ImageIndex;
    bool m_IsFrameStarted;
    bool m_IsFramebufferResized;

    vk::ClearColorValue m_ClearColor;

    vk::detail::DynamicLoader         m_Loader;
    vk::detail::DispatchLoaderDynamic m_Dispatch;
    vk::DebugUtilsMessengerEXT        m_DebugMessenger;

    static constexpr u32                   s_MaxFramesInFlight        = 2;
    static constexpr bool                  s_IsValidationLayerEnabled = true;
    static const std::vector<const char *> s_ValidationLayers;
  };
} // namespace Engine::Renderer
