// Zombie Shooter - Object-Oriented Refactor
// SFML + C++

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

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

void centerText(sf::Text& text, float windowWidth, float windowHeight, float yOffset = 0) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    text.setPosition(windowWidth / 2.0f, (windowHeight / 2.0f) + yOffset);
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
        sprite.setScale(1.0f, 1.0f);  // Adjust size as needed
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

        // Rotate by 5 degrees when left/right arrow keys are pressed
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) sprite.rotate(-0.1f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) sprite.rotate(0.1f);

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
        float maxX = 2000 - newBounds.width;  // Restrict to 2000x2000
        float maxY = 2000 - newBounds.height;

        if (!collision) {
            // Clamp position within bounds
            newPosition.x = std::max(minX, std::min(maxX, newPosition.x));
            newPosition.y = std::max(minY, std::min(maxY, newPosition.y));

            sprite.setPosition(newPosition);
        }
        else {
            sprite.setPosition(oldPosition);  // Revert to old position if blocked
        }
    }

    void rotateTowardsMouse(sf::RenderWindow& window) {
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        sf::Vector2f playerPosition = sprite.getPosition();

        // Calculate direction from player to mouse
        sf::Vector2f direction = sf::Vector2f(mousePosition.x, mousePosition.y) - playerPosition;
        float targetAngle = std::atan2(direction.y, direction.x) * 180 / 3.14159265f;

        // Get the current angle (adjusted since SFML sprites default face upwards)
        float currentAngle = sprite.getRotation() - 90;

        // Ensure the shortest rotation direction
        float angleDifference = targetAngle - currentAngle;
        while (angleDifference > 180) angleDifference -= 360;
        while (angleDifference < -180) angleDifference += 360;

        // Use SLERP-like interpolation for smooth rotation
        float rotationSpeed = 10.0f;  // Adjust speed as needed
        float newAngle = currentAngle + angleDifference * std::min(rotationSpeed, 1.0f);

        // Set new rotation with the correct offset
        sprite.setRotation(newAngle + 90);
    }



    void updateBoosts() {
        if (speedBoost && speedBoostClock.getElapsedTime().asSeconds() > 5) speedBoost = false;
        if (damageBoost && damageBoostClock.getElapsedTime().asSeconds() > 5) damageBoost = false;
    }

    sf::Vector2f getDirection() {
        float angle = sprite.getRotation(); // Rotation in degrees
        float rad = angle * 3.14159265f / 180; // Convert to radians
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
        sprite.move(direction * BULLET_SPEED);
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

        // Zombie shooting logic remains unchanged
        if (fireClock.getElapsedTime().asSeconds() > randomFireInterval) {
            sf::Vector2f bulletDirection = playerPosition - sprite.getPosition();
            float bulletLength = std::hypot(bulletDirection.x, bulletDirection.y);
            if (bulletLength != 0) bulletDirection /= bulletLength; // Normalize direction
            zombieBullets.emplace_back(zombieBulletTexture, sprite.getPosition(), bulletDirection);
            fireClock.restart();
            randomFireInterval = ZOMBIE_FIRE_MIN_INTERVAL + static_cast<float>(rand() % int((ZOMBIE_FIRE_MAX_INTERVAL - ZOMBIE_FIRE_MIN_INTERVAL) * 1000)) / 1000.0f;
        }
    }

};

enum class GameState { MENU, PLAYING };

class Menu {
private:
    sf::Font font;
    sf::Text startText;
    sf::Text soundText;
    sf::Text title;
    bool soundOn;
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;
public:
    Menu() : soundOn(true) {
        font.loadFromFile("arial.ttf");
        backgroundTexture.loadFromFile("menu_background.png");
        backgroundSprite.setTexture(backgroundTexture);

        title.setFont(font);
        title.setString("Red Alert");
        title.setCharacterSize(60);
        title.setFillColor(sf::Color::Red);
        centerText(title, WINDOW_WIDTH, WINDOW_HEIGHT, -150);
        
        startText.setFont(font);
        startText.setString("Start Game");
        startText.setCharacterSize(30);
        startText.setFillColor(sf::Color::White);
        centerText(startText, WINDOW_WIDTH, WINDOW_HEIGHT, -50);

        soundText.setFont(font);
        soundText.setString("Sound: On");
        soundText.setCharacterSize(30);
        soundText.setFillColor(sf::Color::White);
        centerText(soundText, WINDOW_WIDTH, WINDOW_HEIGHT, +20);
    }

