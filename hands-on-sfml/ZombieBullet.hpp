#ifndef ZOMBIEBULLET_HPP
#define ZOMBIEBULLET_HPP

#include "Entity.hpp"
#include "Constants.hpp" // Ensure BULLET_SPEED is accessible

class ZombieBullet : public Entity {
public:
    sf::Vector2f direction;

    ZombieBullet(sf::Texture& texture, sf::Vector2f position, sf::Vector2f dir);
    void update(float deltaTime) override;
	void displayInfo() override;
};

#endif // ZOMBIEBULLET_HPP
