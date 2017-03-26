#include "include/common.h"

void start_conn(int epoll_fd, int conn_num, const char *ip, int port)
{
	int i;
	int ret;
	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	for (i = 0; i < conn_num; ++i)
	{
		usleep(1000 * 10);
		int sockfd = socket(PF_INET, SOCK_STREAM, 0);
		if (-1 == sockfd)
		{
			printf("socket() err: %s\n", strerror(errno));
			continue;
		}
		printf("[client]: create 1 sock = %d\n", sockfd);
		ret = connect(sockfd, (struct sockaddr *)&address,sizeof(address));
		if (0 == ret)
		{
			printf("[client]: build connection, sd = %d\n", sockfd);
			Add_epoll_fd(epoll_fd, sockfd, EPOLLOUT);
			Set_nonblocking(sockfd);
		}
		else
		{
			printf("[client]: build connection failed: %s, sd = %d\n", strerror(errno), sockfd);
			close(sockfd);
			continue;
		}
	}
}

int main(int argc, char *argv[])
{
	int i;
	int ret;
	int num;
	int count = 0;
	char buf[BUF_SIZE];

	if (argc <= 3)
	{
		printf("usage: %s ip_address port_number cli_num\n", basename(argv[0]));
		return 1;
	}

	int epoll_fd = epoll_create(5);
	assert(-1 != epoll_fd);

	start_conn(epoll_fd, atoi(argv[3]), argv[1], atoi(argv[2]));
	struct epoll_event events[MAX_EVENT_NUMBER];
	while (1)
	{
		num = epoll_wait(epoll_fd, events, MAX_EVENT_NUMBER, 2000);
		if (num < 0)
		{
			printf("epoll failure\n");
			break;
		}
		for (i = 0; i < num; ++i)
		{
			int sockfd = events[i].data.fd;
			if (events[i].events & EPOLLIN)
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
						printf("sockfd = %d closed_1: %s\n", sockfd, strerror(errno));
						Epoll_close_conn(epoll_fd, sockfd);
						break;
					}
					else if (0 == ret)
					{
						printf("sockfd = %d closed_2: %s\n", sockfd, strerror(errno));
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
				snprintf(buf, BUF_SIZE, "cli_fd=%d: %010d", sockfd, ++count);
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
				printf("sockfd = %d closed_3: %s\n", sockfd, strerror(errno));
				Epoll_close_conn(epoll_fd, sockfd);
			}
		}
	}

	return 0;
}

