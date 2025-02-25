#include "Game.h"
#include "SDL_ttf.h"
#include "SDL.h"
#include "Player.h"
#include "Projectile.h"
#include "Collision.h"
#include "Constants.h"
#include <string>
#include <math.h>

SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;

Game::Game()
{
    window = nullptr;
    gameState = MENU;
    menuItems.push_back(MenuItem("Start Game"));
    menuItems.push_back(MenuItem("Exit"));
    menuItems[0].selected = true;
    player1 = nullptr;
    player2 = nullptr;
}

Game::~Game()
{
}

void Game::initPlayers() {
    player1 = new Player(SCREEN_WIDTH/4,SCREEN_HEIGHT/2,64,64,PLAYER_SPEED*1.2,2,1000, 1, true);
    player1->addTexture("Idle", "assets/img/slime1_idle.png");
    player1->addTexture("Walk", "assets/img/slime1_walk.png");
    player1->addTexture("Attack", "assets/img/slime1_attack.png");
    player1->addTexture("Hurt", "assets/img/slime1_hurt.png");
    player1->initAnimation();
    player1->initProjectile(BULLET_SIZE, BULLET_SPEED*2, "assets/img/projectile_green.png", projectiles, 15);

    player2 = new Player(3*SCREEN_WIDTH/4,SCREEN_HEIGHT/2,64,64,PLAYER_SPEED*1.2,2,1000, 1, false);
    player2->addTexture("Idle", "assets/img/slime2_idle.png");
    player2->addTexture("Walk", "assets/img/slime2_walk.png");
    player2->addTexture("Attack", "assets/img/slime2_attack.png");
    player2->addTexture("Hurt", "assets/img/slime2_hurt.png");
    player2->initProjectile(BULLET_SIZE, BULLET_SPEED*2, "assets/img/projectile.png", projectiles, 15);
    player2->initAnimation();
    gameStart = 3;
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }

    if (TTF_Init() < 0) {
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }

    window = SDL_CreateWindow("SlimeBattle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    
    if (!window) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        return false;
    }

    SDL_Surface* tempSurface = IMG_Load("assets/img/background.png");
    if (tempSurface == NULL) {
        printf("Failed to load background image! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    backgroundMusic = Mix_LoadMUS("assets/audio/background.wav");
    attackSound = Mix_LoadWAV("assets/audio/attack.wav");
    hitSound = Mix_LoadWAV("assets/audio/hurt.wav");
    
    if (!backgroundMusic || !attackSound || !hitSound) {
        printf("Failed to load audio files! SDL_mixer Error: %s\n", Mix_GetError());
    }

    Mix_PlayMusic(backgroundMusic, -1);
    Mix_VolumeMusic(10);

    font = TTF_OpenFont("assets/fonts/DigitalDisco.ttf", 24);
    // initPlayers();
    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    isRunning = true;
    return true;
}

void Game::clearPlayers() {
    delete player1;
    delete player2;
    player1 = nullptr;
    player2 = nullptr;
}

void Game::clearProjectiles() {
    for (auto& p : projectiles) {
        delete p;
    }
    projectiles.clear();
}

void Game::handleInput() {
    if (gameState == MENU) {
        handleMenuInput();
        return;
    }
    Uint32 startTime = SDL_GetTicks(); // Get the start time
    while(gameStart > 0) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background
        SDL_RenderClear(renderer);
        gameStart = 3 - (SDL_GetTicks() - startTime) / 1000;
        renderText(std::to_string(gameStart), SCREEN_WIDTH/2, SCREEN_HEIGHT/2, {255, 0, 0, 255});
        SDL_RenderPresent(renderer);
    }
    handleGameInput();
}

