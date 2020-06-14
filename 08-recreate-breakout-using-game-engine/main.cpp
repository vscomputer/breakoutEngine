#include <SDL.h>
#include <SDL_ttf.h>
#include "Core/Audio.h"
#include "Core/Sound.h"
#include "Core/Music.h"
#include "Core/Image.h"
#include "Core/Graphics.h"
#include "Core/OutlineFont.h"
#include "Core/Input.h"
#include "Core/Game.h";
#include "Core/GameState.h";
#include "Core/StateManager.h"
#include <cstdlib>
//#include <cstdio>

struct Player
{
	SDL_Rect rect;
	int lives;
};

struct Ball
{
	SDL_Rect rect;
	int xVel;
	int yVel;
	bool isLocked;
};

struct Block
{
	SDL_Rect rect;
	bool alive;
	int frame;
};

//Constant variables
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int PADDLE_Y = 550;
const int BALL_WIDTH = 20;
const int BALL_HEIGHT = 20;
const int BALL_SPEED = 10;

const int PLAYER_SPEED = 10;

const int FPS = 30;
const int FRAME_DELAY = 1000 / FPS;

const int GAMEAREA_X1 = 20;
const int GAMEAREA_Y1 = 20;
const int GAMEAREA_X2 = 598;
const int GAMEAREA_Y2 = 600;

const int BLOCK_COLUMNS = 11;
const int BLOCK_ROWS = 5;
const int BLOCK_WIDTH = 50;
const int BLOCK_HEIGHT = 25;

const int GS_SPLASH = 0;
const int GS_RUNNING = 1;
const int GS_GAMEOVER = 2;
const int GS_PAUSED = 3;

//Game variables
Block blocks[BLOCK_COLUMNS * BLOCK_ROWS];
Player player;
Ball ball;
int _gameState;

//Resource variables
Image _backgroundImage;
Image _ballImage;
Image _playerPaddleImage;
Image _splashImage;
Image _blockImage;
Image _gamePausedImage;
Image _gameOverImage;
OutlineFont _gameFont;
Sound _ballBounceSound;
Sound _ballSpawnSound;
Sound _playerScoreSound;
Sound _enemyScoreSound;
Sound _explosionSound;
Music _gameMusic;

Audio audio;

Graphics graphics;

Input _input;

bool LoadFiles();
void FreeFiles();
bool ProgramIsRunning();
void DrawImageFrame(SDL_Surface* image, SDL_Surface* destSurface, int x, int y, int width, int height, int frame);
void DrawImage(SDL_Surface* image, SDL_Surface* destSurface, int x, int y);
void DrawRasterText(SDL_Surface* surface, SDL_Surface* destSurface, char string[], int x, int y, int charSize);
void DrawOutlineText(SDL_Surface* surface, char* string, int x, int y, TTF_Font* font, Uint8 r, Uint8 g, Uint8 b);
bool InitGame();
bool InitSDL();
void FreeGame();
void RunGame();
void ResetGame();
void UpdatePlayer();
void UpdateAI();
void UpdateBall();
bool RectsOverlap(SDL_Rect rect1, SDL_Rect rect2);
void UpdateSplash();
void UpdateGame();
void UpdateGameOver();
void DrawScreen();
void DrawGame();
void DrawSplash();
void DrawGameOver();
void DrawGamePaused();
void SetBlocks();
int NumBlocksLeft();
void DrawBlocks();

int main(int argc, char *argv[])
{
	
	if (!InitGame())
	{
		FreeGame();
		return 0;
	}

	while (ProgramIsRunning())
	{
		const long int oldTime = SDL_GetTicks();
		graphics.fillRect(0, 0, graphics.getWidth(), graphics.getHeight(), 0, 0, 0);
		RunGame();
		DrawScreen();
		const int frameTime = SDL_GetTicks() - oldTime;
		if (frameTime < FRAME_DELAY)
		{
			SDL_Delay(FRAME_DELAY - frameTime);
		}
		graphics.flip();
		_input.update();
	}

	FreeGame();
	return 0;
}

bool InitGame()
{
	if (!InitSDL())
	{
		return false;
	}

	if (!LoadFiles())
	{
		return false;
	}

	_gameMusic.play(-1);

	_gameState = GS_SPLASH;

	return true;
}

