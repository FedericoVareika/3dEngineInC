CFLAGS = -Wall -std=c99 -I/opt/homebrew/include -L/opt/homebrew/lib
LIBS = -lSDL2 -lSDL2_ttf

PROGRAM_NAME = engine

FILES = ./src/*.c ./src/visuals/*.c ./src/math/*.c ./src/rendering/*.c

build: 
	gcc $(CFLAGS) -o $(PROGRAM_NAME) $(FILES) $(LIBS) 

run:
	./$(PROGRAM_NAME)

clean: 
	rm $(PROGRAM_NAME)

debug: 
	gcc $(CFLAGS) -g $(FILES) $(LIBS) 
	lldb a.out
