#ifndef GAMEOVERSCREEN_HPP
#define GAMEOVERSCREEN_HPP

#include <SFML/Graphics.hpp>
#include "Constants.hpp"

class GameOverScreen {
private:
    sf::Font font;
    sf::Text gameOverText;
    sf::Text scoreText;
    sf::Text highScoreText;
    bool isNewHighScore;

    void centerTextGameOver(sf::Text& text, int windowWidth, int windowHeight, int xOffset, int yOffset);

public:
    sf::Text restartText;
    sf::Text exitText;

    GameOverScreen();

    void setFinalScore(int score, int savedHighScore);
    void render(sf::RenderWindow& window);
};

#endif // GAMEOVERSCREEN_HPP
