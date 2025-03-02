#include "Player.h"
#include "Game.h"
#include "SDL_image.h"
#include "SDL.h"
#include "Projectile.h"
#include "Constants.h"
#include <string>
#include <math.h>
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()

// Add to top of Game.cpp
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()

Player::Player()
{
    position = Vector2D();
    velocity = Vector2D();
    width = 32;
    height = 32;
    speed = 1;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = 32;
    srcRect.h = 32;
    destRect.x = 0;
    destRect.y = 0;
    destRect.w = srcRect.w * scale;
    destRect.h = srcRect.h * scale;
}

Player::Player(int x, int y, int w, int h, int s, int sc, Uint32 attack_cooldown, int d, int health, bool isP1)
{
    position = Vector2D(x, y);
    velocity = Vector2D();
    width = w;
    height = h;
    speed = s;
    scale = sc;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = width;
    srcRect.h = height;
    isPlayer1 = isP1;
    lastAttackTime = 0;
    ATTACK_COOLDOWN = attack_cooldown;
    damage = d;
    projectiles = nullptr;
    this->health = health;
    maxHealth = health;
}

Player::~Player()
{
}

void Player::updateAI() {
    if (!isActive || isActiveChild || !isChild) return;
    
    Uint32 currentTime = SDL_GetTicks();
    
    // Check if it's time to change direction
    if (currentTime > aiMoveTimer) {
        aiChooseNewDirection();
        aiMoveTimer = currentTime + (1000 + rand() % 2000); // 1-3 seconds
    }
    
    // Check if it's time to try shooting
    if (currentTime > aiShootTimer) {
        aiTryShoot();
        aiShootTimer = currentTime + (1000 + rand() % 2000); // 1-3 seconds
    }
    
    velocity = aiMoveDirection; // Move in the chosen direction
}

void Player::aiChooseNewDirection() {
    // 20% chance to stop
    if (rand() % 5 == 0) {
        aiMoveDirection = Vector2D(0, 0);
        return;
    }
    
    // Choose a random direction
    int dir = rand() % 8;
    switch (dir) {
        case 0: // Up
            aiMoveDirection = Vector2D(0, -speed);
            lastDirection = "Up";
            break;
        case 1: // Down
            aiMoveDirection = Vector2D(0, speed);
            lastDirection = "Down";
            break;
        case 2: // Left
            aiMoveDirection = Vector2D(-speed, 0);
            lastDirection = "Left";
            break;
        case 3: // Right
            aiMoveDirection = Vector2D(speed, 0);
            lastDirection = "Right";
            break;
        case 4: // Up-Left
            aiMoveDirection = Vector2D(-speed/1.414, -speed/1.414);
            lastDirection = "Left";
            break;
        case 5: // Up-Right
            aiMoveDirection = Vector2D(speed/1.414, -speed/1.414);
            lastDirection = "Right";
            break;
        case 6: // Down-Left
            aiMoveDirection = Vector2D(-speed/1.414, speed/1.414);
            lastDirection = "Left";
            break;
        case 7: // Down-Right
            aiMoveDirection = Vector2D(speed/1.414, speed/1.414);
            lastDirection = "Right";
            break;
    }
}

void Player::aiTryShoot() {
    // Only shoot if cooldown is ready
    if (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN) {
        // Set projectile direction
        if (lastDirection == "Up") {
            projectileVelocity = Vector2D(0, -projectileSpeed);
        } else if (lastDirection == "Down") {
            projectileVelocity = Vector2D(0, projectileSpeed);
        } else if (lastDirection == "Left") {
            projectileVelocity = Vector2D(-projectileSpeed, 0);
        } else if (lastDirection == "Right") {
            projectileVelocity = Vector2D(projectileSpeed, 0);
        }
        
        // Attack
        attack(SDL_GetTicks());
        Mix_PlayChannel(-1, Game::attackSound, 0);
    }
}

