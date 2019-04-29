#include "util_client.h"

/*  Metoden sjekker om det er nok argumenter til
*   programmet.
*
*   Input:
*       argc: antall argumenter
*       argv: argumentene
*/
int usage(int argc, char const *argv[]) {
    if(argc < 3) {
        printf("Usage: %s [host] [port]\n", argv[0]);
        return 1;
    }
    return 0;
}

/*  Metoden for bruker-meny.
*   Bruker taster et tall fra 1 til 4.
*   Hvis Barneprossessene har terminert vil bruker-meny
*   bare kunne avslutte programmet.
*/
void user_loop() {
    DEBUG_PRINT3(getpid());
    int retur = 1;
    int t = 1;
    while (t) {
        printf("----------------------------------------\n");
        printf("1: Hent en jobb fra serveren\n");
        printf("2: Hent X antall jobber fra serveren\n");
        printf("3: Hent alle jobber fra serveren\n");
        printf("4: Avslutt progerammet\n");
        printf("----------------------------------------\n");

        printf("Tast nummeret til oppgaven du vil ha utfort: ");
        int input = int_from_user();
        if(input < 1 || input > 4) {
            printf("Ikke gyldig tall!\n");
        }

        switch(input) {
            case 1:
                if(!children_terminated) {
                    retur = get_jobs(1);
                }
                break;

            case 2:
                if(!children_terminated) {
                    retur = get_x_jobs();
                }
                break;

            case 3:
                if(!children_terminated) {
                    get_all_jobs();
                    retur = 0;
                }
                break;

            case 4:
                if(!children_terminated) {
                    terminate_children();
                }
                t = 0;
                printf("Avslutter....\n");
                break;
        }
        if(!retur) {
            printf("\nIngen flere jobber kan hentes\n");
        }
    }
}

/*  Metoden printer debug-informasjon om jobbtype
*   fra server.
*
*   Input:
*       type: tallformat av typen som konverteres til en karakter
*/
void debug_print_jobtype(unsigned char type) {
    switch (type) {
        case 0:
        DEBUG_PRINT4(getpid(), 'O');
        break;

        case 1:
        DEBUG_PRINT4(getpid(), 'E');
        break;

        case 7:
        DEBUG_PRINT4(getpid(), 'Q');
        break;
    }
}

/*  Metoden leser inn jobteksten fra server med lengde text_len
*
*   Input:
*       jobtext: bufferet teksten lagres i
*       text_len: lengden på teksten
*/
void read_text_from_sock(char jobtext[], unsigned int text_len) {
    int ret = 0;
    size_t bytes_read = 0;
    while(bytes_read < text_len) {
        ret = read(sock, jobtext+bytes_read, text_len-bytes_read);
        check_error_read(ret);
        bytes_read += (size_t) ret;
    }
    DEBUG_PRINT_READ2(getpid(), bytes_read);
}

