#define _POSIX_C_SOURCE 200112L

#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>

#include "util_client.h"

void parent(char const *argv[]);
void child(int fd, FILE *stdstream);

/*  Klient program
*   Forker programmet til en foreldre-prosess og
*   to barneprosesser. Prosessene kaller egne metoder.
*   Det opprettes signal-handlere for prosessene.
*
* Input:
*       argv[1]: adressen eller maskinnavn vi vi koble til
*       argv[2]: porten vi kommuniserer med
*
*/
int main(int argc, char const *argv[]) {
    DEBUG_PRINT_START(getpid());

    DEBUG_PRINT_ARG(getpid(), argv);
    if(usage(argc, argv)) {
        return EXIT_FAILURE;
    }

    int retur;
    struct sigaction sig_child_parent;
    memset(&sig_child_parent, 0, sizeof(struct sigaction));
    sig_child_parent.sa_handler = &signal_handler;
    retur = sigaction(SIGUSR1, &sig_child_parent, NULL);
    check_error_sigaction(retur);

    struct sigaction sig_ctrl_c;
    memset(&sig_ctrl_c, 0, sizeof(struct sigaction));
    sig_ctrl_c.sa_handler = &ctrl_c_handler;
    retur = sigaction(SIGINT, &sig_ctrl_c, NULL);
    check_error_sigaction(retur);

    parent_pid = getpid();

    retur = pipe(fd1);
    check_error_pipe(retur);

    retur = pipe(fd2);
    check_error_pipe(retur);

    if(!fork()) {
        DEBUG_PRINT14(getpid());
        close(fd1[1]);
        child(fd1[0], stdout);
        close(fd1[0]);
        DEBUG_PRINT15(getpid());
        return EXIT_SUCCESS;
    }
    else if(!fork()) {
        DEBUG_PRINT14(getpid());
        close(fd2[1]);
        child(fd2[0], stderr);
        close(fd2[0]);
        DEBUG_PRINT15(getpid());
        return EXIT_SUCCESS;
    }
    else {
        parent(argv);
    }
}

/*  Metode for foreldre-prosessen.
*   Kobler til serveren og oppretter bruker-meny
*
*   Input:
*       argv: addresse og port det kobles til
*/
void parent(char const *argv[]) {
    int retur;
    close(fd1[0]);
    close(fd2[0]);

    struct addrinfo hints;
    struct addrinfo *result, *rp;

    unsigned short port;
    retur = check_port(argv[2], &port);
    if(retur) {
        printf("Port not an integer \n[port] = %s \n", argv[2]);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP;

    retur = getaddrinfo(argv[1], argv[2], &hints, &result);
    if(retur) {
        fprintf(stderr, "argv[1] = %s \nagrv[2] = %s\n", argv[1], argv[2]);
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retur));
        terminate_children();
        exit(EXIT_FAILURE);
    }

    DEBUG_PRINT1(getpid());
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    check_error_socket(sock);

    DEBUG_PRINT2(getpid());
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        retur = connect(sock, rp->ai_addr,rp->ai_addrlen);
        if(!retur) {
            break;
        }
    }

    if(rp == NULL) {
        fprintf(stderr, "No address succeeded!\n");
        freeaddrinfo(result);
        terminate_children();
        retur = close(sock);
        check_error_close(retur);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    user_loop();
    terminate_normal();
}

/*  Metode for barneprosessene.
*   Leser inn tekstlengde og tekst gjennom pipe fra
*   foreldren. Er tekstlengden 0 avslutter barnet, hvis Ikke
*   skrives den til stdstream. Barnet sender signal når den AVSLUTTER
*   eller skriver ut teksten.
*
*   Input:
*       fd: fildeskriptoren som barnet og foreldren kommuniserer med
*       stdstream: strømmen teksten skrives til
*/
void child(int fd, FILE *stdstream) {
    int ret;
    while(1) {
        unsigned int text_len = 0;
        ret = read(fd, &text_len, sizeof(unsigned int));
        check_error_read_child(ret);
        DEBUG_PRINT11(getpid(), text_len);
        if(text_len == 0) {
            kill(parent_pid, SIGUSR1);
            return;
        }

        child_text = (char *) malloc(sizeof(char) * (text_len+1));
        if(child_text == NULL) {
            perror("malloc");
        }

        ret = read(fd, child_text, text_len);
        check_error_read_child(ret);
        DEBUG_PRINT12(getpid(), ret);

        child_text[text_len] = '\0';

        DEBUG_PRINT13(getpid());
        fprintf(stdstream, "%s\n", child_text);

        kill(parent_pid, SIGUSR1);
        free(child_text);
        child_text = NULL;
    }
}
