#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <vector>

#include "Tile.h"

using namespace sf;

const int SPRITE_SIZE = 16;
const int columns = 20, rows = 20;

const int tileSpaceX = 12, tileSpaceY = 55;
int W = tileSpaceX + SPRITE_SIZE * columns + 8, H = tileSpaceY + SPRITE_SIZE * rows + 8;

Tile tiles[columns][rows];

bool won;
bool lost;
bool smileyPressed;
bool gameStarted;

RenderWindow window(VideoMode(W, H), "Minesweeper", Style::Titlebar | Style::Close);
Sprite tileSpace, infoSpace, outline;

bool outOfBounds(int x, int y) {
	return x < 0 || x >= columns || y < 0 || y >= rows;
}

void checkForEmptyAdjacentTiles(int x, int y){
	// We don't want to show marked tiles.
	if (tiles[x][y].isMarked())
		return;

	// If the tile is a number, show it and then return.
	if (tiles[x][y].isNumber()) {
		tiles[x][y].show();
		return;
	}

	// For loop for the adjacent tiles to the current tile. Checks if any of them are empty and then does the same for those tiles with recursion.
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			int X = x - 1 + i, Y = y - 1 + j;
			if (!outOfBounds(X, Y) && (tiles[X][Y].getHiddenState() == State::EMPTY && tiles[X][Y].isHidden() || tiles[X][Y].isNumber())) {
				tiles[x][y].show();
				checkForEmptyAdjacentTiles(X, Y);
			}
		}
	}
}

void resetGame(Clock *clock){
	clock->restart();
	
	// If the player has won yet, i.e. marked all bombs yet.
	won = false;
	// If the smiley face is currently being pressed.
	smileyPressed = false;
	// If the player has lost.
	lost = false;
	// If the game is started yet.
	gameStarted = false;
	
	// Resetting the tiles.
	for (int i = 0; i < columns; i++) {
		for (int j = 0; j < rows; j++) {
			tiles[i][j].reset();
		}
	}

	// Random bomb generation

	static const int bombCount = (columns * rows) / 6;

	static struct Point {
		Point(int x, int y) {
			this->x = x;
			this->y = y;
		}

		int x, y;
	};

	std::vector<Point> points;
	for (int i = 0; i < columns; i++)
		for (int j = 0; j < rows; j++) {
			Point point(i, j);
			points.push_back(point);
		}

	for (int n = 0; n < bombCount; n++) {
		int rIndex = n + (rand() % (columns * rows - n));

		Point *current = &points.at(n), *random = &points.at(rIndex);
		Point temp = *current;
		*current = points.at(rIndex);
		*random = temp;

		tiles[current->x][current->y].setState(BOMB);
	}
	
	for (int i = 0; i < columns; i++) {
		for (int j = 0; j < rows; j++) {
			if (tiles[i][j].isBomb())
				continue;

			// Count how many bombs are adjacent to each tile.

			int n = 0;

			if (i > 0 && j > 0 && tiles[i - 1][j - 1].isBomb()) n++;
			if (j > 0 && tiles[i][j - 1].isBomb()) n++;
			if (i < columns - 1 && j > 0 && tiles[i + 1][j - 1].isBomb()) n++;
			if (i > 0 && tiles[i - 1][j].isBomb()) n++;
			if (i < columns - 1 && tiles[i + 1][j].isBomb()) n++;
			if (i > 0 && j < rows - 1 && tiles[i - 1][j + 1].isBomb()) n++;
			if (j < columns - 1 && tiles[i][j + 1].isBomb()) n++;
			if (i < columns - 1 && j < rows - 1 && tiles[i + 1][j + 1].isBomb()) n++;

			tiles[i][j].setState(n);
		}
	}
}

// When the player has lost we show the boms.
void showBombs(){
	for (int i = 0; i < columns; i++) {
		for (int j = 0; j < rows; j++) {
			// If a tile has been flagged but doesn't contain a bomb.
			if (!tiles[i][j].isBomb() && tiles[i][j].isFlag()) {
				tiles[i][j].setState(NOTBOMB);
				tiles[i][j].show();
			}
			// If a tile is a bomb, show it.
			if (tiles[i][j].isBomb())
				tiles[i][j].show();
		}
	}
}

/* 
	Returns the amount of bombs minus how many marked tiles there are.
	In other words, it will not necessarily return a positive number.
*/
int bombsLeft() {
	int counter = 0;
	for (int i = 0; i < columns; i++) {
		for (int j = 0; j < rows; j++) {
			if (tiles[i][j].isBomb())
				counter++;
			if (tiles[i][j].isFlag())
				counter--;
		}
	}
	return counter;
}

