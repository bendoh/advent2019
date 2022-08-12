all: run

CPP_ARGS=\
	-Wall \
	-Wno-missing-braces \
	-arch x86_64 \
	-std=c++17 \
	-mmacosx-version-min=10.15 \
	-framework OpenGL \
    -framework GLUT \
	-framework Carbon \
	-lpng
advent-2019:
	g++ $(CPP_ARGS) -o bin/advent-2019 advent-2019.cpp

run: advent-2019
	# bin/advent-2019 1 <inputs/2019/1
	bin/advent-2019 2 <inputs/2019/2
