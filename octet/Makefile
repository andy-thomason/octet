

ifeq ($(OS),Windows_NT)
    SRC = $(shell find src)
    CC = cl
    CCFLAGS += -D WIN32 gdi32.lib user32.lib shell32.lib -D OCTET_PREFIX
    O = /Fe
    EXE = .exe
else
    SRC = $(shell find src)
    UNAME_S := $(shell uname -s)
    O = -o
    EXE = 
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CC = clang
        CCFLAGS += -g -O2 -Wswitch -D OSX -F/System/Library/Frameworks -framework GLUT -framework OpenGL -framework OpenAL -lstdc++ -DOCTET_PREFIX=\"\" -Iopen_source/bullet
    endif
endif

# add your binary here
BINARIES = bin/example_box$(EXE) \
	bin/example_duck$(EXE)  \
	bin/example_invaderers$(EXE)  \
	bin/example_physics$(EXE)  \
	bin/example_ping$(EXE)  \
	bin/example_polygon$(EXE)  \
	bin/example_sphere$(EXE)  \
	bin/example_text$(EXE)  \
	bin/example_triangle$(EXE)  \
	#bin/example_particles$(EXE)  \


all: $(BINARIES)

clean:
	rm -f $(BINARIES)


bin/example_box$(EXE): src/examples/example_box/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_duck$(EXE): src/examples/example_duck/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_invaderers$(EXE): src/examples/example_invaderers/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_particles$(EXE): src/examples/example_particles/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_physics$(EXE): src/examples/example_physics/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_ping$(EXE): src/examples/example_ping/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_polygon$(EXE): src/examples/example_polygon/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_sphere$(EXE): src/examples/example_sphere/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_text$(EXE): src/examples/example_text/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_triangle$(EXE): src/examples/example_triangle/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

