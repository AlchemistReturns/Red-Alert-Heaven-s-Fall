#include "GameOverScreen.hpp"

GameOverScreen::GameOverScreen() : isNewHighScore(false) {
    font.loadFromFile("arial.ttf");

    gameOverText.setFont(font);
    gameOverText.setString("Game Over!");
    gameOverText.setCharacterSize(60);
    gameOverText.setFillColor(sf::Color::Red);
    centerTextGameOver(gameOverText, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -200);

    scoreText.setFont(font);
    scoreText.setCharacterSize(40);
    scoreText.setFillColor(sf::Color::White);
    centerTextGameOver(scoreText, WINDOW_WIDTH, WINDOW_HEIGHT, -100, -100);

    highScoreText.setFont(font);
    highScoreText.setCharacterSize(35);
    highScoreText.setFillColor(sf::Color::Yellow);
    centerTextGameOver(highScoreText, WINDOW_WIDTH, WINDOW_HEIGHT, -100, -50);

    restartText.setFont(font);
    restartText.setString("Press R to Restart");
    restartText.setCharacterSize(30);
    restartText.setFillColor(sf::Color::White);
    centerTextGameOver(restartText, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 50);

    exitText.setFont(font);
    exitText.setString("Press Esc to Exit");
    exitText.setCharacterSize(30);
    exitText.setFillColor(sf::Color::White);
    centerTextGameOver(exitText, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 130);
}

void GameOverScreen::centerTextGameOver(sf::Text& text, int windowWidth, int windowHeight, int xOffset, int yOffset) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.width / 2, bounds.height / 2);
    text.setPosition(windowWidth / 2 + xOffset, windowHeight / 2 + yOffset);
}

void GameOverScreen::setFinalScore(int score, int savedHighScore) {
    scoreText.setString("Final Score: " + std::to_string(score));

    if (score > savedHighScore) {
        highScoreText.setString("New High Score! Congratulations!");
    }
    else {
        highScoreText.setString("High Score: " + std::to_string(savedHighScore));
    }
}

void GameOverScreen::render(sf::RenderWindow& window) {
    window.clear();
    window.setView(window.getDefaultView());
    window.draw(gameOverText);
    window.draw(scoreText);
    window.draw(highScoreText);
    window.draw(restartText);
    window.draw(exitText);
    window.display();
}
