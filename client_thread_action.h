#define _CRT_SECURE_NO_WARNINGS

#include<windows.h>
#include<stdio.h>
#include<time.h>
#include "../../../common/common_structures.h"

void disconnect(Thread_args* arguments);

void connect_to_client(Thread_args* arguments);

void end_game(Thread_args* arguments);

void get_skill(Thread_args* arguments);

void get_ping_initialize(Thread_args* arguments);

void get_chest(Thread_args* arguments);

void move_player(Thread_args* arguments, const char* buf);

int get_ping_game(Thread_args* arguments);

void use_skill(Thread_args* args);