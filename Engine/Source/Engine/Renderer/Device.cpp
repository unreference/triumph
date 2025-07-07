#include <set>

#include "Engine/Utility/Logger.hpp"

#include "Engine/Renderer/Device.hpp"

namespace Engine::Renderer
{
  const std::vector<const char *> Device::s_Extensions = {
    vk::KHRSwapchainExtensionName,
  };

  Device::Device( const vk::Instance instance, const vk::SurfaceKHR surface )
  {
    PickPhysicalDevice( instance, surface );
    CreateLogicalDevice( surface );
  }

  vk::Device Device::Get() const
  {
    return m_Device;
  }

  vk::PhysicalDevice Device::GetPhysicalDevice() const
  {
    return m_PhysicalDevice;
  }

  vk::Queue Device::GetGraphicsQueue() const
  {
    return m_GraphicsQueue;
  }

  vk::Queue Device::GetPresentQueue() const
  {
    return m_PresentQueue;
  }

  const Device::QueueFamilyIndices & Device::GetQueueFamilyIndices() const
  {
    return m_QueueFamilyIndices;
  }

  void Device::Wait() const
  {
    m_Device.waitIdle();
  }

  void Device::PickPhysicalDevice( const vk::Instance   instance,
                                   const vk::SurfaceKHR surface )
  {
    auto devices = instance.enumeratePhysicalDevices();

    if ( devices.empty() )
    {
      throw std::runtime_error( "Failed to find a GPU with Vulkan support!" );
    }

    for ( const auto & device : devices )
    {
      if ( IsDeviceSuitable( device, surface ) )
      {
        m_PhysicalDevice = device;
        break;
      }
    }

    if ( !m_PhysicalDevice )
    {
      throw std::runtime_error( "Failed to find a suitable GPU!" );
    }

    auto props = m_PhysicalDevice.getProperties();
    // LOG_INFO( "Selected GPU: {}", props.deviceName.data() );
  }

  void Device::CreateLogicalDevice( const vk::SurfaceKHR surface )
  {
    m_QueueFamilyIndices = GetQueueFamilies( m_PhysicalDevice, surface );

    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    const std::set                         UniqueFamilies = {
      m_QueueFamilyIndices.m_GraphicsFamily.value(),
      m_QueueFamilyIndices.m_PresentFamily.value() };

    constexpr f32 Priority = 1.0f;
    for ( const u32 Family : UniqueFamilies )
    {
      vk::DeviceQueueCreateInfo queue = {};
      queue.queueFamilyIndex          = Family;
      queue.queueCount                = 1;
      queue.pQueuePriorities          = &Priority;

      queueInfos.push_back( queue );
    }

    constexpr vk::PhysicalDeviceFeatures Features = {};

    vk::DeviceCreateInfo device    = {};
    device.queueCreateInfoCount    = static_cast<u32>( queueInfos.size() );
    device.pQueueCreateInfos       = queueInfos.data();
    device.pEnabledFeatures        = &Features;
    device.enabledExtensionCount   = static_cast<u32>( s_Extensions.size() );
    device.ppEnabledExtensionNames = s_Extensions.data();

    try
    {
      m_Device = m_PhysicalDevice.createDevice( device );
    }
    catch ( const vk::SystemError & E )
    {
      throw std::runtime_error(
        std::format( "Failed to create logical device: {}", E.what() ) );
    }

    m_GraphicsQueue =
      m_Device.getQueue( m_QueueFamilyIndices.m_GraphicsFamily.value(), 0 );
    m_PresentQueue =
      m_Device.getQueue( m_QueueFamilyIndices.m_PresentFamily.value(), 0 );
  }

  Device::QueueFamilyIndices
  Device::GetQueueFamilies( const vk::PhysicalDevice device,
                            const vk::SurfaceKHR     surface )
  {
    QueueFamilyIndices indices = {};

    const auto Families = device.getQueueFamilyProperties();
    for ( u32 i = 0; i < Families.size(); ++i )
    {
      if ( Families[ i ].queueFlags & vk::QueueFlagBits::eGraphics )
      {
        indices.m_GraphicsFamily = i;
      }

      if ( device.getSurfaceSupportKHR( i, surface ) )
      {
        indices.m_PresentFamily = i;
      }

      if ( indices.IsComplete() )
      {
        break;
      }
    }

    return indices;
  }

  bool Device::IsDeviceSuitable( const vk::PhysicalDevice device,
                                 const vk::SurfaceKHR     surface )
  {
    const QueueFamilyIndices Indices     = GetQueueFamilies( device, surface );
    const bool               IsSupported = IsExtensionSupported( device );
    bool                     isValidSwapChain = false;

    if ( IsSupported )
    {
      const auto Formats      = device.getSurfaceFormatsKHR( surface );
      const auto PresentModes = device.getSurfacePresentModesKHR( surface );

      isValidSwapChain = !Formats.empty() && !PresentModes.empty();
    }

    return Indices.IsComplete() && IsSupported && isValidSwapChain;
  }

  bool Device::IsExtensionSupported( const vk::PhysicalDevice device )
  {
    const auto Supported = device.enumerateDeviceExtensionProperties();
    std::set<std::string> required = { s_Extensions.begin(),
                                       s_Extensions.end() };

    for ( const auto & extension : Supported )
    {
      required.erase( extension.extensionName.data() );
    }

    return required.empty();
  }
} // namespace Engine::Renderer
