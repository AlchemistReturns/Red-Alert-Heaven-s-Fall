#include "Obstacle.hpp"

Obstacle::Obstacle(sf::Texture& texture, sf::Vector2f position) {
    sprite.setTexture(texture);
    sprite.setScale(1.0f, 1.0f);
    sprite.setPosition(position);

    texture.setSmooth(true);
}

void Obstacle::render(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Obstacle::getBounds() {
    return sprite.getGlobalBounds();
}
