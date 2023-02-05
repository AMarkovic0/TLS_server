/******************************************************************************
*
* tcp_server.h this is a file which contains necessary function declarations,
* definitions of macros and preprocessing instructions for tcp server program execution.
* This file implements TCP server library undepndetly.
*
* This code uses POSIX libraries.
*
* Created by: Aleksa Markovic
******************************************************************************/

#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<errno.h>
#include<unistd.h>

//Networking headers:
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<net/if.h>
#include<sys/ioctl.h>
#include<sys/wait.h>
#include<netdb.h>
#include<poll.h>
#include<openssl/ssl.h>
#include<openssl/err.h>

#include"log.h"

#define MAX_IP_SIZE 15		// Size of the IPv4 address
#define BUF_SIZE 255		// Buffer size
#define WIFI_INTERFACE "wlp2s0" // Network interface
#define NUM_OF_DEVICES 5	// Step for mem allocation
#define POLL_TIMEOUT 1500	// [ms]
#define LOCALHOST (char*)"0.0.0.0"   // localhost ip address

/*
* This function initializes and binds a TCP server
* In:
*	unsigned int port 	-> Port number
* Out:
*	uint8_t check		-> success notification
*/
uint8_t tcp_server_init(unsigned int port);

/*
* Starts server listening
* Before this function call, please initialize the server
* Out:
*	uint8_t check		-> success notification
*/
uint8_t tcp_server_listen();

/*
* Send message to the client. Assumes connection has been established.
* In:
* 	int sockfd	-> socket file descriptor
*	char* w_buf	-> Message buffer pointer
* Out:
*	ssize_t res	-> number of bytes sent
*/
ssize_t tcp_server_send(int sockfd, char* w_buf);

/*
* Send message to the client via ssl. Assumes connection has been established.
* In:
* 	SSL *ssl	-> socket file descriptor
*	char* w_buf	-> Message buffer pointer
* Out:
*	ssize_t res	-> number of bytes sent
*/
ssize_t tcp_server_ssl_send(SSL *ssl, char* w_buf);

/*
* Reads client message and puts the message in the buffer.
* Assumes connection has been established.
* In:
* 	int sockfd	-> socket file descriptor
*	char* r_buf	-> Message buffer pointer
* Out:
*	ssize_t ret	-> number of bytes received
*/
ssize_t tcp_server_recv(int sockfd, char* r_buf);

/*
* Reads client message via ssl and puts the message in the buffer.
* Assumes connection has been established.
* In:
* 	SSL *ssl        -> ssl descriptor
*	char* r_buf	-> Message buffer pointer
* Out:
*	ssize_t ret	-> number of bytes received
*/
ssize_t tcp_server_ssl_recv(SSL *ssl, char *r_buf);

/*
 * Returns private variable ssl pointer for the connection.
 * Out:
 *      SSL *ret        -> address of the ssl object
 */
SSL *tcp_server_get_ssl();

/*
 * Polling function
 * In:
 * 	char* r_buf	-> Read buffer
*/
void tcp_server_poll(char* r_buf);

/* Function declaration - reading callback
 * Must be implemented
 * In:
 * 	char* r_buf	-> read buffer
 * 	int fd		-> socket file descriptor
*/
void read_callback(char* r_buf, int fd);

/*
 * Closes the server
 * Out:
 * 	uint8_t 	-> success notification
*/
uint8_t tcp_server_close();

/*
* Gets and writes server IP address in the IPaddr buffer
* In:
*	char* IPaddr	-> Address buffer
*/
void getIP(char* IPaddr);

#ifdef __cplusplus
}
#endif

#endif // _TCP_SERVER_H_