bool InitSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		return false;
	}

	if(!audio.init())
	{
		return false;
	}

	if (TTF_Init() == -1)
	{
		return false;
	}

	if (!graphics.init(800, 600, false))
		return false;

	_input.init();

	return true;

}

void ResetGame()
{
	player.rect.x = (GAMEAREA_X2 - GAMEAREA_X1) / 2 - PADDLE_WIDTH / 2 + GAMEAREA_X1;
	player.rect.y = PADDLE_Y;
	player.rect.w = PADDLE_WIDTH;
	player.rect.h = PADDLE_HEIGHT;


	ball.rect.x = SCREEN_WIDTH / 2 - BALL_WIDTH / 2;
	ball.rect.y = SCREEN_HEIGHT / 2 - BALL_HEIGHT / 2;
	ball.rect.w = BALL_WIDTH;
	ball.rect.h = BALL_HEIGHT;

	_ballSpawnSound.play();

	SetBlocks();
	ball.isLocked = true;
	player.lives = 3;
}

bool LoadFiles()
{	
	//Error checking images
	if (_backgroundImage.load((char*)"graphics/background.bmp", &graphics) == false)
		return false;
	if (_splashImage.load((char*)"graphics/splash.bmp", &graphics) == false)
		return false;
	if (_gameOverImage.load((char*)"graphics/gameover.bmp", &graphics) == false)
		return false;
	if (_gamePausedImage.load((char*)"graphics/gamepaused.bmp", &graphics) == false)
		return false;
	if (_ballImage.load((char*)"graphics/ball.bmp", &graphics) == false)
		return false;
	if (_playerPaddleImage.load((char*)"graphics/player.bmp", &graphics)== false)
		return false;
	if (_blockImage.load((char*)"graphics/blocks.bmp",50,25, &graphics) == false)
		return false;

	//Load font
	if (_gameFont.load((char*)"graphics/brick.ttf", 20) == false)
		return false;	

	//Load sounds
	if(_ballBounceSound.load((char*)"audio/ball_bounce.wav") == false)
		return false;

	if(_ballSpawnSound.load((char*)"audio/ball_spawn.wav") == false)
		return false;

	if(_explosionSound.load((char*)"audio/explosion.wav") == false)
		return false;

	//Error check sounds
	if (!_ballBounceSound.isLoaded())
		return false;

	if (!_ballSpawnSound.isLoaded())
		return false;

	if (!_explosionSound.isLoaded())
		return false;

	//Load music
	if(_gameMusic.load((char*)"audio/song.ogg") == false)
		return false;

	//Everything is good
	return true;
}

bool ProgramIsRunning()
{
	SDL_Event event;

	bool running = true;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			running = false;
	}

	if(_input.keyHit(SDL_SCANCODE_ESCAPE))
	{
		if (_gameState == GS_RUNNING)
		{
			_gameState = GS_PAUSED;
		}
		else if (_gameState == GS_PAUSED)
		{
			_gameState = GS_RUNNING;
		}
	}

	return running;
}

void DrawImageFrame(SDL_Surface* image, SDL_Surface* destSurface, int x, int y, int width, int height, int frame)
{
	//Draws a specific frame from an image
	SDL_Rect destRect;
	destRect.x = x;
	destRect.y = y;

	const int columns = image->w / width;

	SDL_Rect sourceRect;
	sourceRect.y = (frame / columns)*height;
	sourceRect.x = (frame%columns)*width;
	sourceRect.w = width;
	sourceRect.h = height;

	SDL_BlitSurface(image, &sourceRect, destSurface, &destRect);
}


void RunGame()
{
	switch (_gameState)
	{
	case GS_SPLASH:
		UpdateSplash();
		break;
	case GS_RUNNING:
		UpdateGame();
		break;
	case GS_GAMEOVER:
		UpdateGameOver();
		break;
	default:
		break;
	}

}

