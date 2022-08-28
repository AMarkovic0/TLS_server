#include "tcp_server.h"

static uint8_t cnt = 0;

static int sockfd;
static struct sockaddr_in server_addr;

static struct sockaddr_in *new_addresses;
static struct pollfd *fds;
static unsigned int fds_size = 0;

static SSL_CTX *ctx;
static SSL **ssls;
const SSL_METHOD *method;

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

static void _tcp_server_init_ssl() {
        create_context();
        configure_context();
}

uint8_t tcp_server_init(unsigned int port)
{
	char ip[MAX_IP_SIZE];

	signal(SIGPIPE , SIG_IGN); // block SIGPIPE signal in case client disconnect

	getIP(ip);

        _tcp_server_init_ssl();
        if(0 == _tcp_server_init_socket(&sockfd))
                _tcp_server_bind(port, LOCALHOST, &server_addr);

        fds = (struct pollfd*)malloc(sizeof(struct pollfd)*NUM_OF_DEVICES);
        ssls = (SSL**)malloc(NUM_OF_DEVICES*sizeof(SSL*));
        new_addresses = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in)*NUM_OF_DEVICES);
        fds_size += NUM_OF_DEVICES;

	return 0;
}

void create_context()
{
        method = TLS_server_method();
        ctx = SSL_CTX_new(method);

        if (!ctx) {
                perror("Unable to create SSL context");
                ERR_print_errors_fp(stderr);
                exit(EXIT_FAILURE);
        }
}

void configure_context()
{
        /* Set the key and cert */
        if (SSL_CTX_use_certificate_file(ctx, "certs/cert-localhost.pem", SSL_FILETYPE_PEM) <= 0) {
                ERR_print_errors_fp(stderr);
                exit(EXIT_FAILURE);
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, "certs/key-localhost.pem", SSL_FILETYPE_PEM) <= 0 ) {
                ERR_print_errors_fp(stderr);
                exit(EXIT_FAILURE);
        }
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
                fds_size += NUM_OF_DEVICES;
                fds = (struct pollfd*)realloc(fds, (fds_size)*sizeof(struct pollfd));
                new_addresses = (struct sockaddr_in*)realloc(
                                new_addresses,
                                (fds_size)*sizeof(struct sockaddr_in)
                );
                ssls = (SSL**)realloc(ssls, (fds_size)*sizeof(SSL*));
        }

        fds[cnt].fd = new_socket;
	fds[cnt].events = POLLIN;

        ssls[cnt-1] = SSL_new(ctx);
        SSL_set_fd(ssls[cnt-1], fds[cnt].fd);

        if (SSL_accept(ssls[cnt-1]) <= 0)
                ERR_print_errors_fp(stderr);
        else {
                dbg("SSL accept successful.\n");
                SSL_write(ssls[cnt-1], "AAAAAA", strlen("AAAAAA"));
        }

	return 1;
}

ssize_t tcp_server_send(int sockfd, char *w_buf)
{
	return send(sockfd, w_buf, strlen(w_buf), 0);
}

ssize_t tcp_server_ssl_send(SSL *ssl, char *w_buf) {
        return SSL_write(ssl, w_buf, strlen(w_buf));
}

ssize_t tcp_server_recv(int sockfd, char *r_buf)
{
	return recv(sockfd, r_buf, BUF_SIZE, MSG_DONTWAIT);
}

ssize_t tcp_server_ssl_recv(SSL *ssl, char *r_buf)
{
        return SSL_read(ssl, r_buf, sizeof(r_buf));
}

SSL *tcp_server_get_ssl(int sockfd)
{
        for(int i = 0; i < fds_size; i++) {
                if(sockfd == SSL_get_fd(ssls[i]))
                        return ssls[i];
        }

        return NULL;
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
		}

		for(int i = 0; i < cnt+1; i++) {
			if (POLLIN != fds[i].revents || 0 == fds[i].revents)
				continue;

			if((fds[i].fd != sockfd)) {
				res = tcp_server_ssl_recv(ssls[i-1], r_buf);

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
                        tcp_server_close_connection(&close_connection);
		}
	}
}

void tcp_server_close_connection(int *connection_id)
{
        SSL_shutdown(ssls[*connection_id-1]);
        SSL_free(ssls[*connection_id-1]);
        close(fds[*connection_id].fd);
        fds[*connection_id].revents = 0;
        if(cnt != *connection_id && cnt != 1) {
                cnt++;
                memmove(&fds[*connection_id], &fds[*connection_id+1], cnt-(*connection_id+1));
                memmove(
                        &new_addresses[*connection_id-1],
                        &new_addresses[*connection_id],
                        cnt-*connection_id
                );
                memmove(
                        &ssls[*connection_id-1],
                        &ssls[*connection_id],
                        cnt-*connection_id
                );
                cnt--;
        }
        *connection_id = 0;
        cnt--;
}

uint8_t tcp_server_close()
{
	int res = close(sockfd);
        SSL_CTX_free(ctx);
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
