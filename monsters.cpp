#include "cmath"
#include "monsters.h"
#include <thread>

character::character(int speed,
    int xLoc,
    int yLoc,
    int moveNum,
    int creationNum,
    bool dead,
    int hp,
    dice damage,
    char symbol,
    int color,
    unsigned int characteristics)
    : speed(speed)
    , xLoc(xLoc)
    , yLoc(yLoc)
    , moveNum(moveNum)
    , creationNum(creationNum)
    , dead(dead)
    , hp(hp)
    , damage(damage)
    , symbol(symbol)
    , color(color)
    , characteristics(characteristics)
{
}

monster::monster(const monster_description& m, int creationNum, int x, int y)
    : character(m.getSpeed().roll(),
          x,
          y,
          0,
          creationNum,
          false,
          m.getHitPoints().roll(),
          m.getDamage(),
          m.getSymbol(),
          m.getFirstColor(),
          m.getAbilities())
    , name(m.getMonstersName())
    , desc(m.getDescription())
    , rarity(m.getRarity())
{
    lastPCY = 0;
    lastPCX = 0;
    descGenerated = true;
}

monster::monster(int speed,
    int moveNum,
    int creationNum,
    bool dead,
    int hp,
    dice damage,
    char symbol,
    int color,
    unsigned int characteristics,
    std::string& name,
    std::string& desc,
    int rarity,
    int lastPCX,
    int lastPCY,
    bool descGenerated,
    int xLoc,
    int yLoc)
    : character(speed, xLoc, yLoc, moveNum, creationNum, dead, hp, damage, symbol, color, characteristics)
    , name(name)
    , desc(desc)
    , rarity(rarity)
    , lastPCX(lastPCX)
    , lastPCY(lastPCY)
    , descGenerated(descGenerated)
{
}

player::player(int xLoc, int yLoc, dice damage)
    : character(10, xLoc, yLoc, 0, 0, false, 200, damage, '@', 0, PC)
{
}

void character::setLocation(int x, int y)
{
    xLoc = x;
    yLoc = y;
}

void character::increaseMove()
{
    moveNum += 1000 / speed;
}

void character::kill(dungeon_t* d)
{
    setLocation(0, 0);
    dead = true;
    d->numOfAliveMonsters--;
}

int32_t turn_cmp(const void* monster1, const void* monster2)
{
    if(static_cast<const character*>(monster2)->moveNum == static_cast<const character*>(monster1)->moveNum) {
        return static_cast<const character*>(monster1)->creationNum -
            static_cast<const character*>(monster2)->creationNum;
    }

    return (
        ((static_cast<const character*>(monster1))->moveNum) - ((static_cast<const character*>(monster2))->moveNum));
}

