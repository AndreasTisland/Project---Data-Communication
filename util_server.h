#ifndef __UTIL_SERVER_H
#define __UTIL_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>

#include "debug_server.h"

#define MAXJOBS 16777215

typedef struct __attribute__((packed)) message {
    unsigned char jobinfo;
    unsigned int text_len;
    char jobtext[];
} message;

int sock, request_socket;
FILE *file;

int usage(int argc, char const *argv[]);
int check_port(char const *port_string, unsigned short *port);
void get_job(char msg_client[]);
void write_to_sock(char msg[], unsigned int msg_len);
void write_fread_sock(char type);
void send_msg_client(unsigned char ret);
void ctrl_c_handler(int signal);
unsigned char check_error_fread(FILE *file, size_t bytes_read);
void check_error_write(int ret);
void check_error_read(int ret);
void check_error_close(int ret);
void check_error_socket(int ret);
void check_error_bind(int ret);
void check_error_listen(int ret);
void check_error_accept(int sock);
void debug_print_numjobs(unsigned int num_jobs);
void terminate_normal();
void terminate_error();
unsigned char get_checksum(char *text, int text_len);

#endif
