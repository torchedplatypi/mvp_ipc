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

void signalHandler(int signum) {
   switch (signum){
      case SIGINT:
         printf("\nReceived SIGINT. Releasing PID lock file.\n");
         unlink(GPS_PIPE_PATH);
         release_pid_lock(pid_file, PID_FILE_NAME);
         exit(-1);
         break;
      case SIGUSR2:
         printf("\nReceived SIGUSR2. Some process has checked in as terminated.");
         break;
      default:
         break;
   }
}

int main(int argc, char **argv) {

   // Set up the signal handler for SIGINT
   struct sigaction sa;
   sa.sa_handler = signalHandler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_SIGINFO;

   if (sigaction(SIGINT, &sa, NULL) == -1) {
      perror("Error setting up SIGINT handler");
      exit(-1);
   }
   if (sigaction(SIGUSR2, &sa, NULL) == -1) {
      perror("Error setting up SIGUSR2 handler");
      exit(-1);
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
      fprintf(stderr, "powerMan: probable expected behavior.\n");
   }

   int fifofd = open(GPS_PIPE_PATH, O_RDONLY | O_NONBLOCK);
   if (fifofd == -1){
      perror("powerMan: open gps fifo pipe failed");
      fprintf(stderr, "powerMan: no mechanism to determine motion from powerManager, must shutdown.\n");
      //RPITEST: system("shutdown -P now");
   }

   time_t last_valid_read_time = time(NULL);
   while(1){
      time_t current_time = time(NULL);
      double elapsed_time = difftime(current_time, last_valid_read_time);

      //probably poll before read
      gpsPipePacket_t gpsData;
      size_t readcount = read(fifofd, &gpsData, sizeof(gpsPipePacket_t));
      //readcount : -1 EAGAIN (would've blocked, pipe open, no data), 0 (EOF, pipe closed), >0 read data
      if(readcount == -1){
         perror("pipe read error");
      }else if(readcount == 8){
         last_valid_read_time = time(NULL);
         printf("read from pipe %lu bytes.\n", readcount);
         printf("lat: %f, lon: %f\n", gpsData.lat, gpsData.lon);
      }

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
