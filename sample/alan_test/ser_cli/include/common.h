#ifndef _ALAN_COMMON_H_
#define _ALAN_COMMON_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <assert.h>
#include <libgen.h>

#define MAX_EVENT_NUMBER 10000
#define BUF_SIZE 1024

void Add_epoll_fd(int epoll_fd, int fd, int ev);
void Mod_epoll_fd(int epoll_fd, int fd, int ev);
void Epoll_close_conn(int epoll_fd, int fd);
int Set_nonblocking(int fd);
int Set_reuse_addr(int sd);

#endif /* _ALAN_COMMON_H_ */

