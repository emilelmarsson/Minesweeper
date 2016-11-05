#include "Tile.h"

Tile::Tile(){
	reset();
}

Tile::~Tile(){

}

State Tile::getHiddenState() {
	return hiddenState;
}

State Tile::getVisibleState() {
	return visibleState;
}

int Tile::getRenderIndex() {
	if (hidden) {
		if (visibleState == EITHER)
			return 12;
		if (visibleState == FLAG)
			return 13;
		return 14;
	}
	
	if (hiddenState == EXPLODEDBOMB)
		return 11;
	if (hiddenState == NOTBOMB)
		return 10;

	return hiddenState;
}

void Tile::setState(State state) {
	hiddenState = state;
}

void Tile::setState(int adjacentBombCount) {
	if (adjacentBombCount < 1 || adjacentBombCount > 8)
		hiddenState = State::EMPTY;
	else
		hiddenState = static_cast<State>(adjacentBombCount);
}

bool Tile::isBomb(){
	return hiddenState == BOMB;
}

bool Tile::isMarked(){
	return visibleState == FLAG || visibleState == EITHER;
}

bool Tile::isFlag() {
	return visibleState == FLAG;
}

bool Tile::isHidden() {
	return hidden;
}

bool Tile::isNumber() {
	return hiddenState > 0 && hiddenState < 9;
}

void Tile::show(){
	hidden = false;
}

void Tile::hide(){
	hidden = true;
}

bool Tile::leftClick(){
	if (isMarked())
		return false;

	hidden = false;

	if (hiddenState == BOMB) {
		hiddenState = EXPLODEDBOMB;
		return true;
	}

	return false;
}

void Tile::rightClick(){
	switch(visibleState){
		case TILE:
			visibleState = FLAG;
			break;
		case FLAG:
			visibleState = EITHER;
			break;
		case EITHER:
			visibleState = TILE;
			break;
		default:
			visibleState = FLAG;
	}
}

void Tile::reset() {
	hiddenState = EMPTY;
	visibleState = TILE;
	hidden = true;
}