void Player::createChildren() {
    if (hasChildren || isChild) return;
    // First child - positioned slightly to the left of parent
    childA = new Player(position.x - width/2, position.y+height/2, width, height, speed*1.2, 2, ATTACK_COOLDOWN*0.8, damage/2, maxHealth/2, isPlayer1);
    childA->addTexture("Idle", isPlayer1 ? "assets/img/slime1_idle.png" : "assets/img/slime2_idle.png");
    childA->addTexture("Walk", isPlayer1 ? "assets/img/slime1_walk.png" : "assets/img/slime2_walk.png");
    childA->addTexture("Attack", isPlayer1 ? "assets/img/slime1_attack.png" : "assets/img/slime2_attack.png");
    childA->addTexture("Hurt", isPlayer1 ? "assets/img/slime1_hurt.png" : "assets/img/slime2_hurt.png");
    childA->initAnimation();
    childA->initProjectile(projectileSize*childScale, projectileSpeed*1.8, projectileTexturePath, *projectiles, 7);
    childA->isChild = true;
    childA->isActive = true;
    childA->isActiveChild = true; // Initially the first child is active

    // Initialize AI for child A
    childA->aiMoveTimer = SDL_GetTicks() + (1000 + rand() % 2000);
    childA->aiShootTimer = SDL_GetTicks() + (1000 + rand() % 2000);
    childA->aiChooseNewDirection();
    
    // Second child - positioned slightly to the right of parent
    childB = new Player(position.x + width/2, position.y+height/2, width, height, speed*1.2, 2, ATTACK_COOLDOWN*0.8, damage/2, maxHealth/2, isPlayer1);
    childB->addTexture("Idle", isPlayer1 ? "assets/img/slime1_idle.png" : "assets/img/slime2_idle.png");
    childB->addTexture("Walk", isPlayer1 ? "assets/img/slime1_walk.png" : "assets/img/slime2_walk.png");
    childB->addTexture("Attack", isPlayer1 ? "assets/img/slime1_attack.png" : "assets/img/slime2_attack.png");
    childB->addTexture("Hurt", isPlayer1 ? "assets/img/slime1_hurt.png" : "assets/img/slime2_hurt.png");
    childB->initAnimation();
    childB->initProjectile(projectileSize*childScale, projectileSpeed*1.8, projectileTexturePath, *projectiles, 7);
    childB->isAIControlled = true;
    childB->isChild = true;
    childB->isActive = true;
    childB->isActiveChild = false; // Initially the first child is active

    // Initialize AI for child B
    childB->aiMoveTimer = SDL_GetTicks() + (1000 + rand() % 2000);
    childB->aiShootTimer = SDL_GetTicks() + (1000 + rand() % 2000);
    childB->aiChooseNewDirection();

    hasChildren = true;
    isActive = false;

    // Seed random number generator for AI behavior
    srand(static_cast<unsigned int>(SDL_GetTicks()));
}

bool Player::allChildrenDead() {
    if (!hasChildren) return false;
    return (childA->health <= 0) && (childB->health <= 0);
}

void Player::initProjectile(int bulletSize, int bulletSpeed, std::string path, std::vector<Projectile*> &projectiles, int ricochetLimit) {
    projectileTexturePath = path;
    projectileSize = bulletSize;
    projectileSpeed = bulletSpeed;
    projectileRicochetLimit = ricochetLimit;
    this->projectiles = &projectiles;
}

void Player::createProjectile(){
    Projectile* p = new Projectile(position.x+width/2*scale, position.y+height/2*scale, projectileSize, projectileSize, projectileSpeed, projectileRicochetLimit, isPlayer1);
    p->velocity = projectileVelocity;
    p->loadTexture(projectileTexturePath);
    (*projectiles).push_back(p);
    std::cout << "Created projectile at position: " << p->position << std::endl; // Debug statement
    std::cout << "Projectile velocity: " << p->velocity << std::endl; // Debug statement
    std::cout << "Projectile speed: " << p->speed << std::endl; // Debug statement
    std::cout << "Projectile size: " << p->width << "x" << p->height << std::endl; // Debug statement
    std::cout << "Projectile ricochet limit: " << p->ricochetLimit << std::endl; // Debug statement
    
}

void Player::initAnimation() {
    addAnimation("IdleFaceDown"     , 0, 6, 100);
    addAnimation("IdleFaceUp"       , 1, 6, 100);
    addAnimation("IdleFaceLeft"     , 2, 6, 100);
    addAnimation("IdleFaceRight"    , 3, 6, 100);
    addAnimation("WalkDown"         , 0, 8, 100);
    addAnimation("WalkUp"           , 1, 8, 100);
    addAnimation("WalkLeft"         , 2, 8, 100);
    addAnimation("WalkRight"        , 3, 8, 100);
    addAnimation("Attack"           , 0,10, 100);
    addAnimation("HurtFaceDown"     , 0, 5, 100);
    addAnimation("HurtFaceUp"       , 1, 5, 100);
    addAnimation("HurtFaceLeft"     , 2, 5, 100);
    addAnimation("HurtFaceRight"    , 3, 5, 100);
    playAnimation("Idle");
}

