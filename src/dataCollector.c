#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "processChores.h"
#include "customTypes.h"

#define GPS_PIPE_PATH "/tmp/gps_pipe"

#define PID_FILE_NAME "datacollector.pid"
int pid_file;

volatile sig_atomic_t terminateFlag = 0;
void signalHandler(int signum) {
   switch (signum){
      case SIGINT:
      case SIGPIPE:
         printf("\nReceived SIGINT or SIGPIPE. Releasing PID lock file.\n");
         unlink(GPS_PIPE_PATH);
         release_pid_lock(pid_file, PID_FILE_NAME);
         exit(-1);
         break;
      case SIGUSR1:
         terminateFlag = 1;
         break;
      default:
         break;
   }
}

int main() {

   // Set up the signal handler for SIGINT
   struct sigaction sa;
   sa.sa_handler = signalHandler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;

   if (sigaction(SIGINT, &sa, NULL) == -1) {
      perror("Error setting up SIGINT handler");
      exit(-1);
   }
   if (sigaction(SIGPIPE, &sa, NULL) == -1) {
      perror("Error setting up SIGPIPE handler");
      exit(-1);
   }
   if (sigaction(SIGUSR1, &sa, NULL) == -1) {
      perror("Error setting up SIGUSR1 handler");
      exit(-1);
   }

   pid_file = acquire_pid_lock(PID_FILE_NAME);
   if( pid_file == -1 ){
      handle_pid_lock_failure();
   }

   // Create or open the named pipe for writing
   mkfifo(GPS_PIPE_PATH, 0666);
   int pipe_fd = open(GPS_PIPE_PATH, O_WRONLY);

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
      int wc = write(pipe_fd, &gpsData, sizeof(gpsData));
      if( wc == -1 ){
         perror("pipe write error");
      }
      sleep(2);
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

