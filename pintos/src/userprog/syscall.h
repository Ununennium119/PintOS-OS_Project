#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

/* Checks if a CONDITION is valid, if not, exits with status code -1 in FRAME */
#define IS_VALID(CONDITION, FRAME)                     \
        if (CONDITION)                     \
            exit_syscall(FRAME, -1);               

#endif /* userprog/syscall.h */
