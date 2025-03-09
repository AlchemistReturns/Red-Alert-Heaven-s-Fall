#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Entity.hpp"
#include "Obstacle.hpp"
#include "Constants.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

class Player : public Entity {
public:
    int health;
    bool speedBoost = false;
    bool damageBoost = false;
    sf::Clock speedBoostClock;
    sf::Clock damageBoostClock;

    Player(sf::Texture& texture);

    void move(std::vector<Obstacle>& obstacles);
    void updateBoosts();
    sf::Vector2f getDirection();
	void displayInfo() override;
};

#endif // PLAYER_HPP
