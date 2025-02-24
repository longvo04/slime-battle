#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "Vector2D.h"
#include <string>

class Projectile {
public:
    Vector2D position;
    Vector2D velocity;
    bool isPlayer1;
    int width;
    int height;
    int speed;
    int ricochetCount = 0;
    int ricochetLimit = 20;
    SDL_Texture* texture;
    SDL_Rect destRect;

    Projectile operator=(const Projectile& p) {
        position = p.position;
        velocity = p.velocity;
        isPlayer1 = p.isPlayer1;
        width = p.width;
        height = p.height;
        speed = p.speed;
        ricochetCount = p.ricochetCount;
        ricochetLimit = p.ricochetLimit;
        texture = p.texture;
        destRect = p.destRect;
        return *this;
    }
    Projectile(int x, int y, int w, int h, int s, int ricLimit, bool isPlayer1);
    ~Projectile();

    void update();
    void draw();
    void bounceOffWalls(int screenWidth, int screenHeight);
    void loadTexture(const std::string& path);
};