#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP

#include "Entity.hpp"
#include "ZombieBullet.hpp"
#include "Obstacle.hpp"
#include "Constants.hpp"
#include <vector>
#include <cmath>
#include <cstdlib> // For rand()

class Zombie : public Entity {
public:
    int health;
    sf::Clock fireClock;
    sf::Clock spawnClock;
    float randomFireInterval;

    Zombie(sf::Texture& texture, sf::Vector2f position);

    void update(float deltaTime, sf::Vector2f playerPosition, std::vector<ZombieBullet>& zombieBullets,
        sf::Texture& zombieBulletTexture, std::vector<Obstacle>& obstacles);
	void displayInfo() override;
};

#endif // ZOMBIE_HPP
