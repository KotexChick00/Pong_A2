#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

class TTFManager {
public:
    TTFManager() {
        if (TTF_Init() == -1) {
            std::cerr << "TTF initialization failed: " << TTF_GetError() << std::endl;
            exit(1);
        }
    }
    ~TTFManager() {
        TTF_Quit();
    }
};

class WindowManager {
public:
    SDL_Renderer* renderer;

    WindowManager(int width, int height) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
            exit(1);
        }

        window = SDL_CreateWindow("Pong OOP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  width, height, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            exit(1);
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            exit(1);
        }
    }

    ~WindowManager() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    SDL_Window* window;
};

class Paddle {
public:
    static const int WIDTH = 20;
    static const int HEIGHT = 100;
    static const int SPEED = 10;

    SDL_Rect rect;

    Paddle(int x, int y) {
        rect = { x, y, WIDTH, HEIGHT };
    }

    void moveUp() {
        if (rect.y > 0) {
            rect.y -= SPEED;
        }
    }

    void moveDown(int windowHeight) {
        if (rect.y < windowHeight - HEIGHT) {
            rect.y += SPEED;
        }
    }

	void moveLeft() {
		if (rect.x > 0) {
			rect.x -= SPEED;
		}
	}

	void moveRight(int windowWidth) {
		if (rect.x < windowWidth - WIDTH) {
			rect.x += SPEED;
		}
	}

    void render(SDL_Renderer* renderer) const {
        SDL_RenderFillRect(renderer, &rect);
    }
};

class Ball {
public:
    static const int SIZE = 20;
    static const int BASE_SPEED = 3;

    SDL_Rect rect;
    int velX, velY;

    Ball(int x, int y) : velX(BASE_SPEED), velY(BASE_SPEED) {
        rect = { x, y, SIZE, SIZE };
    }

    void reset(int windowWidth, int windowHeight, bool goRight) {
        rect.x = windowWidth / 2 - SIZE / 2;
        rect.y = windowHeight / 2 - SIZE / 2;
        velX = goRight ? BASE_SPEED : -BASE_SPEED;
        velY = BASE_SPEED;
    }

    void update() {
        rect.x += velX;
        rect.y += velY;
    }

    void checkWallCollision(int windowHeight) {
        if (rect.y <= 0 || rect.y + SIZE >= windowHeight) {
            velY = -velY;
        }
    }

    bool checkPaddleCollision(const Paddle& paddle) {
        return SDL_HasIntersection(&rect, &paddle.rect);
    }

    void render(SDL_Renderer* renderer) const {
        SDL_RenderFillRect(renderer, &rect);
    }
};

class Scoreboard {
public:
    Scoreboard(SDL_Renderer* renderer, const char* fontPath, int fontSize)
        : renderer(renderer), leftScore(0), rightScore(0) {
        font = TTF_OpenFont(fontPath, fontSize);
        if (!font) {
            std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
            exit(1);
        }
        updateTexture();
    }

    ~Scoreboard() {
        SDL_DestroyTexture(texture);
        TTF_CloseFont(font);
    }

    void updateScores(int left, int right) {
        leftScore = left;
        rightScore = right;
        updateTexture();
    }

    void render() {
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
    }

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* texture = nullptr;
    SDL_Rect textRect;
    int leftScore;
    int rightScore;

    void updateTexture() {
        if (texture) SDL_DestroyTexture(texture);

        std::string scoreText = std::to_string(leftScore) + " - " + std::to_string(rightScore);
        SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), { 255, 255, 255 });
        texture = SDL_CreateTextureFromSurface(renderer, surface);

        textRect.w = surface->w;
        textRect.h = surface->h;
        textRect.x = (WINDOW_WIDTH - surface->w) / 2;
        textRect.y = 20;

        SDL_FreeSurface(surface);
    }
};

class Game {
public:
    // gameMode: 1 = PvP, 2 = PvE.
    Game() :
        windowManager(WINDOW_WIDTH, WINDOW_HEIGHT),
        ttfManager(),  // TTFManager initializes SDL_ttf.
        leftPaddle(50, WINDOW_HEIGHT / 2 - Paddle::HEIGHT / 2),
        rightPaddle(WINDOW_WIDTH - 70, WINDOW_HEIGHT / 2 - Paddle::HEIGHT / 2),
        ball(WINDOW_WIDTH / 2 - Ball::SIZE / 2, WINDOW_HEIGHT / 2 - Ball::SIZE / 2),
        scoreboard(windowManager.renderer, "arial.ttf", 32),
        leftScore(0),
        rightScore(0),
        gameMode(0)
    {
    }

