#include "utilities.h"
#include <iostream>
#include <random>
#include "dungeon.h"

bool checkForVectorString(std::vector<std::string>* vector, std::string string)
{
    if(vector->empty()) {
        return false;
    }
    std::vector<std::string>::iterator it;

    for(it = vector->begin(); it != vector->end(); it++) {
        if(*it == string) {
            return true;
        }
    }
    return false;
}

bool is_number (char* s){
    
    for(; *s != '\0'; s++){
        if(!std::isdigit(*s)){
            return false;
        }
    }
    return true;
}

int randomInt(int min, int max) // min and max inclusive
{
    if(min > max) {
        std::cout << "Error: " << min << " > " << max;
        return -1;
    }

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(min, max);
    return uni(rng);
}

bool bitChecker(unsigned int characteristic, int place)
{ // checks if the place bit in characteristics is a 1. Starts at 0
    return (characteristic & (1 << place)) > 0;
}

std::array<char, 2> SymbolColorGrabber(dungeon_t* d, int y, int x)
{
    std::array<char, 2> temp = { '?', '?' };
    if(d->monsters[y][x] != nullptr) {
        temp[0] = d->monsters[y][x]->symbol;
        temp[1] = d->monsters[y][x]->color;
    } else if(d->objects[y][x] != nullptr) {
        temp[0] = d->objects[y][x]->get_object_symbol();
        temp[1] = d->objects[y][x]->get_color();
    } else {
        temp[0] = d->gameBoard[y][x];
        temp[1] = COLOR_WHITE;
    }
    return temp;
}