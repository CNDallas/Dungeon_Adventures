#include "io.h"
#include "monster_descriptions.h"
#include "object_descriptions.h"
#include <chrono>
#include <thread>

#define maxTwoNums(a, b) ((a) > (b) ? (a) : (b))
#define minTwoNums(a, b) ((a) < (b) ? (a) : (b))

void initTerminal()
{
    initscr();
    if(has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_BLACK, COLOR_WHITE, COLOR_BLACK);
    init_pair(8, COLOR_BLACK, COLOR_WHITE);
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    refresh();
}

void startScreen(dungeon_t* d)
{
    WINDOW* backDrop = newwin(24, 80, 0, 0);
    WINDOW* startWin = newwin(7, 78, 16, 1);
    wrefresh(backDrop);
    const char* title = "                   ////////-     `///            .:+oo+/.                       "
                        "                   mNNNmmmNNm/    -NNN`         /hNNNmmmNm                      "
                        "                   mNN+```sNNm    -NNN`        sNNm+.` `                        "
                        "          ------   mNNmdddNNm/    -NNN`       `NNN+   hhyhh/   ------           "
                        "          ======   mNNy+dNNd-     -NNN`        dNNy`    mNN+   ======           "
                        "          ------   mNN+ `omNm/    -NNNssssss-  .hNNds++sNNN+   ------           "
                        "                   ydd/   -hdho   -ddddddddd:   `:shddddhs/`                    "
                        "                                                ````                            "
                        "           ./+-        :oooooooo   ./oyyyo:`  .sssssssss.       ./+-            "
                        "           sNNm`       +hhhmNNNh   omdhymNNs  -hhhhhmNNN-       sNNm`           "
                        "           `:/.          :yNNm/`     `./dNm+   ````:mNN/        `:/.            "
                        "                         hhhmNmy`  ..`./dNm+      :mNN/                         "
                        "                            `mNN+  :odmNm+`      :mNN/                          "
                        "                       mdyssdNNm.  mmmmmmmmm-    :NNN:                          "
                        "                       /shhhhs/`   `````````     .yhh:                          ";

    mvwaddstr(backDrop, 1, 0, title);
    wrefresh(backDrop);
    getMonstersFromFile(d);
    parse_object_descriptions(d);
    int player_health = 200;
    int player_speed = 10;
    bool player_debugTools = false;
    std::unique_ptr<dice> player_damage(new dice(0, 1, 4));
    int index = 0;
    int refresh = true;
    bool toContinue = true;
    while(toContinue) {
        if(refresh == true) {
            werase(startWin);
            werase(backDrop);
            mvwaddstr(backDrop, 1, 0, title);
            wattron(startWin, COLOR_PAIR(COLOR_YELLOW));
            mvwaddstr(startWin, 1, 30, "Start Game");
            wattron(startWin, COLOR_PAIR(COLOR_GREEN));
            mvwaddstr(startWin, 2, 30, "Settings");
            wattron(startWin, COLOR_PAIR(COLOR_BLUE));
            mvwaddstr(startWin, 3, 30, "Replay Last Game");
            wattroff(startWin, COLOR_PAIR(COLOR_BLUE));
            mvwaddstr(startWin, 4, 30, "Quit Game");
            wrefresh(backDrop);
            refresh = false;
        }
        mvwaddstr(startWin, 1 + index, 28, ">");
        wrefresh(startWin);
        int key = getch();
        if(key == KEY_DOWN) {
            mvwaddstr(startWin, 1 + index, 28, " ");
            index = index == 3 ? 0 : index + 1;
        } else if(key == KEY_UP) {
            mvwaddstr(startWin, 1 + index, 28, " ");
            index = index == 0 ? 3 : index - 1;
        } else if(key == 10) {
            if(index == 0) {
                gen_dungeon(d);
                generatePlayer(d);
                generateAllCustomMonsters(d);
                generateAllObjects(d);
                dijkstra_nontunnel(d);
                dijkstra_tunnel(d);
                moveMonsters(d);
                d->uniqueMonsters.clear();
                d->artifacts.clear();
                d->ptrPC->hp = player_health;
                d->ptrPC->speed = player_speed;
                d->ptrPC->damage = *player_damage;
                d->ptrPC->debugTools = player_debugTools;
                d->fog = true;
                resetReplay(d);
                for(int i = 0; i < 12; i++) {
                    if(i < 10) {
                        d->ptrPC->inventory[i].reset(nullptr);
                    }
                    d->ptrPC->equipment[i].reset(nullptr);
                }
                playerControls(d);
                refresh = true;
            } else if(index == 1) {
                playerSettings(startWin, d, &player_health, &player_speed, &player_debugTools, &player_damage);
                refresh = true;
            } else if(index == 2) {
                playerReplay(d);
                refresh = true;
            } else {
                wclear(backDrop);
                wclear(startWin);
                int i;
                mvwaddstr(backDrop, 11, 25, "Exiting the game in    seconds!");
                for(i = 5; i > 0; i--) {
                    char result[2];
                    sprintf(result, "%d", i);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    mvwaddstr(backDrop, 11, 45, result);
                    wrefresh(backDrop);
                }
                delwin(startWin);
                delwin(backDrop);
                toContinue = false;
                endwin();
                exit(1);
            }
        }
    }
}