void drawDigit(int x, int y, int digit){
	if (digit > 9) {
		digit = 9;
	}
	// If the the number is negative we want to draw a minus sign.
	else if (digit < 0)
		// We set digit to 20 because the minus sign is located at the (13 * 20)th horizontal pixel in the image.
		digit = 20;

	infoSpace.setTextureRect(IntRect(digit * 13, 0, 13, 23));
	infoSpace.setPosition(x, y);
	window.draw(infoSpace);
}

int main()
{
	// Seed for random generation
	srand(time(NULL));

	// Set the title bar icon.
	Image icon;
	icon.loadFromFile("../res/icon.png");
	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	window.setFramerateLimit(60);

	// Loading the textures
	Texture tile, info, frame;
	tile.loadFromFile("../res/tiles.png");
	info.loadFromFile("../res/info.png");
	frame.loadFromFile("../res/outline.png");

	tileSpace.setTexture(tile);
	infoSpace.setTexture(info);
	outline.setTexture(frame);

	Clock clock;
	int lastTime;

	resetGame(&clock);

	while (window.isOpen())
	{
		// If the game hasn't started yet we don't want to start counting
		if(!gameStarted)
			clock.restart();

		Event event;
		while (window.pollEvent(event))
		{
			Vector2i position = Mouse::getPosition(window);
			int x = position.x;
			int y = position.y;

			if (event.type == Event::Closed)
				window.close();

			if (event.type == Event::MouseButtonPressed) {
				// The position of the smiley face
				if (x >= W / 2 - 13 && x <= W / 2 + 13 && y > 15 && y <= 42) {
					smileyPressed = true;
				}
			}

			else if (event.type == Event::MouseButtonReleased) {
				smileyPressed = false;
				
				// If the smiley face is pressed we want to reset the game.
				if (x >= W / 2 - 13 && x <= W / 2 + 13 && y > 15 && y <= 42) {
					resetGame(&clock);
				}
			}

			// If the mouse button is pressed and the mouse is within the tile space.
			if (event.type == Event::MouseButtonPressed && x - tileSpaceX >= 0 && x - tileSpaceX < rows * SPRITE_SIZE && y - tileSpaceY >= 0 && y - tileSpaceY < columns * SPRITE_SIZE) {
				// Convert the pixel coordinates to tile coordinates.
				x = (x - tileSpaceX) / SPRITE_SIZE;
				y = (y - tileSpaceY) / SPRITE_SIZE;

				// As soon as the player click inside the tile space we want to start the game
				if (!gameStarted)
					gameStarted = true;

				// While the user hasn't lost or won they should be able to click the tiles
				if (!lost && !won) {
					if (event.key.code == Mouse::Left) {
						lost = tiles[x][y].leftClick();
						
						// Show the bombs when the user has lost
						if (lost) 
							showBombs();
						// If the tile is empty we want to check for adjacent tiles that also are empty.
						else
							checkForEmptyAdjacentTiles(x, y);
					}else if (event.key.code == Mouse::Right) {
						tiles[x][y].rightClick();
					}

					// If all of the bombs are flagged the user has won.
					bool broken = false;
					for (int i = 0; i < columns && !broken; i++) {
						for (int j = 0; j < rows && !broken; j++) {
							if (tiles[i][j].isBomb()) {
								if (!tiles[i][j].isFlag()) {
									broken = true;
								}
							}
						}
					}
					if (!broken)
						won = true;
				}
			}
		}

		// Clear window
		window.clear();

		// Draw the background
		RectangleShape background(Vector2f(W, H));
		background.setFillColor(Color(192, 192, 192));
		window.draw(background);

		// Drawing the frame
		outline.setTextureRect(IntRect(20, 0, 11, 11));
		outline.setPosition(0, 0);
		window.draw(outline);

		outline.setTextureRect(IntRect(57, 0, 16, 11));
		for (int i = 0; i < columns; i++) {
			outline.setPosition(11 + SPRITE_SIZE * i, 0);
			window.draw(outline);
		}

		outline.setTextureRect(IntRect(31, 0, 9, 11));
		outline.setPosition(11 + SPRITE_SIZE * columns, 0);
		window.draw(outline);

		outline.setTextureRect(IntRect(0, 0, 11, 33));
		outline.setPosition(0, 11);
		window.draw(outline);

		outline.setTextureRect(IntRect(11, 33, 9, 33));
		outline.setPosition(11 + columns * SPRITE_SIZE, 11);
		window.draw(outline);

		outline.setTextureRect(IntRect(20, 21, 12, 11));
		outline.setPosition(0, 44);
		window.draw(outline);

		outline.setTextureRect(IntRect(40, 0, 16, 11));
		for (int i = 0; i < columns; i++) {
			outline.setPosition(12 + SPRITE_SIZE * i, 44);
			window.draw(outline);
		}

		outline.setTextureRect(IntRect(33, 21, 8, 11));
		outline.setPosition(12 + SPRITE_SIZE * columns, 44);
		window.draw(outline);

		outline.setTextureRect(IntRect(74, 0, 13, 16));
		for (int j = 0; j < rows; j++) {
			outline.setPosition(0, 55 + SPRITE_SIZE * j);
			window.draw(outline);
		}

		outline.setTextureRect(IntRect(88, 0, 8, 16));
		for (int j = 0; j < rows; j++) {
			outline.setPosition(12 + SPRITE_SIZE * columns, 55 + SPRITE_SIZE * j);
			window.draw(outline);
		}

		outline.setTextureRect(IntRect(20, 12, 12, 8));
		outline.setPosition(0, 55 + SPRITE_SIZE * columns);
		window.draw(outline);

		outline.setTextureRect(IntRect(42, 12, 16, 8));
		for (int i = 0; i < columns; i++) {
			outline.setPosition(12 + SPRITE_SIZE * i, 55 + SPRITE_SIZE * rows);
			window.draw(outline);
		}

		outline.setTextureRect(IntRect(33, 12, 8, 8));
		outline.setPosition(12 + SPRITE_SIZE * columns, 55 + SPRITE_SIZE * columns);
		window.draw(outline);

		// Draw the different smiley faces
		if(smileyPressed){
			infoSpace.setTextureRect(IntRect(130, 0, 26, 26));
		}else if (won) {
			infoSpace.setTextureRect(IntRect(234, 0, 26, 26));
		}else if (!lost)
			infoSpace.setTextureRect(IntRect(156, 0, 26, 26));
		else
			infoSpace.setTextureRect(IntRect(208, 0, 26, 26));
		infoSpace.setPosition(W / 2 - 13, 15);
		window.draw(infoSpace);

		// Draw rectangles behind the timers to get an outline.
		RectangleShape timerBackground1(Vector2f(40, 24));
		timerBackground1.setPosition(Vector2f(16, 15));
		timerBackground1.setFillColor(Color(128, 128, 128));
		window.draw(timerBackground1);

		RectangleShape timerBackground2(Vector2f(40, 24));
		timerBackground2.setPosition(Vector2f(17, 16));
		timerBackground2.setFillColor(Color::White);
		window.draw(timerBackground2);

		RectangleShape timerBackground3(Vector2f(40, 24));
		timerBackground3.setPosition(Vector2f(W - 55, 15));
		timerBackground3.setFillColor(Color(128, 128, 128));
		window.draw(timerBackground3);

		RectangleShape timerBackground4(Vector2f(40, 24));
		timerBackground4.setPosition(Vector2f(W - 54, 16));
		timerBackground4.setFillColor(Color::White);
		window.draw(timerBackground4);

		// Draw the bomb counter
		int bombs = bombsLeft();

		/* 
			If there are more flags than bombs, call drawDigit with an arbitrary negative number as the first digit.
			Then you make it positive.
		*/
		if (bombs < 0) {
			drawDigit(17, 16, -10);
			bombs = abs(bombs);
		}else
			drawDigit(17, 16, bombs / 100);
		drawDigit(30, 16, (bombs % 100) / 10);
		drawDigit(43, 16, (bombs % 100) % 10);

		// Draw the clock
		int time;
		if (!lost && !won) {
			time = clock.getElapsedTime().asSeconds();
			lastTime = time;
		}else {
			time = lastTime;
		}

		drawDigit(W - 54, 16, time / 100);
		drawDigit(W - 41, 16, (time % 100) / 10);
		drawDigit(W - 28, 16, (time % 100) % 10);
		
		// Draw all of the tiles
		for(int i = 0; i < columns; i++)
			for (int j = 0; j < rows; j++){
				tileSpace.setTextureRect(IntRect(tiles[i][j].getRenderIndex() * SPRITE_SIZE, 0, SPRITE_SIZE, SPRITE_SIZE));
				tileSpace.setPosition(tileSpaceX + i * SPRITE_SIZE, tileSpaceY + j * SPRITE_SIZE);
				window.draw(tileSpace);
			}

		window.display();
	}

	return 0;
}