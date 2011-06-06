#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Global.h"

#include <sys/time.h>
#include <stdint.h>

class Simulator
{
public:

    enum sim_mode { SIMMODE_REAL_IO, SIMMODE_SIMULATE_IO };

    Simulator();
    ~Simulator();

    // simulation mode
    void     set_simulation_mode(sim_mode mode);
    sim_mode get_simulation_mode(void);

    // bytes inserted
    void inc_bytes_inserted(uint64_t count);
    uint64_t get_bytes_inserted(void);

    // bytes read
    void inc_bytes_read(uint64_t count);
    uint64_t get_bytes_read(void);

    // bytes written
    void inc_bytes_written(uint64_t count);
    uint64_t get_bytes_written(void);

    // num ioops
    void inc_ioops(uint64_t count);
    uint64_t get_ioops(void);

    // time passed
    void inc_time_passed(time_t sec_passed, suseconds_t usec_passed);
    struct timeval get_time_passed(void);

    // fragmentation
    void add_cur_fragmentation(int disk_runs);
    float get_avg_fragmentation(void);

protected:

    void sanity_check();

    // are we going to simulate IO or perform real IO?
    sim_mode m_sim_mode;

    uint64_t Bytes_inserted;
    uint64_t Bytes_read;
    uint64_t Bytes_written;
    uint64_t Num_ioops;
    uint64_t Fragm_total;
    long int Fragm_count;
    struct timeval Time_passed;
};

#endif
