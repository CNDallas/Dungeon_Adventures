#ifndef DUNGEON_H
#define DUNGEON_H

#define DUNGEON_HEIGHT 21
#define DUNGEON_WIDTH 80
#define PC 1024
#define MIN_ROOMS 5
#define MAX_ROOMS 9
#define ROOM_MIN_X 4
#define ROOM_MIN_Y 2
#define ROOM_MAX_X 4
#define ROOM_MAX_Y 8

#include "heap.h"

typedef enum dim { dim_x, dim_y, num_dims } dim_t;

typedef int pair_t[num_dims];

typedef enum block {
    blk_rock = ' ',
    blk_immutable_rock = '*',
    blk_room = '.',
    blk_corridor = '#',
    blk_stairs_up = '<',
    blk_stairs_down = '>'
} block;

enum stats {
    stats_dungeons_visted,
    stats_monsters_killed,
    stats_unique_monsters_killed,
    stats_artifacts_found,
    stats_monster_moves,
    stats_players_hp,
    stats_damage_dealt,
    stats_damage_received,
    stats_time_played
};

#include "monster_descriptions.h"
#include "monsters.h"
#include "object.h"
#include "object_descriptions.h"
#include <array>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <memory>
#include <utility>
#include <vector>

class roomStruct
{
public:
    int xCord;
    int yCord;
    int room_width;
    int room_height;

    roomStruct(int room_width, int room_height);
};

class character;

class monster;

class player;

class dungeon_t
{
public:
    std::array<std::array<block, DUNGEON_WIDTH>, DUNGEON_HEIGHT> gameBoard = {};
    std::array<std::array<int, DUNGEON_WIDTH>, DUNGEON_HEIGHT> hardness = {};
    std::array<std::array<int, DUNGEON_WIDTH>, DUNGEON_HEIGHT> tunnelPath = {};
    std::array<std::array<int, DUNGEON_WIDTH>, DUNGEON_HEIGHT> nonTunnelPath = {};
    std::shared_ptr<character> monsters[DUNGEON_HEIGHT][DUNGEON_WIDTH] = {};
    std::unique_ptr<object> objects[DUNGEON_HEIGHT][DUNGEON_WIDTH] = {};
    std::vector<std::string> artifacts = {};
    std::vector<std::string> uniqueMonsters = {};
    std::vector<monster_description> monster_descriptions = {};
    std::vector<object_description> object_descriptions = {};
    std::array<std::array<std::vector<std::array<char,2>>,DUNGEON_WIDTH>,DUNGEON_HEIGHT> replay = {};
    std::array<std::vector<int>,9> stats; //0-Dungeons visited, 
    heap_t turnHeap;
    int numMonsters;
    int numObjects;
    int numOfAliveMonsters;
    std::vector<std::shared_ptr<monster>> characterVector = {};
    std::vector<std::unique_ptr<roomStruct>> rooms = {};
    pair_t pcLoc = {};
    std::shared_ptr<player> ptrPC = nullptr;
    bool fog = true;

    dungeon_t(int numMonsters, int numObjects);
};

void printGame(dungeon_t* d);

void printGameHardness(dungeon_t* d);

int empty_dungeon(dungeon_t* d);

int place_rooms(dungeon_t* d);

int make_rooms(dungeon_t* d);

int gen_dungeon(dungeon_t* d);

int connect_two_rooms(dungeon_t* d, const roomStruct& room1, const roomStruct& room2);

int connect_rooms(dungeon_t* d);

int place_player(dungeon_t* d);

int place_stairs(dungeon_t* d);

int place_hardness(dungeon_t* d);

void clearRooms(dungeon_t* d);

void dijkstra_corridor(dungeon_t* d, pair_t from, pair_t to);

int mergeRooms(dungeon_t* d, int left, int middle, int right);

int mergeSortRooms(dungeon_t* d, int left, int right);

#endif
