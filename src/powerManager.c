#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "processChores.h"
#include "customTypes.h"

#define GPS_PIPE_PATH "/tmp/gps_pipe"

#define PID_FILE_NAME "powermanager.pid"
int pid_file;

// Signal handler function for SIGINT
void sigintHandler(int signum) {
   printf("\nReceived SIGINT. Releasing PID lock file.\n");
   unlink(GPS_PIPE_PATH);
   release_pid_lock(pid_file, PID_FILE_NAME);
   exit(-1);
}

int main(int argc, char **argv) {

   // Set up the signal handler for SIGINT
   struct sigaction sa;
   sa.sa_handler = sigintHandler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;

   if (sigaction(SIGINT, &sa, NULL) == -1) {
      perror("Error setting up SIGINT handler");
   }

   pid_file = acquire_pid_lock(PID_FILE_NAME);
   if( pid_file == -1 ){
      handle_pid_lock_failure();
   }


   //logical steps

   //BLOCKING:
   //acquire process ids of all terminatable processes
   //dataCollector, compressor, awsUploader
   //if(failed critically) { system("shutdown -P now"); }

   int rc = mkfifo(GPS_PIPE_PATH, 0666);
   if( rc != 0){
      perror("powerMan: failed mkfifo(GPS_PIPE)");
   }

   int fifofd = open(GPS_PIPE_PATH, O_RDONLY | O_NONBLOCK);
   if (fifofd == -1){
      perror("powerMan: open gps fifo pipe failed");
      fprintf(stderr, "powerMan: no mechanism to determine motion from powerManager, must shutdown.\n");
      //system("shutdown -P now");
   }

   time_t begin_time = time(NULL);
   while(1){
      time_t current_time = time(NULL);
      double elapsed_time = difftime(current_time, begin_time);

      //probably poll before read
      gpsPipePacket_t gpsData;
      size_t readcount = read(fifofd, &gpsData, sizeof(gpsPipePacket_t));
      //readcount : -1 EAGAIN (would've blocked, pipe open, no data), 0 (EOF, pipe closed), >0 read data
      if(readcount == -1){
         perror("pipe read error");
      }

      printf("read from pipe %lu bytes.\n", readcount);
      printf("lat: %f, lon: %f\n", gpsData.lat, gpsData.lon);

      if(elapsed_time > 60){
         perror("powerMan: Failed to acquire any GPS data read from GPS fifo pipe.");
         break;
      }

      sleep(1);
   }

   //simulate work

   //close GPS data pipe and clean up pipe off filesystem
   close(fifofd);
   unlink(GPS_PIPE_PATH);

   //release pid lock file so we no longer signal that an instance of powerManager is running
   release_pid_lock(pid_file, PID_FILE_NAME);
   return 0;
}
