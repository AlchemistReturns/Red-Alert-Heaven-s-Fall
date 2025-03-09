#include "Game.hpp"

Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Red Alert"), gameState(GameState::MENU), menu(loadHighScore()) {
    cameraView.setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    cameraView.setCenter(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

    playerTexture.loadFromFile("assets/player.png");
    bulletTexture.loadFromFile("assets/bullet.png");
    zombieTexture.loadFromFile("assets/zombie.png");
    zombieBulletTexture.loadFromFile("assets/zombie_bullet.png");

    player = new Player(playerTexture);
    player->health = PLAYER_MAX_HEALTH;

    powerUpHealthTexture.loadFromFile("assets/powerup_health.png");
    powerUpSpeedTexture.loadFromFile("assets/powerup_speed.png");
    powerUpDamageTexture.loadFromFile("assets/powerup_damage.png");

    if (!backgroundTexture.loadFromFile("assets/background.jpg")) {
        std::cerr << "Error loading background image!\n";
    }

    if (!blockTexture.loadFromFile("assets/block.png")) {
        std::cerr << "Error loading obstacle texture!\n";
    }
    if (!waterTexture.loadFromFile("assets/water.jpg")) {
        std::cerr << "Error loading obstacle texture!\n";
    }
    if (!vaseTexture.loadFromFile("assets/vase.png")) {
        std::cerr << "Error loading obstacle texture!\n";
    }
    if (!pillarTexture.loadFromFile("assets/cloud.png")) {
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

    if (!backgroundMusic.openFromFile("assets/World War Z Theme Song.ogg")) {
        std::cerr << "Error loading background music!" << std::endl;
    }
    else {
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(100);
        backgroundMusic.play();
    }


}

Game::~Game() {
    delete player;
}

void Game::run() {
    while (window.isOpen()) {
        handleEvents();
        update();
        render();
    }
}

void Game::spawnPowerUp() {
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

void Game::checkHighScore() {
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



void Game::checkPowerUpCollisions() {
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

void Game::restartGame() {
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



void Game::handleEvents() {
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



void Game::checkCollisions() {
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


void Game::update() {
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





void Game::render() {
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
int Game::loadHighScore() {
    std::ifstream file("highscore.txt");
    int score = 0;
    if (file.is_open()) {
        file >> score;
        file.close();
    }
    return score;
}

void Game::saveHighScore() {
    std::ofstream file("highscore.txt");
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
};