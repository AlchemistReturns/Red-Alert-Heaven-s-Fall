#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <SFML/Graphics.hpp>
#include "Constants.hpp"

class Entity {
public:
    sf::Sprite sprite;

    Entity(sf::Texture& texture, float scale);
    virtual void update(float deltaTime);
    virtual void render(sf::RenderWindow& window);
    virtual void displayInfo() = 0;
};

#endif // ENTITY_HPP

