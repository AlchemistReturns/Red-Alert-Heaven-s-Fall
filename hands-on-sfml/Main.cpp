#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream> 

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 900;
const float PLAYER_SPEED = 0.125f;
const float BULLET_SPEED = 0.5f;
const float ZOMBIE_SPEED = 0.005f;
const int MAX_ZOMBIES = 5;
const float MIN_SPAWN_DISTANCE = 200.0f;
const float ZOMBIE_FIRE_MIN_INTERVAL = 1.0f;
const float ZOMBIE_FIRE_MAX_INTERVAL = 3.0f;
const int ZOMBIE_HEALTH = 3;
const int PLAYER_MAX_HEALTH = 20;

void centerTextMenu(sf::Text& text, float windowWidth, float windowHeight, float yOffset = 0) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    text.setPosition(windowWidth / 2.0f, (windowHeight / 2.0f) + yOffset);
}

void centerTextGameOver(sf::Text& text, float windowWidth, float windowHeight, float xOffset = 0, float yOffset = 0) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    text.setPosition((windowWidth / 2.0f) + xOffset, (windowHeight / 2.0f) + yOffset);
}


class Entity {
public:
    sf::Sprite sprite;
    Entity(sf::Texture& texture, float scale) {
        sprite.setTexture(texture);
        sprite.setScale(scale, scale);
    }
    virtual void update(float deltaTime) {}
    virtual void render(sf::RenderWindow& window) {
        window.draw(sprite);
    }
};

class Obstacle {
public:
    sf::Sprite sprite;

    Obstacle(sf::Texture& texture, sf::Vector2f position) {
        sprite.setTexture(texture);
        sprite.setScale(1.0f, 1.0f);
        sprite.setPosition(position);

        texture.setSmooth(true);

    }

    void render(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    sf::FloatRect getBounds() {
        return sprite.getGlobalBounds();
    }
};


class Player : public Entity {
public:
    int health;
    bool speedBoost = false;
    bool damageBoost = false;
    sf::Clock speedBoostClock;
    sf::Clock damageBoostClock;

    Player(sf::Texture& texture) : Entity(texture, 0.25f), health(PLAYER_MAX_HEALTH) {
        sprite.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width / 2, bounds.height / 2);
    }
    void move(std::vector<Obstacle>& obstacles) {
        sf::Vector2f newPosition = sprite.getPosition();
        sf::Vector2f oldPosition = newPosition;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) newPosition.y -= PLAYER_SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) newPosition.y += PLAYER_SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) newPosition.x -= PLAYER_SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) newPosition.x += PLAYER_SPEED;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) sprite.rotate(-0.05f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) sprite.rotate(0.05f);

        // Check collision with obstacles
        sf::FloatRect newBounds = sprite.getGlobalBounds();
        newBounds.left = newPosition.x;
        newBounds.top = newPosition.y;

        bool collision = false;
        for (auto& obstacle : obstacles) {
            if (newBounds.intersects(obstacle.sprite.getGlobalBounds())) {
                collision = true;
                break;
            }
        }

        // Move only if no collision
        float minX = 0, minY = 0;
        float maxX = 2000 - newBounds.width; 
        float maxY = 2000 - newBounds.height;

        if (!collision) {
            newPosition.x = std::max(minX, std::min(maxX, newPosition.x));
            newPosition.y = std::max(minY, std::min(maxY, newPosition.y));

            sprite.setPosition(newPosition);
        }
        else {
            sprite.setPosition(oldPosition);  // Revert to old position if blocked
        }
    }

    void updateBoosts() {
        if (speedBoost && speedBoostClock.getElapsedTime().asSeconds() > 5) speedBoost = false;
        if (damageBoost && damageBoostClock.getElapsedTime().asSeconds() > 5) damageBoost = false;
    }

    sf::Vector2f getDirection() {
        float angle = sprite.getRotation() - 90;
        float rad = angle * 3.14159265f / 180; 
        return sf::Vector2f(std::cos(rad), std::sin(rad));
    }
};

class PowerUp : public Entity {
public:
    enum Type { HEALTH, SPEED, DAMAGE };
    Type type;

    PowerUp(sf::Texture& texture, sf::Vector2f position, Type powerUpType)
        : Entity(texture, 0.2f), type(powerUpType) {
        sprite.setPosition(position);
    }

