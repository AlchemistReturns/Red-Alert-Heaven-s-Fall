#ifndef POWERUP_HPP
#define POWERUP_HPP

#include "Entity.hpp"
#include "Player.hpp"
#include "Constants.hpp"
#include <algorithm>

class PowerUp : public Entity {
public:
    enum Type { HEALTH, SPEED, DAMAGE };
    Type type;

    PowerUp(sf::Texture& texture, sf::Vector2f position, Type powerUpType);

    void applyEffect(Player& player);
	void displayInfo() override;
};

#endif // POWERUP_HPP
