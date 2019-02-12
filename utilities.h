#ifndef UTILITIES_H
#define UTILITIES_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <array>

#define color_lu_entry(color) \
    {                         \
#color, COLOR_##color \
    }
static const struct {
    const char* name;
    const uint32_t value;
} colors_lookup[] = {
    /* Same deal here as above in abilities_lookup definition. */
    /* We can use this convenient macro here, but we can't use a *
     * similar macro above because of PASS and TELE.             */
    /* color_lu_entry(BLACK), Can't display COLOR_BLACK */
    "BLACK", COLOR_WHITE, color_lu_entry(BLUE), color_lu_entry(CYAN), color_lu_entry(GREEN), color_lu_entry(MAGENTA),
    color_lu_entry(RED), color_lu_entry(WHITE), color_lu_entry(YELLOW), { 0, 0 }
};
class dungeon_t;

bool checkForVectorString(std::vector<std::string>* vector, std::string string);
bool is_number (char* s);
int randomInt(int min, int max); // min and max inclusive
bool bitChecker(unsigned int characteristic, int place);
std::array<char, 2> SymbolColorGrabber(dungeon_t *d, int y, int x);

#endif
