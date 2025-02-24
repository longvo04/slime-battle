#include "Projectile.h"
#include "SDL_image.h"
#include <iostream>
#include "Game.h"
#include "Constants.h"

Projectile::Projectile(int x, int y, int w, int h, int s, int ricLimit, bool isPlayer1)
    : position(x, y), width(w), height(h), speed(s) {
    this->isPlayer1 = isPlayer1;
    velocity = Vector2D(1, 1); // Initial velocity
    destRect = {x, y, width, height};
    texture = nullptr;
    ricochetCount = 0;
    this->ricochetLimit = ricLimit;
}

Projectile::~Projectile() {
}

void Projectile::loadTexture(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load projectile texture: " << IMG_GetError() << std::endl;
        return;
    }
    texture = SDL_CreateTextureFromSurface(Game::renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
    }
}

void Projectile::update() {
    position.x += velocity.x * speed;
    position.y += velocity.y * speed;
    destRect.x = static_cast<int>(position.x);
    destRect.y = static_cast<int>(position.y);
    destRect.w = width;
    destRect.h = height;
    bounceOffWalls(SCREEN_WIDTH, SCREEN_HEIGHT);
}

void Projectile::draw() {
    SDL_RenderCopy(Game::renderer, texture, nullptr, &destRect);
}

void Projectile::bounceOffWalls(int screenWidth, int screenHeight) {
    if (position.x <= 0 || position.x + width >= screenWidth) {
        velocity.x *= -1;
        ricochetCount = (ricochetCount+1) % 20;
    }
    if (position.y <= 0 || position.y + height >= screenHeight) {
        velocity.y *= -1;
        ricochetCount = (ricochetCount+1) % 20;
    }
}