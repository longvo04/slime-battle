#pragma once
#include "SDL.h"
#include "Animation.h"
#include "Vector2D.h"
#include <map>
#include <string>


class Player {
public:
    Vector2D position;
    Vector2D velocity;
    int width;
    int height;
    int scale = 1;
    int speed;
    int score = 0;
    bool isPlayer1 = true;
    std::string lastDirection = "Down";

    std::map<std::string, SDL_Texture*> textures;
    SDL_Texture *texture;
	SDL_Rect srcRect, destRect;
	int frames = 0;
	int animationSpeed = 100;
    int animationIndex = 0;
	std::map<std::string, Animation> animations;


    void initAnimation();
    void addTexture(std::string textureName, std::string path);
    void setTexture(std::string textureName);
    void addAnimation(std::string animationName, int index, int frames, int speed);
    void playAnimation(std::string AnimationType);
    void playIdleAnimation();
    void playMoveAnimation();
    void playWalkAnimation();
    void play(std::string animName);
    void handlePlayer1Input();
    void handlePlayer2Input();
    void handleInput();

    Player();
    Player(int x, int y, int w, int h, int s, int sc, bool isP1 = true);
    ~Player();
    void update();
    void keepInBounds();
    void draw();
};