void Player::update()
{
    if (hasChildren) {
        childA->update();
        childB->update();
        return;
    }
    if (!isActive) return;
    if (isChild && isAIControlled) {
        updateAI();
    }
    if (state == ATTACK || state == HURT) {
        velocity = Vector2D();
        if (isOnLastFrame()) {
            if (state == ATTACK) {
                std::cout << "On last frame of attack animation, creating projectile." << std::endl; // Debug statement
                createProjectile();
            }
            resetAnimation();
            playAnimation("Idle");
            state = IDLE;
        }
    }
    position.x += velocity.x;
    position.y += velocity.y;
    keepInBounds();
    
    if (animationSpeed > 0 && frames > 0) {
        Uint32 elapsedTime = SDL_GetTicks() - startTime;
        srcRect.x = srcRect.w * static_cast<int>((elapsedTime / animationSpeed) % frames);
    }

    srcRect.y = animationIndex * height;

    destRect.x = static_cast<int>(position.x);
    destRect.y = static_cast<int>(position.y);
    destRect.w = width * scale;
    destRect.h = height * scale;
}

void Player::keepInBounds()
{
    if (position.x + 0.25*width*scale < 0) {
        position.x = -0.25*width*scale;
    }
    if (position.y + 0.25*width*scale < 0) {
        position.y = -0.25*width*scale;
    }
    if (position.x + width*0.75 * scale > SCREEN_WIDTH) {
        position.x = SCREEN_WIDTH - width*0.75 * scale;
    }
    if (position.y + height*0.75 * scale > SCREEN_HEIGHT) {
        position.y = SCREEN_HEIGHT - height*0.75 * scale;
    }
}

void Player::drawCooldownCircle(float x, float y, float radius, float progress) {
    const int segments = 30;  // Number of segments to draw
    float maxAngle = progress * 2 * M_PI;  // Convert progress (0-1) to radians
    
    // Draw background circle
    SDL_SetRenderDrawColor(Game::renderer, 100, 100, 100, 100);  // Gray, semi-transparent
    for (int i = 0; i < segments; i++) {
        float startAngle = (float)i / segments * 2 * M_PI;
        float endAngle = (float)(i + 1) / segments * 2 * M_PI;
        
        SDL_RenderDrawLine(Game::renderer,
            x + cos(startAngle) * radius,
            y + sin(startAngle) * radius,
            x + cos(endAngle) * radius,
            y + sin(endAngle) * radius);
    }
    
    // Draw remaining cooldown in brighter color
    SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);  // White
    for (int i = 0; i < segments; i++) {
        float startAngle = (float)i / segments * 2 * M_PI;
        if (startAngle > maxAngle) break;  // Stop at progress point
        
        float endAngle = (float)(i + 1) / segments * 2 * M_PI;
        if (endAngle > maxAngle) endAngle = maxAngle;
        
        SDL_RenderDrawLine(Game::renderer,
            x + cos(startAngle) * radius,
            y + sin(startAngle) * radius,
            x + cos(endAngle) * radius,
            y + sin(endAngle) * radius);
    }
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void Player::draw()
{
    if (hasChildren) {
        childA->draw();
        childB->draw();
        return;
    }
    if (!isActive) return;
    // Draw player
    SDL_RenderCopy(Game::renderer, texture, &srcRect, &destRect);
    // Draw cooldown indicators
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastAttackTime < ATTACK_COOLDOWN) {
        float progress = 1.0f - (float)(currentTime - lastAttackTime) / ATTACK_COOLDOWN;
        float centerX = destRect.x + destRect.w / 2;
        float centerY = destRect.y + destRect.h / 2;
        drawCooldownCircle(centerX, centerY, destRect.w * 0.4f, progress);
    }
    if (!isChild) {
        int barWidth = 80;
        int barHeight = 10;
        SDL_Rect healthBarBg = {destRect.x + (destRect.w - barWidth) / 2, destRect.y + 40, barWidth, barHeight};
        SDL_Rect healthBar = {destRect.x + (destRect.w - barWidth) / 2, destRect.y + 40, (int)(barWidth * (health / (float)maxHealth)), barHeight};
        
        SDL_SetRenderDrawColor(Game::renderer, 100, 100, 100, 255); // Gray background
        SDL_RenderFillRect(Game::renderer, &healthBarBg);

        if (isPlayer1) {
            SDL_SetRenderDrawColor(Game::renderer, 50, 168, 102, 255); // Green health
        } else {
            SDL_SetRenderDrawColor(Game::renderer, 92, 30, 179, 255); // Purple health
        }
        SDL_RenderFillRect(Game::renderer, &healthBar);
    } else {
        // Child health bars - smaller and positioned above the character
        int barWidth = 40;
        int barHeight = 5;
        SDL_Rect healthBarBg = {destRect.x + (destRect.w - barWidth) / 2, destRect.y + 40, barWidth, barHeight};
        SDL_Rect healthBar = {destRect.x + (destRect.w - barWidth) / 2, destRect.y + 40, (int)(barWidth * (health / (float)maxHealth)), barHeight};
        
        SDL_SetRenderDrawColor(Game::renderer, 100, 100, 100, 255); // Gray background
        SDL_RenderFillRect(Game::renderer, &healthBarBg);
        
        // Use same color as parent player
        if (isPlayer1) {
            SDL_SetRenderDrawColor(Game::renderer, 50, 168, 102, 255); // Green health
        } else {
            SDL_SetRenderDrawColor(Game::renderer, 92, 30, 179, 255); // Purple health
        }
        SDL_RenderFillRect(Game::renderer, &healthBar);
        
        // Indicate which child is active with a small marker
        if (isActiveChild) {
            SDL_Rect activeMarker = {destRect.x + destRect.w / 2 - 3, destRect.y + 35, 6, 3};
            SDL_RenderFillRect(Game::renderer, &activeMarker);
        }
    }
}

