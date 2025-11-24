#ifndef DEBUGTYPES_H
#define DEBUGTYPES_H

//#define DEBUGGING

#ifdef DEBUGGING
#define debug(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#define debugln(x) Serial.println(x)
#else
#define debug(fmt,...)
#define debugln(x)
#endif

#endif