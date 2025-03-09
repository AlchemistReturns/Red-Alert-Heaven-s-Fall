#include "PowerUp.hpp"
#include <iostream>

PowerUp::PowerUp(sf::Texture& texture, sf::Vector2f position, Type powerUpType)
    : Entity(texture, 0.2f), type(powerUpType) {
    sprite.setPosition(position);
}

void PowerUp::displayInfo() {
	std::cout << "PowerUp created" << std::endl;
}

void PowerUp::applyEffect(Player& player) {
    switch (type) {
    case HEALTH:
        player.health = std::min(player.health + 5, PLAYER_MAX_HEALTH);
        break;
    case SPEED:
        player.speedBoost = true;
        player.speedBoostClock.restart();
        break;
    case DAMAGE:
        player.damageBoost = true;
        player.damageBoostClock.restart();
        break;
    }
}
