#include "Helper.hpp"

void centerTextMenu(sf::Text& text, float windowWidth, float windowHeight, float yOffset) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    text.setPosition(windowWidth / 2.0f, (windowHeight / 2.0f) + yOffset);
}

void centerTextGameOver(sf::Text& text, float windowWidth, float windowHeight, float xOffset, float yOffset) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    text.setPosition((windowWidth / 2.0f) + xOffset, (windowHeight / 2.0f) + yOffset);
}
