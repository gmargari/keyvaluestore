#ifndef KVDISKFILE_H
#define KVDISKFILE_H

#include "Global.h"
#include "KVSerialization.h"

class KVDiskFile {

public:

    /**
     * constructor
     */
    KVDiskFile();
    
    /**
     * destructor
     */
    ~KVDiskFile();

    /**
     * serialize and append <key, value> at disk file
     */
    void write(const char *key, const char *value);

protected:

};

#endif
