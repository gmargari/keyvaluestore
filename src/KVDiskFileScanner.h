#ifndef KVDISKFILESCANNER_H
#define KVDISKFILESCANNER_H

#include "KVScanner.h"
#include "KVDiskFile.h"

class KVDiskFileScanner: public KVScanner {

public:

    /**
     * constructor
     */
    KVDiskFileScanner();

    /**
     * destructor
     */
    ~KVDiskFileScanner();

    /**
     * get next <key, value> pair
     */
    bool next(char *key, char *value);
    
protected:
    KVDiskFile  *m_diskfile;
//     KVDiskFile::iterator iter;

};

#endif
