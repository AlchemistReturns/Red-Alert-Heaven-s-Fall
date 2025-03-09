#include "ZombieBullet.hpp"
#include <iostream>

ZombieBullet::ZombieBullet(sf::Texture& texture, sf::Vector2f position, sf::Vector2f dir)
    : Entity(texture, 0.1f), direction(dir) {
    sprite.setPosition(position);
}

void ZombieBullet::displayInfo() {
	std::cout << "ZombieBullet created" << std::endl;
}

void ZombieBullet::update(float deltaTime) {
    sprite.move(direction * BULLET_SPEED * 0.3f);
}
