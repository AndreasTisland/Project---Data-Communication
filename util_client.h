#ifndef __UTIL_CLIENT_H
#define __UTIL_CLIENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>

#include "debug_client.h"

#define MAXJOBS 16777215

typedef struct __attribute__((packed)) message {
    unsigned char jobinfo;
    unsigned int text_len;
    char jobtext[];
} message;

int sock;
int fd1[2];
int fd2[2];
int children_terminated;
pid_t parent_pid;

char *child_text;
message *msg;

void user_loop();
int usage(int argc, char const *argv[]);
int check_port(char const *port_string, unsigned short *port);
int int_from_user();
int get_jobs();
int get_x_jobs();
void get_all_jobs();
void terminate_normal();
void terminate_error();
void terminate_children();
void signal_handler();
void ctrl_c_handler(int signal);
void check_error_write(int retur);
void check_error_read(int retur);
void check_error_read_child(int retur);
void check_error_close(int ret);
void check_error_socket(int ret);
void check_error_sigaction(int ret);
void check_error_pipe(int ret);
void check_checksum(unsigned char old_checksum, unsigned char new_checksum);
unsigned char get_checksum(char *text, int text_len);

#endif
