#ifndef _NETWORK_H
#define _NETWORK_H

#include <string.h>
#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>
#include <cmath>
#include <sstream>
#include <queue>

typedef double Basetype;
typedef int BasetypeInt;
extern std::unordered_map<std::string, int> regions_map;
extern std::unordered_map<std::string, int> sectors_map;
extern std::vector<std::string> regions;
extern std::vector<std::string> sectors;
extern std::vector<int> rows;
extern int network_size;
extern Basetype** flows;
extern MYSQL* mysql;

Basetype** create_double_array(Basetype val);
void free_double_array(Basetype** array);
BasetypeInt** create_double_int_array(BasetypeInt val);
void free_double_int_array(BasetypeInt** array);
int get_index(int i, int r);
int get_sector(int index);
int get_region(int index);
Basetype* create_array(Basetype val);
BasetypeInt* create_array_int(BasetypeInt val);
int region_index(std::string region);
int sector_index(std::string sector);
bool in_array(int v, int* a, int length);

int init_network();
int read_network(int year);
int connect();
int disconnect();

#endif

