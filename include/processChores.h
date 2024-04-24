#ifndef PROCESS_CHORES_H
#define PROCESS_CHORES_H

char* get_pid_lock_filepath(const char* pidFilename);

int acquire_pid_lock(const char* pidFilename);
void release_pid_lock(int pid_file, const char* pidFilename);
void handle_pid_lock_failure();

#endif
