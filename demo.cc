#include <SFML/Graphics.hpp>

#include "entity.h"
#include <functional>

typedef World<struct Drawable, struct Position, struct Velocity,
              struct DelayedAction>
    MyWorld;

struct Drawable {
  sf::Sprite* sprite;
};

struct Position {
  double x;
  double y;
};

struct Velocity {
  double dx;
  double dy;
};

struct DelayedAction {
  int loops;
  std::function<void(MyWorld::Entity& e)> action;
};

void flip(MyWorld::Entity& e) {
  auto& v = e.getOrAdd<Velocity>();
  v.dx = -v.dx;
  auto& d = e.getOrAdd<DelayedAction>();
  d.action = flip;
  d.loops = 50000;
}

int main() {
  // create the window
  sf::RenderWindow window(sf::VideoMode(800, 600), "My window");

  sf::Texture texture;
  if (!texture.loadFromFile("Box-Turtle-Transparent-PNG.png")) {
    return 1;
  }
  sf::Sprite sprite;
  sprite.setTexture(texture);

  MyWorld world;

  world.add_entity(Drawable{&sprite}, Position{}, Velocity{0.01, 0},
                   DelayedAction{50000, flip});
  world.add_entity(Drawable{&sprite}, Position{-1000, 0}, Velocity{0.015, 0});

  // run the program as long as the window is open
  while (window.isOpen()) {
    // check all the window's events that were triggered since the last
    // iteration of the loop
    sf::Event event;
    while (window.pollEvent(event)) {
      // "close requested" event: we close the window
      if (event.type == sf::Event::Closed) window.close();
      break;
    }

    // clear the window with black color
    window.clear(sf::Color::Black);

    world.each<Position, Velocity>([&](MyWorld::Entity& e) {
      auto& p = e.get<Position>();
      auto& v = e.get<Velocity>();
      p.x += v.dx;
      p.y += v.dy;
    });

    world.each<DelayedAction>([&](MyWorld::Entity& e) {
      auto& d = e.get<DelayedAction>();
      if (!d.loops--) {
        auto copied = d;
        e.remove<DelayedAction>();
        d.action(e);
      }
    });

    world.each<Drawable, Position>([&](MyWorld::Entity& e) {
      auto& p = e.get<Position>();
      auto& d = e.get<Drawable>();
      d.sprite->setPosition(p.x, p.y);
      window.draw(*e.get<Drawable>().sprite);
    });

    // end the current frame
    window.display();
  }

  return 0;
}
