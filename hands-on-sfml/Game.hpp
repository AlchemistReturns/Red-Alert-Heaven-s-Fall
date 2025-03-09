#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <ctime>
#include "Constants.hpp"
#include "Player.hpp"
#include "Bullet.hpp"
#include "Obstacle.hpp"
#include "ZombieBullet.hpp"
#include "Zombie.hpp"
#include "PowerUp.hpp"
#include "Menu.hpp"
#include "GameOverScreen.hpp"

class Game {
private:
    sf::RenderWindow window;
    sf::Texture playerTexture, bulletTexture, zombieTexture, zombieBulletTexture;
    Player* player;
    std::vector<Bullet> bullets;
    std::vector<Obstacle> obstacles;
    std::vector<ZombieBullet> zombieBullets;
    std::vector<Zombie> zombies;
    sf::Font font;
    sf::Text zombieKillText;
    sf::Music backgroundMusic;
    sf::RectangleShape healthBar;
    int zombiesKilled = 0;
    int highScore = 0;
    sf::Clock spawnClock;
    float zombieSpawnInterval = 3.0f;
    sf::Texture powerUpHealthTexture, powerUpSpeedTexture, powerUpDamageTexture;
    std::vector<PowerUp> powerUps;
    sf::Clock powerUpSpawnClock;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
    sf::Texture blockTexture;
    sf::Texture waterTexture;
    sf::Texture pillarTexture;
    sf::Texture vaseTexture;
    sf::View cameraView;
    sf::View miniMapView;
    GameState gameState;
    Menu menu;
    bool isPaused = false;
    sf::Text pauseText;
    sf::RectangleShape pauseOverlay;
    sf::RectangleShape pauseMenu;
    sf::Text resumeText;
    sf::Text exitText;
    GameOverScreen gameOverScreen;

public:
    Game();
    ~Game();
    void run();
    void spawnPowerUp();
    void checkHighScore();
    void checkPowerUpCollisions();
    void restartGame();
    void handleEvents();
    void checkCollisions();
    void update();
    void render();
    int loadHighScore();
    void saveHighScore();
};

#endif // GAME_HPP
