#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>

#define PID_FILE_PREPATH "/var/run/user/"
char* get_pid_lock_filepath(const char* pidFilename ) {
   uid_t uid = getuid();

   size_t path_len = strlen(PID_FILE_PREPATH);
   size_t uid_len = snprintf(NULL, 0, "%d", uid);
   size_t fn_len = strlen(pidFilename);
   size_t total_len = path_len + uid_len + 1 + fn_len + 1;

   char* pid_lock_filename = (char *)malloc(total_len);
   if (pid_lock_filename != NULL){
      snprintf(pid_lock_filename, total_len, "%s%d/%s", PID_FILE_PREPATH, uid, pidFilename);
   }else{
      perror("E1: malloc failure: could not allocate pid filepath string.\n");
      return NULL;
   }
   return pid_lock_filename;
}

int acquire_pid_lock(const char* pidFilename) {
   char* pid_lock_filepath = get_pid_lock_filepath(pidFilename);
   int pid_file = -1;
   if( pid_lock_filepath != NULL ){
      //open the .pid file for writing
      pid_file = open(pid_lock_filepath, O_RDWR | O_CREAT, 0644);
      if ( pid_file != -1 ){
         if( flock(pid_file, LOCK_EX | LOCK_NB) == 0 ){
            pid_t pid = getpid();
            dprintf(pid_file, "%d\n", pid);
            printf("pid file lock acquired. Process ID %d written to file: %s\n", pid, pid_lock_filepath);
            return pid_file;
         }else{
            perror("E4: Failed to acquire pid file lock.\n");
            return -1;
         }
      }else{
         perror("E2: Failed to acquire pid file lock.\n");
      }
   }else{
      perror("E3: Failed to determine pid lock filename.\n"); 
   }
   free(pid_lock_filepath);

   return pid_file;
}

void release_pid_lock(int pid_file, const char* pidFilename) {
   if( pid_file != -1) {
      flock(pid_file, LOCK_UN);
      close(pid_file);
      printf("%s pid lock released.\n", pidFilename);
   }
   char* pid_lock_filepath = get_pid_lock_filepath(pidFilename);
   if( pid_lock_filepath != NULL ){
      remove(pid_lock_filepath);
   }else{
      perror("E5: Failed to remove pid lock file in cleanup.\n");
   }
}

void handle_pid_lock_failure() {
   perror("could not acquire pid lock. another instance of powerManager may be running.\n");
   perror("terminating this instance of powerManager now\n");
   //TODO: look for another instance of powerManager to confirm
   //      if not found, system("shutdown -P now");
   exit(1);
}
