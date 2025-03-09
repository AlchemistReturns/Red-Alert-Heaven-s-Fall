#include "Bullet.hpp"
#include <iostream>

Bullet::Bullet(sf::Texture& texture, sf::Vector2f position, sf::Vector2f dir)
    : Entity(texture, 0.1f), direction(dir) {
    sprite.setPosition(position);
}

void Bullet::update(float deltaTime) {
    sprite.move(direction * BULLET_SPEED * 0.6f);
}

void Bullet::displayInfo() {
	std::cout << "Bullet created" << std::endl;
}
