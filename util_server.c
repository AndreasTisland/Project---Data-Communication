#include "util_server.h"

#define MAXTEXTLEN 100000
char *messages = NULL;
message *msg = NULL;

/*  Metoden sjekker om det er nok argumenter til
*   programmet.
*
*   Input:
*       argc: antall argumenter
*       argv: argumentene
*
*   Return:
*       1 hvis usage er feil og 0 hvis riktig
*/
int usage(int argc, char const *argv[]) {
    if(argc < 3) {
        printf("Usage: %s [filename] [port]\n", argv[0]);
        return 1;
    }
    return 0;
}

/*  Metode som skriver en melding til socketen.
*
*   Input:
*       msg: meldingen som skal sendes
*       msg_len: antall bytes som skal sendes
*/
void write_to_sock(char msg[], unsigned int msg_len) {
    DEBUG_PRINT11(getpid());
    DEBUG_PRINT10(getpid(), msg_len);
    size_t bytes_sent = 0;
    int bytes_written = 0;
    while(bytes_sent < msg_len) {
        bytes_written = write(sock, msg+bytes_sent, msg_len-bytes_sent);
        check_error_write(bytes_written);
        bytes_sent += (size_t) bytes_written;
    }
    DEBUG_PRINT12(getpid(), bytes_sent);
}

/*  Metoden sender beskjed til klient gitt
*   av type. Enten "filen er ferdig lest" eller
*   "feil ved fillesing"
*
*   Input:
*       type: beskjeden som skal sendes
*/
void write_type_sock(char type) {
    DEBUG_PRINT16(getpid(), type);
    char msg[5] = { 0 };
    msg[0] = type << 5;
    int bytes_written = write(sock, msg, sizeof(msg));
    check_error_write(bytes_written);
    if(type == 5) {
        terminate_error();
    }
}

/*  Metode som free-er "msg" og "messages".
*   Skriver gitt melding til klient.
*
*   Input:
*       ret: meldinen som skrives til klient
*/
void send_msg_client(unsigned char ret) {
    free(msg);
    msg = NULL;
    free(messages);
    messages = NULL;
    write_type_sock(ret);
}

/*  Metode som leser inn jobber fra fil og sender
*   dem til klient i en melding. Jobbene lagres i "messages"
*   som sendes til klienten. Klienten har sent antall jobber,
*   og metoden leser og legger til i "messages" til antallet
*   er lest eller filen er ferdig.
*
*   Input:
*       file: filen jobbene leses fra
*       msg_client: melding fra klient som inneholder antall jobber
*/
void get_job(char msg_client[]) {
    size_t bytes_read;
    unsigned char ret = 0;
    unsigned int num_jobs = 0;
    memcpy(&num_jobs, &msg_client[1], 3);
    debug_print_numjobs(num_jobs);

    unsigned int current_jobs = 0;
    unsigned int current_size = 0;
    messages = NULL;

    printf("\nLeser jobb fra fil...\n");
    while(current_jobs < num_jobs) {
        msg = NULL;
        unsigned int msg_len = 0;

        DEBUG_PRINT5(getpid());
        char jobtype = 0;
        bytes_read = fread(&jobtype, sizeof(char), 1, file);
        ret = check_error_fread(file, bytes_read);
        if(ret) {
            write_to_sock(messages, current_size);
            send_msg_client(ret);
            return;
        }

        unsigned int text_len = 0;
        bytes_read = fread(&text_len, sizeof(unsigned int), 1, file);
        ret = check_error_fread(file, bytes_read);
        if(ret) {
            send_msg_client(ret);
            return;
        }

        DEBUG_PRINT8(getpid(), text_len);
        if(text_len > MAXTEXTLEN) {
            printf("For stor tekstlengde! Korrupt fil\n");
            send_msg_client(3);
            terminate_error();
        }

        msg_len = sizeof(unsigned char) + sizeof(unsigned int) + text_len;
        DEBUG_PRINT10(getpid(), msg_len);
        msg = (message *) malloc(msg_len);
        if(msg == NULL) {
            perror("malloc");
            send_msg_client(5);
            return;
        }

        DEBUG_PRINT7(getpid(), jobtype);
        switch(jobtype) {
            case 'O':
                jobtype = 0;
            break;

            case 'E':
                jobtype = 1;
            break;

            case 'Q':
                jobtype = 7;
            break;

            default :
                printf("Korrupt fil, ugyldig jobbtype\n");
                send_msg_client(3);
                terminate_error();
        }

        bytes_read = fread(msg->jobtext, sizeof(char), text_len, file);
        DEBUG_PRINT13(getpid(), bytes_read);

        if(bytes_read != text_len) {
            printf("Korrupt fil, feil tekstlengde\n");
            send_msg_client(3);
            terminate_error();
        }

        ret = check_error_fread(file, bytes_read);
        if(ret) {
            send_msg_client(ret);
            return;
        }

        unsigned char checksum = get_checksum(msg->jobtext, text_len);
        DEBUG_PRINT9(getpid(), checksum);

        unsigned char jobinfo;
        jobtype = (unsigned char) jobtype << 5;
        jobinfo = checksum | (unsigned char) jobtype;

        msg->jobinfo = jobinfo;
        text_len = ntohs(text_len);
        msg->text_len = text_len;

        messages = (char *) realloc(messages, current_size + msg_len);
        if(messages == NULL) {
            perror("realloc");
            send_msg_client(5);
            return;
        }

        memcpy(&messages[current_size], msg, msg_len);
        current_size += msg_len;
        free(msg);
        msg = NULL;
        current_jobs++;
    }

    write_to_sock(messages, current_size);

    free(messages);
    messages = NULL;
}

