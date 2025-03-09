#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <SFML/Graphics.hpp>

class Obstacle {
public:
    sf::Sprite sprite;

    Obstacle(sf::Texture& texture, sf::Vector2f position);

    void render(sf::RenderWindow& window);
    sf::FloatRect getBounds();
};

#endif
