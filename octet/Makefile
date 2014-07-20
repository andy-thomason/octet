

# boilerplate from http://stackoverflow.com/questions/714100/os-detecting-makefile
ifeq ($(OS),Windows_NT)
    SRC = $(shell dir/s/b src)
    CC = cl
    CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        CCFLAGS += -D AMD64
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
        CCFLAGS += -D IA32
    endif
else
    SRC = $(shell find src)
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CC = clang
        CCFLAGS += -g -O2 -Wswitch -D OSX -F/System/Library/Frameworks -framework GLUT -framework OpenGL -framework OpenAL -lstdc++ -DOCTET_PREFIX=\"\" -Iopen_source/bullet
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
    endif
endif

BINARIES = bin/example_box \
	bin/example_duck \
	bin/example_invaderers \
	bin/example_physics \
	bin/example_ping \
	bin/example_polygon \
	bin/example_sphere \
	bin/example_text \
	bin/example_triangle \
	#bin/example_particles \


all: $(BINARIES)

bin:
	mkdir -p bin

clean:
	rm -f $(BINARIES)


bin/example_box: src/examples/example_box/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_duck: src/examples/example_duck/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_invaderers: src/examples/example_invaderers/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_particles: src/examples/example_particles/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_physics: src/examples/example_physics/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_ping: src/examples/example_ping/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_polygon: src/examples/example_polygon/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_sphere: src/examples/example_sphere/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_text: src/examples/example_text/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

bin/example_triangle: src/examples/example_triangle/main.cpp $(SRC) | bin
	$(CC) $(CCFLAGS) $< -o $@

