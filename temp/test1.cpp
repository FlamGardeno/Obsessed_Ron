#include "map.h"
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

void set_conio_mode(bool enable) {
  static struct termios oldt, newt;
  if (enable) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  }
}

bool kbhit() {
  struct timeval tv = {0, 0};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

int main() {
  if (!MAP.load_config("map_config.txt")) {
    MAP.set_dimensions(30, 10);
  }

  MapObject hero(1, 2.0f, 2.0f, TYPE::PLAYER);
  
  MapObject town(5, 25.0f, 10.0f, TYPE::TOUN);
  MapObject town1(6, 15.0f, 8.0f, TYPE::TOUN);
  MapObject town2(2, 25.0f, 4.0f, TYPE::TOUN);
  MapObject boss(3, 10.0f, 10.0f, TYPE::BOSS);
  MapObject caravan(4, 5.0f, 7.0f, TYPE::KARAVAN);

  MAP.add_object(hero);
  MAP.add_object(town);

  MAP.add_object(town1);
  MAP.add_object(town2);
  MAP.add_object(boss);
  MAP.add_object(caravan);

  set_conio_mode(true);

  std::cout << "Игра запущена! Инструкция:\n";
  std::cout << "wasd   - Переключение между объектами\n";
  std::cout << "space  - Пауза / Развернуть симуляцию\n";
  std::cout << "e      - Отправить Игрока к выбранной цели\n";
  std::cout << "q      - Выйти из игры\n";
  std::cout << "Нажмите любую клавишу для старта..." << std::endl;
  
  while (!kbhit()) { usleep(10000); }
  char dummy; read(STDIN_FILENO, &dummy, 1);

  bool running = true;
  const float frame_time = 0.033f;

  while (running) {
    if (kbhit()) {
      char key;
      if (read(STDIN_FILENO, &key, 1) > 0) {
        if (key == 'q' || key == 'Q') {
          running = false;
        } else {
          MAP.handle_input(key);
        }
      }
    }

    MAP.frame(frame_time);

    usleep(static_cast<useconds_t>(frame_time * 1000000));
  }

  set_conio_mode(false);
  std::cout << "\nИгра успешно завершена.\n";
  return 0;
}
