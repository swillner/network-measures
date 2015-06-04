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

        snprintf(q, 512, "select count(value) from entries where year=%d and inserted>(select updated from visualizations where name='Betweenness' and year=%d)", year, year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }
        res = mysql_use_result(mysql);
        if((row = mysql_fetch_row(res)) == 0 || atoi(row[0]) == 0) {
            mysql_free_result(res);
            continue;
        }
        mysql_free_result(res);

        err = read_network(year);
        if (err) {
            return err;
        }

        std::cout << "Calculating " << year << "\n";
        err = disconnect();
        if (err) {
            return err;
        }

        Basetype* betweenness = create_array(0);
        Basetype* in_flow = create_array(0);
        Basetype* out_flow = create_array(0);
        for (int v = 0; v < network_size; v++) {
            for (int w = 0; w < network_size; w++) {
                in_flow[v] += flows[w][v];
                out_flow[v] += flows[v][w];
            }
        }

        // Algorithm according to "A space-efficient parallel algorithm for computing betweenness centrality in distributed memory", p. 2
        int s;
    #pragma omp parallel default(none) shared(betweenness, flows, network_size, in_flow, std::cerr)
        {
    #pragma omp for schedule(guided) nowait
            for (s = 0; s < network_size; s++) {
                std::vector<int> S;
                std::queue<int> PQ;
                BasetypeInt* sigma;
                Basetype* delta;
                Basetype* dist;
                BasetypeInt** P;
                BasetypeInt* P_size;
                sigma = create_array_int(0);
                sigma[s] = 1;
                delta = create_array(0);
                P = create_double_int_array(0);
                P_size = create_array_int(0);
                dist = create_array(-1);
                dist[s] = 0;
                PQ.push(s);
                while (!PQ.empty()) {
                    int v = PQ.front();
                    PQ.pop();
                    S.push_back(v);
                    for (int w = 0; w < network_size; w++) {
                        if (w != v && w != s && flows[v][w] > 0) {
                            Basetype c_v_w = flows[v][w];
                            if (c_v_w > 0) {
                                if (dist[w] < 0) {
                                    PQ.push(w);
                                    dist[w] = dist[v] + 1;
                                }
                                if (dist[w] == dist[v] + 1) {
                                    sigma[w] += sigma[v];
                                    P[w][P_size[w]] = v;
                                    P_size[w]++;
                                }
                            }
                        }
                    }
                }

                while (!S.empty()) {
                    int w = S.back();
                    S.pop_back();

                    for (int v_index = 0; v_index < P_size[w]; v_index++) {
                        int v = P[w][v_index];
                        delta[v] += ((1 + delta[w]) * (Basetype) sigma[v]) / (Basetype) sigma[w];
                    }

                    if (w != s) {
                        #pragma omp atomic
                            betweenness[w] += delta[w];
                    }
                }
                delete[] delta;
                delete[] sigma;
                delete[] dist;
                delete[] P_size;
                free_double_int_array(P);
            }
        }

        err = connect();
        if (err) {
            return err;
        }

        snprintf(q, 512, "delete from visualization_data where visualization in (select id from visualizations where name='Betweenness' and year=%d)", year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }

        snprintf(q, 512, "select id from visualizations where name='Betweenness' and year=%d", year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }
        res = mysql_use_result(mysql);
        row = mysql_fetch_row(res);
        int id = atoi(row[0]);
        mysql_free_result(res);

        std::stringstream query("insert into visualization_data (visualization, sector_from, region_from, value) values ", std::ios_base::app | std::ios_base::out);
        for (int js = 0; js < network_size; js++) {
            if (js > 0) {
                query << ",";
            }
            query << "(" << id << ",'" << sectors[get_sector(js)] << "','" << regions[get_region(js)] << "'," << betweenness[js] << ")";
        }
        if (mysql_query(mysql, query.str().c_str())) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }

        snprintf(q, 512, "update visualizations set updated=now() where name='Betweenness' and year=%d", year);
        if (mysql_query(mysql, q)) {
            std::cerr << mysql_error(mysql) << "\n";
            return -2;
        }

        delete[] betweenness;
        free_double_array(flows);

    }

    return 0;
}
