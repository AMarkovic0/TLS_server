#include "tcp_server.h"

static uint8_t cnt = 0;

static int sockfd;
static struct sockaddr_in server_addr;

static int new_sockets[NUM_OF_DEVICES];
static struct sockaddr_in new_addresses[NUM_OF_DEVICES];
static struct pollfd fds[NUM_OF_DEVICES+1];

uint8_t tcp_server_init(unsigned int port, _logs log)
{
	char ip[MAX_IP_SIZE];
	uint8_t check_var = 0;

	signal(SIGPIPE , SIG_IGN); // block SIGPIPE signal in case client disconnect

	getIP(ip, log);

	sockfd = socket(PF_INET , SOCK_STREAM , 0);
	if(log && (-1 == sockfd)) {
		printf("Socket creation failed. \n");
		return -1;
	} else if(!log && (-1 == sockfd)) {
		return -1;
	} else if( log )
		printf( "Socket creation sucessfull. \n");

	memset(&server_addr, '\0', sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

	check_var = bind(sockfd, (struct sockaddr*)&server_addr , sizeof(server_addr));
	if(check_var && log) {
		printf("Bind failed %d. \n", errno);
		return -1;
	} else if(check_var && !log) {
		return -1;
	} else if(!check_var && log)
		printf("Bind sucessfull \n");

	return -1;
}

uint8_t tcp_server_listen(_logs log)
{
	uint8_t check = listen(sockfd, NUM_OF_DEVICES);
	if(check && log) {
		printf("Listen failed. \n");
		return -1;
	} else if(check && !log) {
		return -1;
	} else if(!check && log)
		printf("Server listening... \n");

	return 1;
}

uint8_t tcp_server_accept(_logs log)
{
	int *new_socket = &new_sockets[cnt];
	struct sockaddr_in *new_addr = &new_addresses[cnt];
	socklen_t addr_size = sizeof(*new_addr);
	cnt++;

	*new_socket = accept(sockfd, (struct sockaddr*)new_addr, &addr_size);
	if((*new_socket < 0) && log) {
		printf("Acception failed. \n");
		return *new_socket;
	} else if((*new_socket < 0) && !log) {
		return *new_socket;
	} else if(log)
		printf("Client %d sucessfully accepted. \n", cnt);

	fds[cnt].fd = *new_socket;
	fds[cnt].events = POLLIN;

	return 1;
}

ssize_t tcp_server_send(int sockfd, char* w_buf)
{
	return send(sockfd, w_buf, strlen(w_buf), 0);
}

ssize_t tcp_server_recv(int sockfd, char* r_buf)
{
	return recv(sockfd, r_buf, BUF_SIZE, MSG_DONTWAIT);
}

static uint8_t _check_recv(int res, _logs log)
{
	if(log && (res < 0) && (errno != EWOULDBLOCK)) {
		printf("Read from connection %d failed. \n", cnt);
		return 1;
	} else if ((res < 0) && (errno != EWOULDBLOCK)) {
		return 1;
	}

	if(log && (0 == res)) {
		printf("Clinet %d closed the connection. \n", cnt);
		return 2;
	} else if(0 == res) {
		return 2;
	}

	return 0;
}

void tcp_server_poll(char* r_buf, _logs log)
{
	int res;
	int close_connection = 0;

	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	for(;;) {
		res = poll(fds, NUM_OF_DEVICES, POLL_TIMEOUT);

		if(log && (res < 0)) {
			printf("Poll failed. \n");
			continue;
		} else if (res < 0){
			continue;
		} else if (log && (0 == res)) {
			printf("Poll timeout. \n");
		}

		for(int i = 0; i < NUM_OF_DEVICES+1; i++) {
			if (POLLIN != fds[i].revents || 0 == fds[i].revents)
				continue;

			if((fds[i].fd != sockfd)) {
				res = tcp_server_recv(fds[i].fd, r_buf);

				close_connection = _check_recv(res, log);
				if(0 != close_connection) {
					close_connection = i;
					break;
				}

				read_callback(r_buf, fds[i].fd, log);
			} else {
				tcp_server_accept(log);
			}
		}

		if(close_connection != 0) {
			close(fds[close_connection].fd);
			fds[close_connection].revents = 0;
			close_connection = 0;
			cnt--;
		}
	}
}

uint8_t tcp_server_close(_logs log)
{
	int res = close(sockfd);
	if(log && (-1 == res)) {
		printf("Socket closing failed. \n");
	} else if(log) {
		printf("Socket closing successful \n");
	}
	return res;
}

void getIP(char* IPaddr, _logs log)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_name, WIFI_INTERFACE, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	strcpy(IPaddr, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));

	if(log)
		printf("Host IP Address is: %s\n", IPaddr);

	return;
}