void Game::drawCooldownCircle(float x, float y, float radius, float progress) {
    const int segments = 30;  // Number of segments to draw
    float maxAngle = progress * 2 * M_PI;  // Convert progress (0-1) to radians
    
    // Draw background circle
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 100);  // Gray, semi-transparent
    for (int i = 0; i < segments; i++) {
        float startAngle = (float)i / segments * 2 * M_PI;
        float endAngle = (float)(i + 1) / segments * 2 * M_PI;
        
        SDL_RenderDrawLine(renderer,
            x + cos(startAngle) * radius,
            y + sin(startAngle) * radius,
            x + cos(endAngle) * radius,
            y + sin(endAngle) * radius);
    }
    
    // Draw remaining cooldown in brighter color
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White
    for (int i = 0; i < segments; i++) {
        float startAngle = (float)i / segments * 2 * M_PI;
        if (startAngle > maxAngle) break;  // Stop at progress point
        
        float endAngle = (float)(i + 1) / segments * 2 * M_PI;
        if (endAngle > maxAngle) endAngle = maxAngle;
        
        SDL_RenderDrawLine(renderer,
            x + cos(startAngle) * radius,
            y + sin(startAngle) * radius,
            x + cos(endAngle) * radius,
            y + sin(endAngle) * radius);
    }
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void Game::handleGameInput() {
    if (gameState == MENU) {
        return;
    }

    if (gameState == GAME_OVER) {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_ESCAPE]) {
            clearPlayers();
            clearProjectiles();
            gameState = MENU;
        }
        return;
    }
    player1->handleInput();
    player2->handleInput();
    
    static bool wasSpacePressed = false;    // Track previous state of Space key
    static bool wasReturnPressed = false;   // Track previous state of Return key
    static bool firstFrame = true;
    const Uint8* state = SDL_GetKeyboardState(NULL);

    if (firstFrame) {
        wasSpacePressed = true;
        wasReturnPressed = true;
        firstFrame = false;
    }

    // Handle escape key
    if (state[SDL_SCANCODE_ESCAPE]) {
        clearPlayers();
        clearProjectiles();
        gameState = MENU;
        firstFrame = true;
    }

    Uint32 currentTime = SDL_GetTicks();
    // Handle Space key (Player 1)
    bool isSpacePressed = state[SDL_SCANCODE_SPACE];
    if (isSpacePressed && !wasSpacePressed) {  // Only shoot if key was just pressed
        if (currentTime - player1->lastAttackTime >= player1->ATTACK_COOLDOWN) {
            player1->attack(currentTime);
            Mix_PlayChannel(-1, attackSound, 0);
        }
    }
    wasSpacePressed = isSpacePressed;  // Update previous state
    
    // Handle Return key (Player 2)
    bool isReturnPressed = state[SDL_SCANCODE_RETURN];
    if (isReturnPressed && !wasReturnPressed) {  // Only shoot if key was just pressed
        if (currentTime - player2->lastAttackTime >= player2->ATTACK_COOLDOWN) {
            player2->attack(currentTime);
            Mix_PlayChannel(-1, attackSound, 0);
        }
    }
    wasReturnPressed = isReturnPressed;  // Update previous state
}

void Game::handleMenuInput() {
    if (gameState == PLAYING) {
        return;
    }
    SDL_Keycode key = event.key.keysym.sym;
    switch (key) {
        case SDLK_UP:
        case SDLK_w:
            menuItems[1].selected = false;
            menuItems[0].selected = true;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            menuItems[0].selected = false;
            menuItems[1].selected = true;
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            if (menuItems[0].selected) {
                initPlayers();
                gameState = PLAYING;
            } else if (menuItems[1].selected) {
                isRunning = false;
            }
            break;
    }
}

void Game::handleEvents() {
    SDL_PollEvent(&event);
	switch (event.type)
	{
	case SDL_QUIT :
        isRunning = false;
		break;
	default:
		break;
	}

    handleInput();
}


void Game::renderText(const std::string& text, int x, int y, SDL_Color color)  {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        return;
    }
    SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!labelTexture) {
        return;
    }
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, labelTexture, NULL, &dest);
    SDL_DestroyTexture(labelTexture);
    
    SDL_FreeSurface(surface);

} 

void Game::renderMenu()  {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Color selectedColor = {255, 255, 0, 255};

    // Render title
    renderText("Slime Battle", SCREEN_WIDTH/2 - 100, 100, textColor);

    // Render menu items
    for (size_t i = 0; i < menuItems.size(); i++) {
        renderText(menuItems[i].text,
                  SCREEN_WIDTH/2 - 50,
                  250 + i * 50,
                  menuItems[i].selected ? selectedColor : textColor);
    }
}