    void handleInput(sf::RenderWindow& window, GameState& gameState, sf::Music& backgroundMusic) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);

            // Start the game when "Start Game" is clicked
            if (startText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                gameState = GameState::PLAYING;
            }

            // Toggle sound when "Sound: On/Off" is clicked
            if (soundText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                soundOn = !soundOn;
                soundText.setString(soundOn ? "Sound: On" : "Sound: Off");

                // Toggle music playback
                if (soundOn) {
                    backgroundMusic.play();   // Resume playing
                }
                else {
                    backgroundMusic.pause();  // Pause the music
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
    sf::View miniMapView;  // Separate camera for mini-map
    //sf::RectangleShape miniMapBackground;  // Background of the mini-map
    GameState gameState;
    Menu menu;
    bool isPaused = false; // Track if the game is paused
    sf::Text pauseText; // Pause message text
    // Pause UI elements
    sf::RectangleShape pauseOverlay;  // Semi-transparent background when paused
    sf::RectangleShape pauseMenu;     // Pause menu box
    sf::Text resumeText;              // Resume button text
    sf::Text exitText;                // Exit button text



public:
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Zombie Shooter"), gameState(GameState::MENU) {
        // Initialize the camera view
        cameraView.setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        cameraView.setCenter(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

        playerTexture.loadFromFile("player.png");
        bulletTexture.loadFromFile("bullet.png");
        zombieTexture.loadFromFile("zombie.png");
        zombieBulletTexture.loadFromFile("zombie_bullet.png"); // Load zombie bullet texture here

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
        if (!pillarTexture.loadFromFile("pillar.png")) {
            std::cerr << "Error loading obstacle texture!\n";
        }

        backgroundSprite.setTexture(backgroundTexture);
        backgroundSprite.setScale(
			float(2000) / backgroundTexture.getSize().x,
            float(2000) / backgroundTexture.getSize().y
        );

        // Initialize Pause Text
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
        
        // Initialize Pause Overlay (semi-transparent dark background)
        pauseOverlay.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150)); // Dark semi-transparent overlay

        // Initialize Pause Menu Box
        pauseMenu.setSize(sf::Vector2f(300, 200));
        pauseMenu.setFillColor(sf::Color(50, 50, 50, 220)); // Darker menu box
        pauseMenu.setOutlineColor(sf::Color::White);
        pauseMenu.setOutlineThickness(3);
        pauseMenu.setPosition(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 100);

        // Initialize Resume Button
        resumeText.setFont(font);
        resumeText.setString("Resume");
        resumeText.setCharacterSize(28);
        resumeText.setFillColor(sf::Color::White);
        resumeText.setPosition(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 50);

        // Initialize Exit Button
        exitText.setFont(font);
        exitText.setString("Exit");
        exitText.setCharacterSize(28);
        exitText.setFillColor(sf::Color::White);
        exitText.setPosition(WINDOW_WIDTH / 2 - 30, WINDOW_HEIGHT / 2 + 20);

        zombieKillText.setCharacterSize(20);
        zombieKillText.setFillColor(sf::Color::White);
        zombieKillText.setPosition(10, WINDOW_HEIGHT - 60);

        //obstacles.emplace_back(pillarTexture, sf::Vector2f(500, 400));
        /*obstacles.emplace_back(blockTexture, sf::Vector2f(200, 300));
        obstacles.emplace_back(waterTexture, sf::Vector2f(500, 300));
        obstacles.emplace_back(waterTexture, sf::Vector2f(500, 250));*/
        

        // Mini-map View (fixed-size)
        miniMapView.setSize(2000, 2000);  // Show full game world
        miniMapView.setViewport(sf::FloatRect(0.75f, 0.75f, 0.2f, 0.2f));  // Viewport adjusted (x, y, width, height)

        // Mini-map Background (Fixed UI Element)
        //miniMapBackground.setSize(sf::Vector2f(160, 160));  // Match the mini-map's scaled size
        //miniMapBackground.setFillColor(sf::Color(0, 0, 0, 150));  // Semi-transparent black
        //miniMapBackground.setPosition(WINDOW_WIDTH * 0.75f - 10, WINDOW_HEIGHT * 0.75f - 10);  // Align properly

        // Load and play background music
        if (!backgroundMusic.openFromFile("World War Z Theme Song.ogg")) {
            std::cerr << "Error loading background music!" << std::endl;
        }
        else {
            std::cout << "Music loaded successfully!" << std::endl;
            backgroundMusic.setLoop(true);
            backgroundMusic.setVolume(100);
            backgroundMusic.play();
            std::cout << "Music is playing..." << std::endl;
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
            sf::Texture* chosenTexture = nullptr;  // Explicitly initialize to nullptr

            switch (randomType) {
            case 0: chosenTexture = &powerUpHealthTexture; break;
            case 1: chosenTexture = &powerUpSpeedTexture; break;
            case 2: chosenTexture = &powerUpDamageTexture; break;
            default: chosenTexture = &powerUpHealthTexture; break;  // Fallback in case of error
            }

            // Ensure chosenTexture is not null before using it
            if (chosenTexture) {
                powerUps.emplace_back(*chosenTexture, spawnPosition, static_cast<PowerUp::Type>(randomType));
                powerUpSpawnClock.restart();
            }
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

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::P) {
                    isPaused = !isPaused; // Toggle pause state
                }
            }

            if (isPaused) {
                // Handle mouse clicks on the pause menu buttons
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePos);

                    if (resumeText.getGlobalBounds().contains(worldMousePos)) {
                        isPaused = false; // Resume game
                    }
                    else if (exitText.getGlobalBounds().contains(worldMousePos)) {
                        window.close(); // Exit game
                    }
                }
            }
            else { // Game is not paused, allow other events
                // Check if Spacebar is pressed to fire bullets
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    bullets.emplace_back(bulletTexture, player->sprite.getPosition(), player->getDirection());
                }
            }
        }

        if (gameState == GameState::MENU) {
            menu.handleInput(window, gameState, backgroundMusic); // Pass backgroundMusic
        }

    }



    void checkCollisions() {
        for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
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
                    window.close(); // Player dies, close game
                }
            }
            else {
                ++zombieBulletIt;
            }
        }
    }


    void update() {
        if (!isPaused) { // Only update if the game is not paused
            player->move(obstacles); // Move the player while checking collisions
            player->updateBoosts();
            spawnPowerUp();
            checkPowerUpCollisions();
            //player->rotateTowardsMouse(window); // Ensure player faces the mouse

            // Keep Mini-map Centered on Player
            miniMapView.setCenter(player->sprite.getPosition());

            // Move the camera to follow the player
            sf::Vector2f playerPos = player->sprite.getPosition();
            float halfWidth = WINDOW_WIDTH / 2;
            float halfHeight = WINDOW_HEIGHT / 2;

            float minX = halfWidth, minY = halfHeight;
            float maxX = 2000 - halfWidth;
            float maxY = 2000 - halfHeight;

            // Clamp camera position within background
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

        // Always update UI elements (health bar, score) even if paused
        healthBar.setSize(sf::Vector2f(10 * player->health, 20));
        zombieKillText.setString("Zombies Killed: " + std::to_string(zombiesKilled));
    }





    void render() {
        if (gameState == GameState::MENU) {
            menu.render(window);
        }
        else {
            window.clear(sf::Color::Black); // Placeholder for game render
            window.setView(cameraView); // Apply camera movement
            window.draw(backgroundSprite);
            player->render(window);

            for (auto& bullet : bullets) bullet.render(window);
            for (auto& zombieBullet : zombieBullets) zombieBullet.render(window);
            for (auto& zombie : zombies) zombie.render(window);
            for (auto& powerUp : powerUps) powerUp.render(window);
            for (auto& obstacle : obstacles) obstacle.render(window);

            // Draw the Mini-map
            window.setView(window.getDefaultView());  // Reset to fixed UI
            window.setView(miniMapView);  // Apply mini-map camera
            window.draw(backgroundSprite);  // Mini-map version of the world

            // Draw Player as a small dot
            sf::CircleShape playerDot(50);
            playerDot.setFillColor(sf::Color::Blue);
            playerDot.setPosition(player->sprite.getPosition() * 0.08f);  // Scale position to fit mini-map
            window.draw(playerDot);

            // Draw Zombies as red dots
            for (auto& zombie : zombies) {
                sf::CircleShape zombieDot(50);
                zombieDot.setFillColor(sf::Color::Red);
                zombieDot.setPosition(zombie.sprite.getPosition() * 0.08f);  // Scale position
                window.draw(zombieDot);
            }

            // Draw Obstacles as white squares
            for (auto& obstacle : obstacles) {
                sf::RectangleShape obstacleDot(sf::Vector2f(6, 6));  // Smaller size
                obstacleDot.setFillColor(sf::Color::White);
                obstacleDot.setPosition(obstacle.sprite.getPosition() * 0.08f);  // Scale position
                window.draw(obstacleDot);
            }

            window.setView(window.getDefaultView());
            window.draw(healthBar);
            window.draw(zombieKillText);

            // **Render Pause Message if Game is Paused**
            if (isPaused) {
                // Get the current mouse position
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                // Change color when hovering over "Resume"
                if (resumeText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    resumeText.setFillColor(sf::Color::Yellow); // Highlight when hovered
                }
                else {
                    resumeText.setFillColor(sf::Color::White);  // Default color
                }

                // Change color when hovering over "Exit"
                if (exitText.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    exitText.setFillColor(sf::Color::Red); // Highlight when hovered
                }
                else {
                    exitText.setFillColor(sf::Color::White); // Default color
                }

                // Draw Pause UI
                window.draw(pauseOverlay); // Draw darkened overlay
                window.draw(pauseMenu);    // Draw the menu box
                window.draw(resumeText);   // Draw "Resume" button
                window.draw(exitText);     // Draw "Exit" button
            }


            window.display();
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}

