set(ENGINE_SOURCES
        Source/Engine/Utility/Logger.cpp
        Source/Engine/Renderer/Device.cpp
        Source/Engine/Renderer/SwapChain.cpp
        Source/Engine/Renderer/Renderer.cpp
)

set(ENGINE_HEADERS
        Include/Engine/Core/Macro.hpp
        Include/Engine/Core/Result.hpp
        Include/Engine/Core/Types.hpp
        Include/Engine/Platform/IWindow.hpp
        Include/Engine/Platform/WindowEvents.hpp
        Include/Engine/Platform/WindowFactory.hpp
        Include/Engine/Utility/Logger.hpp
        Include/Engine/Renderer/Device.hpp
        Include/Engine/Renderer/SwapChain.hpp
        Include/Engine/Renderer/Renderer.hpp
)

if (WIN32)
    list(APPEND ENGINE_SOURCES
            Source/Engine/Platform/Win32/Win32Window.cpp
    )
    list(APPEND ENGINE_HEADERS
            Include/Engine/Platform/Win32/Win32Window.hpp
    )
    #elseif ( UNIX )
    #  list( APPEND ENGINE_SOURCES
    #    Source/Engine/Platform/Unix/UnixWindow.cpp
    #  )
    #  list( APPEND ENGINE_HEADERS
    #    Include/Engine/Platform/Unix/UnixWindow.hpp
    #  )
    # 
endif ()

add_library(Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_include_directories(Engine PUBLIC Include PRIVATE Source)

target_link_libraries(Engine PUBLIC Vulkan::Vulkan)

if (WIN32)
    target_link_libraries(Engine PRIVATE
            user32.lib
            gdi32.lib
            shell32.lib
    )
elseif (UNIX)
    find_package(X11 REQUIRED)
    target_link_libraries(Engine PRIVATE
            ${X11_LIBRARIES}
            ${X11_Xcursor_LIB}
            ${X11_Xrandr_LIB}
            ${X11_Xinput_LIB}
    )
endif ()
