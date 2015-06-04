#include "network.h"

std::unordered_map<std::string, int> regions_map;
std::unordered_map<std::string, int> sectors_map;
std::vector<std::string> regions;
std::vector<std::string> sectors;
std::vector<int> rows;
int network_size;
Basetype** flows;
MYSQL* mysql;

Basetype** create_double_array(Basetype val) {
    Basetype** array = new Basetype*[network_size];
    for (int i = 0; i < network_size; i++) {
        array[i] = new Basetype[network_size];
        for (int r = 0; r < network_size; r++) {
            array[i][r] = val;
        }
    }
    return array;
}

void free_double_array(Basetype** array) {
    for (int i = 0; i < network_size; i++) {
        delete[] array[i];
    }
    delete[] array;
}

BasetypeInt** create_double_int_array(BasetypeInt val) {
    BasetypeInt** array = new BasetypeInt*[network_size];
    for (int i = 0; i < network_size; i++) {
        array[i] = new BasetypeInt[network_size];
        for (int r = 0; r < network_size; r++) {
            array[i][r] = val;
        }
    }
    return array;
}

void free_double_int_array(BasetypeInt** array) {
    for (int i = 0; i < network_size; i++) {
        delete[] array[i];
    }
    delete[] array;
}

int get_index(int i, int r) {
    return i + r * sectors.size();
}

int get_sector(int index) {
    return index % sectors.size();
}

int get_region(int index) {
    return index / sectors.size();
}

Basetype* create_array(Basetype val) {
    Basetype* array = new Basetype[network_size];
    for (int i = 0; i < network_size; i++) {
        array[i] = val;
    }
    return array;
}

BasetypeInt* create_array_int(BasetypeInt val) {
    BasetypeInt* array = new BasetypeInt[network_size];
    for (int i = 0; i < network_size; i++) {
        array[i] = val;
    }
    return array;
}

int region_index(std::string region) {
    for (int i = 0; i < regions.size(); i++) {
        if (region == regions[i]) {
            return i;
        }
    }
    return -1;
}

int sector_index(std::string sector) {
    for (int i = 0; i < sectors.size(); i++) {
        if (sector == sectors[i]) {
            return i;
        }
    }
    return -1;
}

bool in_array(int v, int* a, int length) {
    for (int i = 0; i < length; i++) {
        if (a[i] == v) {
            return true;
        }
    }
    return false;
}

int connect() {
    mysql = mysql_init(0);
    mysql = mysql_real_connect(mysql, "localhost", "zeean", "J$58v=H3fd%6", "zeean", 0, 0, 0);
    if (!mysql) {
        std::cerr << "Could not connect to database\n";
        return -1;
    }
    return 0;
}

int init_network() {
    MYSQL_ROW row;
    MYSQL_RES* res;

    int err = connect();
    if (err) {
        return err;
    }

    if (mysql_query(mysql, "select id from regions where parent is null and id!='' order by name")) {
        std::cerr << mysql_error(mysql) << "\n";
    }
    res = mysql_use_result(mysql);
    int i = 0;
    while((row = mysql_fetch_row(res)) != 0) {
        regions_map.insert(std::make_pair(row[0], i));
        regions.push_back(row[0]);
        i++;
    }
    mysql_free_result(res);

    if (mysql_query(mysql, "select id from sectors where parent='ALL' order by id")) {
        std::cerr << mysql_error(mysql) << "\n";
        return -2;
    }
    res = mysql_use_result(mysql);
    i = 0;
    while((row = mysql_fetch_row(res)) != 0) {
        sectors_map.insert(std::make_pair(row[0], i));
        sectors.push_back(row[0]);
        i++;
    }
    mysql_free_result(res);

    network_size = sectors.size() * regions.size();
    return 0;
}

int read_network(int year) {
    MYSQL_ROW row;
    MYSQL_RES* res;
    char q[512];

    snprintf(q, 512, "select round(sum(value*confidence)*count(confidence)/sum(confidence), 3) as flow,sector_from,region_from,sector_to,region_to from entries where year=%d and sector_from!='' and region_from!='' and sector_to!='' and region_to!='' group by sector_from,region_from,sector_to,region_to", year);
    if (mysql_query(mysql, q)) {
        std::cerr << mysql_error(mysql) << "\n";
        return -2;
    }
    res = mysql_use_result(mysql);
    flows = create_double_array(0);
    while((row = mysql_fetch_row(res)) != 0) {
        int i = sectors_map[row[1]];
        int r = regions_map[row[2]];
        int j = sectors_map[row[3]];
        int s = regions_map[row[4]];
        flows[get_index(i, r)][get_index(j, s)] = atof(row[0]);
    }
    mysql_free_result(res);
    return 0;
}

int disconnect() {
    mysql_close(mysql);
    mysql_library_end();
    return 0;
}

