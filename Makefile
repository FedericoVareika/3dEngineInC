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

ONOVECTORIZATION = -mno-sse -mno-avx
CFLAGS = -Wall -std=c11 $(LDFLAGS) $(INC)
OFLAGS = -fno-inline -ffp-contract=on $(ONOVECTORIZATION)
OFLAGS += -O1

build: 
	gcc $(CFLAGS) $(OFLAGS) -o $(PROGRAM_NAME) $(FILES) $(LDLIBS)

# buildO3: 
# 	gcc $(CFLAGS) -o $(PROGRAM_NAME) $(FILES) $(LDLIBS)

run:
	./$(PROGRAM_NAME)

clean: 
	rm $(PROGRAM_NAME)

debug: 
	gcc $(CFLAGS) -O1 -g $(FILES) $(LDLIBS) 
	lldb a.out
