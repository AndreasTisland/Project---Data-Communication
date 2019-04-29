#define _POSIX_C_SOURCE 200112L

#include <signal.h>
#include <sys/socket.h>

#include "util_server.h"

/*  Server program
*   Kobler til en klient som vil lese jobber.
*   Klienten sender melding om server skal sende
*   jobber, terminere normalt eller terminerer
*   grunnet feil
*
* Input:
*       argv[1]: navn på fil server leser fra
*       argv[2]: porten vi kommuniserer med
*
*/
int main(int argc, char const *argv[]) {
    int retur;
    DEBUG_PRINT_START(getpid());

    DEBUG_PRINT_ARG(getpid(), argv);
    if(usage(argc, argv)) {
        return EXIT_FAILURE;
    }

    unsigned short port;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t clientaddrlen = 0;

    struct sigaction sig_ctrl_c;
    memset(&sig_ctrl_c, 0, sizeof(struct sigaction));
    sig_ctrl_c.sa_handler = &ctrl_c_handler;
    sigaction(SIGINT, &sig_ctrl_c, NULL);

    DEBUG_PRINT1(getpid(), argv[1]);
    file = fopen(argv[1], "r");

    retur = check_port(argv[2], &port);
    if(retur) {
        printf("Port not an integer \n[port] = %s \n", argv[2]);
    }

    request_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    check_error_socket(request_socket);

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

    retur = bind(request_socket, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    check_error_bind(retur);

    retur = listen(request_socket, SOMAXCONN);
    check_error_listen(retur);

    DEBUG_PRINT2(getpid());
    sock = accept(request_socket, (struct sockaddr *) &clientaddr, &clientaddrlen);
    check_error_accept(sock);

    close(request_socket);

/*  Løkken tar imot melding fra klienten. Er det type '1' henter den 3
*   bytes til med antall jobber, og henter det antallet jobber.
*   Type '2' vil terminere normalt og type '3' vil terminere grunnet
*   feil.
*/
    while(1) {
        DEBUG_PRINT3(getpid());
        char msg_client[4];
        retur = read(sock, msg_client, sizeof(char));
        check_error_read(retur);
        DEBUG_PRINT4(getpid(), msg_client[0]);

        switch(msg_client[0]) {
            case '1':
            retur = read(sock, &msg_client[1], sizeof(char) * 3);
            check_error_read(retur);
            get_job(msg_client);
            break;

            case '2':
            terminate_normal();
            break;

            case '3':
            terminate_error();
            break;
        }
    }
}
