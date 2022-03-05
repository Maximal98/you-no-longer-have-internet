#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/wait.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <spawn.h>
  
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 1024 /*Assuming length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

extern char **environ;

int run_program(char *prog_name, char *cope, int verbose) {
	//Starting DE Setup
	if( verbose == 1 ) {
		printf("Trying to start DE\n");
	}
	pid_t ChildPid;
	int status;
	char *args[] = {prog_name, cope, NULL};
	//Starting and Error handling
	status = posix_spawn(&ChildPid, prog_name, NULL, NULL, args, environ);

	int error = errno;
	if ( error == 0 ) {
		if( verbose == 1) {
			printf("started DE successfully!\n");
		}
		wait(&status);
	}
	else {
		printf("couldn't start program, returned error code %d \n", error);
	}


}


int wd, fd;

void start_inotify(char *directory) {

  
	fd = inotify_init();
	if ( fd < 0 ) {
		perror( "Couldn't initialize inotify");
	}
  
	wd = inotify_add_watch(fd, directory, IN_CREATE | IN_MODIFY | IN_DELETE); 
	if (wd == -1) {
		printf("Couldn't add watch to %s\n", directory);
	} else {
		printf("Watching:: %s\n", directory);
	}
}

int get_event (int fd_internal) {
	char buffer[BUF_LEN];
	int length, i = 0;
 
	length = read( fd_internal, buffer, BUF_LEN );  
	if ( length == -1 ) {
		return errno;
	}  
  
	while ( i < length ) {
		struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
		if ( event->len ) {
			if ( event->mask & IN_MODIFY && !( event->mask & IN_ISDIR ) && strcmp( event->name, "hosts" ) == 0 ) {
				printf( "The Hosts File was Modified REEEE!!!!\n" );
				run_program("/usr/bin/shutdown", "now", 0);
			}
		}
		i += EVENT_SIZE + event->len;
	}
			

}

 
int main( int argc, char **argv ) {

	start_inotify("/etc/");
	printf("mario\n");
	/* do it forever*/
	while(1) {
		get_event(fd); 
	} 
 
	/* Clean up*/
	inotify_rm_watch( fd, wd );
	close( fd );
	
	return 0;
}