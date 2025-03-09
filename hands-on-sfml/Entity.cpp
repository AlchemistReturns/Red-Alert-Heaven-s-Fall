#include "Entity.hpp"

Entity::Entity(sf::Texture& texture, float scale) {
    sprite.setTexture(texture);
    sprite.setScale(scale, scale);
}

void Entity::update(float deltaTime) {}

void Entity::render(sf::RenderWindow& window) {
    window.draw(sprite);
}
