#ifndef BULLET_HPP
#define BULLET_HPP

#include "Entity.hpp"
#include "Constants.hpp"

class Bullet : public Entity {
public:
    sf::Vector2f direction;

    Bullet(sf::Texture& texture, sf::Vector2f position, sf::Vector2f dir);
    void update(float deltaTime) override;
	void displayInfo() override;
};

#endif // BULLET_HPP
