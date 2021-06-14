CC = g++
CFLAGS = -g -Wall -O2
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

DEPS = src/main.cpp src/Tile.cpp

TARGET = minesweeper

all: $(TARGET)

$(TARGET): $(DEPS)
	$(CC) $(CFLAGS) $(DEPS) -o $(TARGET).o $(SFML_FLAGS)

clean:
	$(RM) $(TARGET).o