    void run() {
        // Display start menu and set game mode.
        gameMode = displayMenu();

        bool running = true;
        while (running) {
            handleInput(running);
            update();
            render();
            SDL_Delay(16);
        }
        // TTF_Quit() is handled by TTFManager's destructor.
    }

private:
    WindowManager windowManager;
    TTFManager ttfManager;
    Paddle leftPaddle;
    Paddle rightPaddle;
    Ball ball;
    Scoreboard scoreboard;
    int leftScore;
    int rightScore;
    int gameMode; // 1 = PvP, 2 = PvE

    // Displays the start menu and waits for the player to select a mode.
    int displayMenu() {
        SDL_Event e;
        bool menuRunning = true;
        int selectedMode = 0;

        while (menuRunning) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT || e.key.keysym.sym == SDLK_q) {
                    exit(0);
                }
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_1) {
                        selectedMode = 1;  // PvP
                        menuRunning = false;
                    }
                    else if (e.key.keysym.sym == SDLK_2) {
                        selectedMode = 2;  // PvE
                        menuRunning = false;
                    }
                }
            }

            SDL_SetRenderDrawColor(windowManager.renderer, 0, 0, 0, 255);
            SDL_RenderClear(windowManager.renderer);

            // Create menu text textures.
            TTF_Font* menuFont = TTF_OpenFont("arial.ttf", 48);
            if (!menuFont) {
                std::cerr << "Failed to load menu font: " << TTF_GetError() << std::endl;
                exit(1);
            }
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Surface* surface1 = TTF_RenderText_Solid(menuFont, "Press 1 for PvP", white);
            SDL_Surface* surface2 = TTF_RenderText_Solid(menuFont, "Press 2 for PvE", white);
			SDL_Surface* surface3 = TTF_RenderText_Solid(menuFont, "Press P to pause, Q to quit", white);
            SDL_Texture* texture1 = SDL_CreateTextureFromSurface(windowManager.renderer, surface1);
            SDL_Texture* texture2 = SDL_CreateTextureFromSurface(windowManager.renderer, surface2);
			SDL_Texture* texture3 = SDL_CreateTextureFromSurface(windowManager.renderer, surface3);
            SDL_Rect rect1;
            rect1.w = surface1->w;
            rect1.h = surface1->h;
            rect1.x = (WINDOW_WIDTH - rect1.w) / 2;
            rect1.y = WINDOW_HEIGHT / 3;
            SDL_Rect rect2;
            rect2.w = surface2->w;
            rect2.h = surface2->h;
            rect2.x = (WINDOW_WIDTH - rect2.w) / 2;
            rect2.y = WINDOW_HEIGHT / 3 + rect1.h + 20;
			SDL_Rect rect3;
			rect3.w = surface3->w;
			rect3.h = surface3->h;
			rect3.x = (WINDOW_WIDTH - rect3.w) / 2;
			rect3.y = WINDOW_HEIGHT / 3 + rect1.h + 80;
            SDL_FreeSurface(surface1);
            SDL_FreeSurface(surface2);
			SDL_FreeSurface(surface3);

            SDL_RenderCopy(windowManager.renderer, texture1, nullptr, &rect1);
            SDL_RenderCopy(windowManager.renderer, texture2, nullptr, &rect2);
			SDL_RenderCopy(windowManager.renderer, texture3, nullptr, &rect3);

            SDL_DestroyTexture(texture1);
            SDL_DestroyTexture(texture2);
            TTF_CloseFont(menuFont);

            SDL_RenderPresent(windowManager.renderer);
            SDL_Delay(16);
        }
        return selectedMode;
    }

    // Displays a pause menu when the game is paused.
    void pauseMenu() {
        bool paused = true;
        TTF_Font* pauseFont = TTF_OpenFont("arial.ttf", 48);
        if (!pauseFont) {
            std::cerr << "Failed to load pause font: " << TTF_GetError() << std::endl;
            exit(1);
        }
        SDL_Color white = { 255, 255, 255, 255 };

        while (paused) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    exit(0);
                }
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_p) {
                        // Resume the game.
                        paused = false;
                    }
                    else if (e.key.keysym.sym == SDLK_q) {
                        // Quit the game.
                        exit(0);
                    }
                }
            }

            // Render the pause menu overlay.
            SDL_SetRenderDrawColor(windowManager.renderer, 0, 0, 0, 200);
            SDL_RenderClear(windowManager.renderer);

            // Create pause text.
            SDL_Surface* pauseSurface = TTF_RenderText_Solid(pauseFont, "Game Paused", white);
            SDL_Texture* pauseTexture = SDL_CreateTextureFromSurface(windowManager.renderer, pauseSurface);
            SDL_Rect pauseRect;
            pauseRect.w = pauseSurface->w;
            pauseRect.h = pauseSurface->h;
            pauseRect.x = (WINDOW_WIDTH - pauseRect.w) / 2;
            pauseRect.y = WINDOW_HEIGHT / 3;

            SDL_Surface* resumeSurface = TTF_RenderText_Solid(pauseFont, "Press P to resume, Q to quit", white);
            SDL_Texture* resumeTexture = SDL_CreateTextureFromSurface(windowManager.renderer, resumeSurface);
            SDL_Rect resumeRect;
            resumeRect.w = resumeSurface->w;
            resumeRect.h = resumeSurface->h;
            resumeRect.x = (WINDOW_WIDTH - resumeRect.w) / 2;
            resumeRect.y = pauseRect.y + pauseRect.h + 20;

            SDL_RenderCopy(windowManager.renderer, pauseTexture, nullptr, &pauseRect);
            SDL_RenderCopy(windowManager.renderer, resumeTexture, nullptr, &resumeRect);

            SDL_RenderPresent(windowManager.renderer);

            SDL_FreeSurface(pauseSurface);
            SDL_FreeSurface(resumeSurface);
            SDL_DestroyTexture(pauseTexture);
            SDL_DestroyTexture(resumeTexture);

            SDL_Delay(16);
        }
        TTF_CloseFont(pauseFont);
    }

    void handleInput(bool& running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_p) {
                    pauseMenu();
                }
            }
        }

        const Uint8* keystates = SDL_GetKeyboardState(NULL);

        // Left paddle controls (WASD)
        if (keystates[SDL_SCANCODE_W]) leftPaddle.moveUp();
        if (keystates[SDL_SCANCODE_S]) leftPaddle.moveDown(WINDOW_HEIGHT);
        if (keystates[SDL_SCANCODE_A]) leftPaddle.moveLeft();
        if (keystates[SDL_SCANCODE_D]) leftPaddle.moveRight(WINDOW_WIDTH);

        if (gameMode == 1) {  // PvP controls (Arrow keys)
            if (keystates[SDL_SCANCODE_UP]) rightPaddle.moveUp();
            if (keystates[SDL_SCANCODE_DOWN]) rightPaddle.moveDown(WINDOW_HEIGHT);
            if (keystates[SDL_SCANCODE_LEFT]) rightPaddle.moveLeft();
            if (keystates[SDL_SCANCODE_RIGHT]) rightPaddle.moveRight(WINDOW_WIDTH);
        }
    }

    void update() {
        ball.update();
        ball.checkWallCollision(WINDOW_HEIGHT);

        // Check for paddle collisions and reverse horizontal velocity.
        if (ball.checkPaddleCollision(leftPaddle) || ball.checkPaddleCollision(rightPaddle)) {
            ball.velX = -ball.velX;
        }

        // In PvE mode, let the computer control the right paddle.
        if (gameMode == 2) {
            int paddleCenter = rightPaddle.rect.y + Paddle::HEIGHT / 2;
            int ballCenter = ball.rect.y + Ball::SIZE / 2;
            if (ballCenter < paddleCenter && rightPaddle.rect.y > 0) {
                rightPaddle.rect.y -= Paddle::SPEED;
            }
            else if (ballCenter > paddleCenter && rightPaddle.rect.y + Paddle::HEIGHT < WINDOW_HEIGHT) {
                rightPaddle.rect.y += Paddle::SPEED;
            }
        }

        // Update score if the ball touches either the left or right wall.
        bool scoreUpdated = false;
        if (ball.rect.x < 0) {
            rightScore++;
            scoreUpdated = true;
        }
        if (ball.rect.x + Ball::SIZE > WINDOW_WIDTH) {
            leftScore++;
            scoreUpdated = true;
        }

        if (scoreUpdated) {
            scoreboard.updateScores(leftScore, rightScore);
            ball.reset(WINDOW_WIDTH, WINDOW_HEIGHT, ball.rect.x < 0);
            SDL_Delay(500);
        }
    }

    void render() {
        // Clear the screen with black.
        SDL_SetRenderDrawColor(windowManager.renderer, 0, 0, 0, 255);
        SDL_RenderClear(windowManager.renderer);

        // Draw center line.
        SDL_SetRenderDrawColor(windowManager.renderer, 255, 255, 255, 255);
        for (int i = 0; i < WINDOW_HEIGHT; i += 20) {
            SDL_RenderDrawPoint(windowManager.renderer, WINDOW_WIDTH / 2, i);
        }

        // Render left paddle in blue.
        SDL_SetRenderDrawColor(windowManager.renderer, 0, 0, 255, 255);
        leftPaddle.render(windowManager.renderer);

        // Render right paddle in red.
        SDL_SetRenderDrawColor(windowManager.renderer, 255, 0, 0, 255);
        rightPaddle.render(windowManager.renderer);

        // Render the ball and scoreboard (they use their own colors).
        SDL_SetRenderDrawColor(windowManager.renderer, 255, 255, 255, 255);
        ball.render(windowManager.renderer);
        scoreboard.render();

        SDL_RenderPresent(windowManager.renderer);
    }

};

int main(int argc, char* argv[]) {
    Game pongGame;
    pongGame.run();
    return 0;
}
