#pragma once

#include <optional>

#include <vulkan/vulkan_raii.hpp>

#include "Engine/Core/Macro.hpp"
#include "Engine/Core/Types.hpp"

namespace Engine::Renderer
{
  class Device
  {
    DISALLOW_COPY( Device );
    DISALLOW_MOVE( Device );

  public:
    struct QueueFamilyIndices
    {
      std::optional<u32> m_GraphicsFamily = {};
      std::optional<u32> m_PresentFamily  = {};

      [[nodiscard]] bool IsComplete() const
      {
        return m_GraphicsFamily.has_value() && m_PresentFamily.has_value();
      }
    };

    explicit Device( const vk::raii::Instance &   instance,
                     const vk::raii::SurfaceKHR & surface );
    ~Device() = default;

    [[nodiscard]] const vk::raii::Device &         Get() const;
    [[nodiscard]] const vk::raii::PhysicalDevice & GetPhysicalDevice() const;
    [[nodiscard]] const vk::raii::Queue &          GetGraphicsQueue() const;
    [[nodiscard]] const vk::raii::Queue &          GetPresentQueue() const;
    [[nodiscard]] const QueueFamilyIndices & GetQueueFamilyIndices() const;

    void Wait() const;

  private:
    static QueueFamilyIndices
                GetQueueFamilies( const vk::raii::PhysicalDevice & device,
                                  const vk::raii::SurfaceKHR &     surface );
    static bool IsExtensionSupported( const vk::raii::PhysicalDevice & device );
    static bool IsDeviceSuitable( const vk::raii::PhysicalDevice & device,
                                  const vk::raii::SurfaceKHR &     surface );

    void PickPhysicalDevice( const vk::raii::Instance &   instance,
                             const vk::raii::SurfaceKHR & surface );
    void CreateLogicalDevice( const vk::raii::SurfaceKHR & surface );

    vk::raii::PhysicalDevice m_pPhysicalDevice;
    vk::raii::Device         m_pDevice;
    vk::raii::Queue          m_pGraphicsQueue;
    vk::raii::Queue          m_pPresentQueue;
    QueueFamilyIndices       m_QueueFamilyIndices;

    static const std::vector<const char *> s_Extensions;
  };
} // namespace Engine::Renderer
