#include "path.h"

typedef struct path {
    heap_node_t* hn;
    uint8_t pos[num_dims];
} path_t;

static dungeon_t* dungeon;

/*
 * Comparator for the heap for the nonTunnel path
 */
int32_t nonTunnel_path_cmp(const void* key, const void* with)
{
    return ((int32_t)dungeon->nonTunnelPath[((path_t*)key)->pos[dim_y]][((path_t*)key)->pos[dim_x]]) -
        ((int32_t)dungeon->nonTunnelPath[((path_t*)with)->pos[dim_y]][((path_t*)with)->pos[dim_x]]);
}

/*
 * Finds the most efficent path from player to all other corridor and room gameboard locations
 */
void dijkstra_nontunnel(dungeon_t* d)
{
    static path_t path[DUNGEON_HEIGHT][DUNGEON_WIDTH], *p;
    static int initialized = 0;
    heap_t h;
    int x, y;

    if(!initialized) {
        dungeon = d;
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
            d->nonTunnelPath[y][x] = UCHAR_MAX;
        }
    }

    d->nonTunnelPath[d->pcLoc[dim_y]][d->pcLoc[dim_x]] = 0;

    heap_init(&h, nonTunnel_path_cmp, nullptr);

    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            if(d->gameBoard[y][x] == blk_corridor || d->gameBoard[y][x] == blk_room ||
                d->gameBoard[y][x] == blk_stairs_down || d->gameBoard[y][x] == blk_stairs_up) {
                path[y][x].hn = heap_insert(&h, &path[y][x]);
            }
        }
    }
    while((p = (path_t*)heap_remove_min(&h))) {
        p->hn = nullptr;
        char yChg, xChg;
        for(yChg = -1; yChg <= 1; yChg++) {
            for(xChg = -1; xChg <= 1; xChg++) {
                if(xChg == 0 && yChg == 0) {
                } else {
                    y = p->pos[dim_y] + yChg;
                    x = p->pos[dim_x] + xChg;
                    if((path[y][x].hn) &&
                        (d->nonTunnelPath[y][x] > d->nonTunnelPath[p->pos[dim_y]][p->pos[dim_x]] + 1)) {
                        d->nonTunnelPath[y][x] = d->nonTunnelPath[p->pos[dim_y]][p->pos[dim_x]] + 1;
                        heap_decrease_key_no_replace(&h, path[y][x].hn);
                    }
                }
            }
        }
    }
    heap_delete(&h);
}

/*
 * Comparator used by the heap for figuring out the tunnel path
 */
int32_t tunnel_path_cmp(const void* key, const void* with)
{
    return ((int32_t)dungeon->tunnelPath[((path_t*)key)->pos[dim_y]][((path_t*)key)->pos[dim_x]]) -
        ((int32_t)dungeon->tunnelPath[((path_t*)with)->pos[dim_y]][((path_t*)with)->pos[dim_x]]);
}

/*
 * Calculates the movement cost for a monster
 */
int tunnel_movement_cost(dungeon_t* d, int y, int x)
{
    return d->hardness[y][x] / 85 + 1;
}

/*
 * Finds the most efficent path from player to all other corridor and room gameboard locations
 */
void dijkstra_tunnel(dungeon_t* d)
{
    static path_t path[DUNGEON_HEIGHT][DUNGEON_WIDTH], *p;
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
            d->tunnelPath[y][x] = UCHAR_MAX;
        }
    }

    d->tunnelPath[d->pcLoc[dim_y]][d->pcLoc[dim_x]] = 0;

    heap_init(&h, tunnel_path_cmp, nullptr);

    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            if(d->gameBoard[y][x] != blk_immutable_rock) {
                path[y][x].hn = heap_insert(&h, &path[y][x]);
            }
        }
    }

    while((p = (path_t*)heap_remove_min(&h))) {

        p->hn = nullptr;
        char yChg, xChg;
        for(yChg = -1; yChg <= 1; yChg++) {
            for(xChg = -1; xChg <= 1; xChg++) {
                if(xChg == 0 && yChg == 0) {
                } else {
                    y = p->pos[dim_y] + yChg;
                    x = p->pos[dim_x] + xChg;
                    if((path[y][x].hn) &&
                        (d->tunnelPath[y][x] > d->tunnelPath[p->pos[dim_y]][p->pos[dim_x]] +
                                tunnel_movement_cost(d, p->pos[dim_y], p->pos[dim_x]))) {
                        d->tunnelPath[y][x] = d->tunnelPath[p->pos[dim_y]][p->pos[dim_x]] +
                            tunnel_movement_cost(d, p->pos[dim_y], p->pos[dim_x]);
                        heap_decrease_key_no_replace(&h, path[y][x].hn);
                    }
                }
            }
        }
    }
    heap_delete(&h);
}

void printTunnels(dungeon_t* d)
{
    int x, y;
    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            printf("%d", d->nonTunnelPath[y][x] % 10);
        }
        printf("\n");
    }
    for(y = 0; y < DUNGEON_HEIGHT; y++) {
        for(x = 0; x < DUNGEON_WIDTH; x++) {
            printf("%d", d->tunnelPath[y][x] % 10);
        }
        printf("\n");
    }
}