int player::calculateSpeedPC()
{
    int speed = 10;
    for(auto it = equipment.begin(); it != equipment.end(); it++) {
        if(*it != nullptr) {
            speed += (*it)->get_speed();
        }
    }
    return speed < 1? 1: speed;
}
int player::calculateDamagePC()
{
    int damage = (equipment[0] == nullptr) ? getDamage().roll() : 0;
    for(auto it = equipment.begin(); it != equipment.end(); it++) {
        if(*it != nullptr) {
            damage += (*it)->get_damage().roll();
        }
    }
    return damage;
}
int moveMonsters(dungeon_t* d)
{
    const character* temp;
    int damage = 0;
    int monsterMoves = 0;
    while((temp = static_cast<const character*>(heap_remove_min(&d->turnHeap)))) {
        int x = temp->xLoc;
        int y = temp->yLoc;
        if(!temp->dead) {
            auto p = d->monsters[y][x]; // Probably not the most elegant way to do this.
            if(p->characteristics == PC) {
                p->moveNum += 1000 / static_cast<player*>(p.get())->calculateSpeedPC();
                heap_insert(&d->turnHeap, p.get());

                if(d->stats[stats_damage_received].size() == d->replay[0][0].size() + 1) {
                    d->stats[stats_damage_received].back() = +damage;
                } else {
                    d->stats[stats_damage_received].push_back(damage);
                }
                if(d->stats[stats_monster_moves].size() == d->replay[0][0].size() + 1) {
                    d->stats[stats_monster_moves].back() = +monsterMoves;
                } else {
                    d->stats[stats_monster_moves].push_back(monsterMoves);
                }

                return 0;
            }
            monsterMoves++;
            bool erratic =
                bitChecker(p->characteristics, abilities_ERRATIC) ? static_cast<bool>(randomInt(0, 1)) : false;
            if(bitChecker(p->characteristics, abilities_SMART) && bitChecker(p->characteristics, abilities_TELE) &&
                !erratic) {                                            // Smart and Telepathic
                if(bitChecker(p->characteristics, abilities_TUNNEL)) { // TUNNEL
                    smart_telepathy(d, p.get(), &y, &x, true);
                } else { // NOT TUNNEL
                    smart_telepathy(d, p.get(), &y, &x, false);
                }
            } else if((bitChecker(p->characteristics, abilities_TELE)) && !erratic) { // Telepathic but not smart
                int tempX = p->xLoc;
                int tempY = p->yLoc;
                if(p->xLoc != d->pcLoc[dim_x]) { // If p's x location is same as pc's x location don't change
                    tempX += ((p->xLoc > d->pcLoc[dim_x]) ? -1 : 1);
                }
                if(p->yLoc != d->pcLoc[dim_y]) { // If p's y location is same as pc's y location don't change
                    tempY += ((p->yLoc > d->pcLoc[dim_y]) ? -1 : 1);
                }
                if(d->gameBoard[tempY][tempX] != blk_immutable_rock) {
                    if(bitChecker(p->characteristics, abilities_TUNNEL)) {
                        x = tempX;
                        y = tempY;
                    } else {
                        if(d->gameBoard[tempY][tempX] != blk_rock) {
                            x = tempX;
                            y = tempY;
                        }
                    }
                }
            } else {
                erraticMove(d, p.get(), &x, &y);
            }
            int move = 1;
            if(bitChecker(p->characteristics, abilities_TUNNEL) && d->gameBoard[y][x] == blk_rock) {
                d->hardness[y][x] = ((d->hardness[y][x] - 85) > 0) ? (d->hardness[y][x] - 85) : 0;
                if(d->hardness[y][x] == 0) {
                    d->gameBoard[y][x] = blk_corridor;
                } else {
                    move = 0;
                }
            }
            if(move && (x != p->xLoc || y != p->yLoc)) {
                if(d->monsters[y][x] != nullptr) { // If spot contains another creature
                    if(!(d->pcLoc[dim_y] == y && d->pcLoc[dim_x] == x)) {
                        int ranX, ranY, i;
                        for(i = 0; i < 8; i++) {
                            ranX = p->xLoc + randomInt(-1, 1);
                            ranY = p->yLoc + randomInt(-1, 1);
                            if(d->monsters[ranY][ranX] == nullptr) { // Find an open spot
                                if(bitChecker(d->monsters[y][x]->characteristics, abilities_TUNNEL)) {
                                    if(d->hardness[ranY][ranX] < 85) {
                                        d->hardness[ranY][ranX] = 0;
                                        d->gameBoard[ranY][ranX] = d->gameBoard[ranY][ranX] == blk_rock ?
                                            blk_corridor :
                                            d->gameBoard[ranY][ranX];
                                        break;
                                    }
                                } else if(d->gameBoard[ranY][ranX] == blk_corridor ||
                                    d->gameBoard[ranY][ranX] == blk_room ||
                                    d->gameBoard[ranY][ranX] == blk_stairs_down ||
                                    d->gameBoard[ranY][ranX] == blk_stairs_up) {
                                    break;
                                }
                            }
                        }

                        if(i == 8) { // Didn't find a new spot for the displaced monster in a reasonable amount of tries
                            d->monsters[y][x]->yLoc = p->yLoc;
                            d->monsters[y][x]->xLoc = p->xLoc;
                            d->monsters[y][x].swap(d->monsters[p->yLoc][p->xLoc]);
                            p->yLoc = y;
                            p->xLoc = x;
                        } else {
                            d->monsters[y][x]->yLoc = ranY;
                            d->monsters[y][x]->xLoc = ranX;
                            d->monsters[y][x].swap(d->monsters[ranY][ranX]);
                            d->monsters[y][x].swap(d->monsters[p->yLoc][p->xLoc]);
                            p->yLoc = y;
                            p->xLoc = x;
                        }

                    } else {
                        int tempDmg = p->damage.roll();
                        damage += tempDmg;
                        if((d->ptrPC->hp -= tempDmg) <= 0) {
                            d->ptrPC->dead = true;
                            d->monsters[y][x] = nullptr;
                            d->pcLoc[dim_x] = 0;
                            d->pcLoc[dim_y] = 0;
                            if(d->stats[stats_damage_received].size() == d->replay[0][0].size() + 1) {
                                d->stats[stats_damage_received].back() += damage;
                            } else {
                                d->stats[stats_damage_received].push_back(damage);
                            }
                            if(d->stats[stats_monster_moves].size() == d->replay[0][0].size() + 1) {
                                d->stats[stats_monster_moves].back() += monsterMoves;
                            } else {
                                d->stats[stats_monster_moves].push_back(monsterMoves);
                            }
                            return 1;
                        }
                    }
                } else {
                    d->monsters[y][x].swap(d->monsters[p->yLoc][p->xLoc]);
                    p->yLoc = y;
                    p->xLoc = x;
                }
            }
            p->moveNum += 1000 / p->speed;
            heap_insert(&d->turnHeap, p.get());
            p.reset();
        }
    }
    if(d->stats[stats_damage_received].size() == d->replay[0][0].size() + 1) {
        d->stats[stats_damage_received].back() += damage;
    } else {
        d->stats[stats_damage_received].push_back(damage);
    }
    if(d->stats[stats_monster_moves].size() == d->replay[0][0].size() + 1) {
        d->stats[stats_monster_moves].back() += monsterMoves;
    } else {
        d->stats[stats_monster_moves].push_back(monsterMoves);
    }
    return 0;
}

