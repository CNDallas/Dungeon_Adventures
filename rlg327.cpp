#include "dungeon.h"
#include "io.h"
#include <chrono>
#include <thread>

int main(int argc, char** argv) {

    auto *d = new dungeon_t(10, 10);
    initTerminal();
    startScreen(d);    
    heap_delete(&d->turnHeap);
    delete (d);
}
