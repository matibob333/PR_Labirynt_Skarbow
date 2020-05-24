#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<stdio.h>
#include<time.h>
#include "../../../common/common_structures.h"

void read_BMP(char* filename, Map_type* map);

void set_treasures(Map_type* map);

void set_exit(Map_type* map);

int find_best_player_index_excluding_arg_player(Map_type* map, int player);
