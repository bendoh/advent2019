all: run

advent-2019:
	g++ -std=c++11 -o bin/advent-2019 advent-2019.cpp

run: advent-2019
	# bin/advent-2019 1 <inputs/2019/1
	bin/advent-2019 2 <inputs/2019/2
		# 146 before changing positions 1->12 and 2->3
