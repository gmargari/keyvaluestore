#ifndef GLOBAL_H
#define GLOBAL_H

#include <sys/time.h>

// set debug level:
//  0 - debugging disabled
//  1 - normal mode
//  2 - heavy debugging
#define DBGLVL 1

// byte conversion
#define b2kb(b) ((b)/(1024.0))
#define b2mb(b) ((b)/(1024.0*1024.0))
#define b2gb(b) ((b)/(1024.0*1024.0*1024.0))

#define kb2b(kb) ((uint64_t)((kb)*(1024)))
#define mb2b(mb) ((uint64_t)((mb)*(1024*1024)))
#define gb2b(gb) ((uint64_t)((gb)*(1024*1024*1024)))

#define kb2mb(kb) ((kb)/(1024.0))
#define kb2gb(kb) ((kb)/(1024.0*1024.0))

#define mb2kb(mb) ((uint64_t)((mb)*(1024)))
#define gb2kb(gb) ((uint64_t)((gb)*(1024*1024)))

#define mb2gb(mb) ((mb)/(1024.0))

#define gb2mb(gb) ((uint64_t)((gb)*(1024)))

// byte conversion
#define sec2hour(s) ((s)/3600.0)
#define sec2min(s)  ((s)/60.0)
#define sec2msec(s) ((s)*1000)
#define sec2usec(s) ((suseconds_t)((s)*1000000))

#define hour2sec(h) ((h)*3600.0)
#define min2sec(m)  ((m)*60.0)
#define msec2sec(m) ((m)/1000.0)
#define usec2sec(u) ((u)/1000000.0))

#endif