void Game::renderGameOver() {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    
    // Display winner text
    SDL_Color winnerColor = {255, 215, 0, 255}; // Gold color
    renderText(winnerText, SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/3, winnerColor);
    
    // Display scores
    SDL_Color player1Color = {50, 168, 102, 255};
    SDL_Color player2Color = {92, 30, 179, 255};
    
    std::string p1ScoreText = "Player 1: " + std::to_string(player1->score);
    std::string p2ScoreText = "Player 2: " + std::to_string(player2->score);
    
    renderText(p1ScoreText, SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2, player1Color);
    renderText(p2ScoreText, SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 50, player2Color);
    
    // Display return to menu message
    Uint32 currentTime = SDL_GetTicks();
    if ((currentTime - gameOverStartTime) / 500 % 2 == 0) { // Blinking effect
        SDL_Color promptColor = {255, 255, 255, 255};
        renderText("Press ESC to return to menu", SCREEN_WIDTH/2 - 180, SCREEN_HEIGHT * 3/4, promptColor);
    }
}

void Game::renderGame() {
    if (gameState == MENU || gameState == GAME_OVER) {
        return;
    }
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
	player1->draw();
    player2->draw();
    for (auto& p : projectiles)
    {
        p->draw();
    }

    // Draw cooldown indicators
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - player1->lastAttackTime < player1->ATTACK_COOLDOWN) {
        float progress = 1.0f - (float)(currentTime - player1->lastAttackTime) / player1->ATTACK_COOLDOWN;
        float centerX = player1->destRect.x + player1->destRect.w / 2;
        float centerY = player1->destRect.y + player1->destRect.h / 2;
        drawCooldownCircle(centerX, centerY, player1->destRect.w * 0.4f, progress);
    }
    
    // Player 2 cooldown
    if (currentTime - player2->lastAttackTime < player2->ATTACK_COOLDOWN) {
        float progress = 1.0f - (float)(currentTime - player2->lastAttackTime) / player2->ATTACK_COOLDOWN;
        float centerX = player2->destRect.x + player2->destRect.w / 2;
        float centerY = player2->destRect.y + player2->destRect.h / 2;
        drawCooldownCircle(centerX, centerY, player2->destRect.w * 0.4f, progress);
    }

    // Draw scores
    SDL_Color textColor = {50, 168, 102, 255};
    std::string mText = "Player 1: " + std::to_string(player1->score);
    renderText(mText, SCREEN_WIDTH/4-50, 30, textColor);
    textColor = {92, 30, 179, 255};
    mText = "Player 2: " + std::to_string(player2->score);
    renderText(mText, 3*SCREEN_WIDTH/4-50, 30, textColor);
    SDL_RenderPresent(renderer);
}

void Game::render()  {
    if (gameState == MENU) {
        renderMenu();
    } else if (gameState == PLAYING) {
        renderGame();
    } else if (gameState == GAME_OVER) {
        renderGameOver();
    }
    SDL_RenderPresent(renderer);
}

void Game::update() {
    if (gameState == MENU || gameState == GAME_OVER) {
        return;
    }
    player1->update();
    player2->update();

    for (auto& p : projectiles)
    {
        p->update();
    }
    projectilesHandler();
    // Check for win condition
    if (player1->score >= 21 || player2->score >= 21) {
        gameState = GAME_OVER;
        gameOverStartTime = SDL_GetTicks();
        
        if (player1->score >= 21) {
            winnerText = "Player 1 Wins!";
        } else {
            winnerText = "Player 2 Wins!";
        }
    }
}

void Game::projectilesHandler() {
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        if ((*it)->ricochetCount >= (*it)->ricochetLimit) {
            delete *it;
            it = projectiles.erase(it);
            continue;
        }
        if (Collision::AABB(player1->destRect, (*it)->destRect, 0.6) && !(*it)->isPlayer1) {
            // hit player 1
            Mix_PlayChannel(-1, hitSound, 0);
            player1->playAnimation("Hurt");
            player2->score += player1->damage;
            delete *it;
            it = projectiles.erase(it);
        } else if (Collision::AABB(player2->destRect, (*it)->destRect, 0.6) && (*it)->isPlayer1) {
            // hit player 2
            Mix_PlayChannel(-1, hitSound, 0);
            player2->playAnimation("Hurt");
            player1->score += player2->damage;
            delete *it;
            it = projectiles.erase(it);
        } else {
            it++;
        }
    }
}

void Game::clean() {
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(attackSound);
    Mix_FreeChunk(hitSound);
    Mix_CloseAudio();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(backgroundTexture);
    TTF_Quit();
    SDL_Quit();
}