void Player::addTexture(std::string textName, std::string path)
{
    SDL_Surface* tempSurface = IMG_Load(path.c_str());
    SDL_Texture* tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    textures.emplace(textName, tex);
}

void Player::setTexture(std::string textName)
{
    if (textures.find(textName) == textures.end())
    {
        std::cout << "Texture not found" << std::endl;
        return;
    }
    texture = textures[textName];
}

void Player::addAnimation(std::string animationName, int index, int frames, int speed)
{
    animations.emplace(animationName, Animation(index, frames, speed));
}

void Player::resetAnimation() {
    startTime = SDL_GetTicks();
    srcRect.x = 0;
}

void Player::playAnimation(std::string animationType) {
    if (animationType == "Idle") {
        playIdleAnimation();
        return;
    }

    if (animationType == "Move") {
        playMoveAnimation();
        return;
    }

    if (animationType == "Attack") {
        playAttackAnimation();
        return;
    }

    if (animationType == "Hurt") {
        playHurtAnimation();
        return;
    }
}

void Player::playHurtAnimation() {
    setTexture("Hurt");
    state = HURT;
    resetAnimation();
    if (lastDirection == "Down") {
        play("HurtFaceDown");
        return;
    }
    if (lastDirection == "Up") {
        play("HurtFaceUp");
        return;
    }
    if (lastDirection == "Left") {
        play("HurtFaceLeft");
        return;
    }
    if (lastDirection == "Right") {
        play("HurtFaceRight");
        return;
    }
}

void Player::playAttackAnimation() {
    setTexture("Attack");
    state = ATTACK;
    resetAnimation();
    play("Attack");
}

void Player::playIdleAnimation() {
    setTexture("Idle");
    state = IDLE;
    if (lastDirection == "Down") {
        play("IdleFaceDown");
        return;
    }
    if (lastDirection == "Up") {
        play("IdleFaceUp");
        return;
    }
    if (lastDirection == "Left") {
        play("IdleFaceLeft");
        return;
    }
    if (lastDirection == "Right") {
        play("IdleFaceRight");
        return;
    }
}

void Player::playWalkAnimation() {
    setTexture("Walk");
    state = WALK;
    if (lastDirection == "Down") {
        play("WalkDown");
        return;
    }
    if (lastDirection == "Up") {
        play("WalkUp");
        return;
    }
    if (lastDirection == "Left") {
        play("WalkLeft");
        return;
    }
    if (lastDirection == "Right") {
        play("WalkRight");
        return;
    }
}

void Player::playMoveAnimation() {
    playWalkAnimation();
}

void Player::getHit(int damage) {
    Mix_PlayChannel(-1, Game::hitSound, 0);
    playAnimation("Hurt");
    health -= damage;
    if (health <= 0) {
        health = 0;
        isActive = false;
        state = DEAD;
    }
}

void Player::iddle() {
    playAnimation("Idle");
    velocity = Vector2D();
}

void Player::play(std::string animationName) {
    if (animations.find(animationName) == animations.end()) {
        std::cout << "Animation not found" << std::endl;
        return;
    }
    frames = animations[animationName].frames;
    animationSpeed = animations[animationName].speed;
    animationIndex = animations[animationName].index;
}

