#ifndef PATH_H
#define PATH_H

#ifdef __cplusplus

#include "dungeon.h"
#include "heap.h"
#include <cstdio>

extern "C" {
#endif
typedef struct dungeon_t dungeon_t;
int32_t nonTunnel_path_cmp(const void* key, const void* with);
int32_t tunnel_path_cmp(const void* key, const void* with);
void dijkstra_nontunnel(dungeon_t* d);
void dijkstra_tunnel(dungeon_t* d);
void printTunnels(dungeon_t* d);

#ifdef __cplusplus
}
#endif

#endif
