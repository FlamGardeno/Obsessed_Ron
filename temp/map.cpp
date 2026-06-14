#include "map.h"
#include <cmath>
#include <algorithm> 
#include <iostream>
#include <fstream>
#include <sstream>

bool MapMoution::move() {
  if (!actor || !target) return false;

  float dx = target->x - actor->x;
  float dy = target->y - actor->y;
  float distance = std::sqrt(dx * dx + dy * dy);
  const float EPSILON = 0.01f; 

  if (distance < EPSILON) {
    actor->x = target->x;
    actor->y = target->y;
    return true; 
  }

  float step = spd * dt;
  if (step >= distance) {
    actor->x = target->x;
    actor->y = target->y;
    return true; 
  }

  actor->x += (dx / distance) * step;
  actor->y += (dy / distance) * step;
  return false; 
}

bool Map::load_config(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "[Map Error] Не удалось открыть файл конфигурации: " << filename << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line == "#") continue;

    std::stringstream ss(line);
    std::string key;
    ss >> key;

    if (key == "width") {
      ss >> map_width;
    } else if (key == "height") {
      ss >> map_height;
    } else if (key == "EMPTY") {
      ss >> empty_config.icon >> empty_config.color_code;
    } else {
      std::string icon, color;
      ss >> icon >> color;

      if (key == "PLAYER")  render_configs[TYPE::PLAYER]  = {icon, color};
      if (key == "BOSS")    render_configs[TYPE::BOSS]    = {icon, color};
      if (key == "TOUN")    render_configs[TYPE::TOUN]    = {icon, color};
      if (key == "KARAVAN") render_configs[TYPE::KARAVAN] = {icon, color};
    }
  }
  file.close();
  return true;
}

void Map::add_object(const MapObject& obj) {
  objects.push_back(obj);
  if (selected_id == -1) {
    selected_id = obj.id;
  }
}

void Map::add_motion(MapObject* actor, MapObject* target, float speed) {
  if (actor && target) {
    moution.push_back({actor, target, speed});
  }
}

void Map::update_movements() {
  auto it = std::remove_if(moution.begin(), moution.end(), [](MapMoution& m) {
    return m.move(); 
  });
  if (it != moution.end()) {
    moution.erase(it, moution.end());
  }
}

void Map::set_selected_id(int id) {
  selected_id = id;
}

int Map::get_selected_id() const {
  return selected_id;
}

void Map::select_next_in_direction(Direction dir) {
  if (objects.empty()) return;

  const MapObject* current = nullptr;
  for (const auto& obj : objects) {
    if (obj.id == selected_id) {
      current = &obj;
      break;
    }
  }

  if (!current) {
    selected_id = objects[0].id;
    return;
  }

  const MapObject* best_candidate = nullptr;
  float min_distance = 1e9f; 

  for (const auto& target : objects) {
    if (target.id == current->id) continue;

    float dx = target.x - current->x;
    float dy = target.y - current->y;

    bool in_sector = false;
    switch (dir) {
      case Direction::UP:    in_sector = (dy < 0); break;
      case Direction::DOWN:  in_sector = (dy > 0); break;
      case Direction::LEFT:  in_sector = (dx < 0); break;
      case Direction::RIGHT: in_sector = (dx > 0); break;
    }

    if (in_sector) {
      float dist = dx * dx + dy * dy; 
      if (dist < min_distance) {
        min_distance = dist;
        best_candidate = &target;
      }
    }
  }

  if (best_candidate) {
    selected_id = best_candidate->id;
  }
}

std::vector<std::string> Map::get_selected_description() {
  std::vector<std::string> lines;
  
  if (is_paused) {
    lines.push_back("\033[1;31m[ПОД ПАУЗОЙ]\033[0m");
  }

  const MapObject* selected = nullptr;
  for (const auto& obj : objects) {
    if (obj.id == selected_id) {
      selected = &obj;
      break;
    }
  }

  if (!selected) {
    lines.push_back("Ничего не выбрано");
    return lines;
  }

  lines.push_back("=== ИНФО ОБЪЕКТА ===");
  lines.push_back("ID: " + std::to_string(selected->id));
  
  std::string type_str = "Неизвестно";
  switch (selected->type) {
    case TYPE::PLAYER:  type_str = "Игрок (PLAYER)"; break;
    case TYPE::TOUN:    type_str = "Город (TOUN)"; break;
    case TYPE::BOSS:    type_str = "Босс (BOSS)"; break;
    case TYPE::KARAVAN: type_str = "Караван (KARAVAN)"; break;
  }
  
  lines.push_back("Тип: " + type_str);
  lines.push_back("Координаты: (" + std::to_string(static_cast<int>(selected->x)) + 
                  ", " + std::to_string(static_cast<int>(selected->y)) + ")");
  
  return lines;
}

void Map::player_action_dummy() {
}

void Map::handle_input(char key) {
  if (key == ' ') {
    is_paused = !is_paused;
    return;
  }

  if (key == 'e') {
    MapObject* selected = nullptr;
    MapObject* player = nullptr;

    for (auto& obj : objects) {
      if (obj.id == selected_id) selected = &obj;
      if (obj.type == TYPE::PLAYER && !player) player = &obj;
    }

    if (selected && player) {
      if (selected->type == TYPE::PLAYER) {
        player_action_dummy();
      } else {
        add_motion(player, selected, 5.0f);
      }
    }
    return;
  }

  switch (key) {
    case 'a': select_next_in_direction(Direction::LEFT);  break;
    case 's': select_next_in_direction(Direction::DOWN);  break;
    case 'w': select_next_in_direction(Direction::UP);    break;
    case 'd': select_next_in_direction(Direction::RIGHT); break;
  }
}

void Map::render_console() {
  std::string default_cell = "\033[" + empty_config.color_code + "m" + empty_config.icon + "\033[0m";
  std::vector<std::vector<std::string>> grid(map_height, std::vector<std::string>(map_width, default_cell));

  for (const auto& obj : objects) {
    int ix = static_cast<int>(std::round(obj.x));
    int iy = static_cast<int>(std::round(obj.y));

    if (ix >= 0 && ix < map_width && iy >= 0 && iy < map_height) {
      if (render_configs.find(obj.type) != render_configs.end()) {
        const auto& config = render_configs[obj.type];
        std::string color_str = "\033[" + config.color_code + "m";
        
        if (obj.id == selected_id) {
          color_str = "\033[7m" + color_str;
        }

        grid[iy][ix] = color_str + config.icon + "\033[0m";
      }
    }
  }

  std::vector<std::string> desc = get_selected_description();

  for (int y = 0; y < map_height; ++y) {
    for (int x = 0; x < map_width; ++x) {
      std::cout << grid[y][x]; 
    }
    
    if (y < static_cast<int>(desc.size())) {
      std::cout << "   " << desc[y];
    }
    
    std::cout << "\n";
  }
  
  if (static_cast<int>(desc.size()) > map_height) {
    for (size_t i = map_height; i < desc.size(); ++i) {
      std::string offset(map_width, ' ');
      std::cout << offset << "   " << desc[i] << "\n";
    }
  }

  std::cout << std::flush;
}

void Map::frame(float new_dt) {
  dt = new_dt; 
  if (!is_paused) {
    update_movements();
  }
  std::cout << "\033[2J\033[1;1H";
  render_console();
}

void Map::set_dimensions(int w, int h) { map_width = w; map_height = h; }
int Map::get_width() { return map_width; }
int Map::get_height() { return map_height; }
