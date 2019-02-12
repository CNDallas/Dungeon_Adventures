#include "dungeon.h"

#define rand_under(numerator, denominator) (rand() < ((RAND_MAX / (denominator)) * numerator))

typedef struct corridor_path {
    heap_node_t* hn;
    int pos[2];
    int from[2];
    int cost;
} corridor_path_t;

/*
 * prints the current gameboard to system use char symbols
 */
void printGame(dungeon_t* d)
{
    int i, j;
    for(i = 0; i < DUNGEON_HEIGHT; i++) {
        for(j = 0; j < DUNGEON_WIDTH; j++) {
            printf("%c", d->gameBoard[i][j]);
        }
        printf("\n");
    }
}

/*
 * Prints the current gambeboard to system using hardness
 * Used for debugging and will be removed from final
 */
void printGameHardness(dungeon_t* d)
{
    int i, j;
    for(i = 0; i < DUNGEON_HEIGHT; i++) {
        for(j = 0; j < DUNGEON_WIDTH; j++) {
            if(d->hardness[i][j] == 255) {
                printf("*");
            } else if(d->hardness[i][j] == 0) {
                if(d->gameBoard[i][j] == blk_room) {
                    printf(".");
                } else {
                    printf("'");
                }
            } else { // prints a range of hardness using 0-9 (change to %26 + 65 to get A-Z)
                printf("%c", d->hardness[i][j] % 10 + 48);
            }
        }
        printf("\n");
    }
}

int connect_rooms(dungeon_t* d)
{
    int i;
    mergeSortRooms(d, 0, (int)d->rooms.size() - 1);

    for(i = (int)d->rooms.size() - 1; i > 0; i--) {
        connect_two_rooms(d, *d->rooms[i - 1], *d->rooms[i]);
    }
    return 0;
}

int connect_two_rooms(dungeon_t* d, const roomStruct& room1, const roomStruct& room2)
{
    // printf("%d,%d\n",room1.xCord,room2.xCord);
    // std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    int currentSpotx;
    int currentSpoty;
    int goalX;
    int goalY;

    if(room1.xCord < room2.xCord) {
        currentSpotx = room1.xCord + room1.room_width - 1;
        currentSpoty = room1.yCord + randomInt(0, room1.room_height - 1);
        goalX = room2.xCord - 1;
        goalY = room2.yCord + randomInt(0, room2.room_height - 1);
    } else {
        currentSpotx = room2.xCord + room2.room_width - 1;
        currentSpoty = room2.yCord + randomInt(0, room2.room_height - 1);
        goalX = room1.xCord - 1;
        goalY = room1.yCord + randomInt(0, room1.room_height - 1);
    }
    pair_t from;
    pair_t to;
    from[dim_y] = currentSpoty;
    from[dim_x] = currentSpotx;
    to[dim_y] = goalY;
    to[dim_x] = goalX;
    dijkstra_corridor(d, from, to);

    return 1;
}

int corridor_path_cmp(const void* key, const void* with)
{
    return ((corridor_path_t*)key)->cost - ((corridor_path_t*)with)->cost;
}

