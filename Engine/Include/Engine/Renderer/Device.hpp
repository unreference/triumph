#pragma once

#include <optional>

#include <vulkan/vulkan.hpp>

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

      bool IsComplete() const
      {
        return m_GraphicsFamily.has_value() && m_PresentFamily.has_value();
      }
    };

    explicit Device( vk::Instance instance, vk::SurfaceKHR surface );
    ~Device() = default;

    vk::Device                 Get() const;
    vk::PhysicalDevice         GetPhysicalDevice() const;
    vk::Queue                  GetGraphicsQueue() const;
    vk::Queue                  GetPresentQueue() const;
    const QueueFamilyIndices & GetQueueFamilyIndices() const;

    void Wait() const;

  private:
    static QueueFamilyIndices GetQueueFamilies( vk::PhysicalDevice device,
                                                vk::SurfaceKHR     surface );
    static bool               IsExtensionSupported( vk::PhysicalDevice device );
    static bool               IsDeviceSuitable( vk::PhysicalDevice device,
                                                vk::SurfaceKHR     surface );

    void PickPhysicalDevice( vk::Instance instance, vk::SurfaceKHR surface );
    void CreateLogicalDevice( vk::SurfaceKHR surface );

    vk::Device         m_Device;
    vk::PhysicalDevice m_PhysicalDevice;
    vk::Queue          m_GraphicsQueue;
    vk::Queue          m_PresentQueue;
    QueueFamilyIndices m_QueueFamilyIndices;

    static const std::vector<const char *> s_Extensions;
  };
} // namespace Engine::Renderer
