#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

constexpr int WINDOW_WIDTH = 1200;
constexpr int WINDOW_HEIGHT = 900;
constexpr float PLAYER_SPEED = 0.125f;
constexpr float BULLET_SPEED = 0.5f;
constexpr float ZOMBIE_SPEED = 0.005f;
constexpr int MAX_ZOMBIES = 5;
constexpr float MIN_SPAWN_DISTANCE = 200.0f;
constexpr float ZOMBIE_FIRE_MIN_INTERVAL = 1.0f;
constexpr float ZOMBIE_FIRE_MAX_INTERVAL = 3.0f;
constexpr int ZOMBIE_HEALTH = 3;
constexpr int PLAYER_MAX_HEALTH = 20;
enum class GameState { MENU, PLAYING, GAME_OVER };

#endif