/*  Metoden leser num_jobs jobber fra server og sender til barneprosessene.
*   Sender først melding om å hente jobber og antallet. Deretter leses jobbene
*   inn fra server og avhengig av typen jobb gjøres en aksjon. Jobben kan sendes
*   til et av barna eller vi kan få beksjed om at filen er ferdig eller serveren
*   har terminert.
*
*   Input:
*       num_jobs: antall jobber som skal leses
*
*   Return:
*       0 hvis vi ikke kan lese flere jobber og 1 ellers
*/
int get_jobs(unsigned int num_jobs) {
    int retur;
    char msg_server[4];
    msg_server[0] = '1';
    memcpy(&msg_server[1], &num_jobs, sizeof(unsigned int));
    DEBUG_PRINT16(getpid(), num_jobs);
    DEBUG_PRINT_WRITE1(getpid(), msg_server[0]);

    retur = write(sock, msg_server, sizeof(msg_server));
    check_error_write(retur);

    unsigned int done_jobs = 0;
    retur = 0;
    while(done_jobs < num_jobs) {
        msg = (message *) malloc(sizeof(unsigned char) + sizeof(unsigned int));
        if(msg == NULL) {
            perror("malloc");
            terminate_children();
            terminate_error();
        }

        retur = read(sock, msg, sizeof(char) + sizeof(unsigned int));
        check_error_read(retur);
        DEBUG_PRINT_READ3(getpid(), retur);

        unsigned char jobinfo = msg->jobinfo;
        unsigned char type = jobinfo >> 5;
        debug_print_jobtype(type);
        unsigned char checksum = (UCHAR_MAX >> 3) & jobinfo;
        DEBUG_PRINT5(getpid(), checksum);

        msg->text_len = htons(msg->text_len);
        unsigned int text_len = msg->text_len;

        DEBUG_PRINT6(getpid(), text_len);
        msg = realloc(msg, sizeof(unsigned char) + sizeof(unsigned int) + text_len);
        if(msg == NULL) {
            perror("realloc");
            terminate_children();
            terminate_error();
        }

        if(text_len > 0) {
            read_text_from_sock(msg->jobtext, text_len);
            unsigned char new_checksum = get_checksum(msg->jobtext, text_len);
            DEBUG_PRINT7(getpid(), new_checksum);
            check_checksum(checksum, new_checksum);
        }

        switch (type) {
            case 0:
                DEBUG_PRINT8(getpid(), 1);

                retur = write(fd1[1], &msg->text_len, sizeof(unsigned int) + text_len);
                check_error_write(retur);

                wait(NULL);
                retur = 1;
                break;

            case 1:
                DEBUG_PRINT8(getpid(), 2);

                retur = write(fd2[1], &msg->text_len, sizeof(unsigned int) + text_len);
                check_error_write(retur);

                wait(NULL);
                retur = 1;
                break;

            case 3:
                printf("Beskjed fra server om korrupt fil, server terminerer\n");
                terminate_children();
                free(msg);
                msg = NULL;
                return 0;
                break;

            case 5:
                DEBUG_PRINT9(getpid());
                printf("Server har terminert grunnet feil\n");
                terminate_children();
                free(msg);
                msg = NULL;
                return 0;
                break;

            case 7:
                printf("Jobb lest ferdig, barneprosessene kan terminere\n");
                terminate_children();
                free(msg);
                msg = NULL;
                return 0;
                break;

            default:
                printf("Ugyldig jobbtype\n");
                free(msg);
                msg = NULL;
                terminate_children();
                terminate_error();
                break;
        }

        free(msg);
        msg = NULL;
        done_jobs++;
    }
    return retur;
}

/*  Metode for henting av x antall jobber. Bruker gir et tallet
*   og dette tallet sendes til get_jobs. Hvis tallet er større enn maks
*   tall ved 3 bytes vil det kalles get_jobs flere ganger.
*
*   Return:
*       Om det kan leses flere jobber eller ikke
*/
int get_x_jobs() {
    printf("Hvor mange jobber vil du ha utfort?\n");
    int x = int_from_user();
    int max = MAXJOBS;
    int retur = 1;
    while(retur && x > 0) {
        if(x > max) {
            retur = get_jobs(max);
            x = x-max;
        }
        else {
            retur = get_jobs(x);
            break;
        }
    }
    return retur;
}

/*  Metode for henting av alle eller resten av Jobbene
*   fra server. Det kalles på get_jobs med antall MAXJOBS som er
*   det største tallet man kan få ved 3 bytes. Denne kalles om igjen
*   til get_jobs returnerer 0 og alle jobbene er lest.
*/
void get_all_jobs() {
    unsigned int x = MAXJOBS;
    while(get_jobs(x));
}

/*  Metode for signal-handling av SIGUSR1-signal
*   som sendes fra barneprosess til foreldre-prosessen.
*   Metoden gjør ingenting.
*/
void signal_handler() {}

