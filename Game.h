#pragma once

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <vector>
#include <string>

class Player;
class Projectile;

class Game
{  
public:
    Game();
	~Game();
    enum GameState {
        MENU,
        PLAYING,
        GAME_OVER
    };
    
    struct MenuItem {
        std::string text;
        SDL_Rect rect;
        bool selected;
    
        MenuItem(const std::string& t) : text(t), selected(false) {}
    };

	bool init();
    void initPlayers();

    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void renderMenu();
    void renderGame();
    void renderGameOver();
	void render();

    void handleInput();
    void handleMenuInput();
    void handleGameInput();
	void handleEvents();
	void update();
	bool running() { return isRunning; }
	void clean();
    void clearPlayers();
    void clearProjectiles();
    void projectilesHandler();
    void drawCooldownCircle(float x, float y, float radius, float progress);

	// void keepInBounds();

	static SDL_Renderer *renderer;
	static SDL_Event event;
	SDL_Window *window;
    SDL_Texture* backgroundTexture;
    Mix_Music* backgroundMusic;
    TTF_Font *font;
    std::string winnerText;
    Uint32 gameOverStartTime;
	bool isRunning = false;
    GameState gameState;
    std::vector<MenuItem> menuItems;
    Player* player1;
    Player* player2;
    std::vector<Projectile*> projectiles;
    int gameStart = 3;
};