void UpdatePlayer()
{
	if(_input.keyDown(SDL_SCANCODE_LEFT))
	{
		player.rect.x -= PLAYER_SPEED;
	}

	if (_input.keyDown(SDL_SCANCODE_RIGHT))
	{
		player.rect.x += PLAYER_SPEED;
	}

	if (_input.keyDown(SDL_SCANCODE_SPACE) && ball.isLocked)
	{
		ball.isLocked = false;
		ball.xVel = rand() % 3 - 1;
		ball.yVel = BALL_SPEED;
	}

	if (player.rect.x < GAMEAREA_X1)
	{
		player.rect.x = GAMEAREA_X1;
	}

	if (player.rect.x > GAMEAREA_X2 - player.rect.w)
	{
		player.rect.x = GAMEAREA_X2 - player.rect.w;
	}
}


void UpdateBall()
{
	if (ball.isLocked)
	{
		const int PaddleCenterX = player.rect.x + player.rect.w / 2;
		ball.rect.x = PaddleCenterX - ball.rect.w / 2;
		ball.rect.y = player.rect.y - ball.rect.h;
	}

	ball.rect.x += ball.xVel;

	if (RectsOverlap(ball.rect, player.rect))
	{
		ball.rect.x -= ball.xVel;
		ball.xVel *= -1;
		_ballBounceSound.play();
	}
	else
	{
		for (auto& block : blocks)
		{
			if (block.alive && RectsOverlap(ball.rect, block.rect))
			{
				ball.rect.x -= ball.xVel;
				ball.xVel *= -1;
				ball.yVel += 1;
				block.alive = false;
				_explosionSound.play();
			}
		}
	}

	ball.rect.y += ball.yVel;

	if (RectsOverlap(ball.rect, player.rect))
	{
		ball.rect.y -= ball.yVel;
		ball.yVel *= -1;
		const int ballCenterX = ball.rect.x + ball.rect.w / 2;
		const int paddleCenterX = player.rect.x + player.rect.w / 2;

		ball.xVel = (ballCenterX - paddleCenterX) / 5;
		_ballBounceSound.play();
	}
	else
	{
		for (auto& block : blocks)
		{
			if (block.alive && RectsOverlap(ball.rect, block.rect))
			{
				ball.rect.y -= ball.yVel;
				ball.yVel *= -1;
				block.alive = false;
				_explosionSound.play();
			}
		}
	}

	if (ball.rect.y < GAMEAREA_Y1)
	{
		ball.rect.y = GAMEAREA_Y1;
		ball.yVel *= -1;
		_ballBounceSound.play();
	}

	if (ball.rect.x > GAMEAREA_X2 - ball.rect.w)
	{
		ball.rect.x = GAMEAREA_X2 - ball.rect.w;
		ball.xVel *= -1;
		_ballBounceSound.play();
	}

	if (ball.rect.x < GAMEAREA_X1)
	{
		ball.rect.x = GAMEAREA_X1;
		ball.xVel *= -1;
		_ballBounceSound.play();
	}

	if (ball.rect.y > GAMEAREA_Y2)
	{
		ball.isLocked = true;

		const int paddleCenterX = player.rect.x + player.rect.w / 2;
		ball.rect.x = paddleCenterX - ball.rect.w / 2;
		ball.rect.y = player.rect.y - ball.rect.h;

		player.lives--;
		_ballSpawnSound.play();
	}

}

bool RectsOverlap(SDL_Rect rect1, SDL_Rect rect2)
{
	if (rect1.x >= rect2.x + rect2.w)
		return false;
	if (rect1.y >= rect2.y + rect2.h)
		return false;
	if (rect2.x >= rect1.x + rect1.w)
		return false;
	if (rect2.y >= rect1.y + rect1.h)
		return false;

	return true;
}

void UpdateSplash()
{
	const auto keys = const_cast<Uint8*>(SDL_GetKeyboardState(nullptr));
	if (keys[SDL_SCANCODE_RETURN])
	{
		ResetGame();
		_gameState = GS_RUNNING;
	}
}



void UpdateGame()
{
	UpdatePlayer();
	UpdateBall();
	if (player.lives <= 0)
	{
		_gameState = GS_GAMEOVER;
	}
	if (NumBlocksLeft() <= 0)
	{
		ResetGame();
	}
}

