
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>       // EXIT_SUCCESS
#include <stdio.h>        // perror
#include <unistd.h>       // read/write/lseek/STDIN_FILENO
#include <fcntl.h>
#include <string.h>
#include <time.h>

//----------------------------------------------------------
int main(int argc, char** argv) {
   int fd, i ;
   struct flock fl ;

   if (argc != 2) {
      printf("Usage: %s <process 'id' char>\n", argv[0]) ;
      exit(EXIT_FAILURE) ;
   }

   srand( (unsigned) time(NULL)) ;

   if ((fd = open("try.txt", O_WRONLY | O_CREAT, 0600)) < 0) {
       perror("create file try.txt") ;
       return( EXIT_FAILURE ) ;
   }

   fl.l_type = F_WRLCK ;        // lock for w
   fl.l_whence = SEEK_SET ;     // from a place we set
   fl.l_start = 0 ;             // relative to file start
   fl.l_len = 0 ;               // lock section: [0..EOF]


  
   sleep(rand() % 20) ;
   
   // try to lock and fail if impossible
   while(fcntl(fd, F_SETLK, &fl) == -1) {
      printf("process %c falied to lock\n", argv[1][0]) ;
      sleep(1) ;
   }

   printf("process %c locked the file\n", argv[1][0]) ;
   
   for (i= 0; i < 5; i++) {
      write(fd, argv[1], 1) ;
      printf("process %c wrote on  the file\n", argv[1][0]) ;
      sleep(rand() % 4) ;
   }

   fl.l_type = F_UNLCK ;        // unlock file
   if (fcntl(fd, F_SETLK, &fl) == -1) 
         printf("process %c failed to unlock file\n", argv[1][0]) ;
   else
         printf("process %c unlock file\n", argv[1][0]) ;

   close(fd) ;
   
   return( EXIT_SUCCESS ) ;
}  