#include "tcp_server.h"

static uint8_t cnt = 0;

static int sockfd;
static struct sockaddr_in server_addr;

static struct sockaddr_in *new_addresses;
static struct pollfd *fds;
static unsigned int fds_size = 0;

static int _tcp_server_init_socket(int *sock) {
        *sock = socket(PF_INET , SOCK_STREAM , 0);

	if(-1 == *sock) {
		dbg("Socket creation failed. \n");
		return -1;
	} else
		dbg( "Socket creation sucessfull. \n");


        return 0;
}

static uint8_t _tcp_server_bind(unsigned int port, char ip[], struct sockaddr_in *server_addr) {
	memset(server_addr, '\0', sizeof(*server_addr));

        server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(port);
	server_addr->sin_addr.s_addr = inet_addr(ip);

	if(bind(sockfd, (struct sockaddr*)server_addr , sizeof(*server_addr))) {
		dbg("Bind failed %d. \n", errno);
		return -1;
	} else
		dbg("Bind sucessfull \n");

        return 0;
}

uint8_t tcp_server_init(unsigned int port)
{
	char ip[MAX_IP_SIZE];

	signal(SIGPIPE , SIG_IGN); // block SIGPIPE signal in case client disconnect

	getIP(ip);

        _tcp_server_init_socket(&sockfd);
        _tcp_server_bind(port, ip, &server_addr);

        fds = (struct pollfd*)malloc(sizeof(struct pollfd)*NUM_OF_DEVICES);
        new_addresses = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in)*NUM_OF_DEVICES);
        fds_size += NUM_OF_DEVICES;

	return 0;
}

uint8_t tcp_server_listen()
{
	uint8_t check = listen(sockfd, NUM_OF_DEVICES);
	if(check) {
		dbg("Listen failed. \n");
		return -1;
	} else if(!check)
		dbg("Server listening... \n");

	return 1;
}

uint8_t tcp_server_accept()
{
	struct sockaddr_in *new_addr = &new_addresses[cnt];
	socklen_t addr_size = sizeof(*new_addr);

	int new_socket = accept(sockfd, (struct sockaddr*)new_addr, &addr_size);

	if(new_socket < 0) {
		dbg("Acception failed. \n");
		return new_socket;
	} else {
                cnt++;
		dbg("Client %d sucessfully accepted. \n", cnt);
        }

        if(fds_size == cnt) {
                fds = (struct pollfd*)realloc(fds, (fds_size+NUM_OF_DEVICES)*sizeof(struct pollfd));
                new_addresses = (struct sockaddr_in*)realloc(new_addresses, (fds_size+NUM_OF_DEVICES)*sizeof(struct sockaddr_in));
                fds_size += NUM_OF_DEVICES;
        }
	fds[cnt].fd = new_socket;
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

static uint8_t _check_recv(int res)
{
	if((res < 0) && (errno != EWOULDBLOCK)) {
		dbg("Read from connection failed. \n");
		return 1;
	}

	if(0 == res) {
		dbg("Clinet closed the connection. \n");
                return 2;
	}

	return 0;
}

void tcp_server_poll(char* r_buf)
{
	int res;
	int close_connection = 0;

	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	for(;;) {
		res = poll(fds, fds_size, POLL_TIMEOUT);

		if(res < 0) {
			dbg("Poll failed. \n");
			continue;
		} else if (0 == res) {
			dbg("Poll timeout. \n");
		}

		for(int i = 0; i < cnt+1; i++) {
			if (POLLIN != fds[i].revents || 0 == fds[i].revents)
				continue;

			if((fds[i].fd != sockfd)) {
				res = tcp_server_recv(fds[i].fd, r_buf);

				close_connection = _check_recv(res);
				if(0 != close_connection) {
					close_connection = i;
					break;
				}

				read_callback(r_buf, fds[i].fd);
			} else {
				tcp_server_accept();
			}
		}

		if(close_connection != 0) {
			close(fds[close_connection].fd);
			fds[close_connection].revents = 0;
                        if(cnt != close_connection && cnt != 1) {
                                cnt++;
                                memmove(&fds[close_connection], &fds[close_connection+1], cnt-(close_connection+1));
                                memmove(
                                        &new_addresses[close_connection-1],
                                        &new_addresses[close_connection],
                                        cnt-(close_connection)
                                );
                                cnt--;
                        }
			close_connection = 0;
			cnt--;
		}
	}
}

uint8_t tcp_server_close()
{
	int res = close(sockfd);
	if(-1 == res) {
		dbg("Socket closing failed. \n");
	} else {
		dbg("Socket closing successful \n");
	}
	return res;
}

void getIP(char* IPaddr)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_name, WIFI_INTERFACE, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	strcpy(IPaddr, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));

        dbg("Host IP Address is: %s\n", IPaddr);

	return;
}