    void applyEffect(Player& player) {
        switch (type) {
        case HEALTH:
            player.health = std::min(player.health + 5, PLAYER_MAX_HEALTH);
            break;
        case SPEED:
            player.speedBoost = true;
            player.speedBoostClock.restart();
            break;
        case DAMAGE:
            player.damageBoost = true;
            player.damageBoostClock.restart();
            break;
        }
    }
};


class Bullet : public Entity {
public:
    sf::Vector2f direction;
    Bullet(sf::Texture& texture, sf::Vector2f position, sf::Vector2f dir) : Entity(texture, 0.1f), direction(dir) {
        sprite.setPosition(position);
    }
    void update(float deltaTime) override {
        sprite.move(direction * BULLET_SPEED);
    }
};

class ZombieBullet : public Entity {
public:
    sf::Vector2f direction;
    ZombieBullet(sf::Texture& texture, sf::Vector2f position, sf::Vector2f dir) : Entity(texture, 0.1f), direction(dir) {
        sprite.setPosition(position);
    }
    void update(float deltaTime) override {
        sprite.move(direction * BULLET_SPEED * 0.3f);
    }

};

class Zombie : public Entity {
public:
    int health;
    sf::Clock fireClock;
    sf::Clock spawnClock;
    float randomFireInterval;

    Zombie(sf::Texture& texture, sf::Vector2f position) : Entity(texture, 0.2f), health(ZOMBIE_HEALTH) {
        randomFireInterval = ZOMBIE_FIRE_MIN_INTERVAL + static_cast<float>(rand() % int((ZOMBIE_FIRE_MAX_INTERVAL - ZOMBIE_FIRE_MIN_INTERVAL) * 1000)) / 1000.0f;
        sprite.setPosition(position);
    }

    void update(float deltaTime, sf::Vector2f playerPosition, std::vector<ZombieBullet>& zombieBullets, sf::Texture& zombieBulletTexture, std::vector<Obstacle>& obstacles) {
        sf::Vector2f direction = playerPosition - sprite.getPosition();
        float angle = std::atan2(direction.y, direction.x) * 180 / 3.14159265f;
        sprite.setRotation(angle + 90);

        // Normalize direction
        float length = std::hypot(direction.x, direction.y);
        if (length != 0) direction /= length;

        // Check for obstacle collision
        sf::Vector2f newPosition = sprite.getPosition() + direction * ZOMBIE_SPEED;
        sf::FloatRect newBounds = sprite.getGlobalBounds();
        newBounds.left = newPosition.x;
        newBounds.top = newPosition.y;

        bool collision = false;
        for (auto& obstacle : obstacles) {
            if (newBounds.intersects(obstacle.sprite.getGlobalBounds())) {
                collision = true;
                break;
            }
        }

        // If collision, find an alternative route
        if (collision) {
            // Try moving in X direction first
            sf::Vector2f alternativeX = sprite.getPosition() + sf::Vector2f(direction.x * ZOMBIE_SPEED, 0);
            sf::FloatRect xBounds = newBounds;
            xBounds.left = alternativeX.x;

            bool xCollision = false;
            for (auto& obstacle : obstacles) {
                if (xBounds.intersects(obstacle.sprite.getGlobalBounds())) {
                    xCollision = true;
                    break;
                }
            }

            if (!xCollision) {
                sprite.setPosition(alternativeX);
                return;
            }

            // Try moving in Y direction if X is blocked
            sf::Vector2f alternativeY = sprite.getPosition() + sf::Vector2f(0, direction.y * ZOMBIE_SPEED);
            sf::FloatRect yBounds = newBounds;
            yBounds.top = alternativeY.y;

            bool yCollision = false;
            for (auto& obstacle : obstacles) {
                if (yBounds.intersects(obstacle.sprite.getGlobalBounds())) {
                    yCollision = true;
                    break;
                }
            }

            if (!yCollision) {
                sprite.setPosition(alternativeY);
                return;
            }

            // If completely blocked, zombie stops moving
        }
        else {
            sprite.move(direction * ZOMBIE_SPEED);
        }

        if (fireClock.getElapsedTime().asSeconds() > randomFireInterval) {
            sf::Vector2f bulletDirection = playerPosition - sprite.getPosition();
            float bulletLength = std::hypot(bulletDirection.x, bulletDirection.y);
            if (bulletLength != 0) bulletDirection /= bulletLength;
            zombieBullets.emplace_back(zombieBulletTexture, sprite.getPosition(), bulletDirection);
            fireClock.restart();
            randomFireInterval = ZOMBIE_FIRE_MIN_INTERVAL + static_cast<float>(rand() % int((ZOMBIE_FIRE_MAX_INTERVAL - ZOMBIE_FIRE_MIN_INTERVAL) * 1000)) / 1000.0f;
        }
    }

};

