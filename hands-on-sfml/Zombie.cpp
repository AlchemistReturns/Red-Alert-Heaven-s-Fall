#include "Zombie.hpp"
#include <iostream>

Zombie::Zombie(sf::Texture& texture, sf::Vector2f position)
    : Entity(texture, 0.2f), health(ZOMBIE_HEALTH) {
    randomFireInterval = ZOMBIE_FIRE_MIN_INTERVAL +
        static_cast<float>(rand() % int((ZOMBIE_FIRE_MAX_INTERVAL - ZOMBIE_FIRE_MIN_INTERVAL) * 1000)) / 1000.0f;
    sprite.setPosition(position);
}

void Zombie::displayInfo() {
	std::cout << "Zombie created" << std::endl;
}

void Zombie::update(float deltaTime, sf::Vector2f playerPosition, std::vector<ZombieBullet>& zombieBullets,
    sf::Texture& zombieBulletTexture, std::vector<Obstacle>& obstacles) {
    sf::Vector2f direction = playerPosition - sprite.getPosition();
    float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159265f;
    sprite.setRotation(angle + 90);

    // Normalize direction
    float length = std::hypot(direction.x, direction.y);
    if (length != 0) direction /= length;

    // Check for obstacle collision
    sf::Vector2f newPosition = sprite.getPosition() + direction * ZOMBIE_SPEED;
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

    // If collision, find an alternative route
    if (collision) {
        // Try moving in X direction first
        sf::Vector2f alternativeX = sprite.getPosition() + sf::Vector2f(direction.x * ZOMBIE_SPEED, 0);
        sf::FloatRect xBounds = newBounds;
        xBounds.left = alternativeX.x;

        bool xCollision = false;
        for (auto& obstacle : obstacles) {
            if (xBounds.intersects(obstacle.sprite.getGlobalBounds())) {
                xCollision = true;
                break;
            }
        }

        if (!xCollision) {
            sprite.setPosition(alternativeX);
            return;
        }

        // Try moving in Y direction if X is blocked
        sf::Vector2f alternativeY = sprite.getPosition() + sf::Vector2f(0, direction.y * ZOMBIE_SPEED);
        sf::FloatRect yBounds = newBounds;
        yBounds.top = alternativeY.y;

        bool yCollision = false;
        for (auto& obstacle : obstacles) {
            if (yBounds.intersects(obstacle.sprite.getGlobalBounds())) {
                yCollision = true;
                break;
            }
        }

        if (!yCollision) {
            sprite.setPosition(alternativeY);
            return;
        }

        // If completely blocked, zombie stops moving
    }
    else {
        sprite.move(direction * ZOMBIE_SPEED);
    }

    // Zombie shooting logic
    if (fireClock.getElapsedTime().asSeconds() > randomFireInterval) {
        sf::Vector2f bulletDirection = playerPosition - sprite.getPosition();
        float bulletLength = std::hypot(bulletDirection.x, bulletDirection.y);
        if (bulletLength != 0) bulletDirection /= bulletLength;

        zombieBullets.emplace_back(zombieBulletTexture, sprite.getPosition(), bulletDirection);

        fireClock.restart();
        randomFireInterval = ZOMBIE_FIRE_MIN_INTERVAL +
            static_cast<float>(rand() % int((ZOMBIE_FIRE_MAX_INTERVAL - ZOMBIE_FIRE_MIN_INTERVAL) * 1000)) / 1000.0f;
    }
}