void Player::handlePlayer1Input() {
    if (!isActive) return;
    // Moving
    const Uint8* state = SDL_GetKeyboardState(NULL);
    bool isMoving = false;
    if (state[SDL_SCANCODE_W]) {
        velocity.y = -speed;
        lastDirection = "Up";
        isMoving = true;
    }
    if (state[SDL_SCANCODE_S]) {
        velocity.y = speed;
        lastDirection = "Down";
        isMoving = true;
    }
    if (state[SDL_SCANCODE_A]) {
        velocity.x = -speed;
        lastDirection = "Left";
        isMoving = true;
    }
    if (state[SDL_SCANCODE_D]) {
        velocity.x = speed;
        lastDirection = "Right";
        isMoving = true;
    }
    if (velocity.x != 0 && velocity.y != 0) {
        velocity.x /= 1.414;
        velocity.y /= 1.414;
    }
    if (!isMoving) {
        velocity.x = 0;
        velocity.y = 0;
        playAnimation("Idle");
    } else {
        playAnimation("Move");
    }

    // Attacking
    static bool wasSpacePressed = false;    // Track previous state of Space key
    bool isSpacePressed = state[SDL_SCANCODE_SPACE];
    if (isSpacePressed && !wasSpacePressed) {  // Only shoot if key was just pressed
        if (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN) {
            attack(SDL_GetTicks());
            Mix_PlayChannel(-1, Game::attackSound, 0);
        }
    }
    wasSpacePressed = isSpacePressed;  // Update previous state
}

void Player::handlePlayer2Input() {
    if (!isActive) return;
    // Moving
    const Uint8* state = SDL_GetKeyboardState(NULL);
    bool isMoving = false;
    if (state[SDL_SCANCODE_UP]) {
        velocity.y = -speed;
        lastDirection = "Up";
        isMoving = true;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        velocity.y = speed;
        lastDirection = "Down";
        isMoving = true;
    }
    if (state[SDL_SCANCODE_LEFT]) {
        velocity.x = -speed;
        lastDirection = "Left";
        isMoving = true;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        velocity.x = speed;
        lastDirection = "Right";
        isMoving = true;
    }
    if (velocity.x != 0 && velocity.y != 0) {
        velocity.x /= 1.414;
        velocity.y /= 1.414;
    }
    if (!isMoving) {
        velocity.x = 0;
        velocity.y = 0;
        playAnimation("Idle");
    } else {
        playAnimation("Move");
    }

    // Attacking
    static bool wasReturnPressed = false;    // Track previous state of Return key
    bool isReturnPressed = state[SDL_SCANCODE_RETURN];
    if (isReturnPressed && !wasReturnPressed) {  // Only shoot if key was just pressed
        if (SDL_GetTicks() - lastAttackTime >= ATTACK_COOLDOWN) {
            attack(SDL_GetTicks());
            Mix_PlayChannel(-1, Game::attackSound, 0);
        }
    }
    wasReturnPressed = isReturnPressed;  // Update previous state
}

bool Player::collision(SDL_Rect target) {
    if (
        destRect.x + destRect.w*0.75 >= target.x 
        && destRect.y + destRect.h*0.75 >= target.y
        && target.x + target.w >= destRect.x + destRect.w*0.25
        && target.y + target.h >= destRect.y + destRect.h*0.25
    ) {
        return true;
    }
    return false;
}

bool Player::isOnLastFrame() {
    if (animationSpeed <= 0 || frames <= 0) return false;
    
    Uint32 elapsedTime = SDL_GetTicks() - startTime;
    int currentFrame = static_cast<int>((elapsedTime / animationSpeed) % frames);
    return currentFrame == frames - 1;
}

void Player::attack(Uint32 currentTime) {
    playAnimation("Attack");
    lastAttackTime = currentTime;
    projectileVelocity = velocity.direction()*projectileSpeed;
    if (projectileVelocity.x != 0 && projectileVelocity.y != 0) {
        projectileVelocity.x /= 1.414;
        projectileVelocity.y /= 1.414;
    }
    if (projectileVelocity== Vector2D()) {  // If player is not moving, use last direction
        if (lastDirection == "Up") projectileVelocity= Vector2D(0, -projectileSpeed);
        if (lastDirection == "Down") projectileVelocity= Vector2D(0, projectileSpeed);
        if (lastDirection == "Left") projectileVelocity= Vector2D(-projectileSpeed, 0);
        if (lastDirection == "Right") projectileVelocity= Vector2D(projectileSpeed, 0);
    }
}

void Player::handleInput() {
    if (hasChildren) {
        childA->handleInput();
        childB->handleInput();
        return;
    }
    if (state == ATTACK || state == HURT) {
        return;
    }
    if (!isActive) return;
    if (!isActiveChild) return;
    if (isPlayer1) {
        handlePlayer1Input();
    } else {
        handlePlayer2Input();
    }
}