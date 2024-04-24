#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "processChores.h"
#include "customTypes.h"

#define PID_FILE_NAME "datacollector.pid"
#define GPS_PIPE_PATH "/tmp/gps_pipe"

volatile sig_atomic_t terminateFlag = 0;

void handleSignal(int signal) {
   if (signal == SIGUSR1) {
      terminateFlag = 1;
   }
}

int main() {
   int pid_file = acquire_pid_lock(PID_FILE_NAME);
   if( pid_file == -1 ){
      handle_pid_lock_failure();
   }


   signal(SIGUSR1, handleSignal);

   // Create or open the named pipe for writing
   mkfifo(GPS_PIPE_PATH, 0666);
   int pipe_fd = open(GPS_PIPE_PATH, O_WRONLY | O_NONBLOCK);

   // Signal powerManager that collector is ready
   //kill(/*powerManager pid*/, SIGUSR2);

   // Write GPS data to the pipe
   gpsPipePacket_t gpsData;
   gpsData.lat = 19.00;
   gpsData.lon = 21.00;
   gpsPipePacket_t gpsData2;
   gpsData.lat = 17.00;
   gpsData.lon = 22.00;
   printf("attempt write to pipe");
   fflush(stdout);
   while(1){
      write(pipe_fd, &gpsData, sizeof(gpsData));
   }

   // Simulate ongoing work
   sleep(5);

   // Terminate if signaled by powerManager
   while (!terminateFlag) {
      // Replace this with your actual logic based on termination conditions
      sleep(1);
   }

   // Cleanup and exit
   close(pipe_fd);
   unlink(GPS_PIPE_PATH);

   release_pid_lock(pid_file, PID_FILE_NAME);
   printf("collector: Exiting\n");
   return 0;
}

