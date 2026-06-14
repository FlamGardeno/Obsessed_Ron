#ifndef MAP_H
#define MAP_H

#include <map>
#include <vector>
#include <string>
#include "globals.h"

enum class Direction { UP, RIGHT, DOWN, LEFT };
enum class TYPE { PLAYER, TOUN, BOSS, KARAVAN };

struct TypeRenderConfig {
  std::string icon;
  std::string color_code;
};

struct MapObject {
  int id;          
  float x;         
  float y;
  TYPE type; 

  MapObject(int _id, float _x, float _y, TYPE _type) 
    : id(_id), x(_x), y(_y), type(_type) {}
};

struct MapMoution {
  MapObject* actor;
  MapObject* target; 
  float spd;
  
  bool move(); 
}; 

class Map {
private:
  Map() = default;
  ~Map() = default;
  Map(const Map&) = delete;
  Map& operator=(const Map&) = delete;

  std::vector<MapObject> objects;
  std::vector<MapMoution> moution; 
  std::map<int, char> tag_to_char_map;
  int map_width = 0;
  int map_height = 0;

  std::map<TYPE, TypeRenderConfig> render_configs;
  TypeRenderConfig empty_config = {".", "90"}; 
  
  int selected_id = -1;
  bool is_paused = false;

  void select_next_in_direction(Direction dir);
  
  std::vector<std::string> get_selected_description();
  
  void player_action_dummy();

public:
  static Map& get_instance() {
    static Map instance;
    return instance;
  }

  bool load_config(const std::string& filename);

  void set_dimensions(int w, int h);
  int get_width();
  int get_height();

  void add_object(const MapObject& obj);
  void add_motion(MapObject* actor, MapObject* target, float speed);
  void update_movements();
  
  void set_selected_id(int id);
  int get_selected_id() const;
  
  void handle_input(char key);

  void render_console();
  void frame(float new_dt); 
}; 

#define MAP Map::get_instance()

#endif // MAP_H