void dijkstra_corridor(dungeon_t* d, pair_t from, pair_t to)
{
    static corridor_path_t path[DUNGEON_HEIGHT][DUNGEON_WIDTH], *p;
    static int initialized = 0;
    heap_t h;
    int x, y;

    if(!initialized) {
        for(y = 0; y < DUNGEON_HEIGHT; y++) {
            for(x = 0; x < DUNGEON_WIDTH; x++) {
                path[y][x].pos[dim_y] = y;
                path[y][x].pos[dim_x] = x;
            }
        }
        initialized = 1;
    }

    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            path[y][x].cost = INT_MAX;
        }
    }

    path[from[dim_y]][from[dim_x]].cost = 0;

    heap_init(&h, corridor_path_cmp, nullptr);

    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            if(d->gameBoard[y][x] != blk_immutable_rock) {
                path[y][x].hn = heap_insert(&h, &path[y][x]);
            } else {
                path[y][x].hn = nullptr;
            }
        }
    }

    while((p = (corridor_path_t*)heap_remove_min(&h))) {
        p->hn = nullptr;
        if((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
            for(x = to[dim_x], y = to[dim_y]; (x != from[dim_x]) || (y != from[dim_y]);
                p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
                if(d->gameBoard[y][x] != blk_room) {
                    d->gameBoard[y][x] = blk_corridor;
                    d->hardness[y][x] = 0;
                }
            }
            heap_delete(&h);
            return;
        }

        int yChg, xChg;
        for(yChg = -1; yChg <= 1; yChg++) {
            for(xChg = -1; xChg <= 1; xChg++) {
                if(!((xChg - yChg) == 1 || (xChg - yChg) == -1)) {
                } else {
                    if((path[p->pos[dim_y] + yChg][p->pos[dim_x] + xChg].hn != nullptr) &&
                        (path[p->pos[dim_y] + yChg][p->pos[dim_x] + xChg].cost >
                            p->cost + d->hardness[p->pos[dim_y]][p->pos[dim_x]])) {
                        path[p->pos[dim_y] + yChg][p->pos[dim_x] + xChg].cost =
                            p->cost + d->hardness[p->pos[dim_y]][p->pos[dim_x]];
                        path[p->pos[dim_y] + yChg][p->pos[dim_x] + xChg].from[dim_y] = p->pos[dim_y];
                        path[p->pos[dim_y] + yChg][p->pos[dim_x] + xChg].from[dim_x] = p->pos[dim_x];
                        heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + yChg][p->pos[dim_x] + xChg].hn);
                    }
                }
            }
        }
    }
}

int mergeRooms(dungeon_t* d, int left, int middle, int right)
{
    int i, j, k;
    int sizeLeft = middle - left + 1;
    int sizeRight = right - middle;
    std::unique_ptr<roomStruct>* L[sizeLeft];
    std::unique_ptr<roomStruct>* R[sizeRight];
    for(i = 0; i < sizeLeft; i++) {
        L[i] = &d->rooms[left + i];
    }
    for(j = 0; j < sizeRight; j++) {
        R[j] = &d->rooms[middle + 1 + j];
    }
    i = j = 0;
    k = left;
    while(i < sizeLeft && j < sizeRight) {
        if(L[i]->get()->xCord <= R[j]->get()->xCord) {
            d->rooms[k++].swap(*L[i++]);
        } else {
            d->rooms[k++].swap(*R[j++]);
        }
    }

    while(i < sizeLeft) {
        d->rooms[k++].swap(*L[i++]);
    }
    while(j < sizeRight) {
        d->rooms[k++].swap(*R[j++]);
    }
    return 0;
}

int mergeSortRooms(dungeon_t* d, int left, int right)
{
    if(left < right) {
        int middle = left + (right - left) / 2;
        mergeSortRooms(d, left, middle);
        mergeSortRooms(d, middle + 1, right);
        mergeRooms(d, left, middle, right);
    }
    return 0;
}

int empty_dungeon(dungeon_t* d)
{
    uint8_t x, y;
    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            d->gameBoard[y][x] = blk_rock;
            if(y == 0 || y == DUNGEON_HEIGHT - 1 || x == 0 || x == DUNGEON_WIDTH - 1) {
                d->gameBoard[y][x] = blk_immutable_rock;
                d->hardness[y][x] = 255;
            }
        }
    }

    return 0;
}

