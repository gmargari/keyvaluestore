#include "../RequestThrottle.h"

#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <math.h>

using namespace std;

int main(void) {
    struct timeval now, start, end;
    int num_req, req_per_sec;
    double total_time;
    RequestThrottle *throttler;

    gettimeofday(&now, NULL);
//now.tv_usec = 100;
    cout << "seed: " << now.tv_usec << endl;
    srand(now.tv_usec);

    num_req = 1 + rand() % 1000;
    req_per_sec = 1 + rand() % 20;
    cout << "num of requests: " << num_req << endl;
    cout << "target rate: " << req_per_sec << " rps" << endl;

    throttler = new RequestThrottle(req_per_sec);

    gettimeofday(&start, NULL);
    for (int i = 0; i < num_req; i++) {
        throttler->throttle();
        usleep(rand() % 100000); // perform actual request
    }
    gettimeofday(&end, NULL);

    delete throttler;

    total_time = ceil(end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0);
    cout << "actual rate: " << ((double)num_req / total_time) << " rps (" << num_req << " req / " << total_time << " sec)" << endl;

    if (num_req / total_time <= req_per_sec) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}
