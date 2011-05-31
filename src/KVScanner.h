#ifndef KVSCANNER_H
#define KVSCANNER_H

class KVScanner {

public:

    /**
     * get next <key, value> pair
     */
    virtual bool next(const char **key, const char **value) = 0;

protected:

};

#endif
