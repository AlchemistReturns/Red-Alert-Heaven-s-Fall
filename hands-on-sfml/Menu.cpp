#include "Menu.hpp"

Menu::Menu(int highScore) : soundOn(true), highScore(highScore) {
    font.loadFromFile("arial.ttf");
    backgroundTexture.loadFromFile("assets/menu_background.jpg");
    backgroundSprite.setTexture(backgroundTexture);

    title.setFont(font);
    title.setString("Red Alert");
    title.setCharacterSize(60);
    title.setFillColor(sf::Color::Red);
    centerTextMenu(title, WINDOW_WIDTH, WINDOW_HEIGHT, -150);

    startText.setFont(font);
    startText.setString("Start Game");
    startText.setCharacterSize(30);
    startText.setFillColor(sf::Color::Black);
    centerTextMenu(startText, WINDOW_WIDTH, WINDOW_HEIGHT, -50);

    soundText.setFont(font);
    soundText.setString("Sound: On");
    soundText.setCharacterSize(30);
    soundText.setFillColor(sf::Color::Black);
    centerTextMenu(soundText, WINDOW_WIDTH, WINDOW_HEIGHT, +20);

    highScoreText.setFont(font);
    highScoreText.setString("High Score: " + std::to_string(highScore));
    highScoreText.setCharacterSize(30);
    highScoreText.setFillColor(sf::Color::Black);
    centerTextMenu(highScoreText, WINDOW_WIDTH, WINDOW_HEIGHT, +90);
}

void Menu::centerTextMenu(sf::Text& text, int windowWidth, int windowHeight, int yOffset) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.width / 2, bounds.height / 2);
    text.setPosition(windowWidth / 2, windowHeight / 2 + yOffset);
}

void Menu::handleInput(sf::RenderWindow& window, GameState& gameState, sf::Music& backgroundMusic) {
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        if (startText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            gameState = GameState::PLAYING;
        }

        if (soundText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            soundOn = !soundOn;
            soundText.setString(soundOn ? "Sound: On" : "Sound: Off");

            if (soundOn) {
                backgroundMusic.play();
            }
            else {
                backgroundMusic.pause();
            }
        }
    }
}

void Menu::render(sf::RenderWindow& window) {
    window.clear();
    window.draw(backgroundSprite);
    window.draw(title);
    window.draw(startText);
    window.draw(soundText);
    window.draw(highScoreText);
    window.display();
}

void Menu::updateHighScore(int newHighScore) {
    highScore = newHighScore;
    highScoreText.setString("High Score: " + std::to_string(highScore));
}
