#include "Global.h"
#include "GeomCompactionManager.h"

#include "MemStore.h"
#include "DiskStore.h"
#include "MapInputStream.h"
#include "DiskFile.h"
#include "DiskFileInputStream.h"

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>

/*============================================================================
 *                             GeomCompactionManager
 *============================================================================*/
GeomCompactionManager::GeomCompactionManager(MemStore *memstore, DiskStore *diskstore)
: CompactionManager(memstore, diskstore)
{
    m_R = DEFAULT_GEOM_R;
    m_P = DEFAULT_GEOM_P;
}

/*============================================================================
 *                            ~GeomCompactionManager
 *============================================================================*/
GeomCompactionManager::~GeomCompactionManager()
{

}

/*============================================================================
 *                                   set_R
 *============================================================================*/
void GeomCompactionManager::set_R(int r)
{
    assert(r >= 2 || (m_P && r >= 1));
    m_R = r;
}

/*============================================================================
 *                                   get_R
 *============================================================================*/
int GeomCompactionManager::get_R(void)
{
    return m_R;
}

/*============================================================================
 *                                   set_P
 *============================================================================*/
void GeomCompactionManager::set_P(int p)
{
    assert(p >= 1);
    m_P = p;
}

/*============================================================================
 *                                   get_P
 *============================================================================*/
int GeomCompactionManager::get_P(void)
{
    return m_P;
}

/*============================================================================
 *                             partition_minsize
 *============================================================================*/
int GeomCompactionManager::partition_minsize(int partition_num)
{
    assert(m_R >= 2 || (m_P && m_R >= 1));
    if (m_P) {
        return (int)pow(m_R, partition_num);
    } else {
        return (int)pow(m_R, partition_num);
    }
}

/*============================================================================
 *                             partition_maxsize
 *============================================================================*/
int GeomCompactionManager::partition_maxsize(int partition_num)
{
    // the only difference between P being and not being constant
    // is that in the first case partition 0 has limit R maxsize in the
    // second case it has maxsize R-1.
    assert(m_R >= 2 || (m_P && m_R >= 1));
    if (m_P) {
        return (int)pow(m_R, partition_num + 1);
    } else {
        return (m_R - 1) * (int)pow(m_R, partition_num);
    }
}

/*============================================================================
 *                              partition_num
 *============================================================================*/
int GeomCompactionManager::partition_num(int partition_size)
{
    for (int i = 0; ; i++) {
        if (partition_size <= partition_maxsize(i)) {
            assert(partition_size >= partition_minsize(i));
            return i;
        }
    }

    assert("can't get here" && 0);
}


/*============================================================================
 *                             compute_current_R
 *============================================================================*/
int GeomCompactionManager::compute_current_R()
{
    int size_of_bytes_inserted = 0;

    size_of_bytes_inserted = 1; // 1 for memstore that will be flushed to disk
    for (uint i = 0; i < m_partition_size.size(); i++) {
        size_of_bytes_inserted += m_partition_size[i];
    }
    return (int)ceil( pow(size_of_bytes_inserted, 1.0 / m_P) );
}

/*============================================================================
 *                                flush_bytes
 *============================================================================*/
void GeomCompactionManager::flush_bytes(void)
{
    DiskFile *disk_file;
    vector<InputStream *> istreams_to_merge;
    vector<DiskFile *> &r_disk_files = m_diskstore->m_disk_files;
    vector<DiskFileInputStream *> &r_disk_istreams = m_diskstore->m_disk_istreams;
    int size, count, part_num;

    assert(sanity_check());

    // if we bound number of partitions, re-adjust R so num of partitions is <= P
    if (m_P) {
        set_R(compute_current_R());
    }

    // merge the memstore with the sub-index in the first partition.
    // if the merged sub-index is greater than the maximum size of the partition,
    // merge it with the next biggest sub-index. repeat until the merged
    // sub-index fits within a partition (we may need to create a new partition
    // that will store the sub-index, if new sub-index doesn't fit in any
    // existing partition)
    count = 0;
    size = 1; // 1 for memstore
    for (uint i = 0; i < m_partition_size.size(); i++) {
        if (m_partition_size[i]) {
            count++;
            size += m_partition_size[i];
        }
        if (size <= partition_maxsize(i)) {
            break;
        }
    }
    assert(count <= (int)r_disk_files.size());

    // add memstore stream
    m_memstore->m_inputstream->reset();
    istreams_to_merge.push_back(m_memstore->m_inputstream);

    // add disk streams
    for (int i = 0; i < count; i++) {
        r_disk_istreams[i]->reset();
        istreams_to_merge.push_back(r_disk_istreams[i]);
    }

    // merge sub-indexes with memstore, writing output to a new file
    disk_file = merge_streams(istreams_to_merge);

    // clear memstore
    memstore_clear();

    // delete all files merged as well as their input streams
    for (int i = 0; i < count; i++) {
        r_disk_files[i]->delete_from_disk();
        delete r_disk_files[i];
        delete r_disk_istreams[i];
    }
    r_disk_files.erase(r_disk_files.begin(), r_disk_files.begin() + count);
    r_disk_istreams.erase(r_disk_istreams.begin(), r_disk_istreams.begin() + count);

    // add new file at the front, since it contains most recent <k,v> pairs
    r_disk_files.insert(r_disk_files.begin(), disk_file);
    r_disk_istreams.insert(r_disk_istreams.begin(), new DiskFileInputStream(disk_file, MERGE_BUFSIZE));

    // update 'm_partition_size' vector: zero the sizes of the 'count' partitions merged
    for (uint i = 0; i < m_partition_size.size(); i++) {
        if (m_partition_size[i] && count > 0) {
            m_partition_size[i] = 0;
            count--;
        }
    }
    // update 'm_partition_size' vector: add new partition
    part_num = partition_num(size);
    if (part_num == (int)m_partition_size.size()) {
        m_partition_size.push_back(size);
    } else {
        assert(part_num < (int)m_partition_size.size());
        m_partition_size[part_num] = size;
    }

    assert(sanity_check());
}

/*============================================================================
 *                              print_partitions
 *============================================================================*/
void GeomCompactionManager::print_partitions()
{
    printf("part.size: |"); for (uint i = 0; i < m_partition_size.size(); i++) printf("%3d |", m_partition_size[i]);   printf("\n");
    printf("minsize:   |"); for (uint i = 0; i < m_partition_size.size(); i++) printf("%3d |", partition_minsize(i));  printf("\n");
    printf("maxsize:   |"); for (uint i = 0; i < m_partition_size.size(); i++) printf("%3d |", partition_maxsize(i));  printf("\n");
}

/*============================================================================
 *                                sanity_check
 *============================================================================*/
int GeomCompactionManager::sanity_check()
{
    static uint64_t lastsize = 0;
    uint64_t cursize = 0;

    assert(m_diskstore->m_disk_files.size() == m_diskstore->m_disk_istreams.size());
    assert(m_partition_size.size() >= m_diskstore->m_disk_files.size());
    for (uint i = 0; i < m_partition_size.size(); i++) {
        // if P != 0, there's a case when R increases before a flush that the assertion below does not hold
        assert(m_P != 0 || (m_partition_size[i] == 0 || m_partition_size[i] >= partition_minsize(i)));
        assert(m_partition_size[i] <= partition_maxsize(i));
        cursize += m_partition_size[i];
    }
    assert(cursize == lastsize || cursize == lastsize + 1);
    lastsize = cursize;

    if (m_P) {
        assert((int)m_diskstore->m_disk_files.size() <= m_P);
    }

    return 1;
}
