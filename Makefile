all: assignment8

assignment8: rlg327.cpp heap.h path.h monsters.h io.h  dice.h utilities.h object_descriptions.h monster_descriptions.h object.h
	g++ -Wall -Werror -ggdb rlg327.cpp dungeon.cpp dice.cpp heap.cpp path.cpp monsters.cpp io.cpp object_descriptions.cpp monster_descriptions.cpp object.cpp utilities.cpp -lncurses -o rlg327 

clean:
	rm rlg327 -f
