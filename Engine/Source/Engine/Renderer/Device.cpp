#include <set>

#include "Engine/Utility/Logger.hpp"

#include "Engine/Renderer/Device.hpp"

namespace Engine::Renderer
{
  const std::vector<const char *> Device::s_Extensions = {
    vk::KHRSwapchainExtensionName,
  };

  Device::Device( const vk::raii::Instance &   instance,
                  const vk::raii::SurfaceKHR & surface )
    : m_pPhysicalDevice( nullptr )
    , m_pDevice( nullptr )
    , m_pGraphicsQueue( nullptr )
    , m_pPresentQueue( nullptr )
  {
    PickPhysicalDevice( instance, surface );
    CreateLogicalDevice( surface );
  }

  const vk::raii::Device & Device::Get() const
  {
    return m_pDevice;
  }

  const vk::raii::PhysicalDevice & Device::GetPhysicalDevice() const
  {
    return m_pPhysicalDevice;
  }

  const vk::raii::Queue & Device::GetGraphicsQueue() const
  {
    return m_pGraphicsQueue;
  }

  const vk::raii::Queue & Device::GetPresentQueue() const
  {
    return m_pPresentQueue;
  }

  const Device::QueueFamilyIndices & Device::GetQueueFamilyIndices() const
  {
    return m_QueueFamilyIndices;
  }

  void Device::Wait() const
  {
    m_pDevice.waitIdle();
  }

  void Device::PickPhysicalDevice( const vk::raii::Instance &   instance,
                                   const vk::raii::SurfaceKHR & surface )
  {
    const auto Devices = instance.enumeratePhysicalDevices();

    if ( Devices.empty() )
    {
      LOG_ERROR( "Failed to find a GPU with Vulkan support!" );
    }

    for ( const auto & Device : Devices )
    {
      if ( IsDeviceSuitable( Device, surface ) )
      {
        m_pPhysicalDevice =
          std::move( const_cast<vk::raii::PhysicalDevice &>( Device ) );
        break;
      }
    }

    if ( !*m_pPhysicalDevice )
    {
      LOG_FATAL( "Failed to find a suitable GPU with Vulkan support!" );
    }

    const auto Properties = m_pPhysicalDevice.getProperties();
    LOG_INFO( "Selected GPU: {}", Properties.deviceName.data() );
  }

  void Device::CreateLogicalDevice( const vk::raii::SurfaceKHR & surface )
  {
    m_QueueFamilyIndices = GetQueueFamilies( m_pPhysicalDevice, surface );

    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    const std::set<u32>                    UniqueFamilies = {
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
      m_pDevice = m_pPhysicalDevice.createDevice( device );
    }
    catch ( const vk::SystemError & E )
    {
      LOG_FATAL( "Failed to create logical device: {}", E.what() );
    }

    m_pGraphicsQueue =
      m_pDevice.getQueue( m_QueueFamilyIndices.m_GraphicsFamily.value(), 0 );
    m_pPresentQueue =
      m_pDevice.getQueue( m_QueueFamilyIndices.m_PresentFamily.value(), 0 );
  }

  Device::QueueFamilyIndices
  Device::GetQueueFamilies( const vk::raii::PhysicalDevice & device,
                            const vk::raii::SurfaceKHR &     surface )
  {
    QueueFamilyIndices indices = {};

    const auto Families = device.getQueueFamilyProperties();
    for ( u32 i = 0; i < Families.size(); ++i )
    {
      if ( Families[ i ].queueFlags & vk::QueueFlagBits::eGraphics )
      {
        indices.m_GraphicsFamily = i;
      }

      if ( device.getSurfaceSupportKHR( i, *surface ) )
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

  bool Device::IsDeviceSuitable( const vk::raii::PhysicalDevice & device,
                                 const vk::raii::SurfaceKHR &     surface )
  {
    const QueueFamilyIndices Indices     = GetQueueFamilies( device, surface );
    const bool               IsSupported = IsExtensionSupported( device );
    bool                     isValidSwapChain = false;

    if ( IsSupported )
    {
      const auto Formats      = device.getSurfaceFormatsKHR( *surface );
      const auto PresentModes = device.getSurfacePresentModesKHR( *surface );

      isValidSwapChain = !Formats.empty() && !PresentModes.empty();
    }

    return Indices.IsComplete() && IsSupported && isValidSwapChain;
  }

  bool Device::IsExtensionSupported( const vk::raii::PhysicalDevice & device )
  {
    const auto Supported = device.enumerateDeviceExtensionProperties();
    std::set<std::string> required = { s_Extensions.begin(),
                                       s_Extensions.end() };

    for ( const auto & Extension : Supported )
    {
      required.erase( Extension.extensionName.data() );
    }

    return required.empty();
  }
} // namespace Engine::Renderer