int erraticMove(dungeon_t* d, character* p, int* x, int* y)
{

    *y = p->yLoc + randomInt(-1, 1);
    *x = p->xLoc + randomInt(-1, 1);
    if(!(bitChecker(p->characteristics, abilities_TUNNEL))) {

        while(d->gameBoard[*y][*x] == blk_rock || d->gameBoard[*y][*x] == blk_immutable_rock ||
            (*x == p->xLoc && *y == p->yLoc)) {
            *y = p->yLoc + randomInt(-1, 1);
            *x = p->xLoc + randomInt(-1, 1);
        }
    } else {
        while(d->gameBoard[*y][*x] == blk_immutable_rock || (*x == p->xLoc && *y == p->yLoc)) {
            *y = p->yLoc + randomInt(-1, 1);
            *x = p->xLoc + randomInt(-1, 1);
        }
    }
    return 0;
}

int smart_telepathy(dungeon_t* d, character* p, int* y, int* x, bool tunnel)
{

    int i, j;
    int low = 1000;
    for(i = p->yLoc - 1; i <= p->yLoc + 1; i++) {
        for(j = p->xLoc - 1; j <= p->xLoc + 1; j++) {
            if(j > 1 && j < 78 && i > 1 && i < 19 && ((tunnel) ? d->tunnelPath[i][j] : d->nonTunnelPath[i][j]) < low) {
                *y = i;
                *x = j;
                low = tunnel ? d->tunnelPath[i][j] : d->nonTunnelPath[i][j];
            }
        }
    }
    return 0;
}

void generateMonsters(dungeon_t* d)
{
    int i;
    for(i = 0; i < d->numMonsters; i++) {
        bool monsterMade = false;
        while(!monsterMade) {
            int randX = randomInt(1, 79);
            int randY = randomInt(1, 20);
            if(d->monsters[randY][randX] == nullptr &&
                (d->gameBoard[randY][randX] == blk_corridor || d->gameBoard[randY][randX] == blk_room ||
                    d->gameBoard[randY][randX] == blk_stairs_down || d->gameBoard[randY][randX] == blk_stairs_up) &&
                (std::sqrt((d->pcLoc[dim_x] - randX) * (d->pcLoc[dim_x] - randX) +
                    (d->pcLoc[dim_y] - randY) * (d->pcLoc[dim_y] - randY))) >
                    4) { // Make sure that the PC isn't within a radius of 3

                unsigned int characteristics = randomInt(1, 15);
                std::unique_ptr<dice> damage(new dice(randomInt(0, 10), randomInt(0, 5), randomInt(0, 10)));
                std::unique_ptr<std::string> name(new std::string("Random Bob"));
                std::unique_ptr<std::string> desc(
                    new std::string("This monster was generated by a computer. How cool!"));

                std::shared_ptr<monster> m(
                    new monster(randomInt(5, 20), 0, i + 1, false, randomInt(100, 500), *damage, characteristics + 64,
                        characteristics % 8, characteristics, *name, *desc, randomInt(0, 100), 0, 0, false));
                m->setLocation(randX, randY);
                d->characterVector.push_back(m);
                d->monsters[randY][randX] = d->characterVector[i];
                heap_insert(&d->turnHeap, d->characterVector.back().get());
                monsterMade = true;
            }
        }
    }
}

