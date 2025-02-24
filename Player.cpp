#include "Player.h"
#include "Game.h"
#include "SDL_image.h"
#include "SDL.h"
#include "Projectile.h"
#include <string>
#include "Constants.h"

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

Player::Player(int x, int y, int w, int h, int s, int sc, Uint32 attack_cooldown, bool isP1)
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
    projectiles = nullptr;
}





Player::~Player()
{
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
    if (state == ATTACK || state == HURT) {
        velocity = Vector2D();
        if (isOnLastFrame()) {
            if (state == ATTACK) {
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

void Player::draw()
{
    SDL_RenderCopy(Game::renderer, texture, &srcRect, &destRect);
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
}

void Player::handlePlayer2Input() {
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
    if (state == ATTACK || state == HURT) {
        return;
    }
    if (isPlayer1) {
        handlePlayer1Input();
    } else {
        handlePlayer2Input();
    }
}