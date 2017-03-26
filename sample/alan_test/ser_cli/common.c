#include "include/common.h"

void Add_epoll_fd(int epoll_fd, int fd, int ev)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLERR;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
}

void Mod_epoll_fd(int epoll_fd, int fd, int ev)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = ev | EPOLLET | EPOLLERR;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

int Set_nonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, old_option | O_NONBLOCK);
	return old_option;
}

void Epoll_close_conn(int epoll_fd, int fd)
{
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}

int Set_reuse_addr(int sd)
{
	int option;
	return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
}