void playerSettings(WINDOW* startWin,
    dungeon_t* d,
    int* player_health,
    int* player_speed,
    bool* player_debugTools,
    std::unique_ptr<dice>* player_damage)
{
    werase(startWin);
    int numMonsters = d->numMonsters;
    int numObjects = d->numObjects;
    int monNumLoc = 50;
    int objNumLoc = 49;
    wattron(startWin, COLOR_PAIR(COLOR_YELLOW));
    std::string m = "Number of Monsters: " + std::to_string(numMonsters);
    mvwaddstr(startWin, 1, 30, m.c_str());
    wattron(startWin, COLOR_PAIR(COLOR_GREEN));
    std::string o = "Number of Objects: " + std::to_string(numObjects);
    mvwaddstr(startWin, 2, 30, o.c_str());
    wattron(startWin, COLOR_PAIR(COLOR_BLUE));
    mvwaddstr(startWin, 3, 30, "Save Settings");
    wattroff(startWin, COLOR_PAIR(COLOR_BLUE));
    mvwaddstr(startWin, 4, 30, "Return To Lobby");
    mvwaddstr(startWin, 6, 13, "Use Left and Right to adjust settings or type a number");
    int index = 0;

    bool end = false;
    while(!end) {
        mvwaddstr(startWin, 1 + index, 28, ">");
        if(index == 0) {
            wattron(startWin, COLOR_PAIR(8));
            wmove(startWin, index + 1, monNumLoc + 1);
            char output = winch(startWin);
            waddch(startWin, output);
            wattroff(startWin, COLOR_PAIR(8));

            wattron(startWin, COLOR_PAIR(COLOR_GREEN));
            wmove(startWin, 2, objNumLoc);
            output = winch(startWin);
            waddch(startWin, output);

            wmove(startWin, 2, objNumLoc + 1);
            output = winch(startWin);
            waddch(startWin, output);
            wattroff(startWin, COLOR_PAIR(COLOR_GREEN));
        } else if(index == 1) {
            wattron(startWin, COLOR_PAIR(8));
            wmove(startWin, index + 1, objNumLoc + 1);
            char output = winch(startWin);
            waddch(startWin, output);

            wattron(startWin, COLOR_PAIR(COLOR_YELLOW));
            wmove(startWin, 1, monNumLoc);
            output = winch(startWin);
            waddch(startWin, output);

            wmove(startWin, 1, monNumLoc + 1);
            output = winch(startWin);
            waddch(startWin, output);
            wattroff(startWin, COLOR_PAIR(COLOR_YELLOW));
        } else {
            wattron(startWin, COLOR_PAIR(COLOR_YELLOW));
            wmove(startWin, 1, monNumLoc);
            char output = winch(startWin);
            waddch(startWin, output);

            wmove(startWin, 1, monNumLoc + 1);
            output = winch(startWin);
            waddch(startWin, output);

            wattron(startWin, COLOR_PAIR(COLOR_GREEN));
            wmove(startWin, 2, objNumLoc);
            output = winch(startWin);
            waddch(startWin, output);

            wmove(startWin, 2, objNumLoc + 1);
            output = winch(startWin);
            waddch(startWin, output);
            wattroff(startWin, COLOR_PAIR(COLOR_GREEN));
        }
        wrefresh(startWin);
        int key = getch();
        switch(key) {
        case KEY_DOWN:
            mvwaddstr(startWin, 1 + index, 28, " ");
            index = index == 3 ? 0 : index + 1;
            break;
        case KEY_UP:
            mvwaddstr(startWin, 1 + index, 28, " ");
            index = index == 0 ? 3 : index - 1;
            break;
        case 10: // ENTER key
            if(index == 2) {
                d->numMonsters = numMonsters;
                d->numObjects = numObjects;
                wmove(startWin, 6, 0);
                wclrtoeol(startWin);
                mvwaddstr(startWin, 6, 30, "Settings Saved");
            }
            if(index == 3) {
                end = true;
            }
            break;

        case KEY_LEFT:
        case KEY_RIGHT: {
            int valueToAdd = (key == KEY_LEFT) ? -1 : 1;
            if(index == 0) {
                int temp = numMonsters + valueToAdd;
                if(temp > 0 && temp <= 50) {
                    numMonsters = temp;
                    std::string output = std::to_string(numMonsters) + " ";
                    wattron(startWin, COLOR_PAIR(COLOR_YELLOW));
                    mvwaddstr(startWin, 1, monNumLoc, output.c_str());
                    wattroff(startWin, COLOR_PAIR(COLOR_YELLOW));
                } else if(temp <= 0) {
                    wmove(startWin, 6, 0);
                    wclrtoeol(startWin);
                    mvwaddstr(startWin, 6, 23, "Monster count must be at least one");
                } else {
                    wmove(startWin, 6, 0);
                    wclrtoeol(startWin);
                    mvwaddstr(startWin, 6, 20, "Monster count cannot exceed 50 monsters");
                }
            } else if(index == 1) {
                int temp = numObjects + valueToAdd;
                if(temp > 0 && temp <= 50) {
                    numObjects = temp;
                    std::string output = std::to_string(numObjects) + " ";
                    wattron(startWin, COLOR_PAIR(COLOR_GREEN));
                    mvwaddstr(startWin, 2, objNumLoc, output.c_str());
                    wattroff(startWin, COLOR_PAIR(COLOR_GREEN));
                } else if(temp <= 0) {
                    wmove(startWin, 6, 0);
                    wclrtoeol(startWin);
                    mvwaddstr(startWin, 6, 24, "Object count must be at least one");
                } else {
                    wmove(startWin, 6, 0);
                    wclrtoeol(startWin);
                    mvwaddstr(startWin, 6, 21, "Object count cannot exceed 50 monsters");
                }
            }
        } break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            if(index == 0 || index == 1) {
                int value = 10 + key - 48;
                int pos = (index == 0) ? monNumLoc : objNumLoc;
                int colorPair = (index == 0) ? COLOR_YELLOW : COLOR_GREEN;
                int spot = 0;
                wmove(startWin, index + 1, pos + 1);
                wattron(startWin, COLOR_PAIR(colorPair));
                waddch(startWin, (char)key);

                bool valid = true;
                while(valid) {
                    wmove(startWin, index + 1, pos);
                    if(spot == 0) {
                        wattron(startWin, COLOR_PAIR(8));
                    } else {
                        wattron(startWin, COLOR_PAIR(colorPair));
                    }
                    char output = 48 + (value / 10);
                    waddch(startWin, output);
                    if(spot == 1) {
                        wattron(startWin, COLOR_PAIR(8));
                    } else {
                        wattron(startWin, COLOR_PAIR(colorPair));
                    }
                    output = 48 + value % 10;
                    waddch(startWin, output);
                    if(spot == 1) {
                        wattroff(startWin, COLOR_PAIR(8));
                    } else {
                        wattroff(startWin, COLOR_PAIR(colorPair));
                    }
                    wrefresh(startWin);
                    int key2 = getch();
                    switch(key2) {
                    case KEY_UP:
                        if(index == 0) {
                            d->numMonsters = value;
                        } else {
                            d->numObjects = value;
                        }
                        valid = false;

                        wattron(startWin, COLOR_PAIR(colorPair));
                        wmove(startWin, index + 1, pos + spot);
                        output = 48 + ((spot == 0) ? value / 10 : value % 10);
                        waddch(startWin, output);
                        wattroff(startWin, COLOR_PAIR(colorPair));

                        mvwaddstr(startWin, 1 + index, 28, " ");
                        index = index == 0 ? 3 : index - 1;
                        break;

                    case 10:
                    case KEY_DOWN:
                        if(index == 0) {
                            d->numMonsters = value;
                            numMonsters = value;
                        } else {
                            d->numObjects = value;
                            numObjects = value;
                        }
                        valid = false;

                        wattron(startWin, COLOR_PAIR(colorPair));
                        wmove(startWin, index + 1, pos + spot);
                        output = 48 + ((spot == 0) ? value / 10 : value % 10);
                        waddch(startWin, output);
                        wattroff(startWin, COLOR_PAIR(colorPair));

                        mvwaddstr(startWin, 1 + index, 28, " ");
                        index = index == 3 ? 0 : index + 1;
                        break;

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9': {
                        int temp = 0;
                        if(spot == 0) {
                            temp = (key2 - 48) * 10 + value % 10;
                        } else {
                            temp = (key2 - 48) + (value / 10) * 10;
                        }
                        if(temp > 50) {
                            wmove(startWin, 6, 0);
                            wclrtoeol(startWin);
                            mvwaddstr(startWin, 6, 32, "Cannot exceed 50");
                            break;
                        } else if(temp < 1) {
                            wmove(startWin, 6, 0);
                            wclrtoeol(startWin);
                            mvwaddstr(startWin, 6, 30, "Must be atleast one");
                            break;
                        } else {
                            value = temp;
                        }
                    }
                    case KEY_RIGHT:
                    case KEY_LEFT:
                        spot = (spot == 0) ? 1 : 0;
                        break;
                    default:
                        wmove(startWin, 6, 0);
                        wclrtoeol(startWin);
                        mvwaddstr(startWin, 6, 27, "Must enter numerical digit");
                        break;
                    }
                }
            }
        } break;
        case 'c':
            key = getch();
            if(key == 'h') {
                key = getch();
                if(key == 'e') {
                    key = getch();
                    if(key == 'a') {
                        key = getch();
                        if(key == 't') {
                            key = getch();
                            if(key == 's') {
                                *player_debugTools = true;
                                wmove(startWin, 6, 0);
                                wclrtoeol(startWin);
                                mvwaddstr(startWin, 6, 17, "Cheats Enabled - Press Enter to return to menu");
                                wrefresh(startWin);
                                char input[9];
                                getnstr(input, sizeof(input) - 1);
                                while(std::strcmp(input, "health") == 0 || std::strcmp(input, "damage") == 0 ||
                                    std::strcmp(input, "speed") == 0) {
                                    if(std::strcmp(input, "health") == 0) {
                                        wmove(startWin, 6, 0);
                                        wclrtoeol(startWin);
                                        mvwaddstr(startWin, 6, 22, "Enter health amount and press enter");
                                        wrefresh(startWin);
                                        char number[8];
                                        getnstr(number, sizeof(number) - 1);
                                        if(is_number(number)) {
                                            *player_health = atoi(number);
                                            wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            std::string health = "Player's health set to: " + std::string(number);
                                            wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            mvwaddstr(startWin, 6, 25, health.c_str());
                                            wrefresh(startWin);
                                        } else {
                                            wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            mvwaddstr(startWin, 6, 20, "Invalid number exiting out of cheat setup");
                                            wrefresh(startWin);
                                            
                                            break;
                                        } 
                                        getnstr(input, sizeof(input) - 1);
                                        
                                    } else if(std::strcmp(input, "speed") == 0) {
                                        wmove(startWin, 6, 0);
                                        wclrtoeol(startWin);
                                        mvwaddstr(startWin, 6, 22, "Enter speed amount and press enter");
                                        wrefresh(startWin);
                                        char number[8];
                                        getnstr(number, sizeof(number) - 1);                                        
                                        if(is_number(number)) {
                                            *player_speed = atoi(number);
                                            wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            std::string speed = "Player's speed set to: " + std::string(number);
                                            wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            mvwaddstr(startWin, 6, 25, speed.c_str());
                                            wrefresh(startWin);
                                        } else {
                                            wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            mvwaddstr(startWin, 6, 20, "Invalid number exiting out of cheat setup");
                                            wrefresh(startWin);                                            
                                            break;
                                        } 
                                        getnstr(input, sizeof(input) - 1);

                                    } else if(std::strcmp(input, "damage") == 0) {
                                        wmove(startWin, 6, 0);
                                        wclrtoeol(startWin);
                                        mvwaddstr(startWin, 6, 20, "Enter damage base amount and press enter");
                                        wrefresh(startWin);
                                        char base[7];
                                        char rolls[4];
                                        char sides[4];
                                        getnstr(base, sizeof(base) - 1);                                        
                                        if(is_number(base)) {
                                            wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            mvwaddstr(startWin, 6, 20, "Enter number of rolls and press enter");
                                            wrefresh(startWin);
                                            getnstr(rolls, 3);                                            
                                            if(is_number(rolls)) {
                                                wmove(startWin, 6, 0);
                                                wclrtoeol(startWin);
                                                mvwaddstr(startWin, 6, 20, "Enter number of sides and press enter");
                                                wrefresh(startWin);
                                                getnstr(sides, sizeof(sides) -1);                                                
                                                if(is_number(sides)) {
                                                    std::unique_ptr<dice> damage(
                                                        new dice(atoi(base), atoi(rolls), atoi(sides)));
                                                    *player_damage = std::move(damage);
                                                    wmove(startWin, 6, 0);
                                                    wclrtoeol(startWin);
                                                    std::string di = "Players damage dice set to: " +
                                                        player_damage->get()->toString();
                                                    mvwaddstr(startWin, 6, 20, di.c_str());
                                                    wrefresh(startWin);
                                                } else {
                                                    wmove(startWin, 6, 0);
                                                    wclrtoeol(startWin);
                                                    mvwaddstr(
                                                        startWin, 6, 20, "Invalid number exiting out of cheat setup");
                                                    wrefresh(startWin);                                                    
                                                    break;
                                                }
                                            } else {
                                                wmove(startWin, 6, 0);
                                                wclrtoeol(startWin);
                                                mvwaddstr(startWin, 6, 20, "Invalid number exiting out of cheat setup");
                                                wrefresh(startWin);                                                
                                                break;
                                            }
                                        } else {
                                            wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            mvwaddstr(startWin, 6, 20, "Invalid number exiting out of cheat setup");
                                            wrefresh(startWin);                                            
                                            break;
                                        }
                                        getnstr(input, sizeof(input) - 1);;
                                        wmove(startWin, 6, 0);
                                        wclrtoeol(startWin);
                                        wrefresh(startWin);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        case 'd':
            key = getch();
            if(key == 'i') {
                key = getch();
                if(key == 's') {
                    key = getch();
                    if(key == 'a') {
                        key = getch();
                        if(key == 'b') {
                            key = getch();
                            if(key == 'l') {
                                key = getch();
                                if(key == 'e') {
                                    *player_health = 200;
                                    *player_speed = 10;
                                    *player_debugTools = false;
                                    std::unique_ptr<dice> damage(new dice(0, 1, 4));
                                    *player_damage = std::move(damage);
                                    wmove(startWin, 6, 0);
                                            wclrtoeol(startWin);
                                            mvwaddstr(startWin, 6, 20, "Cheats have been disabled --- player reset");
                                            wrefresh(startWin);
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            break;
        }
    }
}

void resetReplay(dungeon_t* d)
{
    for(int y = 0; y < DUNGEON_HEIGHT; y++) {
        for(int x = 0; x < DUNGEON_WIDTH; x++) {
            d->replay[y][x].clear();
            d->replay[y][x].push_back(SymbolColorGrabber(d, y, x));
        }
    }
    d->stats[stats_dungeons_visted].clear();
    d->stats[stats_dungeons_visted].push_back(1);

    d->stats[stats_monsters_killed].clear();
    d->stats[stats_monsters_killed].push_back(0);

    d->stats[stats_unique_monsters_killed].clear();
    d->stats[stats_unique_monsters_killed].push_back(0);

    d->stats[stats_artifacts_found].clear();
    d->stats[stats_artifacts_found].push_back(0);

    d->stats[stats_monster_moves].clear();
    d->stats[stats_monster_moves].push_back(0);

    d->stats[stats_players_hp].clear();
    d->stats[stats_players_hp].push_back(d->ptrPC->hp);

    d->stats[stats_damage_dealt].clear();
    d->stats[stats_damage_dealt].push_back(0);

    d->stats[stats_damage_received].clear();
    d->stats[stats_damage_received].push_back(0);

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    int ct = value.count() % 100000000;

    d->stats[stats_time_played].clear();
    d->stats[stats_time_played].push_back(ct);
}

void printReplayHelper(dungeon_t* d, WINDOW* replayWin, int replaySpot)
{
    for(int y = 0; y < DUNGEON_HEIGHT; y++) {
        for(int x = 0; x < DUNGEON_WIDTH; x++) {
            wattron(replayWin, COLOR_PAIR(d->replay[y][x].at(replaySpot)[1]));
            mvwaddch(replayWin, y, x, d->replay[y][x].at(replaySpot)[0]);
            wattroff(replayWin, COLOR_PAIR(d->replay[y][x].at(replaySpot)[1]));
        }
    }
    std::string sizes = "";
    for(int i = 0; i < (int)d->stats.size(); i++) {
        sizes += std::to_string(d->stats[i].size()) + " ";
    }
    sizes += std::to_string(d->replay[0][0].size());
    std::string players_hp =
        "PC's Health: " + std::to_string(d->stats[stats_players_hp].at(replaySpot)); // 13 + 3 -> 16
    std::string move = "-Replay Mode -- Move Number: " + std::to_string(replaySpot) + "/" +
        std::to_string(d->stats[0].size() - 1) + "-"; // 30 + 5 -> 35
    std::string monster_moves =
        "Monster Moves: " + std::to_string(d->stats[stats_monster_moves].at(replaySpot)); // 15 + 3 -> 18
    std::string artifacts_found =
        "Artifacts Found: " + std::to_string(d->stats[stats_artifacts_found].at(replaySpot)); // 17 + 2 -> 19
    std::string monsters_killed =
        "Monsters Killed: " + std::to_string(d->stats[stats_monsters_killed].at(replaySpot)); // 17 + 4 -> 21
    std::string dungeons_visited =
        "Dungeons Explored: " + std::to_string(d->stats[stats_dungeons_visted].at(replaySpot)); // 19 + 3 -> 22
    std::string damage_dealt =
        "Damage Dealt by PC: " + std::to_string(d->stats[stats_damage_dealt].at(replaySpot)); // 20 + 4 -> 24
    std::string unique_monsters_killed = "Unique Monsters Killed: " +
        std::to_string(d->stats[stats_unique_monsters_killed].at(replaySpot)); // 22 + 2 -> 26
    std::string time_played = "Time Played: " +
        ((replaySpot == 0) ? "0" : (std::to_string(d->stats[stats_time_played].at(replaySpot) / 1000))) +
        " seconds"; // 21 + 5 -> 26
    std::string damage_received =
        "Damage Received by PC: " + std::to_string(d->stats[stats_damage_received].at(replaySpot)); // 23 + 4 -> 27

    wmove(replayWin, 21, 0);
    wclrtoeol(replayWin);
    wmove(replayWin, 22, 0);
    wclrtoeol(replayWin);
    wmove(replayWin, 23, 0);
    wclrtoeol(replayWin);
    wattron(replayWin, COLOR_PAIR(COLOR_YELLOW));
    mvwaddstr(replayWin, 0, 20, move.c_str());
    wattroff(replayWin, COLOR_PAIR(COLOR_YELLOW));

    mvwaddstr(replayWin, 21, 1, damage_received.c_str());
    mvwaddstr(replayWin, 22, 1, time_played.c_str());
    mvwaddstr(replayWin, 23, 1, unique_monsters_killed.c_str());

    mvwaddstr(replayWin, 21, 29, damage_dealt.c_str());
    mvwaddstr(replayWin, 22, 29, dungeons_visited.c_str());
    mvwaddstr(replayWin, 23, 29, monsters_killed.c_str());

    mvwaddstr(replayWin, 21, 54, artifacts_found.c_str());
    mvwaddstr(replayWin, 22, 54, monster_moves.c_str());
    mvwaddstr(replayWin, 23, 54, players_hp.c_str());

    wrefresh(replayWin);
}

void playerReplay(dungeon_t* d)
{
    WINDOW* replayWin = newwin(24, 80, 0, 0);
    if(d->replay[0][0].empty()) {
        mvwaddstr(replayWin, 11, 32, "No Replay Found");
        mvwaddstr(replayWin, 12, 21, "Hit Any Button To Return To Main Menu");
        wrefresh(replayWin);
        getch();
        delwin(replayWin);
    } else {
        int replaySpot = 0;
        int key = -1;
        double speed = 1;
        while(key != 27) {
            printReplayHelper(d, replayWin, replaySpot);
            key = getch();
            switch(key) {
            case KEY_LEFT:
                replaySpot = (replaySpot == 0) ? 0 : replaySpot - 1;
                break;
            case KEY_RIGHT:
                replaySpot = (replaySpot == (int)d->stats[0].size() - 1) ? replaySpot : replaySpot + 1;
                break;
            case 'a': // Auto play starting at  game speed
                key = -1;
                nodelay(stdscr, TRUE);
                printReplayHelper(d, replayWin, ++replaySpot);
                while((key = getch()) != 'a') {
                    if(key == '+' || key == '=') {
                        speed *= 1.1;
                    } else if(key == '-' || key == '_') {
                        speed *= .9;
                    }
                    if(replaySpot < ((int)d->stats[0].size() - 1)) {
                        wattron(replayWin, COLOR_PAIR(COLOR_YELLOW));
                        char output[5];
                        sprintf(output, "%2.2f", speed);
                        mvwaddstr(replayWin, 23, 75, output);
                        wattroff(replayWin, COLOR_PAIR(COLOR_YELLOW));
                        wrefresh(replayWin);
                        int tb =
                            d->stats[stats_time_played].at(replaySpot) - d->stats[stats_time_played].at(replaySpot - 1);
                        std::this_thread::sleep_for(std::chrono::milliseconds(tb) / speed);
                        printReplayHelper(d, replayWin, ++replaySpot);
                    } else {
                        break;
                    }
                }

                nodelay(stdscr, FALSE);

                break;
            default:
                break;
            }
        }
    }
}

int playerControls(dungeon_t* d)
{
    WINDOW* gameWin = newwin(21, 80, 1, 0);
    WINDOW* msgWin = message("New Game!");
    // moveMonsters(d);
    ioPrintMonsterBoard(d, gameWin);
    int key = getch();
    //'A';
    int gameOver = 0;
    while(gameOver == 0) {
        switch(key) {
        case '7': // upper left
        case 'y':
            gameOver = mv_player(d, gameWin, -1, -1);
            break;

        case '8': // up
        case 'k':
            gameOver = mv_player(d, gameWin, -1, 0);
            break;

        case '9': // upper right
        case 'u':
            gameOver = mv_player(d, gameWin, -1, 1);
            break;

        case '6': // right
        case 'l':
            gameOver = mv_player(d, gameWin, 0, 1);
            break;

        case '3': // lower right
        case 'n':
            gameOver = mv_player(d, gameWin, 1, 1);
            break;

        case '2': // down
        case 'j':
            gameOver = mv_player(d, gameWin, 1, 0);
            break;

        case '1': // lower left
        case 'b':
            gameOver = mv_player(d, gameWin, 1, -1);
            break;

        case '4': // left
        case 'h':
            gameOver = mv_player(d, gameWin, 0, -1);
            break;

        case '>': // downstairs
            stairs(d, gameWin, 0);
            break;

        case '<': // upstairs
            stairs(d, gameWin, 1);
            break;

        case '5': // rest
        case ' ':
            gameOver = mv_player(d, gameWin, 0, 0);
            break;

        case 'm': // Display Monster List
            monsterDisplay(d);
            ioPrintMonsterBoard(d, gameWin);
            break;

        case 'i': // Display Inventory
        case 'w':
        case 'd':
        case 'x':
        case 'I':
            inventory(d, gameWin);
            ioPrintMonsterBoard(d, gameWin);
            break;

        case 'e': // Display equipment
        case 't':
            equipment(d, gameWin);
            ioPrintMonsterBoard(d, gameWin);
            break;

        case 'f': // Toggle fog
            if(d->ptrPC->debugTools) {
                d->fog = !d->fog;
                ioPrintMonsterBoard(d, gameWin);
            }

            break;

        case 'g': // Teleport
            if(d->ptrPC->debugTools) {
                teleport(d, gameWin);
                ioPrintMonsterBoard(d, gameWin);
            }
            break;
        case 'L': // Local monster select and lookup
            monsterTeleport(d, gameWin);
            ioPrintMonsterBoard(d, gameWin);
            break;

        case 'Q': // Quit
            message("");
            ioPrintQuit(gameWin);
            gameOver = true;
            break;

        case 'A': // auto
            if(d->ptrPC->debugTools) {
                d->fog = false;
                while(true) {
                    if((gameOver = mv_player(d, gameWin, -2, -2))) {
                        break;
                    }
                }
            }
            break;
        default:
            break;
        }
        if(gameOver) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            delwin(gameWin);
            delwin(msgWin);
            clear();
            refresh();
            endwin();
            heap_delete(&d->turnHeap);
            return 1;
        }
        key = getch();
    }
    return 0;
}

void ioPrintMonsterBoard(dungeon_t* d, WINDOW* gameWin)
{
    refresh();
    int i, j;

    for(i = 0; i < DUNGEON_HEIGHT; i++) {
        for(j = 0; j < DUNGEON_WIDTH; j++) {
            if(d->fog) {
                mvwaddch(gameWin, i, j, d->ptrPC->player_known[i][j]);
            } else {
                if(d->monsters[i][j] != nullptr) {
                    wattron(gameWin, COLOR_PAIR(d->monsters[i][j]->color));
                    mvwaddch(gameWin, i, j, d->monsters[i][j]->symbol);
                    wattroff(gameWin, COLOR_PAIR(d->monsters[i][j]->color));
                } else if(d->objects[i][j] != nullptr) {
                    wattron(gameWin, COLOR_PAIR(d->objects[i][j]->get_color()));
                    mvwaddch(gameWin, i, j, d->objects[i][j]->get_object_symbol());
                    wattroff(gameWin, COLOR_PAIR(d->objects[i][j]->get_color()));
                } else {
                    mvwaddch(gameWin, i, j, d->gameBoard[i][j]);
                }
            }
        }
    }
    wrefresh(gameWin);
    if(d->fog) {
        for(i = maxTwoNums(d->pcLoc[dim_y] - 2, 0); i < minTwoNums(d->pcLoc[dim_y] + 3, DUNGEON_HEIGHT); i++) {
            for(j = maxTwoNums(d->pcLoc[dim_x] - 2, 0); j < minTwoNums(d->pcLoc[dim_x] + 3, DUNGEON_WIDTH); j++) {
                if(d->monsters[i][j] != nullptr) {
                    wattron(gameWin, COLOR_PAIR(d->monsters[i][j]->color));
                    mvwaddch(gameWin, i, j, d->monsters[i][j]->symbol);
                    wattroff(gameWin, COLOR_PAIR(d->monsters[i][j]->color));
                } else if(d->objects[i][j] != nullptr) {
                    wattron(gameWin, COLOR_PAIR(d->objects[i][j]->get_color()));
                    mvwaddch(gameWin, i, j, d->objects[i][j]->get_object_symbol());
                    wattroff(gameWin, COLOR_PAIR(d->objects[i][j]->get_color()));
                } else {
                    wattron(gameWin, A_BOLD);
                    wattron(gameWin, COLOR_PAIR(COLOR_YELLOW));
                    mvwaddch(gameWin, i, j, d->gameBoard[i][j]);
                    wattroff(gameWin, COLOR_PAIR(COLOR_YELLOW));
                    wattroff(gameWin, A_BOLD);
                }
            }
        }
    }

    wrefresh(gameWin);
}
void replayUpdateHelper(dungeon_t* d, int moveNumber)
{
    for(int y = 0; y < DUNGEON_HEIGHT; y++) {
        for(int x = 0; x < DUNGEON_WIDTH; x++) {
            d->replay[y][x].push_back(SymbolColorGrabber(d, y, x));
        }
    }
    for(int i = 0; i < (int)d->stats.size() - 1; i++) {
        //        std::string output = "Index: " + std::to_string(i) + " MV#: " + std::to_string(moveNumber) + " Size: "
        //        + std::to_string(d->stats[i].size()) + " Size(9): " + std::to_string(d->stats.size()) + " time size: "
        //        +  std::to_string(d->stats[stats_time_played].size()) + "\n"; message(output.c_str());
        //        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if((int)d->stats[i].size() != moveNumber + 1) {
            d->stats[i].push_back(d->stats[i].at(moveNumber - 1));
        }
    }
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    int ct = value.count() % 100000000;
    d->stats[stats_time_played].push_back(std::abs(ct - d->stats[stats_time_played].at(0)));
}

int mv_player(dungeon_t* d, WINDOW* gameWin, int relativeY, int relativeX)
{
    if(relativeY ==
        -2) { // Auto Play - If you some how you wait 700+ hours and get to 1 billion moves it will stack smash
        if(d->gameBoard[d->pcLoc[dim_y]][d->pcLoc[dim_x]] == blk_stairs_down) {
            stairs(d, gameWin, 0);
            return 0;
        } else if(d->gameBoard[d->pcLoc[dim_y]][d->pcLoc[dim_x]] == blk_stairs_up) {
            stairs(d, gameWin, 1);
            return 0;
        }

        // std::this_thread::sleep_for(std::chrono::milliseconds(300));
        relativeX = randomInt(-1, 1);
        relativeY = randomInt(-1, 1);
    }
    int moveNumber = d->replay[0][0].size();
    if(relativeY == 0 && relativeX == 0) {
        std::string output = std::to_string(moveNumber) + " - " + std::to_string(d->pcLoc[dim_x]) + "," +
            std::to_string(d->pcLoc[dim_y]) + " -- Player HP: " + std::to_string(d->ptrPC->hp);
        message(output.c_str());
        if(moveMonsters(d)) {
            d->stats[stats_players_hp].push_back(d->ptrPC->hp);
            replayUpdateHelper(d, moveNumber);
            ioLoseGame(gameWin);
            return 1;
        }
        updateFog(d);
        ioPrintMonsterBoard(d, gameWin);
        d->stats[stats_players_hp].push_back(d->ptrPC->hp);
        replayUpdateHelper(d, moveNumber);
        return 0;
    }
    int y = d->pcLoc[dim_y] + relativeY;
    int x = d->pcLoc[dim_x] + relativeX;
    if(d->gameBoard[y][x] == blk_rock || d->gameBoard[y][x] == blk_immutable_rock) {
        const char* msg = "Bang your head against the rock all you want, it wont budge!";
        message(msg);
    } else {
        bool stats = true;
        if(d->monsters[y][x] != nullptr) { // Combat
            int damage = d->ptrPC->calculateDamagePC();
            d->stats[stats_damage_dealt].push_back(damage);
            if(((d->monsters[y][x]->hp -= damage)) <= 0) {
                d->monsters[y][x]->xLoc = 0;
                d->monsters[y][x]->yLoc = 0;
                d->monsters[y][x]->dead = true;
                if(bitChecker(d->monsters[y][x]->characteristics, 8)) {
                    d->monsters[y][x] = nullptr;
                    d->stats[stats_monsters_killed].push_back(d->stats[stats_monsters_killed].at(moveNumber - 1) + 1);
                    ioWinGame(gameWin);
                    replayUpdateHelper(d, moveNumber);
                    return 1;
                } else if(bitChecker(d->monsters[y][x]->characteristics, 7)) {
                    d->uniqueMonsters.push_back((reinterpret_cast<monster*>(d->monsters[y][x].get()))->name);
                    message("Unique Monsters Taste The Best!");
                    d->stats[stats_unique_monsters_killed].push_back(
                        d->stats[stats_unique_monsters_killed].at(moveNumber - 1) + 1);
                } else {
                    message("Nom");
                }
                d->stats[stats_monsters_killed].push_back(d->stats[stats_monsters_killed].at(moveNumber - 1) + 1);
                d->monsters[y][x] = nullptr;
            } else {
                std::string output = std::to_string(damage) + " damage done to " +
                    (reinterpret_cast<monster*>(d->monsters[y][x].get()))->name + " who has " +
                    std::to_string(d->monsters[y][x]->hp) + " hp left!";
                message(output.c_str());
            }
            stats = false;

        } else {
            d->stats[stats_damage_dealt].push_back(0);
            if(d->objects[y][x] != nullptr) { // add item to inventory
                int i;
                std::string output = "Inventory is full";
                for(i = 0; i < 10; i++) {
                    if(d->ptrPC->inventory[i] == nullptr) {
                        if(d->objects[y][x]->get_artifact()) {
                            if(!checkForVectorString(&d->artifacts, d->objects[y][x]->get_name())) {
                                d->artifacts.push_back(d->objects[y][x]->get_name());
                                output = "Picked up artifact: " + d->objects[y][x]->get_name();
                                d->stats[stats_artifacts_found].push_back(
                                    d->stats[stats_artifacts_found].at(moveNumber - 1) + 1);
                            } else {
                                output = "Repicked up artifact: " + d->objects[y][x]->get_name();
                            }
                        } else {
                            output = "Picked up: " + d->objects[y][x]->get_name();
                        }
                        d->ptrPC->inventory[i].swap(d->objects[y][x]);
                        break;
                    }
                }
                message(output.c_str());
                stats = false;
            }
            d->monsters[y][x] = d->monsters[d->pcLoc[dim_y]][d->pcLoc[dim_x]];
            d->monsters[d->pcLoc[dim_y]][d->pcLoc[dim_x]] = nullptr;
            d->pcLoc[dim_y] = d->ptrPC->yLoc = y;
            d->pcLoc[dim_x] = d->ptrPC->xLoc = x;
        }
        dijkstra_nontunnel(d);
        dijkstra_tunnel(d);

        if(moveMonsters(d)) {
            ioLoseGame(gameWin);
            d->stats[stats_players_hp].push_back(d->ptrPC->hp);
            replayUpdateHelper(d, moveNumber);
            return 1;
        }
        updateFog(d);
        ioPrintMonsterBoard(d, gameWin);
        if(stats) {
            std::string output = std::to_string(moveNumber) + " - " + std::to_string(x) + "," + std::to_string(y) +
                " -- Player HP: " + std::to_string(d->ptrPC->hp);
            message(output.c_str());
        }
    }
    d->stats[stats_players_hp].push_back(d->ptrPC->hp);
    replayUpdateHelper(d, moveNumber);
    return 0;
}

WINDOW* message(const char* msg)
{
    WINDOW* msgWin = newwin(1, 80, 0, 0);
    wclear(msgWin);
    mvwaddstr(msgWin, 0, 0, msg);
    wrefresh(msgWin);
    return msgWin;
}

void stairs(dungeon_t* d, WINDOW* gameWin, int up)
{
    if(d->gameBoard[d->pcLoc[dim_y]][d->pcLoc[dim_x]] == (up ? blk_stairs_up : blk_stairs_down)) {
        message("All aboard the stair ride!!");
        gen_dungeon(d);
        generatePlayer(d);
        generateAllCustomMonsters(d);
        generateAllObjects(d);
        dijkstra_nontunnel(d);
        dijkstra_tunnel(d);
        moveMonsters(d);

        d->stats[stats_dungeons_visted].push_back(d->stats[stats_dungeons_visted].back() + 1);
        replayUpdateHelper(d, d->replay[0][0].size());
        ioPrintMonsterBoard(d, gameWin);
        refresh();
    } else {
        const char* msg = up ? "Maybe try finding a rope?" : "You're going to need shovel!";
        message(msg);
    }
}

void monsterDisplay(dungeon_t* d)
{
    WINDOW* monsterWin = newwin(19, 40, 2, 20);

    monsterList(d, monsterWin, 0);

    int key = getch();
    int lines = 17; // Lines of display
    int listIndex = 0;
    int exit = 0;
    while(!exit) {
        if(key == KEY_DOWN && ((listIndex + 1 + lines) < d->numMonsters)) {
            listIndex++;
            monsterList(d, monsterWin, listIndex);
        } else if(key == KEY_UP && listIndex != 0) {
            listIndex--;
            monsterList(d, monsterWin, listIndex);
        } else if(key == 27) {
            delwin(monsterWin);
            exit = 1;
            refresh();
            break;
        }
        key = getch();
    }
}

void monsterList(dungeon_t* d, WINDOW* monsterWin, int listIndex)
{
    wclear(monsterWin);
    box(monsterWin, 0, 0);
    const char* header = "------------Symbol--Location------------";
    mvwaddstr(monsterWin, 0, 0, header);
    int i;
    for(i = 1; i < d->numMonsters + 1 && i < 18; i++) {
        mvwaddch(monsterWin, i, 14, d->characterVector[i - 1 + listIndex]->symbol);
        if(d->characterVector[i - 1 + listIndex]->dead == 1) {
            mvwaddstr(monsterWin, i, 23, "Murdered!");
        } else {
            int xDif = d->characterVector[i - 1 + listIndex]->xLoc - d->pcLoc[dim_x];
            int yDif = d->characterVector[i - 1 + listIndex]->yLoc - d->pcLoc[dim_y];
            if(xDif > 0) {
                char result[3];
                sprintf(result, "%2d", xDif);
                mvwaddstr(monsterWin, i, 21, result);
                mvwaddstr(monsterWin, i, 24, "East");
            } else {
                char result[3];
                sprintf(result, "%2d", -1 * xDif);
                mvwaddstr(monsterWin, i, 21, result);
                mvwaddstr(monsterWin, i, 24, "West");
            }
            if(yDif > 0) {
                char result[3];
                sprintf(result, "%2d", yDif);
                mvwaddstr(monsterWin, i, 29, result);
                mvwaddstr(monsterWin, i, 32, "South");
            } else {
                char result[3];
                sprintf(result, "%2d", -1 * yDif);
                mvwaddstr(monsterWin, i, 29, result);
                mvwaddstr(monsterWin, i, 32, "North");
            }
        }
    }
    wrefresh(monsterWin);
}

void ioPrintQuit(WINDOW* gameWin)
{
    wclear(gameWin);
    int i;
    mvwaddstr(gameWin, 10, 26, "Exiting to menu in   seconds!");
    for(i = 5; i > 0; i--) {
        char result[2];
        sprintf(result, "%d", i);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        mvwaddstr(gameWin, 10, 45, result);
        wrefresh(gameWin);
    }
}

void ioLoseGame(WINDOW* gameWin)
{
    clear();
    refresh();
    wclear(gameWin);
    mvwaddstr(gameWin, 10, 26, "You're not very good at this");
    wrefresh(gameWin);
}

void ioWinGame(WINDOW* gameWin)
{
    clear();
    refresh();
    wclear(gameWin);
    mvwaddstr(gameWin, 10, 26, "You won. Congratulations!!!!");
    wrefresh(gameWin);
}

int updateFog(dungeon_t* d)
{
    int i, j;
    for(i = maxTwoNums(d->pcLoc[dim_y] - 2, 0); i < minTwoNums(d->pcLoc[dim_y] + 3, DUNGEON_HEIGHT); i++) {
        for(j = maxTwoNums(d->pcLoc[dim_x] - 2, 0); j < minTwoNums(d->pcLoc[dim_x] + 3, DUNGEON_WIDTH); j++) {
            d->ptrPC->player_known[i][j] = d->gameBoard[i][j];
        }
    }
    return 0;
}

int teleport(dungeon_t* d, WINDOW* gameWin)
{
    d->fog = false;
    ioPrintMonsterBoard(d, gameWin);
    bool placed = false;
    int x = d->pcLoc[dim_x];
    int y = d->pcLoc[dim_y];
    int prevX = x;
    int prevY = y;
    wattron(gameWin, COLOR_PAIR(2));
    mvwaddch(gameWin, y, x, '*');
    wattroff(gameWin, COLOR_PAIR(2));
    wrefresh(gameWin);
    int key = getch();
    while(!placed) {
        switch(key) {
        case '7': // upper left
        case 'y':
            --y;
            --x;
            break;

        case '8': // up
        case 'k':
            --y;
            break;

        case '9': // upper right
        case 'u':
            --y;
            ++x;
            break;

        case '6': // right
        case 'l':
            ++x;
            break;

        case '3': // lower right
        case 'n':
            ++x;
            ++y;
            break;

        case '2': // down
        case 'j':
            ++y;
            break;

        case '1': // lower left
        case 'b':
            ++y;
            --x;
            break;

        case '4': // left
        case 'h':
            --x;
            break;

        case 'r':

            x = randomInt(1, 79);
            y = randomInt(1, 19);

        case 'g': {
            d->fog = true;
            if(d->monsters[y][x] != nullptr) {
                d->monsters[d->pcLoc[dim_y]][d->pcLoc[dim_x]].swap(d->monsters[y][x]);
                d->monsters[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->xLoc = d->pcLoc[dim_x];
                d->monsters[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->yLoc = d->pcLoc[dim_y];
            } else {
                d->monsters[y][x] = std::move(d->monsters[d->pcLoc[dim_y]][d->pcLoc[dim_x]]);
            }
            d->pcLoc[dim_y] = y;
            d->pcLoc[dim_x] = x;
            d->ptrPC->xLoc = x;
            d->ptrPC->yLoc = y;
            ioPrintMonsterBoard(d, gameWin);
            placed = true;
        } break;
        default:
            break;
        }
        if(placed) {
            break;
        }
        if(x != prevX || y != prevY) {
            if(x == 0 || x == 79 || y == 0 || y == 20) {
                const char* error = "That's a wall";
                x = prevX;
                y = prevY;
                message(error);
            } else {
                ioPrintMonsterBoard(d, gameWin);
                wattron(gameWin, COLOR_PAIR(2));
                mvwaddch(gameWin, y, x, '*');
                wattroff(gameWin, COLOR_PAIR(2));
                wrefresh(gameWin);
                prevY = y;
                prevX = x;
            }
        }

        key = getch();
    }
    return 0;
}

void inventoryList(dungeon_t* d, WINDOW* inventoryWin, int listIndex)
{
    wclear(inventoryWin);
    box(inventoryWin, 0, 0);
    const char* header = "#-Name------------Type-------Damage-----Defense-Speed-Artifact";
    mvwaddstr(inventoryWin, 0, 1, header);
    int i;
    for(i = 0; i < 10; i++) {
        if(i == listIndex) {
            wattron(inventoryWin, COLOR_PAIR(6));
        }
        mvwaddch(inventoryWin, i + 1, 1, (char)i + 48);
        if(d->ptrPC->inventory[i] != nullptr) {
            mvwaddstr(inventoryWin, i + 1, 3, d->ptrPC->inventory[i]->get_name().c_str());
            if(strlen(d->ptrPC->inventory[i]->get_name().c_str()) > 15) {
                char temp[] = "... ";
                mvwaddstr(inventoryWin, i + 1, 15, temp);
            }
            mvwaddstr(inventoryWin, i + 1, 19, typeString(d->ptrPC->inventory[i]->get_type()).c_str());
            mvwaddstr(inventoryWin, i + 1, 30, d->ptrPC->inventory[i]->get_damage().toString().c_str());
            waddstr(inventoryWin, "      ");
            mvwaddstr(inventoryWin, i + 1, 41, std::to_string(d->ptrPC->inventory[i]->get_defense()).c_str());
            waddstr(inventoryWin, "       ");
            mvwaddstr(inventoryWin, i + 1, 49, std::to_string(d->ptrPC->inventory[i]->get_speed()).c_str());
            waddstr(inventoryWin, "     ");
            if(d->ptrPC->inventory[i]->get_artifact()) {
                mvwaddstr(inventoryWin, i + 1, 55, "True");
            } else {
                mvwaddstr(inventoryWin, i + 1, 55, "False");
            }

        } else {
            mvwaddstr(inventoryWin, i + 1, 3, "Empty");
        }
        if(i == listIndex) {
            wattroff(inventoryWin, COLOR_PAIR(6));
        }
    }
    wrefresh(inventoryWin);
}

void inventory(dungeon_t* d, WINDOW* gameWin)
{
    WINDOW* inventoryWin = newwin(12, 64, 6, 8);
    inventoryList(d, inventoryWin, 0);

    int key = getch();
    int ringIndex = 0;
    int listIndex = 0;
    int exit = false;
    while(!exit) {
        if(key == KEY_DOWN && listIndex < 9) {
            listIndex++;
        } else if(key == KEY_UP && listIndex != 0) {
            listIndex--;
        } else if(key == '=') {
            ringIndex = 1;
            message("Ring slot l selected");
        } else if(key == '-') {
            ringIndex = 0;
            message("Ring slot a selected");
        } else if(key == 'w') { // Wear item
            if(d->ptrPC->inventory[listIndex] == nullptr) {
                message("Emperor's new clothes?");
            } else if(d->ptrPC->inventory[listIndex]->get_type() <= objtype_RING) {
                if(d->ptrPC->inventory[listIndex]->get_type() == objtype_RING) {
                    if(ringIndex == 0) {
                        message("Equipped: Ring a");
                    } else {
                        message("Equipped: Ring l");
                    }
                    d->ptrPC->inventory[listIndex].swap(
                        d->ptrPC->equipment[(d->ptrPC->inventory[listIndex])->get_type() - 1 + ringIndex]);
                } else {
                    std::string output = "Equipped: " + typeString(d->ptrPC->inventory[listIndex]->get_type());
                    message(output.c_str());
                    d->ptrPC->inventory[listIndex].swap(
                        d->ptrPC->equipment[(d->ptrPC->inventory[listIndex])->get_type() - 1]);
                }

            } else {
                message("Not an item that can be equipped");
            }
        } else if(key == 'd') { // Drop Item
            if(d->ptrPC->inventory[listIndex] == nullptr) {
                message("That's kind of you to drop nothing");
            } else if(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]] != nullptr) {
                if(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_artifact()) {
                    if(!checkForVectorString(&d->artifacts, d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_name())) {
                        d->artifacts.push_back(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_name());
                        message("Swapped for an artifact on the dungeon floor");
                        d->stats[stats_artifacts_found].back()++;
                    } else {
                        message("To keep this artifact or to not keep this artifact?");
                    }
                } else {
                    message("Swapped with item on dungeon floor");
                }
            } else {
                message("Item dropped");
            }
            d->ptrPC->inventory[listIndex].swap(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]);
        } else if(key == 'x') { // Expunge item
            if(d->ptrPC->inventory[listIndex] == nullptr) {
                message("Nothing comes from nothing");
            } else if(d->ptrPC->inventory[listIndex]->get_artifact()) {
                message("Artifact Expunged Forever");
            } else {
                message("Item expunged");
            }
            d->ptrPC->inventory[listIndex].reset();
        } else if(key == 'I') {
            if(d->ptrPC->inventory[listIndex] == nullptr) {
                message("Item description seems to be missing");
            } else {
                delwin(inventoryWin);
                itemDescription(d, listIndex, false);
                ioPrintMonsterBoard(d, gameWin);
                inventoryWin = newwin(14, 64, 6, 7);
            }
        } else if(key == 27) { // exit inventory list
            delwin(inventoryWin);
            exit = 1;
            refresh();
            break;
        }
        inventoryList(d, inventoryWin, listIndex);
        key = getch();
    }
}

void equipmentList(dungeon_t* d, WINDOW* equipmentWin, int listIndex)
{
    wclear(equipmentWin);
    box(equipmentWin, 0, 0);
    const char* header = "Slot-------Name-------------Damage-----Defense-Speed-Artifact";
    mvwaddstr(equipmentWin, 0, 1, header);
    int i;
    for(i = 0; i < 12; i++) {
        if(i == listIndex) {
            wattron(equipmentWin, COLOR_PAIR(6));
        }
        if(i == 10) {
            mvwaddstr(equipmentWin, i + 1, 1, "Ring a     ");
        } else if(i == 11) {
            mvwaddstr(equipmentWin, i + 1, 1, "Ring l     ");
        } else {
            mvwaddstr(equipmentWin, i + 1, 1, typeString((object_type_t)(i + 1)).c_str());
        }
        if(d->ptrPC->equipment[i] != nullptr) {
            mvwaddstr(equipmentWin, i + 1, 12, d->ptrPC->equipment[i]->get_name().c_str());
            if(strlen(d->ptrPC->equipment[i]->get_name().c_str()) > 16) {
                char temp[] = "... ";
                mvwaddstr(equipmentWin, i + 1, 25, temp);
            }
            mvwaddstr(equipmentWin, i + 1, 29, d->ptrPC->equipment[i]->get_damage().toString().c_str());
            waddstr(equipmentWin, "      ");
            mvwaddstr(equipmentWin, i + 1, 40, std::to_string(d->ptrPC->equipment[i]->get_defense()).c_str());
            waddstr(equipmentWin, "       ");
            mvwaddstr(equipmentWin, i + 1, 48, std::to_string(d->ptrPC->equipment[i]->get_speed()).c_str());
            waddstr(equipmentWin, "     ");
            if(d->ptrPC->equipment[i]->get_artifact()) {
                mvwaddstr(equipmentWin, i + 1, 54, "True");
            } else {
                mvwaddstr(equipmentWin, i + 1, 54, "False");
            }

        } else {
            mvwaddstr(equipmentWin, i + 1, 12, "No equipped Item");
        }
        if(i == listIndex) {
            wattroff(equipmentWin, COLOR_PAIR(6));
        }
    }
    wrefresh(equipmentWin);
}

void equipment(dungeon_t* d, WINDOW* gameWin)
{
    WINDOW* equipmentWin = newwin(14, 64, 6, 7);
    equipmentList(d, equipmentWin, 0);

    int key = getch();

    int listIndex = 0;
    int exit = false;
    while(!exit) {
        if(key == KEY_DOWN && listIndex < 11) {
            listIndex++;
        } else if(key == KEY_UP && listIndex != 0) {
            listIndex--;
        } else if(key == 'd') { // Drop Item
            if(d->ptrPC->inventory[listIndex] == nullptr) {
                message("That's kind of you to drop nothing");
            } else if(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]] != nullptr) {
                if(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_artifact()) {
                    if(!checkForVectorString(&d->artifacts, d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_name())) {
                        d->artifacts.push_back(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_name());
                        message("Swapped for an artifact on the dungeon floor");
                        d->stats[stats_artifacts_found].back()++;
                    } else {
                        message("To keep this artifact or to not keep this artifact?");
                    }
                } else {
                    message("Swapped with item on dungeon floor");
                }
            } else {
                message("Item dropped");
            }
            d->ptrPC->equipment[listIndex].swap(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]);
        } else if(key == 'x') { // Expunge item
            if(d->ptrPC->inventory[listIndex] == nullptr) {
                message("Nothing comes from nothing");
            } else if(d->ptrPC->equipment[listIndex]->get_artifact()) {
                message("Artifact Expunged Forever");
            } else {
                message("Item expunged");
            }
            d->ptrPC->equipment[listIndex].reset();
        } else if(key == 't') { // Take off item
            int invIndex = -1;
            int i;
            for(i = 0; i < 10; i++) {
                if(d->ptrPC->inventory[i] == nullptr) {
                    invIndex = i;
                    break;
                }
            }
            if(d->ptrPC->equipment[listIndex] == nullptr) {
                message("Probably shouldn't take off your naked skin");
            } else if(invIndex != -1) {
                d->ptrPC->equipment[listIndex].swap(d->ptrPC->inventory[invIndex]);
                std::string output = "Item added to inventory slot: " + std::to_string(invIndex);
                message(output.c_str());
            } else if((d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]] == nullptr) ||
                d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_type() ==
                    d->ptrPC->equipment[listIndex]->get_type()) {
                if(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_artifact()) {
                    if(!checkForVectorString(&d->artifacts, d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_name())) {
                        d->artifacts.push_back(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]->get_name());
                        message("Swapped for an artifact on the dungeon floor");
                        d->stats[stats_artifacts_found].back()++;
                    } else {
                        message("To keep this artifact or to not keep this artifact?");
                    }
                } else {
                    message("Item dropped");
                }
                d->ptrPC->equipment[listIndex].swap(d->objects[d->pcLoc[dim_y]][d->pcLoc[dim_x]]);

            } else {
                message("Nowhere to throw item. So you're stuck with it");
            }
        } else if(key == 'I') {
            if(d->ptrPC->equipment[listIndex] == nullptr) {
                message("Item description seems to be missing");
            } else {
                delwin(equipmentWin);
                itemDescription(d, listIndex, true);
                ioPrintMonsterBoard(d, gameWin);
                equipmentWin = newwin(14, 64, 6, 7);
            }
        } else if(key == 27) { // exit equipment list
            delwin(equipmentWin);
            exit = 1;
            refresh();
            break;
        }
        equipmentList(d, equipmentWin, listIndex);
        key = getch();
    }
}

void itemDescription(dungeon_t* d, int index, bool isInEquipment)
{
    message("Hit esc to go back to previous screen");
    WINDOW* itemDescriptionBorderWin = newwin(21, 80, 1, 0);
    WINDOW* itemDescriptionWin = newwin(19, 78, 2, 1);
    box(itemDescriptionBorderWin, 0, 0);
    const char* header = "Object Description";
    mvwaddstr(itemDescriptionBorderWin, 0, 31, header);
    std::string name;
    std::string desc;
    std::string hit;
    std::string dam;
    std::string dodge;
    std::string def;
    std::string weight;
    std::string speed;
    std::string attr;
    std::string value;
    std::string artifact;
    std::string rrty;
    int color;
    object_description od;
    if(isInEquipment) {
        std::vector<object_description>::iterator it;
        for(it = d->object_descriptions.begin(); it != d->object_descriptions.end(); it++) {
            if(it->get_name() == d->ptrPC->equipment[index]->get_name()) {
                od = *it;
                break;
            }
        }

    } else {
        std::vector<object_description>::iterator it;
        for(it = d->object_descriptions.begin(); it != d->object_descriptions.end(); it++) {
            if(it->get_name() == d->ptrPC->inventory[index]->get_name()) {
                od = *it;
                break;
            }
        }
    }
    name = "Name: " + od.get_name();
    desc = od.get_description();
    hit = "Hit: " + od.get_hit().toString();
    dam = "Damage: " + od.get_damage().toString();
    dodge = "Dodge: " + od.get_dodge().toString();
    def = "Defense: " + od.get_defense().toString();
    weight = "Weight: " + od.get_weight().toString();
    speed = "Speed: " + od.get_speed().toString();
    attr = "Attributes: " + od.get_characteristics().toString();
    value = "Value: " + od.get_value().toString();
    std::string temp = od.get_artifact() ? "True" : "False";
    artifact = "Artifact: " + temp;
    rrty = "Rarity: " + std::to_string(od.get_rarity());
    color = od.get_color();

    wattron(itemDescriptionWin, COLOR_PAIR(color));
    mvwaddstr(itemDescriptionWin, 0, 0, name.c_str());
    wattroff(itemDescriptionWin, COLOR_PAIR(color));
    mvwaddstr(itemDescriptionWin, 1, 0, hit.c_str());
    mvwaddstr(itemDescriptionWin, 2, 0, dam.c_str());
    mvwaddstr(itemDescriptionWin, 3, 0, dodge.c_str());
    mvwaddstr(itemDescriptionWin, 4, 0, def.c_str());
    mvwaddstr(itemDescriptionWin, 5, 0, weight.c_str());
    mvwaddstr(itemDescriptionWin, 6, 0, speed.c_str());
    mvwaddstr(itemDescriptionWin, 7, 0, attr.c_str());
    mvwaddstr(itemDescriptionWin, 8, 0, value.c_str());
    mvwaddstr(itemDescriptionWin, 9, 0, artifact.c_str());
    mvwaddstr(itemDescriptionWin, 10, 0, rrty.c_str());
    mvwaddstr(itemDescriptionWin, 11, 0, "Description:");
    mvwaddstr(itemDescriptionWin, 12, 0, desc.c_str());
    wrefresh(itemDescriptionBorderWin);
    wrefresh(itemDescriptionWin);
    int key = getch();
    while(key != 27) {
        key = getch();
    }
    delwin(itemDescriptionBorderWin);
    delwin(itemDescriptionWin);
    message("");
    refresh();
}
void printMarkerHelper(dungeon_t* d, WINDOW* gameWin, int x, int y)
{
    const char* header = "Monster Lookup";
    ioPrintMonsterBoard(d, gameWin);
    box(gameWin, 0, 0);
    mvwaddstr(gameWin, 0, 33, header);
    wattron(gameWin, COLOR_PAIR(8));
    if(d->monsters[y][x] != nullptr) {
        if(y == d->pcLoc[dim_y] && x == d->pcLoc[dim_x]) {
            mvwaddch(gameWin, y, x, '@');
        } else {
            mvwaddch(gameWin, y, x, d->monsters[y][x]->symbol);
        }
    } else {
        mvwaddch(
            gameWin, y, x, d->objects[y][x] == nullptr ? d->gameBoard[y][x] : d->objects[y][x]->get_object_symbol());
    }
    wattroff(gameWin, COLOR_PAIR(8));
    wrefresh(gameWin);
}
void monsterTeleport(dungeon_t* d, WINDOW* gameWin)
{
    int exit = false;
    int x = d->pcLoc[dim_x];
    int y = d->pcLoc[dim_y];
    int prevX = x;
    int prevY = y;
    printMarkerHelper(d, gameWin, x, y);
    int key = getch();
    while(!exit) {
        switch(key) {
        case '7': // upper left
        case 'y':
            --y;
            --x;
            break;

        case '8': // up
        case 'k':
            --y;
            break;

        case '9': // upper right
        case 'u':
            --y;
            ++x;
            break;

        case '6': // right
        case 'l':
            ++x;
            break;

        case '3': // lower right
        case 'n':
            ++x;
            ++y;
            break;

        case '2': // down
        case 'j':
            ++y;
            break;

        case '1': // lower left
        case 'b':
            ++y;
            --x;
            break;

        case '4': // left
        case 'h':
            --x;
            break;

        case 't':
            if(d->monsters[y][x] == nullptr) {
                message("Monster description can't be found for some reason");
            } else {
                if(y == d->pcLoc[dim_y] && x == d->pcLoc[dim_x]) {
                    message("NPD - A disorder in which a person has an inflated sense of self-importance");
                } else {
                    monsterDescription(d, x, y);
                    printMarkerHelper(d, gameWin, x, y);
                }
            }
            break;
        case 27:
            exit = true;
            break;
        default:
            break;
        }
        if(exit) {
            break;
        }
        if(x != prevX || y != prevY) {
            if(x == 0 || x == 79 || y == 0 || y == 20) {
                const char* error = "That's a wall";
                x = prevX;
                y = prevY;
                message(error);
            } else if(x > 2 + d->pcLoc[dim_x] || x < d->pcLoc[dim_x] - 2 || y > 2 + d->pcLoc[dim_y] ||
                y < d->pcLoc[dim_y] - 2) {
                message("Hello darkness my old friend");
                x = prevX;
                y = prevY;
            } else {
                printMarkerHelper(d, gameWin, x, y);
                if(d->monsters[y][x] != nullptr) {
                    std::string output;
                    if(y == d->pcLoc[dim_y] && x == d->pcLoc[dim_x]) {
                        output = "Here lies a soon to be dead woman (or man)(or whatever noun you identify as)";
                    } else {
                        output = "You're creeping on the monster known as: " +
                            static_cast<monster*>(d->monsters[y][x].get())->name;
                    }
                    message(output.c_str());
                } else {
                    message("Pick a monster, any monster");
                }
                prevY = y;
                prevX = x;
            }
        }
        key = getch();
    }
}

void monsterDescription(dungeon_t* d, int x, int y)
{
    message("Hit esc to go back to previous screen");
    WINDOW* monsterDescriptionBorderWin = newwin(21, 80, 1, 0);
    WINDOW* monsterDescriptionWin = newwin(19, 78, 2, 1);
    box(monsterDescriptionBorderWin, 0, 0);
    const char* header = "Monster Description";
    mvwaddstr(monsterDescriptionBorderWin, 0, 31, header);
    monster_description md;
    std::vector<monster_description>::iterator it;
    for(it = d->monster_descriptions.begin(); it != d->monster_descriptions.end(); it++) {
        if(it->getMonstersName() == static_cast<monster*>(d->monsters[y][x].get())->get_name()) {
            md = *it;
            break;
        }
    }
    std::string name = "Name: " + md.getMonstersName();
    std::string desc = "Description: " + md.getDescription();
    std::string hit = "Hit Points: " + md.getHitPoints().toString();
    std::string dam = "Damage: " + md.getDamage().toString();
    std::string speed = "Speed: " + md.getSpeed().toString();
    std::string abilities = "Abilities: " + md.getAbilitiesString();
    std::string rrty = "Rarity: " + std::to_string(md.getRarity());
    int color = md.getFirstColor();
    wattron(monsterDescriptionWin, COLOR_PAIR(color));
    mvwaddstr(monsterDescriptionWin, 0, 0, name.c_str());
    wattroff(monsterDescriptionWin, COLOR_PAIR(color));
    mvwaddstr(monsterDescriptionWin, 1, 0, hit.c_str());
    mvwaddstr(monsterDescriptionWin, 2, 0, dam.c_str());
    mvwaddstr(monsterDescriptionWin, 3, 0, speed.c_str());
    mvwaddstr(monsterDescriptionWin, 4, 0, abilities.c_str());
    mvwaddstr(monsterDescriptionWin, 5, 0, rrty.c_str());
    mvwaddstr(monsterDescriptionWin, 6, 0, desc.c_str());
    wrefresh(monsterDescriptionBorderWin);
    wrefresh(monsterDescriptionWin);
    refresh();
    int key = getch();
    while(key != 27) {
        key = getch();
    }
    delwin(monsterDescriptionBorderWin);
    delwin(monsterDescriptionWin);
    message("");
    refresh();
}
