#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "processChores.h"
#include "customTypes.h"

#define PID_FILE_NAME "minpipewriter.pid"
#define GPS_PIPE_PATH "/tmp/gps_pipe"

int main(int argc, char** argv) {
   int pid_file = acquire_pid_lock(PID_FILE_NAME);
   if( pid_file == -1 ){
      handle_pid_lock_failure();
   }

   // Create or open the named pipe for writing
   int rc = mkfifo(GPS_PIPE_PATH, 0666);
   if( rc != 0 ){
      fprintf(stderr, "minWriter: error rc from mkfifo: %d\n", rc);
      perror("minWriter: failed mkfifo(/tmp/gps_pipe)");
   }

   int pipe_fd = open(GPS_PIPE_PATH, O_WRONLY | O_NONBLOCK);
   if( pipe_fd == -1 ){
      perror("minWriter: failed open pipe for write");
   }

   gpsPipePacket_t gpsData;
   for(int i = 0; i < 5; i++){
      gpsData.lat = 19.00+i;
      gpsData.lon = 23.00+i;
      write(pipe_fd, &gpsData, sizeof(gpsData));
      sleep(10);
   }

   // Cleanup and exit
   close(pipe_fd);
   unlink(GPS_PIPE_PATH);

   release_pid_lock(pid_file, PID_FILE_NAME);
   printf("collector: Exiting\n");
   return 0;
}
