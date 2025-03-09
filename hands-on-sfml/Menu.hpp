#ifndef MENU_HPP
#define MENU_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Constants.hpp"

class Menu {
private:
    sf::Font font;
    sf::Text startText;
    sf::Text soundText;
    sf::Text title;
    sf::Text highScoreText;
    bool soundOn;
    int highScore;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    void centerTextMenu(sf::Text& text, int windowWidth, int windowHeight, int yOffset);

public:
    Menu(int highScore);

    void handleInput(sf::RenderWindow& window, GameState& gameState, sf::Music& backgroundMusic);
    void render(sf::RenderWindow& window);
    void updateHighScore(int newHighScore);
};

#endif // MENU_HPP
