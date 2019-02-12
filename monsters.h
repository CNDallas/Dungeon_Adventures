#ifndef MONSTERS_H
#define MONSTERS_H

#include "heap.h"
#include "path.h"

#ifdef __cplusplus

#include <cstdint>
#include <cstdio>
#include <vector>
#include <cstring>
#include <array>
#include <memory>
#include "object.h"

class dice;

class character {
public:
    int speed;
    int xLoc;
    int yLoc;
    int moveNum;
    int creationNum;
    bool dead;
    int hp;
    dice damage;
    char symbol;
    int color;
    unsigned int characteristics;
    //heap_node_t *hn;

    character(int speed, int xLoc, int yLoc, int moveNum, int creationNum, bool dead, int hp, dice damage,
              char symbol,
              int color, unsigned int characteristics);
    dice getDamage() const{
        return damage;
    }

    void setLocation(int x, int y);

    void increaseMove();

    void kill(dungeon_t *d);


};

class monster : public character {
public:
    std::string name;
    std::string desc;
    int rarity;
    int lastPCX;
    int lastPCY;
    bool descGenerated;

    monster(int speed, int moveNum, int creationNum, bool dead, int hp, dice damage, char symbol,
            int color, unsigned int characteristics, std::string &name, std::string &desc, int rarity, int lastPCX,
            int lastPCY, bool descGenerated, int xLoc = 0, int yLoc = 0);

    monster(const monster_description &m, int creationNum, int x = 0, int y = 0);
    const std::string& get_name() const
    {
        return name;
    }
};

class player : public character {
public:
    std::array<std::array<block, DUNGEON_WIDTH>, DUNGEON_HEIGHT> player_known = {blk_rock};
    std::array<std::unique_ptr<object>,12> equipment;
    std::array<std::unique_ptr<object>,10> inventory;
    player(int xLoc, int yLoc, dice damage);
    bool debugTools = false;
    
    int calculateDamagePC();
    int calculateSpeedPC();
};


extern "C" {
#endif

typedef struct dungeon_t dungeon_t;
int moveMonsters(dungeon_t *d);
int erraticMove(dungeon_t *d, character *p, int *x, int *y);
void generateMonsters(dungeon_t *d);
void printMonsterBoard(dungeon_t *d);
int smart_telepathy(dungeon_t *d, character *p, int *y, int *x, bool tunnel);
void generateAllCustomMonsters(dungeon_t *d);
void generatePlayer(dungeon_t *d);
void clearMonsters(dungeon_t *d);

#ifdef __cplusplus
}
#endif

#endif
