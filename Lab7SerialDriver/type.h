#ifndef TYPE_H
#define TYPE_H

#define NULL      0
#define NPROC     9
#define SSIZE   512

#define FREE      0    /* PROC status */
#define READY     1
#define SLEEP     2
#define ZOMBIE    3
#define BLOCK     4

#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

typedef struct proc{
    struct proc *next;
    int    ksp;
    int    uss, usp;

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];
    int    kstack[SSIZE];      // per proc stack area
}PROC;

#endif