/*  Metode som printer debug om antall jobber. Hvis vi henter maks
*   antall jobber skrives antallet maks og ellers brukes num_jobs.
*
*   Input:
*       num_jobs: antall jobber som skal hentes
*/
void debug_print_numjobs(unsigned int num_jobs) {
    if(num_jobs == MAXJOBS) {
        DEBUG_PRINT14(getpid());
    }
    else {
        DEBUG_PRINT15(getpid(), num_jobs);
    }
}

/*  Metode som kjører når bruker trykker "Ctrl+C"
*   Metoden sender en jobb til klienten med type 5
*   og tekstlengde 0. Deretter kalles terminate_error()
*   som terminere progerammet
*
*   Input:
*       signal: signalet som ble mottat
*/
void ctrl_c_handler(int signal) {
    printf("Mottat signal: %d\n", signal);
    char type = 5;
    char msg_client[5] = { 0 };
    msg_client[0] = type << 5;
    int bytes_written = write(sock, msg_client, sizeof(msg_client));
    check_error_write(bytes_written);
    if(messages != NULL) {
        free(messages);
    }
    if(msg != NULL) {
        free(msg);
    }
    terminate_error();
}

/*  Metoden sjekker om det har oppstått feil ved
*   lesing fra "file" eller om filen er lest ferdig.
*   Hvis det har oppstått feil terminrerer serveren og
*   sender beskjed til klient.
*   Hvis filen er lest ferdig sendes beskjed til klient
*   og server venter på beksjed fra klient.
*
*   Input:
*       file: filen som sjekkes
*
*   Return:
*       Meldingstype(5 eller 7), eller 0 for verken feil eller filslutt
*/
unsigned char check_error_fread(FILE *file, size_t bytes_read) {
    if(ferror(file)) {
        fprintf(stderr, "Feil ved lesing fra fil...\n");
        fprintf(stderr, "Retur fra fread: %ld\n", bytes_read);
        return 5;
    }
    else if(feof(file)) {
        DEBUG_PRINT6(getpid());
        return 7;
    }
    return 0;
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "write"
*
*   Input:
*       ret: returverdien fra write som sjekkes
*/
void check_error_write(int bytes_written) {
    if(bytes_written == -1) {
        perror("write");
        terminate_error();
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "read"
*
*   Input:
*       ret: returverdien fra read som sjekkes
*/
void check_error_read(int bytes_read) {
    if(bytes_read == -1) {
        perror("read");
        terminate_error();
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "close"
*
*   Input:
*       ret: returverdien fra close som sjekkes
*/
void check_error_close(int ret) {
    if(ret == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "socket"
*
*   Input:
*       ret: returverdien fra socket som sjekkes
*/
void check_error_socket(int ret) {
    if(ret == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "bind"
*
*   Input:
*       ret: returverdien fra bind som sjekkes
*/
void check_error_bind(int ret) {
    if(ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "listen"
*
*   Input:
*       ret: returverdien fra listen som sjekkes
*/
void check_error_listen(int ret) {
    if(ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "accept"
*
*   Input:
*       ret: returverdien fra accept som sjekkes
*/
void check_error_accept(int sock_ret) {
    if(sock_ret == -1) {
        perror("accept");
        int retur = close(request_socket);
        check_error_close(retur);
        exit(EXIT_FAILURE);
    }
}

/*  Metode for normal terminering.
*   Lukker socketen og filen og terminerer.
*/
void terminate_normal() {
    printf("Terminerer normalt...\n");
    int retur = close(sock);
    check_error_close(retur);
    fclose(file);
    exit(EXIT_SUCCESS);
}

/*  Metode for terminering grunnet feil.
*   Lukker socketen og filen og terminerer.
*/
void terminate_error() {
    printf("Terminerer grunnet feil...\n");
    int retur = close(sock);
    check_error_close(retur);
    fclose(file);
    exit(EXIT_FAILURE);
}

/*  Metoden regner ut checksummen til en gitt tekst ved å
*   summere alle karakterene i teksten og deretter ta modulo 32.
*
*   Input:
*       text: teksten som skal regnes på
*       text_len: lengden på teksten
*
*   Return:
*       returnerer svaret av utregningen
*/
unsigned char get_checksum(char text[], int text_len) {
    int i;
    unsigned char sum = 0;
    for(i = 0; i < text_len; i++) {
        sum += text[i];
    }
    sum = sum % 32;
    return sum;
}

/*  Metoden sjekker om den oppgitte porten er gyldig og setter "port"
*   til denne porten
*
*   Input:
*       port_string: porten som et argument til programmet
*       port: port-variablen som skal brukes i programmet
*
*   Return:
*       1 hvis feilet og 0 ellers
*/
int check_port(char const *port_string, unsigned short *port) {
    char *endptr;

    int retur = strtol(port_string, &endptr, 10);
    if(endptr == port_string && retur == 0) {
        return 1;
    }

    if(retur > USHRT_MAX) {
        printf("Port number too big, causing overflow\n");
    }

    *port = (unsigned short) retur;
    return 0;
}
