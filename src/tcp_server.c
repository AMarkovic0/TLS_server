#include "tcp_server.h"

static int sockfd;
static struct sockaddr_in server_addr;

static struct sockaddr_in *new_address;
static struct pollfd pfd;

static SSL_CTX *ctx;
static SSL *ssld;
const SSL_METHOD *method;

static void _create_context()
{
        method = TLS_server_method();
        ctx = SSL_CTX_new(method);

        if (!ctx) {
                perror("Unable to create SSL context");
                ERR_print_errors_fp(stderr);
                exit(EXIT_FAILURE);
        }
}

static void _configure_context()
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

static void _tcp_server_init_ssl() {
        _create_context();
        _configure_context();
}

static int _tcp_server_init_socket(int *sock) {
        *sock = socket(PF_INET , SOCK_STREAM , 0);

	if(-1 == *sock) {
		dbg("Socket creation failed. \n");
		return -1;
	} else
		dbg( "Socket creation successful. \n");


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
		dbg("Bind successful \n");

        return 0;
}


static uint8_t _accept_ssl() {
	ssld = SSL_new(ctx);
        SSL_set_fd(ssld, pfd.fd);

        if (SSL_accept(ssld) <= 0) {
                ERR_print_errors_fp(stderr);
		return 0;
        } else {
                dbg("SSL accept successful.\n");
        }

	return 1;
}

static uint8_t _tcp_server_accept()
{
	socklen_t addr_size = sizeof(new_address);
	int new_socket = accept(sockfd, (struct sockaddr*)new_address, &addr_size);

	if(new_socket < 0) {
		dbg("Acception failed. \n");
		return 0;
	} else {
		dbg("Client successfully accepted. \n");
        }

        pfd.fd = new_socket;
	pfd.events = POLLIN;

	return _accept_ssl() & 1;
}

void _tcp_server_close_connection()
{
	dbg("Closing connection on socket.\n");

        SSL_shutdown(ssld);
        SSL_free(ssld);
        close(pfd.fd);
        pfd.revents = 0;

	dbg("Returning to the parent process.\n");
	exit(EXIT_SUCCESS);
}

static void _handle_connection(char* r_buf)
{
	int res;

	if (_tcp_server_accept());
		// TODO: Notify connection has been established

	for(;;) {
		res = poll(&pfd, 1, POLL_TIMEOUT);

		if (res < 0) {
			dbg("Poll failed. \n");
			continue;
		} else if (POLLIN != pfd.revents || 0 == pfd.revents) {
			continue;
		}

		if ((pfd.fd != sockfd)) {
			do {
				res = tcp_server_ssl_recv(ssld, r_buf);

				if (0 == res) {
					// TODO: Notify connection has been closed
					_tcp_server_close_connection();
				} else if (0 > res) {
					dbg("Read from connection failed. \n");
					break;
				}

				read_callback(r_buf, pfd.fd);
			} while(res);
		}
	}
}

void getIP(char* IPaddr)
{
	struct ifreq ifr;

	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_name, WIFI_INTERFACE, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFADDR, &ifr);
	strcpy(IPaddr, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));

        dbg("Host IP Address is: %s\n", IPaddr);

	return;
}

SSL *tcp_server_get_ssl()
{
	return ssld;
}

uint8_t tcp_server_init(unsigned int port)
{
	char ip[MAX_IP_SIZE];

	signal(SIGPIPE , SIG_IGN); // block SIGPIPE signal in case client disconnect

	//getIP(ip);

        _tcp_server_init_ssl();
        if(0 == _tcp_server_init_socket(&sockfd))
                _tcp_server_bind(port, LOCALHOST, &server_addr);

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

void tcp_server_poll(char* r_buf)
{
	int pid;

	for (int i = 0; i < NUM_OF_DEVICES; i++) {
		pid = fork();

		if (0 > pid) {
			dbg("Fork failed. Exiting the program.\n");
			exit(EXIT_FAILURE);
		} else if (0 == pid) {
			_handle_connection(r_buf);
		}
	}

	for (int i = 0; i < NUM_OF_DEVICES; i++)
		wait(NULL);
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