enum class GameState { MENU, PLAYING, GAME_OVER };

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
public:
    Menu(int highScore) : soundOn(true), highScore(highScore) {
        font.loadFromFile("arial.ttf");
        backgroundTexture.loadFromFile("menu_background.jpg");
        backgroundSprite.setTexture(backgroundTexture);

        title.setFont(font);
        title.setString("Red Alert");
        title.setCharacterSize(60);
        title.setFillColor(sf::Color::Red);
        centerTextMenu(title, WINDOW_WIDTH, WINDOW_HEIGHT, -150);
        
        startText.setFont(font);
        startText.setString("Start Game");
        startText.setCharacterSize(30);
        startText.setFillColor(sf::Color::White);
        centerTextMenu(startText, WINDOW_WIDTH, WINDOW_HEIGHT, -50);

        soundText.setFont(font);
        soundText.setString("Sound: On");
        soundText.setCharacterSize(30);
        soundText.setFillColor(sf::Color::White);
        centerTextMenu(soundText, WINDOW_WIDTH, WINDOW_HEIGHT, +20);

        highScoreText.setFont(font);
        highScoreText.setString("High Score: " + std::to_string(highScore));
        highScoreText.setCharacterSize(30);
        highScoreText.setFillColor(sf::Color::White);
        centerTextMenu(highScoreText, WINDOW_WIDTH, WINDOW_HEIGHT, +90);
    }


    void handleInput(sf::RenderWindow& window, GameState& gameState, sf::Music& backgroundMusic) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            if (startText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                gameState = GameState::PLAYING;
            }

            if (soundText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
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


    void render(sf::RenderWindow& window) {
        window.clear();
        window.draw(backgroundSprite);
        window.draw(title);
        window.draw(startText);
        window.draw(soundText);
        window.draw(highScoreText);  
        window.display();
    }

    void updateHighScore(int newHighScore) {
        highScore = newHighScore;
        highScoreText.setString("High Score: " + std::to_string(highScore));
    }
};


