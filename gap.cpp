#include "network.h"

int main(int argc, char* argv[]) {
    char q[512];
    MYSQL_ROW row;
    MYSQL_RES* res;

    int err = init_network();
    if (err) {
        return err;
    }

    for (int year = 1990; year <= 2011; year++) {
        std::cout << "Querying " << year << "\n";

        snprintf(q, 512, "select count(value) from entries where year=%d and inserted>(select updated from visualizations where name='GAP2' and year=%d)", year, year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }
        res = mysql_use_result(mysql);
        if ((row = mysql_fetch_row(res)) == 0 || atoi(row[0]) == 0) {
            mysql_free_result(res);
            continue;
        }
        mysql_free_result(res);

        err = read_network(year);
        if (err) {
            return err;
        }

        std::cout << "Calculating " << year << "\n";

        Basetype* in_flow = create_array(0);
        Basetype* out_flow = create_array(0);
        int regions_size = regions.size();
        Basetype* total_output = new Basetype[regions_size];
        for (int r = 0; r < regions_size; r++) {
            total_output[r] = 0;
        }
        for (int v = 0; v < network_size; v++) {
            for (int w = 0; w < network_size; w++) {
                in_flow[v] += flows[w][v];
                out_flow[v] += flows[v][w];
            }
            total_output[get_region(v)] += out_flow[v];
        }
        int sectors_size = sectors.size();
        Basetype** in_flow_by_sector = new Basetype*[sectors_size];
        for (int i = 0; i < sectors_size; i++) {
            in_flow_by_sector[i] = create_array(0);
        }
        for (int v = 0; v < network_size; v++) {
            for (int w = 0; w < network_size; w++) {
                in_flow_by_sector[get_sector(v)][w] += flows[v][w];
            }
        }

        snprintf(q, 512, "delete from visualization_data where visualization in (select id from visualizations where (name='GAP1' or name='GAP2') and year=%d)", year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }

        snprintf(q, 512, "select id from visualizations where name='GAP1' and year=%d", year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }
        res = mysql_use_result(mysql);
        row = mysql_fetch_row(res);
        int id1 = atoi(row[0]);
        mysql_free_result(res);

        snprintf(q, 512, "select id from visualizations where name='GAP2' and year=%d", year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }
        res = mysql_use_result(mysql);
        row = mysql_fetch_row(res);
        int id2 = atoi(row[0]);
        mysql_free_result(res);

        for (int r = 0; r < regions_size; r++) {
            Basetype* damage1 = create_array(0);
            Basetype* damage2 = create_array(0);
            int js;

            #pragma omp parallel default(shared) private(js)
            {
                #pragma omp for schedule(guided) nowait
                for (js = 0; js < network_size; js++) {
                    if (get_region(js) == r) {
                        damage1[js] = 1;
                    } else {
                        for (int i = 0; i < sectors_size; i++) {
                            Basetype damage = flows[get_index(i, r)][js] / in_flow_by_sector[i][js];
                            if (damage1[js] < damage) {
                                damage1[js] = damage;
                            }
                        }
                    }
                }
            }
            #pragma omp parallel default(shared) private(js)
            {
                #pragma omp for schedule(guided) nowait
                for (js = 0; js < network_size; js++) {
                    if (get_region(js) == r) {
                        damage2[js] = 1;
                    } else {
                        for (int i = 0; i < sectors_size; i++) {
                            Basetype damage = 0;
                            for (int r = 0; r < regions_size; r++) {
                                int ir = get_index(i, r);
                                damage += damage1[ir] * flows[ir][js] / in_flow_by_sector[i][js];
                            }
                            if (damage2[js] < damage) {
                                damage2[js] = damage;
                            }
                        }
                    }
                }
            }

            Basetype* region_damage1 = new Basetype[regions_size];
            Basetype* region_damage2 = new Basetype[regions_size];
            for (int r = 0; r < regions_size; r++) {
                region_damage1[r] = 0;
                region_damage2[r] = 0;
            }
            for (int js = 0; js < network_size; js++) {
                int s = get_region(js);
                if (total_output[s] > 0) {
                    region_damage1[s] += damage1[js] * out_flow[js] / total_output[s];
                    region_damage2[s] += damage2[js] * out_flow[js] / total_output[s];
                }
            }
            delete[] damage1;
            delete[] damage2;

            std::stringstream query1("insert into visualization_data (visualization, region_from, region_to, value) values ", std::ios_base::app | std::ios_base::out);
            bool first1 = true;
            std::stringstream query2("insert into visualization_data (visualization, region_from, region_to, value) values ", std::ios_base::app | std::ios_base::out);
            bool first2 = true;
            for (int s = 0; s < regions_size; s++) {
                region_damage1[s] = round(region_damage1[s] * 1000) / 1000;
                if (region_damage1[s] > 0) {
                    if (first1) {
                        first1 = false;
                    } else {
                        query1 << ",";
                    }
                    query1 << "(" << id1 << ",'" << regions[r] << "','" << regions[s] << "'," << region_damage1[s] << ")";
                }
                region_damage2[s] = round(region_damage2[s] * 1000) / 1000;
                if (region_damage2[s] > 0) {
                    if (first2) {
                        first2 = false;
                    } else {
                        query2 << ",";
                    }
                    query2 << "(" << id2 << ",'" << regions[r] << "','" << regions[s] << "'," << region_damage2[s] << ")";
                }
            }
            if (!first1) {
                if (mysql_query(mysql, query1.str().c_str())) {
                    std::cerr << mysql_error(mysql) << "\n";
                    return -2;
                }
            }
            if (!first2) {
                if (mysql_query(mysql, query2.str().c_str())) {
                    std::cerr << mysql_error(mysql) << "\n";
                    return -2;
                }
            }

            delete[] region_damage1;
            delete[] region_damage2;
        }

        delete[] in_flow;
        delete[] out_flow;
        free_double_array(flows);
        for (int i = 0; i < sectors_size; i++) {
            delete[] in_flow_by_sector[i];
        }
        delete[] in_flow_by_sector;
        delete[] total_output;

        snprintf(q, 512, "update visualizations set updated=now() where (name='GAP1' or name='GAP2') and year=%d", year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }
    }

    return disconnect();
}

