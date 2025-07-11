#pragma once

#include <Engine/Core/ApplicationBase.hpp>

#include "Game/Application.hpp"

Engine::Core::ApplicationBase * Engine::Core::Create()
{
  return new Game::Application();
}
