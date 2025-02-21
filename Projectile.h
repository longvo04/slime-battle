#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "Vector2D.h"
#include <string>

class Projectile {
public:
    Vector2D position;
    Vector2D velocity;
    bool player1;
    int width;
    int height;
    int speed;
    SDL_Texture* texture;
    SDL_Rect destRect;

    Projectile(int x, int y, int w, int h, int s, bool player1);
    ~Projectile();

    void update();
    void draw();
    void bounceOffWalls(int screenWidth, int screenHeight);
    void loadTexture(const std::string& path);
};