void UpdateGameOver()
{
	const auto keys = const_cast<Uint8*>(SDL_GetKeyboardState(nullptr));
	if (keys[SDL_SCANCODE_SPACE])
	{
		_gameState = GS_SPLASH;
	}
}


void DrawGame()
{
	_backgroundImage.draw(0, 0, &graphics);
	_ballImage.draw(ball.rect.x, ball.rect.y, &graphics);
	_playerPaddleImage.draw(player.rect.x, player.rect.y, &graphics);
	DrawBlocks();

	char blocksText[64];
	char livesText[64];

	sprintf_s(blocksText, "Blocks: %d", NumBlocksLeft());
	sprintf_s(livesText, "Lives: %d", player.lives);

	_gameFont.draw(blocksText, 645, 150, 255, 255, 255, &graphics);
	_gameFont.draw(livesText, 645, 170, 255, 255, 255, &graphics);

}

void DrawSplash()
{
	_splashImage.draw(0, 0, &graphics);
}



void DrawGameOver()
{
	DrawGame();
	_gameOverImage.draw(0, 0, &graphics);
}

void DrawGamePaused()
{
	DrawGame();
	_gamePausedImage.draw(0, 0, &graphics);
}

void SetBlocks()
{
	int i = 0;

	for (int x = 0; x < BLOCK_COLUMNS; x++)
	{
		for (int y = 0; y < BLOCK_ROWS; y++)
		{
			blocks[i].rect.x = x * BLOCK_WIDTH + GAMEAREA_X1 + x * 3;
			blocks[i].rect.y = (y * 2) * BLOCK_HEIGHT + GAMEAREA_Y1 + y * 3;
			blocks[i].rect.w = BLOCK_WIDTH;
			blocks[i].rect.h = BLOCK_HEIGHT;
			blocks[i].alive = true;
			blocks[i].frame = i % 4;
			i++;
		}
	}
}

int NumBlocksLeft()
{
	auto result = 0;
	for (auto& block : blocks)
	{
		if (block.alive)
		{
			result++;
		}
	}
	return result;
}

void DrawBlocks()
{
	for (auto& block : blocks)
	{
		if (block.alive)
		{
			_blockImage.draw(block.rect.x, block.rect.y, block.frame, &graphics);			
		}
	}
}

void FreeGame()
{
	graphics.kill();
	audio.stopChannel(1);	//Stop the music
	FreeFiles();        //Release the files we loaded
	audio.kill();		//Close the audio system
	TTF_Quit();         //Close the font system
	SDL_Quit();         //Close SDL
}

void FreeFiles()
{
	//Free images
	_backgroundImage.free();	
	_splashImage.free();
	_gameOverImage.free();
	_gamePausedImage.free();
	_ballImage.free();
	_playerPaddleImage.free();
	_ballImage.free();

	//Free font
	_gameFont.free();

	//Free sounds
	_ballBounceSound.free();
	_ballSpawnSound.free();
	_explosionSound.free();

	//Free music
	_gameMusic.free();

	//Free input
	_input.kill();
}

void DrawImage(SDL_Surface* image, SDL_Surface* destSurface, int x, int y)
{
	SDL_Rect destRect;
	destRect.x = x;
	destRect.y = y;

	SDL_BlitSurface(image, NULL, destSurface, &destRect);
}

void DrawOutlineText(SDL_Surface* surface, char* string, int x, int y, TTF_Font* font, Uint8 r, Uint8 g, Uint8 b)
{
	SDL_Color color;

	color.r = r;
	color.g = g;
	color.b = b;
	color.a = 1;

	SDL_Surface* renderedText = TTF_RenderText_Solid(font, string, color);

	SDL_Rect pos;

	pos.x = x;
	pos.y = y;

	SDL_BlitSurface(renderedText, nullptr, surface, &pos);
	SDL_FreeSurface(renderedText);
}



void DrawScreen()
{
	switch (_gameState)
	{
	case GS_SPLASH:
		DrawSplash();
		break;
	case GS_RUNNING:
		DrawGame();
		break;
	case GS_GAMEOVER:
		DrawGameOver();
		break;
	case GS_PAUSED:
		DrawGamePaused();
		break;
	default:
		break;
	}
}
