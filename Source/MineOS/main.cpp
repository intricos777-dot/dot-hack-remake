#include "MineOS.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
  (void)argc; (void)argv;
  
  mine::MineOS mineos("/home/sin/Projects/dot-hack-remake/scripts");
  
  if (!mineos.init("/home/sin/Projects/dot-hack-remake/scripts")) {
    std::cerr << "Failed to initialize Mine OS" << std::endl;
    return 1;
  }
  
  std::cout << "Mine OS // living-sin-blood build — The World client, Aida, and the personalized story are online." << std::endl;
  
  mine::Renderer& r = mineos.rend();
  
  // Main game loop
  uint32_t last = SDL_GetTicks();
  bool running = true;
  while (running) {
    uint32_t now = SDL_GetTicks();
    float dt = std::min(0.1f, (float)(now - last) / 1000.0f);
    last = now;

    mine::Input in;
    mine::pollInput(in);  // Assuming this is declared somewhere or use mineos.pollInput()
    
    // ... handle input, including F1 for 3D toggle
    
    mine::handleBridge(dt);
    mine::stateTimer_ += dt;

    // Periodic world-state feed so Aida stays in context
    if (mine::shell_ == mine::ShellState::Desktop) {
      if (stateTimer_ >= 5.0f) { stateTimer_ = 0.0f; mine::bridge_.sendState(mine::st_, false); }
    }

    // Render - toggle between 2D and 3D with F1
    if (in.key == SDLK_F1 && in.shift) {
      RenderMode newMode = (mine::rend.getMode() == mine::RenderMode::ThreeD) ? 
        mine::RenderMode::TwoD : mine::RenderMode::ThreeD;
      mine::rend.setMode(newMode);
    }
    
    r.begin();
    if (mine::shell_ == mine::ShellState::Boot) mine::drawBoot(dt);
    else if (mine::shell_ == mine::ShellState::Login) mine::drawLogin(in, dt);
    else mine::drawDesktop(in, dt);
    mine::drawOverlays(dt);
    r.end();

    if (mine::in_.key == SDLK_ESCAPE && mine::shell_ != mine::ShellState::Boot) {
      // menu-ish: nothing for now
    }
    if (mine::quit_) break;
  }
  
  mineos.shutdown();
  return 0;
}
