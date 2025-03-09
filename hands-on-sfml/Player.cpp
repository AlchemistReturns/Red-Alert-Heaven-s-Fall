#include "Player.hpp"
#include <iostream>

Player::Player(sf::Texture& texture) : Entity(texture, 0.25f), health(PLAYER_MAX_HEALTH) {
    sprite.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2, bounds.height / 2);
}

void Player::displayInfo() {
	std::cout << "Player created" << std::endl;
}

void Player::move(std::vector<Obstacle>& obstacles) {
    sf::Vector2f newPosition = sprite.getPosition();
    sf::Vector2f oldPosition = newPosition;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) newPosition.y -= PLAYER_SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) newPosition.y += PLAYER_SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) newPosition.x -= PLAYER_SPEED;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) newPosition.x += PLAYER_SPEED;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) sprite.rotate(-0.05f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) sprite.rotate(0.05f);

    // Check collision with obstacles
    sf::FloatRect newBounds = sprite.getGlobalBounds();
    newBounds.left = newPosition.x;
    newBounds.top = newPosition.y;

    bool collision = false;
    for (auto& obstacle : obstacles) {
        if (newBounds.intersects(obstacle.sprite.getGlobalBounds())) {
            collision = true;
            break;
        }
    }

    // Move only if no collision
    float minX = 0, minY = 0;
    float maxX = 2000 - newBounds.width;
    float maxY = 2000 - newBounds.height;

    if (!collision) {
        newPosition.x = std::max(minX, std::min(maxX, newPosition.x));
        newPosition.y = std::max(minY, std::min(maxY, newPosition.y));

        sprite.setPosition(newPosition);
    }
    else {
        sprite.setPosition(oldPosition);  // Revert to old position if blocked
    }
}

void Player::updateBoosts() {
    if (speedBoost && speedBoostClock.getElapsedTime().asSeconds() > 5) speedBoost = false;
    if (damageBoost && damageBoostClock.getElapsedTime().asSeconds() > 5) damageBoost = false;
}

sf::Vector2f Player::getDirection() {
    float angle = sprite.getRotation() - 90;
    float rad = angle * 3.14159265f / 180;
    return sf::Vector2f(std::cos(rad), std::sin(rad));
}
