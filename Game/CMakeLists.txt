set(GAME_SOURCES
        Source/Main.cpp
)

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Engine)

set_property(
        DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game
)