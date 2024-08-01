INC = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib
LDLIBS = -lSDL2 -lSDL2_ttf  
PROGRAM_NAME = engine

FILES = ./src/*.c 
FILES += ./src/visuals/*.c 
FILES += ./src/math/*.c 
FILES += ./src/rendering/*.c 
FILES += ./src/loading/*.c 
FILES += ./src/data_structures/*.c

CFLAGS = -Wall -std=c11 $(LDFLAGS) $(INC)
# CFLAGS += -O2
CFLAGS += -O0

build: 
	gcc $(CFLAGS) -o $(PROGRAM_NAME) $(FILES) $(LDLIBS)

run:
	./$(PROGRAM_NAME)

# run_map:
# 	./$(PROGRAM_NAME) ArtisansHub
	# ./$(PROGRAM_NAME) ArtisansHub Low

clean: 
	rm $(PROGRAM_NAME)

debug: 
	gcc $(CFLAGS) -g $(FILES) $(LDLIBS) 
	lldb a.out
