#ifndef TILE_H
#define TILE_H

enum State {
	EMPTY, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, BOMB, FLAG, EITHER, TILE, EXPLODEDBOMB, NOTBOMB
};

class Tile
{
public:
	Tile();
	~Tile();

	State getHiddenState();
	State getVisibleState();
	int getRenderIndex();

	void setState(State state);
	void setState(int adjacentBombCount);
	bool isBomb();
	bool isMarked();
	bool isFlag();
	bool isHidden();
	bool isNumber();

	void show();
	void hide();

	bool leftClick();
	void rightClick();

	void reset();

private:
	State hiddenState;
	State visibleState;
	bool hidden;
};

#endif
