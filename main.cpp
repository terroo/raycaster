#include "raycaster.hpp"

int main(){
  auto ray = std::make_unique<RayCaster>();
  ray->run();
  return EXIT_SUCCESS;
}

