INC = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib
LDLIBS = -lSDL2 -lSDL2_ttf

PROGRAM_NAME = engine

FILES = ./src/*.c ./src/visuals/*.c ./src/math/*.c ./src/rendering/*.c ./src/loading/*.c

# CIMGUI
# Paths
# CIMGUI_PATH = ./extern/cimgui
# CIMGUI_BUILD_PATH = $(CIMGUI_PATH)/build

# Compiler flags
# LDFLAGS += -L./extern/cimgui 
# LDLIBS += -lcimgui

# -I$(CIMGUI_PATH)
# INC +=  -I$(CIMGUI_PATH) -I$(CIMGUI_PATH)/imgui
# INC +=  -I$(IMGUI_INCLDIR) -I$(IMGUI_IMPL_INCLDIR) -I$(INCLDIR)
CFLAGS = -Wall -std=c11 $(LDFLAGS) $(INC)

build: 
	gcc $(CFLAGS) -o $(PROGRAM_NAME) $(FILES) $(LDLIBS)

run:
	./$(PROGRAM_NAME)

clean: 
	rm $(PROGRAM_NAME)

debug: 
	gcc $(CFLAGS) -g $(FILES) $(LIBS) 
	lldb a.out
