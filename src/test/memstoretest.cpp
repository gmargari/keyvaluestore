#include "../Global.h"
#include "../Map.h"
#include "../MemStore.h"
#include "../Statistics.h"

#include <iostream>
#include <stdlib.h>
#include <assert.h>

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
    memstore->add_map(Slice(k2, strlen(k2)));
    memstore->add_map(Slice(k3, strlen(k3)));

    memstore->put(Slice("a1", 2), Slice("x", 1), t++); // belongs to map 0
    memstore->put(Slice("a2", 2), Slice("x", 1), t++); // belongs to map 0
    memstore->put(Slice("p3", 2), Slice("x", 1), t++); // belongs to map 2
    memstore->put(Slice("c1", 2), Slice("x", 1), t++); // belongs to map 0
    memstore->put(Slice("f3", 2), Slice("x", 1), t++); // belongs to map 1
    memstore->put(Slice("z1", 2), Slice("x", 1), t++); // belongs to map 2
    memstore->put(Slice("f4", 2), Slice("x", 1), t++); // belongs to map 1
    memstore->put(Slice("n1", 2), Slice("x", 1), t++); // belongs to map 1
    memstore->put(Slice("d1", 2), Slice("x", 1), t++); // belongs to map 0
    memstore->put(Slice("f1", 2), Slice("x", 1), t++); // belongs to map 0
    memstore->put(Slice("f2", 2), Slice("x", 1), t++); // belongs to map 0
    memstore->put(Slice("p1", 2), Slice("x", 1), t++); // belongs to map 1
    memstore->put(Slice("p2", 2), Slice("x", 1), t++); // belongs to map 1
    memstore->put(Slice("zz", 2), Slice("x", 1), t++); // belongs to map 2

    memstore->put(Slice("zz", 2), Slice("y", 1), t++); // belongs to map 2 (replace)
    memstore->put(Slice("a1", 2), Slice("y", 1), t++); // belongs to map 0 (replace)
    memstore->put(Slice("n1", 2), Slice("y", 1), t++); // belongs to map 1 (replace)
    memstore->put(Slice("p2", 2), Slice("y", 1), t++); // belongs to map 1 (replace)
    memstore->put(Slice("p3", 2), Slice("y", 1), t++); // belongs to map 2 (replace)
    memstore->put(Slice("f2", 2), Slice("y", 1), t++); // belongs to map 0 (replace)
    memstore->put(Slice("f3", 2), Slice("y", 1), t++); // belongs to map 1 (replace)

    map1_num = memstore->get_map_num_keys(Slice(k1, strlen(k1)));
    map2_num = memstore->get_map_num_keys(Slice(k2, strlen(k2)));
    map3_num = memstore->get_map_num_keys(Slice(k3, strlen(k3)));

    if (map1_num != 6 || map2_num != 5 || map3_num != 3) {
        cout << "num of keys per map should be <6, 5, 3>, but is <" << map1_num << ", " << map2_num << ", " << map3_num << ">" << endl;
        return EXIT_FAILURE;
    }

    memstore->clear_map(Slice(k3, strlen(k3))); // clear map 2
    memstore->put(Slice("a3", 2), Slice("y", 1), t++); // belongs to map 0
    memstore->put(Slice("z3", 2), Slice("y", 1), t++); // belongs to map 2
    memstore->put(Slice("z4", 2), Slice("y", 1), t++); // belongs to map 2
    memstore->put(Slice("k3", 2), Slice("y", 1), t++); // belongs to map 1

    map1_num = memstore->get_map_num_keys(Slice(k1, strlen(k1)));
    map2_num = memstore->get_map_num_keys(Slice(k2, strlen(k2)));
    map3_num = memstore->get_map_num_keys(Slice(k3, strlen(k3)));

    if (map1_num != 7 || map2_num != 6 || map3_num != 2) {
        cout << "num of keys per map should be <7, 6, 2>, but is <" << map1_num << ", " << map2_num << ", " << map3_num << ">" << endl;
        return EXIT_FAILURE;
    }

    delete memstore;

    cout << "Everything ok!" << endl;

    return EXIT_SUCCESS;
}
