INC = -Iinclude -I/opt/homebrew/include
CFLAGS = -Wall -std=c99 -L/opt/homebrew/lib $(INC)
LIBS = -lSDL2 -lSDL2_ttf

PROGRAM_NAME = engine

FILES = ./src/*.c ./src/visuals/*.c ./src/math/*.c ./src/rendering/*.c ./src/loading/*.c

build: 
	gcc $(CFLAGS) -o $(PROGRAM_NAME) $(FILES) $(LIBS) 

run:
	./$(PROGRAM_NAME)

clean: 
	rm $(PROGRAM_NAME)

debug: 
	gcc $(CFLAGS) -g $(FILES) $(LIBS) 
	lldb a.out
