#include "object.h"
#include "dungeon.h"

void generateObject(dungeon_t* d, std::vector<std::string>* artifacts)
{

    int x, y;
    object_description* od = &d->object_descriptions[randomInt(0, d->object_descriptions.size() - 1)];
    while(od->get_rarity() < randomInt(0, 99)) {
        od = &d->object_descriptions[randomInt(0, d->object_descriptions.size() - 1)];
    }
    if(od->get_artifact()) {
        while(od->get_artifact() &&
            (checkForVectorString(artifacts, od->get_name()) || checkForVectorString(&d->artifacts, od->get_name()))) {
            od = &d->object_descriptions[randomInt(0, d->object_descriptions.size() - 1)];
        }
    }
    if(od->get_artifact()) {
        artifacts->push_back(od->get_name());
    }
    x = randomInt(1, DUNGEON_WIDTH - 2);
    y = randomInt(1, DUNGEON_HEIGHT - 2);
    while((d->gameBoard[y][x] != blk_corridor && d->gameBoard[y][x] != blk_room) || d->objects[y][x] != nullptr) {
        x = randomInt(1, DUNGEON_WIDTH - 2);
        y = randomInt(1, DUNGEON_HEIGHT - 2);
    }
    std::unique_ptr<object> o(new object(od, y, x));
    d->objects[y][x] = std::move(o);
}

void clearObjects(dungeon_t* d)
{
    int x, y;
    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            d->objects[y][x].reset();
        }
    }
    
}

void generateAllObjects(dungeon_t* d)
{
    clearObjects(d);
    std::vector<std::string> artifacts;
    int i;
    memset(d->objects, 0, sizeof(d->objects));
    for(i = 0; i < d->numObjects; i++) {
        generateObject(d, &artifacts);
    }
    std::vector<std::string>().swap(artifacts);
}

void printObjects(dungeon_t* d)
{
    int x, y;
    std::string output;
    int count = 0;
    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            if(d->objects[y][x] != nullptr) {
                output = d->objects[y][x]->get_name();
                std::cout << x << "," << y << " -- " << ++count << ": " << output
                          << " artifact: " << d->objects[y][x]->get_artifact() << std::endl;
            }
        }
    }
}

void printObjectBoard(dungeon_t* d)
{
    int x, y;
    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            if(d->objects[y][x] != nullptr) {
                char output = object_symbol[d->objects[y][x]->get_type()];
                std::cout << output;
            } else {
                std::cout << (char)d->gameBoard[y][x];
            }
        }
        std::cout << std::endl;
    }
}
