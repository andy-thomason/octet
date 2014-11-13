
# /home/andy/data/clang+llvm-3.4.2-x86_64-linux-gnu-ubuntu-14.04/bin/clang
# /home/andy/data/clang+llvm-3.4.1-x86_64-unknown-ubuntu12.04/bin/clang
# /home/andy/data/clang+llvm-3.2-x86_64-linux-ubuntu-12.04/bin/clang
# /home/andy/data/clang+llvm-3.0-x86_64-linux-Ubuntu-11_04/bin/clang
# /home/andy/data/clang+llvm-3.3-amd64-Ubuntu-12.04.2/bin/clang


ifeq ($(OS),Windows_NT)
    SRC = $(shell find src)
    CC = cl
    CCFLAGS += -D WIN32 -D OCTET_WIN32 gdi32.lib user32.lib shell32.lib
    O = /Fe
    EXE = .exe
else
    SRC = $(shell find src)
    UNAME_S := $(shell uname -s)
    O = -o
    EXE = 
    ifeq ($(UNAME_S),Linux)
	EXE=
        CC = clang -I /usr/include/x86_64-linux-gnu/ -I/usr/include/x86_64-linux-gnu/c++/4.8 -fno-inline
        CCFLAGS += -w -g -O2 -D OCTET_LINUX -Iopen_source/bullet -lstdc++ -lm -lglut -lGL -lopenal

    endif
    ifeq ($(UNAME_S),Darwin)
        CC = clang
        CCFLAGS += -g -O2 -Wswitch -D OCTET_MAC -F/System/Library/Frameworks -framework GLUT -framework OpenGL -framework OpenAL -lstdc++ -DOCTET_PREFIX=\"\" -Iopen_source/bullet
    endif
endif

# add your binary here
BINARIES = \
	bin/example_box$(EXE) \
	bin/example_duck$(EXE) \
	bin/example_invaderers$(EXE) \
	bin/example_particles$(EXE) \
	bin/example_shapes$(EXE) \
	bin/example_ping$(EXE) \
	bin/example_polygon$(EXE) \
	bin/example_shader$(EXE) \
	bin/example_sphere$(EXE) \
	bin/example_text$(EXE) \
	bin/example_texture$(EXE) \
	bin/example_triangle$(EXE) \
	bin/example_cellular$(EXE) \
	bin/example_lod$(EXE) \
	bin/example_rollercoaster$(EXE) \


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

bin/example_shapes$(EXE): src/examples/example_shapes/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_ping$(EXE): src/examples/example_ping/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_polygon$(EXE): src/examples/example_polygon/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_sphere$(EXE): src/examples/example_sphere/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_text$(EXE): src/examples/example_text/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_texture$(EXE): src/examples/example_texture/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_triangle$(EXE): src/examples/example_triangle/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_shader$(EXE): src/examples/example_shader/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_compute$(EXE): src/examples/example_compute/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_cubemap$(EXE): src/examples/example_cubemap/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

bin/example_rollercoaster$(EXE): src/examples/example_rollercoaster/main.cpp $(SRC)
	$(CC) $(CCFLAGS) $< $O$@

