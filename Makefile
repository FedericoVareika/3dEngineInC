build: 
	gcc -Wall -std=c99 -I/opt/homebrew/include -L/opt/homebrew/lib -o raster ./src/*.c -lSDL2 -lSDL2_ttf

run:
	./raster

clean: 
	rm raster