/*  Metode for signal-handling av Ctrl+C signal.
*   Barneprossessene terminrers hvis de kjører og
*   foreldre-prosessen terminerer grunnet feil.
*   Hvis melding-structen fra server ikke er free-et vil denne
*   free-es.
*   Barnene free-er teksten sin hvis den ikke er free-et
*
*   Input:
*       signal: signalet som ble mottat
*/
void ctrl_c_handler(int signal) {
    if(getpid() == parent_pid) {
        printf("Mottok signal: %d\n", signal);
        if(msg != NULL) {
            free(msg);
        }
        terminate_error();
    }
    else if(child_text != NULL){
        free(child_text);
        child_text = NULL;
    }
    exit(EXIT_FAILURE);
}

/*  Metode for normal terminering.
*   Det sendes melding type '2', som sier til
*   server at klienten terminerer normalt.
*/
void terminate_normal() {
    printf("Terminerer normalt...\n");
    char c = '2';
    int retur = write(sock, &c, sizeof(char));
    check_error_write(retur);
    close(fd1[1]);
    close(fd2[1]);
    retur = close(sock);
    check_error_close(retur);
    DEBUG_PRINT_SLUTT(getpid());
    exit(EXIT_SUCCESS);
}

/*  Metode for terminering grunnet en feil.
*   Det sendes melding type '3', som sier til
*   server at klienten terminerer grunnet feil.
*/
void terminate_error() {
    printf("Terminerer grunnet feil...\n");
    char c = '3';
    int retur = write(sock, &c, sizeof(char));
    if(retur == -1) {
        perror("write");
    }
    close(fd1[1]);
    close(fd2[1]);
    retur = close(sock);
    check_error_close(retur);
    exit(EXIT_FAILURE);
}

/*  Metoden sender tekstlengde 0 til barneprosessene
*   som gjør at de vil terminere.
*   children_terminated settes høy.
*/
void terminate_children() {
    int retur;
    if(!children_terminated) {
        unsigned int t = 0;
        retur = write(fd1[1], &t, sizeof(unsigned int));
        check_error_write(retur);

        retur = write(fd2[1], &t, sizeof(unsigned int));
        check_error_write(retur);

        children_terminated = 1;
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "write"
*
*   Input:
*       ret: returverdien fra write som sjekkes
*/
void check_error_write(int ret) {
    if(ret == -1) {
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
void check_error_read(int ret) {
    if(ret == -1) {
        perror("read");
        terminate_error();
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "read"
*
*   Input:
*       ret: returverdien fra read som sjekkes
*/
void check_error_read_child(int ret) {
    if(ret == -1) {
        perror("read");
        exit(EXIT_FAILURE);
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
        terminate_children();
        exit(EXIT_FAILURE);
    }
}

/*  Metoden sjekker om det har oppstått feil ved
*   kall på "sigaction"
*
*   Input:
*       ret: returverdien som sjekkes
*/
void check_error_sigaction(int ret) {
    if(ret == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void check_error_pipe(int ret) {
    if(ret == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
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

/*  Metoden sjekker om cecksummen fra server er lik den
*   som klient har regnet ut. Er den feil terminerer klienten.
*
*   Input:
*       old_checksum: checksummen fra serveren
*       new_checksum: checksummen regnet ut av klienten
*/
void check_checksum(unsigned char old_checksum, unsigned char new_checksum) {
    if(old_checksum != new_checksum) {
        fprintf(stderr, "Feil checksum! %d != %d\n", old_checksum, new_checksum);
        terminate_children();
        terminate_error();
    }
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

/*  Metoden spør bruker om et tall og tar imot et tall hvis det er et
*   gyldig tall.
*
*   Return:
*       tallet fra bruker
*/
int int_from_user() {
    int input;
    char buf[64];
    char *endptr;
    while(1) {
        printf("Skriv inn \n");
        fgets(buf, sizeof(buf), stdin);
        input = strtol(buf, &endptr, 10);
        if(endptr == (char*)&buf && input == 0) {
            printf("Ikke et tall!\n");
        }
        else {
            break;
        }
    }
    return input;
}
