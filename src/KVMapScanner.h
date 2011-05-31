#ifndef KVMAPSCANNER_H
#define KVMAPSCANNER_H

#include "KVScanner.h"
#include "KVMap.h"

class KVMapScanner: public KVScanner {

public:

    /**
     * constructor
     */
    KVMapScanner(KVMap *kvmap);

    /**
     * destructor
     */
    ~KVMapScanner();

    /**
     * get next <key, value> pair
     */
    bool next(const char **key, const char **value);

protected:
    KVMap                       *m_kvmap;
    KVMap::kvmap::iterator       iter;

};

#endif
