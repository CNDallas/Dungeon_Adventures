#ifndef IO_H
#define IO_H

#include "dungeon.h"
#include "monsters.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dungeon_t dungeon_t;
typedef struct _win_st WINDOW;

void initTerminal();
void startScreen(dungeon_t *d);
void playerSettings(WINDOW* startWin, dungeon_t* d,int *player_health,int *player_speed,bool *player_debugTools, std::unique_ptr<dice> *player_damage);
int playerControls(dungeon_t *d);
void playerReplay(dungeon_t *d);
void resetReplay(dungeon_t *d);
void ioPrintMonsterBoard(dungeon_t *d, WINDOW *gameWin);
int mv_player(dungeon_t *d, WINDOW *gameWin, int relativeY, int relativeX);
WINDOW *message(const char *msg);
void stairs(dungeon_t *d, WINDOW *gameWin, int up);
void monsterDisplay(dungeon_t *d);
void monsterList(dungeon_t *d, WINDOW *monsterWin, int start);
void ioPrintQuit(WINDOW *gameWin);
void ioLoseGame(WINDOW *gameWin);
void ioWinGame(WINDOW *gameWin);
int updateFog(dungeon_t *d);
int teleport(dungeon_t *d, WINDOW *gameWin);
void inventory (dungeon_t *d, WINDOW* gameWin);
void equipment (dungeon_t *d, WINDOW* gameWin);
void itemDescription (dungeon_t *d,int index, bool isInEquipement);
void monsterTeleport(dungeon_t *d, WINDOW *gameWin);
void monsterDescription (dungeon_t *d,int x, int y);

#ifdef __cplusplus
}
#endif

#endif