int place_rooms(dungeon_t* d)
{
    pair_t p;
    int i;
    int success;
    for(success = 0; !success;) {
        success = 1;
        int size = (int)d->rooms.size();
        for(i = 0; success && i < size; i++) {
            d->rooms[i]->xCord = randomInt(1, DUNGEON_WIDTH - 3 - d->rooms[i]->room_width);
            d->rooms[i]->yCord = randomInt(1, DUNGEON_HEIGHT - 3 - d->rooms[i]->room_height);
            for(p[dim_y] = d->rooms[i]->yCord - 1;
                success && p[dim_y] < d->rooms[i]->yCord + d->rooms[i]->room_height + 1; p[dim_y]++) {
                for(p[dim_x] = d->rooms[i]->xCord - 1;
                    success && p[dim_x] < d->rooms[i]->xCord + d->rooms[i]->room_width + 1; p[dim_x]++) {
                    if(d->gameBoard[p[dim_y]][p[dim_x]] >= blk_corridor) {
                        success = 0;
                        empty_dungeon(d);
                    } else if((p[dim_y] != d->rooms[i]->yCord - 1) &&
                        (p[dim_y] != d->rooms[i]->yCord + d->rooms[i]->room_height) &&
                        (p[dim_x] != d->rooms[i]->xCord - 1) &&
                        (p[dim_x] != d->rooms[i]->xCord + d->rooms[i]->room_width)) {
                        d->gameBoard[p[dim_y]][p[dim_x]] = blk_room;
                        d->hardness[p[dim_y]][p[dim_x]] = 0;
                    }
                }
            }
        }
    }

    return 0;
}

int make_rooms(dungeon_t* d)
{
    int i, numRooms;
    for(i = MIN_ROOMS; i < MAX_ROOMS && rand_under(6, 8); i++)
        ;
    numRooms = i;
    for(i = 0; i < numRooms; i++) {
        std::unique_ptr<roomStruct> room(new roomStruct(ROOM_MIN_X, ROOM_MIN_Y));
        d->rooms.push_back(std::move(room));
        while(rand_under(3, 4) && d->rooms[i]->room_width < ROOM_MAX_X) {
            d->rooms[i]->room_width++;
        }
        while(rand_under(3, 4) && d->rooms[i]->room_height < ROOM_MAX_Y) {
            d->rooms[i]->room_height++;
        }
    }

    return 0;
}

void clearRooms(dungeon_t* d)
{
    std::vector<std::unique_ptr<roomStruct>>().swap(d->rooms);
}

int gen_dungeon(dungeon_t* d)
{    
    empty_dungeon(d);
    clearRooms(d);
    do {
        make_rooms(d);
    } while(place_rooms(d));
    connect_rooms(d);
    place_player(d);
    place_stairs(d);
    place_hardness(d);
    return 0;
}

int place_player(dungeon_t* d)
{
    int ranRoom = randomInt(0, (int)d->rooms.size() - 1);
    d->pcLoc[dim_x] = d->rooms[ranRoom]->xCord + randomInt(0, d->rooms[ranRoom]->room_width - 1);
    d->pcLoc[dim_y] = d->rooms[ranRoom]->yCord + randomInt(0, d->rooms[ranRoom]->room_height - 1);
    return 0;
}

int place_stairs(dungeon_t* d)
{

    int ranX = randomInt(1, DUNGEON_WIDTH - 1);
    int ranY = randomInt(1, DUNGEON_HEIGHT - 1);
    while(d->gameBoard[ranY][ranX] != blk_corridor) {
        ranX = randomInt(1, DUNGEON_WIDTH - 1);
        ranY = randomInt(1, DUNGEON_HEIGHT - 1);
    }
    d->gameBoard[ranY][ranX] = blk_stairs_down;
    while(d->gameBoard[ranY][ranX] != blk_corridor) {
        ranX = randomInt(1, DUNGEON_WIDTH - 1);
        ranY = randomInt(1, DUNGEON_HEIGHT - 1);
    }

    d->gameBoard[ranY][ranX] = blk_stairs_up;

    return 0;
}

int place_hardness(dungeon_t* d)
{

    int x, y;
    for(y = 1; y < DUNGEON_HEIGHT - 1; y++) {
        for(x = 1; x < DUNGEON_WIDTH - 1; x++) {
            if(d->gameBoard[y][x] == blk_rock) {
                d->hardness[y][x] = randomInt(0, 254);
            }
        }
    }

    return 0;
}

roomStruct::roomStruct(int room_width, int room_height)
    : room_width(room_width)
    , room_height(room_height)
{
}

dungeon_t::dungeon_t(int numMonsters, int numObjects)
    : numMonsters(numMonsters)
    , numObjects(numObjects)
    , numOfAliveMonsters(numMonsters)
{ 
}
