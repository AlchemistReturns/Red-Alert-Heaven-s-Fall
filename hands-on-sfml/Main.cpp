#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Constants.hpp"
#include "Entity.hpp"
#include "Obstacle.hpp"
#include "Player.hpp"
#include "PowerUp.hpp"
#include "Bullet.hpp"
#include "ZombieBullet.hpp"
#include "Zombie.hpp"
#include "Menu.hpp"
#include "GameOverScreen.hpp"
#include "Game.hpp"
#include "Helper.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream> 

int main() {
    Game game;
    game.run();
    return 0;
}

