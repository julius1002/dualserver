#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../include/fileutils.h"

char *read_file(char *pathname, char *static_files_loc, size_t *outlen) {
	// TODO validate this pathname against path traversal attacks
	if(strstr(pathname, "..")){
		fprintf(stderr, "Error, parent directory is referred\n");
		return NULL; 
	}
	struct stat st;
	int len = sizeof(char) * strlen(static_files_loc) + sizeof(char) * strlen(pathname) + 2;
	char *path_template = "%s/%s";
	char *buffer = malloc(len);
	if(!buffer){
		fprintf(stderr, "Error malloc(), %s\n", strerror(errno));
		exit(1);
	} 
	    
	int n = snprintf(buffer, len, path_template, static_files_loc, pathname);
	if(n < 0){
		fprintf(stderr, "Error snprintf()\n");
		return NULL;
	}
	printf("path is: %s\n", buffer);

	int s = stat(buffer, &st);
	if(s != 0){
		fprintf(stderr, "Error stat() %s, input: %s\n", strerror(errno), buffer);
		return NULL;
	} else {
		off_t size = st.st_size;
		char *fcontents_buffer = malloc(sizeof(char) * size + 1);
		if(!fcontents_buffer){
		  fprintf(stderr, "Error malloc(), %s\n", strerror(errno));
		  exit(1);
		}

		int fd = open(buffer, O_RDONLY);
		if(fd == -1){
		  fprintf(stderr, "Error open() %s\n", strerror(errno));
		  return NULL;
		}
		ssize_t bytes_read = read(fd, fcontents_buffer, size);
		if(bytes_read == -1){
		  fprintf(stderr, "Error read()\n");
		  return NULL;
		}
                *outlen = size;
		return fcontents_buffer;
	}
	return NULL;
}

enum webfile is_web_file(const char *filename)
{
    const char *dot = strrchr(filename, '.');

    if (dot == NULL) {
        return 0;
    }

    if(strcmp(dot, ".html") == 0) {
	    return HTML;
    } else if(strcmp(dot, ".js") == 0) {
	    return JS;
    } else if(strcmp(dot, ".css") == 0) {
	    return CSS;
    } else {
	    return UNK;
    }
}