class GameOverScreen {
private:
    sf::Font font;
    sf::Text gameOverText;
    sf::Text scoreText;
    sf::Text highScoreText;
    bool isNewHighScore;

public:
    sf::Text restartText;
    sf::Text exitText;
    GameOverScreen() : isNewHighScore(false) {
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

    void setFinalScore(int score, int savedHighScore) {  
        scoreText.setString("Final Score: " + std::to_string(score));

        if (score > savedHighScore) {
            highScoreText.setString("New High Score! Congratulations!");
        }
        else {
            highScoreText.setString("High Score: " + std::to_string(savedHighScore)); 
        }
    }


    void render(sf::RenderWindow& window) {
        window.clear();
        window.setView(window.getDefaultView());
        window.draw(gameOverText);
        window.draw(scoreText);
        window.draw(highScoreText);
        window.draw(restartText);
        window.draw(exitText);
        window.display();
    }
};



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
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Zombie Shooter"), gameState(GameState::MENU), menu(loadHighScore()) {
        cameraView.setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        cameraView.setCenter(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        playerTexture.loadFromFile("player.png");
        bulletTexture.loadFromFile("bullet.png");
        zombieTexture.loadFromFile("zombie.png");
        zombieBulletTexture.loadFromFile("zombie_bullet.png");
        
        player = new Player(playerTexture);
        player->health = PLAYER_MAX_HEALTH;

        powerUpHealthTexture.loadFromFile("powerup_health.png");
        powerUpSpeedTexture.loadFromFile("powerup_speed.png");
        powerUpDamageTexture.loadFromFile("powerup_damage.png");

        if (!backgroundTexture.loadFromFile("background.jpg")) {
            std::cerr << "Error loading background image!\n";
        }

        if (!blockTexture.loadFromFile("block.png")) {
            std::cerr << "Error loading obstacle texture!\n";
        }
        if (!waterTexture.loadFromFile("water.jpg")) {
            std::cerr << "Error loading obstacle texture!\n";
        }
        if (!vaseTexture.loadFromFile("vase.png")) {
            std::cerr << "Error loading obstacle texture!\n";
        }
        if (!pillarTexture.loadFromFile("cloud.png")) {
            std::cerr << "Error loading obstacle texture!\n";
        }

        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setScale(
			float(2000) / backgroundTexture.getSize().x,
            float(2000) / backgroundTexture.getSize().y
        );

        pauseText.setFont(font);
        pauseText.setString("Game Paused\nPress P to Resume");
        pauseText.setCharacterSize(30);
        pauseText.setFillColor(sf::Color::White);
        pauseText.setPosition(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 50);

        player = new Player(playerTexture);
        srand(static_cast<unsigned>(time(0)));
        healthBar.setSize(sf::Vector2f(200, 20));
        healthBar.setFillColor(sf::Color::White);
        healthBar.setPosition(10, WINDOW_HEIGHT - 30);

        font.loadFromFile("arial.ttf");
        zombieKillText.setFont(font);
        
        pauseOverlay.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150)); 

        pauseMenu.setSize(sf::Vector2f(300, 200));
        pauseMenu.setFillColor(sf::Color(50, 50, 50, 220)); 
        pauseMenu.setOutlineColor(sf::Color::White);
        pauseMenu.setOutlineThickness(3);
        pauseMenu.setPosition(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 100);

        resumeText.setFont(font);
        resumeText.setString("Resume");
        resumeText.setCharacterSize(28);
        resumeText.setFillColor(sf::Color::White);
        resumeText.setPosition(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 50);

        exitText.setFont(font);
        exitText.setString("Exit");
        exitText.setCharacterSize(28);
        exitText.setFillColor(sf::Color::White);
        exitText.setPosition(WINDOW_WIDTH / 2 - 30, WINDOW_HEIGHT / 2 + 20);

        zombieKillText.setCharacterSize(20);
        zombieKillText.setFillColor(sf::Color::White);
        zombieKillText.setPosition(10, WINDOW_HEIGHT - 60);

        obstacles.emplace_back(pillarTexture, sf::Vector2f(1000, 800));
        obstacles.emplace_back(pillarTexture, sf::Vector2f(300, 1200));
        

        // Mini-map View 
        miniMapView.setSize(2000, 2000);  
        miniMapView.setViewport(sf::FloatRect(0.75f, 0.75f, 0.2f, 0.2f));  

        if (!backgroundMusic.openFromFile("World War Z Theme Song.ogg")) {
            std::cerr << "Error loading background music!" << std::endl;
        }
        else {
            backgroundMusic.setLoop(true);
            backgroundMusic.setVolume(100);
            backgroundMusic.play();
        }


    }

    ~Game() {
        delete player;
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

    void spawnPowerUp() {
        if (powerUpSpawnClock.getElapsedTime().asSeconds() > 10) {
            sf::Vector2f spawnPosition(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT);
            int randomType = rand() % 3;
            sf::Texture* chosenTexture = nullptr;  

            switch (randomType) {
            case 0: chosenTexture = &powerUpHealthTexture; break;
            case 1: chosenTexture = &powerUpSpeedTexture; break;
            case 2: chosenTexture = &powerUpDamageTexture; break;
            default: chosenTexture = &powerUpHealthTexture; break;  
            }

            if (chosenTexture) {
                powerUps.emplace_back(*chosenTexture, spawnPosition, static_cast<PowerUp::Type>(randomType));
                powerUpSpawnClock.restart();
            }
        }
    }

    void checkHighScore() {
        int savedHighScore = loadHighScore(); 
        if (zombiesKilled > savedHighScore) { 
            highScore = zombiesKilled;
            saveHighScore();
            menu.updateHighScore(highScore);
        }
        else {
            highScore = savedHighScore; 
        }
    }



    void checkPowerUpCollisions() {
        for (auto it = powerUps.begin(); it != powerUps.end();) {
            if (it->sprite.getGlobalBounds().intersects(player->sprite.getGlobalBounds())) {
                it->applyEffect(*player);
                it = powerUps.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void restartGame() {  
        gameState = GameState::PLAYING;
        zombiesKilled = 0;
        bullets.clear();
        zombies.clear();
        zombieBullets.clear();
        player->health = PLAYER_MAX_HEALTH;
        player->sprite.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        spawnClock.restart();
        powerUps.clear();
        powerUpSpawnClock.restart();
    }



    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
           
            if (gameState == GameState::MENU) {
                menu.handleInput(window, gameState, backgroundMusic);  
                return;  
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::P) {
                    isPaused = !isPaused; 
                }
            }

            if (isPaused) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePos);

                    if (resumeText.getGlobalBounds().contains(worldMousePos)) {
                        isPaused = false; 
                    }
                    else if (exitText.getGlobalBounds().contains(worldMousePos)) {
                        window.close(); 
                    }
                }
            }

            else if (gameState == GameState::GAME_OVER) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::R) {
                        restartGame();  
                    }
                    else if (event.key.code == sf::Keyboard::Escape) {
                        window.close(); 
                    }
                }

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePos);

                    if (gameOverScreen.restartText.getGlobalBounds().contains(worldMousePos)) {
                        restartGame();  
                    }
                    else if (gameOverScreen.exitText.getGlobalBounds().contains(worldMousePos)) {
                        window.close();
                    }
                }
            }

            else {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    bullets.emplace_back(bulletTexture, player->sprite.getPosition(), player->getDirection());
                }
            }
        }

        if (gameState == GameState::MENU) {
            menu.handleInput(window, gameState, backgroundMusic);
        }

    }



    void checkCollisions() {
        for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {

            if (bulletIt->sprite.getPosition().x < 0 || bulletIt->sprite.getPosition().x > 2000 ||
                bulletIt->sprite.getPosition().y < 0 || bulletIt->sprite.getPosition().y > 2000) {
                bulletIt = bullets.erase(bulletIt);
                continue; 
            }

            bool bulletRemoved = false;

            // Check if bullet hits an obstacle
            for (auto& obstacle : obstacles) {
                if (bulletIt->sprite.getGlobalBounds().intersects(obstacle.sprite.getGlobalBounds())) {
                    bulletIt = bullets.erase(bulletIt);
                    bulletRemoved = true;
                    break;
                }
            }
            if (bulletRemoved) continue;

            // Check if bullet hits a zombie
            for (auto zombieIt = zombies.begin(); zombieIt != zombies.end();) {
                if (bulletIt->sprite.getGlobalBounds().intersects(zombieIt->sprite.getGlobalBounds())) {
                    zombieIt->health--;
                    bulletIt = bullets.erase(bulletIt);
                    bulletRemoved = true;
                    if (zombieIt->health <= 0) {
                        zombiesKilled++;
                        zombieIt = zombies.erase(zombieIt);
                        checkHighScore();
                    }
                    break;
                }
                else {
                    ++zombieIt;
                }
            }
            if (!bulletRemoved) {
                ++bulletIt;
            }
        }

        for (auto zombieBulletIt = zombieBullets.begin(); zombieBulletIt != zombieBullets.end();) {
            bool zombieBulletRemoved = false;

            // Check if zombie bullet hits an obstacle
            for (auto& obstacle : obstacles) {
                if (zombieBulletIt->sprite.getGlobalBounds().intersects(obstacle.sprite.getGlobalBounds())) {
                    zombieBulletIt = zombieBullets.erase(zombieBulletIt);
                    zombieBulletRemoved = true;
                    break;
                }
            }
            if (zombieBulletRemoved) continue;

            // Check if zombie bullet hits player
            if (zombieBulletIt->sprite.getGlobalBounds().intersects(player->sprite.getGlobalBounds())) {
                player->health--;
                zombieBulletIt = zombieBullets.erase(zombieBulletIt);
                if (player->health <= 0) {
                    checkHighScore();
                    int savedHighScore = loadHighScore(); 
                    gameOverScreen.setFinalScore(zombiesKilled, savedHighScore); 
                    gameState = GameState::GAME_OVER;
                }

            }
            else {
                ++zombieBulletIt;
            }
        }
    }


    void update() {
        if (gameState == GameState::MENU) {
            return; 
        }

        if (!isPaused) { 
            player->move(obstacles); 
            player->updateBoosts();
            spawnPowerUp();
            checkPowerUpCollisions();

            miniMapView.setCenter(player->sprite.getPosition());

            sf::Vector2f playerPos = player->sprite.getPosition();
            float halfWidth = WINDOW_WIDTH / 2;
            float halfHeight = WINDOW_HEIGHT / 2;

            float minX = halfWidth, minY = halfHeight;
            float maxX = 2000 - halfWidth;
            float maxY = 2000 - halfHeight;

            float cameraX = std::max(minX, std::min(maxX, playerPos.x));
            float cameraY = std::max(minY, std::min(maxY, playerPos.y));

            cameraView.setCenter(cameraX, cameraY);
            window.setView(cameraView);

            // Update bullets
            for (auto& bullet : bullets)
                bullet.update(0);

            for (auto& zombieBullet : zombieBullets)
                zombieBullet.update(0);

            for (auto& zombie : zombies)
                zombie.update(0, player->sprite.getPosition(), zombieBullets, zombieBulletTexture, obstacles);

            checkCollisions();

            // Zombie spawning logic
            if (spawnClock.getElapsedTime().asSeconds() > zombieSpawnInterval) {
                sf::Vector2f spawnPosition(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT);

                // Ensure zombies don't spawn inside obstacles
                bool validSpawn = true;
                for (auto& obstacle : obstacles) {
                    if (sf::FloatRect(spawnPosition.x, spawnPosition.y, 40, 40).intersects(obstacle.sprite.getGlobalBounds())) {
                        validSpawn = false;
                        break;
                    }
                }

                if (validSpawn) {
                    zombies.emplace_back(zombieTexture, spawnPosition);
                }

                spawnClock.restart();
            }
        }

        // Update UI elements
        healthBar.setSize(sf::Vector2f(10 * player->health, 20));
        zombieKillText.setString("Zombies Killed: " + std::to_string(zombiesKilled));
    }





    void render() {
        if (gameState == GameState::MENU) {
            menu.render(window);
        }
        else if (gameState == GameState::GAME_OVER) {
            gameOverScreen.render(window);
        }
        else {
            window.clear(sf::Color::Black);
            window.setView(cameraView);
            window.draw(backgroundSprite);
            player->render(window);

            for (auto& bullet : bullets) bullet.render(window);
            for (auto& zombieBullet : zombieBullets) zombieBullet.render(window);
            for (auto& zombie : zombies) zombie.render(window);
            for (auto& powerUp : powerUps) powerUp.render(window);
            for (auto& obstacle : obstacles) obstacle.render(window);

            // Draw the Mini-map
            window.setView(window.getDefaultView());  
            window.setView(miniMapView);  
            window.draw(backgroundSprite);  

            // Draw Player as a small dot
            sf::CircleShape playerDot(50);
            playerDot.setFillColor(sf::Color::Blue);
            playerDot.setPosition(player->sprite.getPosition() * 0.08f);  
            window.draw(playerDot);

            // Draw Zombies as red dots
            for (auto& zombie : zombies) {
                sf::CircleShape zombieDot(50);
                zombieDot.setFillColor(sf::Color::Red);
                zombieDot.setPosition(zombie.sprite.getPosition() * 0.08f);  
                window.draw(zombieDot);
            }

            // Draw Obstacles as white squares
            for (auto& obstacle : obstacles) {
                sf::RectangleShape obstacleDot(sf::Vector2f(6, 6));  
                obstacleDot.setFillColor(sf::Color::White);
                obstacleDot.setPosition(obstacle.sprite.getPosition() * 0.08f);  
                window.draw(obstacleDot);
            }

            window.setView(window.getDefaultView());
            window.draw(healthBar);
            window.draw(zombieKillText);

            if (isPaused) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                if (resumeText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    resumeText.setFillColor(sf::Color::Yellow); // Highlight when hovered
                }
                else {
                    resumeText.setFillColor(sf::Color::White); 
                }

                if (exitText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    exitText.setFillColor(sf::Color::Red); // Highlight when hovered
                }
                else {
                    exitText.setFillColor(sf::Color::White); 
                }

                // Draw Pause UI
                window.draw(pauseOverlay); 
                window.draw(pauseMenu);    
                window.draw(resumeText);   
                window.draw(exitText);    
            }


            window.display();
        }


    }
    int loadHighScore() {
        std::ifstream file("highscore.txt");
        int score = 0;
        if (file.is_open()) {
            file >> score;
            file.close();
        }
        return score;
    }

    void saveHighScore() {
        std::ofstream file("highscore.txt");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}

