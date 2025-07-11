set(GAME_SOURCES
        Source/Main.cpp
        Source/Game/Application.cpp
)

set(GAME_HEADERS
        Include/Main.hpp
        Include/Game/Application.hpp
)

add_executable(${CMAKE_PROJECT_NAME} ${GAME_SOURCES} ${GAME_HEADERS})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC Include PRIVATE Source)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Engine)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME})