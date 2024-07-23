INC = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib
LDLIBS = -lSDL2 -lSDL2_ttf  
PROGRAM_NAME = engine

FILES = ./src/*.c ./src/visuals/*.c ./src/math/*.c ./src/rendering/*.c ./src/loading/*.c

CFLAGS = -Wall -std=c11 $(LDFLAGS) $(INC)
CFLAGS += -O2 

build: 
	gcc $(CFLAGS) -o $(PROGRAM_NAME) $(FILES) $(LDLIBS)

run:
	./$(PROGRAM_NAME)

clean: 
	rm $(PROGRAM_NAME)

debug: 
	gcc $(CFLAGS) -g $(FILES) $(LIBS) 
	lldb a.out
