#include "include/common.h"
#include <libgen.h>

int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printf("usage: %s ip_address port_number\n", basename(argv[0]));
		return 1;
	}

	int i;
	int ret;
	int num;
	int count = 0;
	char buf[BUF_SIZE];
	const char *ip = argv[1];
	int port = atoi(argv[2]);
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
	assert(-1 != ret);

	ret = listen(listenfd, 5);

	struct epoll_event events[MAX_EVENT_NUMBER];
	int epoll_fd = epoll_create(5);
	assert(-1 != epoll_fd);
	Add_epoll_fd(epoll_fd, listenfd, EPOLLIN);
	Set_nonblocking(listenfd);

	// handle 
	while (1)
	{
		num = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, -1);
		if (num < 0)
		{
			printf("epoll failure\n");
			break;
		}
		for (i = 0; i < num; ++i)
		{
			int sockfd = events[i].data.fd;
			if (listenfd == sockfd)
			{
				struct sockaddr_in client_address;
				socklen_t client_addr_len = sizeof(client_address);
				int connfd = accept(listenfd, (struct sockaddr *)&client_address, &client_addr_len);
				assert(-1 != connfd);
				Add_epoll_fd(epoll_fd, connfd, EPOLLIN);
				Set_nonblocking(connfd);
			}
			else if (events[i].events & EPOLLIN)
			{
				printf("sockfd = %d, event in\n", sockfd);
				while (1)
				{
					ret = recv(sockfd, buf, BUF_SIZE - 1, 0);
					if (ret < 0)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK)
						{
							printf("read later\n");
							break;
						}
						Epoll_close_conn(epoll_fd, sockfd);
						break;
					}
					else if (0 == ret)
					{
						Epoll_close_conn(epoll_fd, sockfd);
					}
					else
					{
						buf[ret] = '\0';
						printf("get %d bytes of content: %s\n", ret, buf);
						Mod_epoll_fd(epoll_fd, sockfd, EPOLLOUT);
					}
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				printf("sockfd = %d, event out\n", sockfd);
				snprintf(buf, BUF_SIZE, "%010d", ++count);
				char *p = buf;
				int len = strlen(buf);
				while (1)
				{
					ret = send(sockfd, p, len, 0);
					if (ret <= 0)
					{
						printf("send(%d) err: %s\n", count, strerror(errno));
						Epoll_close_conn(epoll_fd, sockfd);
						break;
					}
					len -= ret;
					p += ret;
					if (len <= 0)
					{
						Mod_epoll_fd(epoll_fd, sockfd, EPOLLIN);
						break;
					}
				}
			}
			else if (events[i].events & EPOLLERR)
			{
				Epoll_close_conn(epoll_fd, sockfd);
			}
		}
	}

	Epoll_close_conn(epoll_fd, listenfd);
	return 0;
}