#pragma once
#include "SDL.h"
#include "Animation.h"
#include "Vector2D.h"
#include <vector>
#include "Projectile.h"
#include <map>
#include <string>


class Player {
public:
    enum PlayerState {
        IDLE,
        WALK,
        ATTACK,
        HURT,
        DEAD
    };
    Vector2D position;
    Vector2D velocity;
    Vector2D projectileVelocity;

    bool isChild = false;
    Player* childA = nullptr;
    Player* childB = nullptr;
    bool isActiveChild = true;
    bool hasChildren = false;
    float childScale = 0.5; // Children are smaller than parent
    bool isActive = true;

    // AI behavior variables
    bool isAIControlled = false;
    Uint32 aiMoveTimer = 0;
    Uint32 aiShootTimer = 0;
    Vector2D aiMoveDirection;
    
    // AI behavior methods
    void updateAI();
    void aiChooseNewDirection();
    void aiTryShoot();
    
    bool allChildrenDead();
    void createChildren();

    int damage;
    int width;
    int height;
    int scale = 1;
    int speed;
    int health;
    int maxHealth;
    int score = 0;
    PlayerState state = IDLE;
    bool isPlayer1 = true;
    std::string lastDirection = "Down";
    Uint32 startTime;
    Uint32 lastAttackTime = 0;  // Track when the last attack happened
    Uint32 ATTACK_COOLDOWN = 2000;  // 2000ms = 2 seconds

    std::map<std::string, SDL_Texture*> textures;
    SDL_Texture *texture;
    std::string projectileTexturePath;
	SDL_Rect srcRect, destRect;
	int frames = 0;
	int animationSpeed = 100;
    int animationIndex = 0;
	std::map<std::string, Animation> animations;
    int projectileSize;
    int projectileSpeed;
    int projectileRicochetLimit;

    void initProjectile(int bulletSize, int bulletSpeed, std::string path, std::vector<Projectile*> &projectiles, int ricochetLimit = 100);
    void initAnimation();
    void addTexture(std::string textureName, std::string path);
    void setTexture(std::string textureName);
    void addAnimation(std::string animationName, int index, int frames, int speed);
    void playAnimation(std::string AnimationType);
    void playIdleAnimation();
    void playMoveAnimation();
    void playWalkAnimation();
    void playAttackAnimation();
    void playHurtAnimation();
    void play(std::string animName);
    void handlePlayer1Input();
    void handlePlayer2Input();
    void handleInput();
    void resetAnimation();

    void getHit(int damage);
    void iddle();
    void drawCooldownCircle(float x, float y, float radius, float progress);

    std::vector<Projectile*>* projectiles;

    void createProjectile();
    void attack(Uint32 currentTime);

    bool collision(SDL_Rect target);
    bool isOnLastFrame();

    Player();
    Player(int x, int y, int w, int h, int s, int sc, Uint32 ATTACK_COOLDOWN, int damage, int maxHealth, bool isP1 = true);
    ~Player();
    void update();
    void keepInBounds();
    void draw();
};