#include "../Global.h"
#include "../Map.h"
#include "../MemStore.h"
#include "../Statistics.h"

#include <iostream>
#include <cstdlib>
#include <cassert>

using namespace std;

/*============================================================================
 *                                 main
 *============================================================================*/
int main() {
    MemStore *memstore = new MemStore();
    int t = 1, map1_num, map2_num, map3_num;
    char *k1 = (char *)"", *k2 = (char *)"f3", *k3 = (char *)"p3";

    global_stats_init(); // avoid assertion error...

    //memstore->add_map((char *)k1); this map is added by default in memstore's constructor
    memstore->add_map((char *)k2, strlen(k2));
    memstore->add_map((char *)k3, strlen(k3));

    memstore->put("a1", 2, "x", 1, t++); // belongs to map 0
    memstore->put("a2", 2, "x", 1, t++); // belongs to map 0
    memstore->put("p3", 2, "x", 1, t++); // belongs to map 2
    memstore->put("c1", 2, "x", 1, t++); // belongs to map 0
    memstore->put("f3", 2, "x", 1, t++); // belongs to map 1
    memstore->put("z1", 2, "x", 1, t++); // belongs to map 2
    memstore->put("f4", 2, "x", 1, t++); // belongs to map 1
    memstore->put("n1", 2, "x", 1, t++); // belongs to map 1
    memstore->put("d1", 2, "x", 1, t++); // belongs to map 0
    memstore->put("f1", 2, "x", 1, t++); // belongs to map 0
    memstore->put("f2", 2, "x", 1, t++); // belongs to map 0
    memstore->put("p1", 2, "x", 1, t++); // belongs to map 1
    memstore->put("p2", 2, "x", 1, t++); // belongs to map 1
    memstore->put("zz", 2, "x", 1, t++); // belongs to map 2

    memstore->put("zz", 2, "y", 1, t++); // belongs to map 2 (replace)
    memstore->put("a1", 2, "y", 1, t++); // belongs to map 0 (replace)
    memstore->put("n1", 2, "y", 1, t++); // belongs to map 1 (replace)
    memstore->put("p2", 2, "y", 1, t++); // belongs to map 1 (replace)
    memstore->put("p3", 2, "y", 1, t++); // belongs to map 2 (replace)
    memstore->put("f2", 2, "y", 1, t++); // belongs to map 0 (replace)
    memstore->put("f3", 2, "y", 1, t++); // belongs to map 1 (replace)

    map1_num = memstore->get_map(k1, strlen(k1))->get_num_keys();
    map2_num = memstore->get_map(k2, strlen(k2))->get_num_keys();
    map3_num = memstore->get_map(k3, strlen(k3))->get_num_keys();

    if (map1_num != 6 || map2_num != 5 || map3_num != 3) {
        cout << "num of keys per map should be <6, 5, 3>, but is <" << map1_num << ", " << map2_num << ", " << map3_num << ">" << endl;
        return EXIT_FAILURE;
    }

    memstore->clear_map(k3, strlen(k3)); // clear map 2
    memstore->put("a3", 2, "y", 1, t++); // belongs to map 0
    memstore->put("z3", 2, "y", 1, t++); // belongs to map 2
    memstore->put("z4", 2, "y", 1, t++); // belongs to map 2
    memstore->put("k3", 2, "y", 1, t++); // belongs to map 1

    map1_num = memstore->get_map(k1, strlen(k1))->get_num_keys();
    map2_num = memstore->get_map(k2, strlen(k2))->get_num_keys();
    map3_num = memstore->get_map(k3, strlen(k3))->get_num_keys();

    if (map1_num != 7 || map2_num != 6 || map3_num != 2) {
        cout << "num of keys per map should be <7, 6, 2>, but is <" << map1_num << ", " << map2_num << ", " << map3_num << ">" << endl;
        return EXIT_FAILURE;
    }

    delete memstore;

    cout << "Everything ok!" << endl;

    return EXIT_SUCCESS;
}