void generateCustomMonster(dungeon_t* d, std::vector<std::string>* unique)
{
    int x, y;
    const monster_description* md = &d->monster_descriptions[randomInt(0, d->monster_descriptions.size() - 1)];
    while((int)md->getRarity() < randomInt(0, 99)) {
        md = &d->monster_descriptions[randomInt(0, d->monster_descriptions.size() - 1)];
    }
    if(bitChecker(md->getAbilities(), abilities_UNIQ)) {
        while(bitChecker(md->getAbilities(), abilities_UNIQ) &&
            (checkForVectorString(unique, md->getMonstersName()) ||
                checkForVectorString(&d->uniqueMonsters, md->getMonstersName()))) {
            md = &d->monster_descriptions[randomInt(0, d->monster_descriptions.size() - 1)];
        }
    }
    if(bitChecker(md->getAbilities(), abilities_UNIQ)) {
        unique->push_back(md->getMonstersName());
    }
    x = randomInt(1, DUNGEON_WIDTH - 2);
    y = randomInt(1, DUNGEON_HEIGHT - 2);
    while((d->gameBoard[y][x] != blk_corridor && d->gameBoard[y][x] != blk_room) ||
        ((std::sqrt((d->pcLoc[dim_x] - x) * (d->pcLoc[dim_x] - x) + (d->pcLoc[dim_y] - y) * (d->pcLoc[dim_y] - y))) <
            4) ||
        d->monsters[y][x] != nullptr) { // Make sure that the PC isn't within a radius of 3
        x = randomInt(1, DUNGEON_WIDTH - 2);
        y = randomInt(1, DUNGEON_HEIGHT - 2);
    }
    std::shared_ptr<monster> m(new monster(*md, d->characterVector.size() + 1));
    m->setLocation(x, y);
    d->monsters[y][x] = m;
    d->characterVector.push_back(m);
    heap_insert(&d->turnHeap, d->characterVector.back().get());
}

void generateAllCustomMonsters(dungeon_t* d)
{
    std::vector<std::string> unique;
    int i;
    for(i = 0; i < d->numMonsters; i++) {
        generateCustomMonster(d, &unique);
    }
    std::vector<std::string>().swap(unique);
}

void printMonsterBoard(dungeon_t* d)
{
    int i, j;
    for(i = 0; i < DUNGEON_HEIGHT; i++) {
        for(j = 0; j < DUNGEON_WIDTH; j++) {
            if(d->monsters[i][j] == nullptr) {
                printf("%c", d->gameBoard[i][j]);
            } else if(d->monsters[i][j]->characteristics == PC) {
                printf("@");
            } else {
                printf("%c", d->monsters[i][j]->symbol);
            }
        }
        printf("\n");
    }
}

void generateTurnHeap(dungeon_t* d)
{
    static bool init = false;
    if(!init) {
        heap_init(&d->turnHeap, turn_cmp, nullptr);
        init = true;
    } else {
        while(heap_remove_min(&d->turnHeap)) {
            ;
        }
        heap_init(&d->turnHeap, turn_cmp, nullptr);
    }
}

void resetPlayerFog(dungeon_t* d)
{
    int i, j;
    for(i = 0; i < DUNGEON_HEIGHT; i++) {
        for(j = 0; j < DUNGEON_WIDTH; j++) {
            if(i == 0 || j == 0 || i == DUNGEON_HEIGHT - 1 || j == DUNGEON_WIDTH - 1) {
                d->ptrPC.get()->player_known[i][j] = blk_immutable_rock;
            } else {
                d->ptrPC.get()->player_known[i][j] = blk_rock;
            }
        }
    }
}

void generatePlayer(dungeon_t* d)
{
    if(d->ptrPC == nullptr) {
        std::unique_ptr<dice> damage(new dice(0, 1, 4));
        std::shared_ptr<player> pc(new player(d->pcLoc[dim_x], d->pcLoc[dim_y], *damage));
        d->ptrPC = std::move(pc);
    } else {
        d->monsters[d->ptrPC->yLoc][d->ptrPC->xLoc] = nullptr;
        d->ptrPC->xLoc = d->pcLoc[dim_x];
        d->ptrPC->yLoc = d->pcLoc[dim_y];
        d->ptrPC->dead = false;
        d->ptrPC->moveNum = 0;
    }
    generateTurnHeap(d);
    heap_insert(&d->turnHeap, d->ptrPC.get());
    resetPlayerFog(d);
    clearMonsters(d);
    d->monsters[d->pcLoc[dim_y]][d->pcLoc[dim_x]] = d->ptrPC;
}

void clearMonsters(dungeon_t* d)
{
    for(std::shared_ptr<monster> m : d->characterVector) {
        if(!m->dead) {
            d->monsters[m->yLoc][m->xLoc] = nullptr;
        }
    }
    std::vector<std::shared_ptr<monster>>().swap(d->characterVector);
    d->numOfAliveMonsters = d->numMonsters;
}
