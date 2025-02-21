#include "Game.h"
#include "SDL_ttf.h"
#include "SDL.h"
#include "Player.h"
#include "Projectile.h"
#include "Constants.h"
#include <string>

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
    player1 = new Player(300.0f,300.0f,64,64,PLAYER_SPEED,2);
    player1->addTexture("Idle", "assets/img/slime1_idle.png");
    player1->addTexture("Walk", "assets/img/slime1_walk.png");
    player1->initAnimation();

    player2 = new Player(500.0f,500.0f,64,64,PLAYER_SPEED,2, false);
    player2->addTexture("Idle", "assets/img/slime2_idle.png");
    player2->addTexture("Walk", "assets/img/slime2_walk.png");
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

    window = SDL_CreateWindow("SlimeBattle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    
    if (!window) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        return false;
    }
    font = TTF_OpenFont("assets/fonts/DigitalDisco.ttf", 24);
    initPlayers();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
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

void Game::handleGameInput() {
    if (gameState == MENU) {
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
    
    // Handle Space key (Player 1)
    bool isSpacePressed = state[SDL_SCANCODE_SPACE];
    if (isSpacePressed && !wasSpacePressed) {  // Only shoot if key was just pressed
        Projectile* p = new Projectile(player1->position.x, player1->position.y, 40, 40, 1, true);
        p->loadTexture("assets/img/projectile_green.png");
        projectiles.push_back(p);
    }
    wasSpacePressed = isSpacePressed;  // Update previous state
    
    // Handle Return key (Player 2)
    bool isReturnPressed = state[SDL_SCANCODE_RETURN];
    if (isReturnPressed && !wasReturnPressed) {  // Only shoot if key was just pressed
        Projectile* p = new Projectile(player2->position.x, player2->position.y, 20, 20, 2, false);
        p->loadTexture("assets/img/projectile.png");
        projectiles.push_back(p);
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

void Game::renderGame() {
    SDL_RenderClear(renderer);
	player1->draw();
    player2->draw();
    for (auto& p : projectiles)
    {
        p->draw();
    }
    // Draw scores
    SDL_Color textColor = {255, 255, 255, 255};
    renderText(std::to_string(player1->score), SCREEN_WIDTH/4, 30, textColor);
    renderText(std::to_string(player2->score), 3*SCREEN_WIDTH/4, 30, textColor);
    SDL_RenderPresent(renderer);
}

void Game::render()  {
    if (gameState == MENU) {
        renderMenu();
    } else {
        renderGame();
    }
    SDL_RenderPresent(renderer);
}

void Game::update() {
    if (gameState == MENU) {
        return;
    }
    player1->update();
    player2->update();

    for (auto& p : projectiles)
    {
        p->update();
    }
}

void Game::clean() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}