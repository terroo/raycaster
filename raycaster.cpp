#include "raycaster.hpp"

RayCaster::RayCaster() : window(
    sf::VideoMode(1280, 720),
    "RayCaster",
    sf::Style::Titlebar | sf::Style::Close
    ){

  fov = {3.14159f / 4};
  depth = {16.f};

  if(!wall_texture.loadFromFile("./resources/mossy.png")){
    throw std::runtime_error("Error loading mossy.png");
  }

  if(!bg_texture.loadFromFile("./resources/bg.jpg")){
    throw std::runtime_error("Error loading bg.jpg");
  }

  bg.setTexture(bg_texture);

  player_x = player_y = {8.f};
  player_angle = {0.f};

  map = {
    "################",
    "#..............#",
    "#..............#",
    "#....###.......#",
    "#....###.......#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#....#...#######",
    "#....#...#.....#",
    "#....#...#.....#",
    "#..............#",
    "#..............#",
    "################",
  };

}

void RayCaster::events(){
  sf::Event event;
  while(window.pollEvent(event)){
    if(event.type == sf::Event::Closed){
      window.close();
    }
  }
}

void RayCaster::draw(){
  window.clear();
  window.draw(bg);
  render();
  draw_minimap();
  //rays();
  window.display();
}

void RayCaster::run(){
  while(window.isOpen()){
    events();
    keys();
    draw();
  }
}

void RayCaster::draw_minimap(){
  //int minimapSize = window.getSize().y - 20;
  int minimapSize = 150;
  int cellSize =  minimapSize / map[0].size();
  sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize));

  for(size_t y = 0; y < map.size(); ++y){
    for(size_t x = 0; x < map[y].size(); ++x){
      if(map[y][x] == '#'){
        cell.setFillColor(sf::Color::White);
      }else{
        cell.setFillColor(sf::Color(50, 50, 50));
      }
      cell.setPosition(10 + x * cellSize, 10 + y * cellSize);
      window.draw(cell);
    }
  }

  sf::CircleShape playerDot(cellSize / 3.f);
  playerDot.setFillColor(sf::Color::Red);
  playerDot.setPosition(
    10 + player_x * cellSize - playerDot.getRadius(),
    10 + player_y * cellSize - playerDot.getRadius()
  );
  window.draw(playerDot);

  sf::Vertex line[] = {

    sf::Vertex(
      sf::Vector2f(
        10 + player_x * cellSize,
        10 + player_y * cellSize
      ),
      sf::Color::Red
    ),

    sf::Vertex(
      sf::Vector2f(
        10 + (player_x + std::cos(player_angle) * 1.5f) * cellSize,
        10 + (player_y + std::sin(player_angle) * 1.5f) * cellSize
      ),
      sf::Color::Red
    ),

  };
  window.draw(line, 2, sf::Lines);
}

void RayCaster::keys(){
  float timer = clock.restart().asSeconds();

  if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
    player_angle -= 1.5f * timer;
  }

  if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
    player_angle += 1.5f * timer;
  }

  if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
    player_x += std::cos(player_angle) * 5.f * timer;
    player_y += std::sin(player_angle) * 5.f * timer;

    if(map[(int)player_y][(int)player_x] == '#'){
      player_x -= std::cos(player_angle) * 5.f * timer;
      player_y -= std::sin(player_angle) * 5.f * timer;
    }
  }

  if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
    player_x -= std::cos(player_angle) * 5.f * timer;
    player_y -= std::sin(player_angle) * 5.f * timer;

    if(map[(int)player_y][(int)player_x] == '#'){
      player_x += std::cos(player_angle) * 5.f * timer;
      player_y += std::sin(player_angle) * 5.f * timer;
    }
  }
}

void RayCaster::rays(){
  int cellSize = (window.getSize().y - 20) / map[0].size();

  int numRays = 100;
  float spread = 0.785f;

  for(int i = 0; i < numRays; ++i){
    float offset = ((i / (float)(numRays - 1)) - 0.5f) * spread * 2.0f;
    float rayAngle = player_angle + offset;

    float rayX = player_x, rayY = player_y;

    while(map[(int)rayY][(int)rayX] != '#'){
      rayX += std::cos(rayAngle) * 0.1f;
      rayY += std::sin(rayAngle) * 0.1f;

      sf::Vertex line[] = {

        sf::Vertex(
            sf::Vector2f(
              10 + player_x * cellSize,
              10 + player_y * cellSize
              ),
            sf::Color::Yellow
        ),

        sf::Vertex(
            sf::Vector2f(
              10 + rayX * cellSize,
              10 + rayY * cellSize
              ),
            sf::Color::Yellow
        ),

      };
      window.draw(line, 2, sf::Lines);

    }
  }
}

void RayCaster::render(){
  for(size_t x = 0; x < window.getSize().x; ++x){
    float ray_angle = (player_angle - fov / 2.f) + ((float)x / (int)window.getSize().x) * fov;

    float dwall = 0.f;
    bool hit_wall = false;
    bool is_vertical_hit = false;

    float eye_x = std::cos(ray_angle);
    float eye_y = std::sin(ray_angle);

    while(!hit_wall && dwall < depth){
      dwall += 0.1f;

      int check_x = (int)(player_x + eye_x * dwall);
      int check_y = (int)(player_y + eye_y * dwall);

      if(check_x < 0 || check_x >= map_width || check_y < 0 || check_y >= map_height){
        hit_wall = true;
        dwall = depth;
      }else{
        if(map[check_y][check_x] == '#'){
          hit_wall = true;

          float wall_x = player_x + eye_x * dwall;
          float wall_y = player_y + eye_y * dwall;

          is_vertical_hit = (std::fabs(wall_x - std::round(wall_x)) < std::fabs(wall_y - std::round(wall_y)));
        }
      }
    }

    float correct = dwall * std::cos(ray_angle - player_angle);

    int ceiling = (float)(window.getSize().y / 2.f) - window.getSize().y / correct;
    int floor   = window.getSize().y - ceiling;
    
    float hit_x = player_x + eye_x * dwall;
    float hit_y = player_y + eye_y * dwall;

    int tex_x = 0;
    if(is_vertical_hit){
      tex_x = static_cast<int>((hit_y - std::floor(hit_y)) * wall_texture.getSize().x);
    }else{
      tex_x = static_cast<int>((hit_x - std::floor(hit_x)) * wall_texture.getSize().x);
    }

    sf::VertexArray wall_strip(sf::Quads, 4);

    wall_strip[0].position = sf::Vector2f(x, ceiling);
    wall_strip[1].position = sf::Vector2f(x + 1, ceiling);
    wall_strip[2].position = sf::Vector2f(x + 1, floor);
    wall_strip[3].position = sf::Vector2f(x, floor);

    wall_strip[0].texCoords = sf::Vector2f(tex_x, 0);
    wall_strip[1].texCoords = sf::Vector2f(tex_x + 1, 0);
    wall_strip[2].texCoords = sf::Vector2f(tex_x + 1, wall_texture.getSize().y);
    wall_strip[3].texCoords = sf::Vector2f(tex_x, wall_texture.getSize().y);

    float light_factor = std::max(0.2f, 1.0f - (dwall / depth));

    sf::Color wall_color(255 * light_factor, 255 * light_factor, 255 * light_factor);

    wall_strip[0].color = wall_color;
    wall_strip[1].color = wall_color;
    wall_strip[2].color = wall_color;
    wall_strip[3].color = wall_color;

    sf::RenderStates states;
    states.texture = &wall_texture;
    window.draw(wall_strip, states);
  }
}
