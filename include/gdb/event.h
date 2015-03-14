#ifndef GDB_EVENT_H
#define GDB_EVENT_H


#include <gdb/gdb.h>
#include <gdb/result.h>


int evt_running(gdbif* gdb, gdb_result_t* result);
int evt_stopped(gdbif* gdb, gdb_result_t* result);


#endif // GDB_EVENT_H
