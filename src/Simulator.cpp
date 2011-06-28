#include "Global.h"
#include "Simulator.h"

#include <assert.h>
#include <sys/time.h>

/*============================================================================
 *                                 Simulator
 *============================================================================*/
Simulator::Simulator()
{
    m_sim_mode = SIMMODE_REAL_IO; // by default, perform real io

    Bytes_inserted = 0;
    Bytes_read = 0;
    Bytes_written = 0;
    Num_ioops = 0;
    Fragm_total = 0;
    Fragm_count = 0;
    Time_passed.tv_sec = 0;
    Time_passed.tv_usec = 0;
}

/*============================================================================
 *                                 ~Simulator
 *============================================================================*/
Simulator::~Simulator()
{

}

/*============================================================================
 *                            set_simulation_mode
 *============================================================================*/
void Simulator::set_simulation_mode(Simulator::sim_mode mode)
{
    m_sim_mode = mode;
}

/*============================================================================
 *                           is_simulation_enabled
 *============================================================================*/
Simulator::sim_mode Simulator::get_simulation_mode(void)
{
    return m_sim_mode;
}


/*============================================================================
 *                            inc_bytes_inserted
 *============================================================================*/
void Simulator::inc_bytes_inserted(uint64_t count)
{
    Bytes_inserted += count;
}

/*============================================================================
 *                            get_bytes_inserted
 *============================================================================*/
uint64_t Simulator::get_bytes_inserted(void)
{
    return Bytes_inserted;
}

/*============================================================================
 *                             inc_bytes_read
 *============================================================================*/
void Simulator::inc_bytes_read(uint64_t count)
{
    Bytes_read += count;
}

/*============================================================================
 *                             get_bytes_read
 *============================================================================*/
uint64_t Simulator::get_bytes_read(void)
{
    return Bytes_read;
}

/*============================================================================
 *                           inc_bytes_written
 *============================================================================*/
void Simulator::inc_bytes_written(uint64_t count)
{
    Bytes_written += count;
}

/*============================================================================
 *                            get_bytes_written
 *============================================================================*/
uint64_t Simulator::get_bytes_written(void)
{
    return Bytes_written;
}

/*============================================================================
 *                                inc_ioops
 *============================================================================*/
void Simulator::inc_ioops(uint64_t count)
{
    Num_ioops += count;
}

/*============================================================================
 *                               get_iooops
 *============================================================================*/
uint64_t Simulator::get_ioops(void)
{
    return Num_ioops;
}

/*============================================================================
 *                             inc_time_passed
 *============================================================================*/
void Simulator::inc_time_passed(time_t sec_passed, suseconds_t usec_passed)
{
    Time_passed.tv_sec += sec_passed;
    Time_passed.tv_usec += usec_passed;
    assert(usec_passed < 1000000);
    if (Time_passed.tv_usec >= 1000000) {
        Time_passed.tv_sec += 1;
        Time_passed.tv_usec -= 1000000;
    }
}

/*============================================================================
 *                             get_time_passed
 *============================================================================*/
struct timeval Simulator::get_time_passed(void)
{
    return Time_passed;
}

/*============================================================================
 *                           add_cur_fragmentation
 *============================================================================*/
void Simulator::add_cur_fragmentation(int disk_runs)
{
    Fragm_total += disk_runs;
    Fragm_count++;
}

/*============================================================================
 *                          get_avg_fragmentation
 *============================================================================*/
float Simulator::get_avg_fragmentation(void)
{
    return (float)Fragm_total / (float)Fragm_count;
}


/*============================================================================
 *                              sanity_check
 *============================================================================*/
int Simulator::sanity_check()
{
    return 1;
}
