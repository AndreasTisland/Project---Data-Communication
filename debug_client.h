#ifndef __DEBUG_CLIENT_H
#define __DEBUG_CLIENT_H

#ifdef DEBUG

#define DEBUG_PRINT_START(pid)              printf(">>> %d <<< STARTER PROGRAM\n", pid)
#define DEBUG_PRINT_SLUTT(pid)              printf(">>> %d <<< AVSLUTTER PROGRAM\n", pid)
#define DEBUG_PRINT_ARG(pid, argv)          printf(">>> %d <<< host: %s \n>>> %d <<< port: %s\n", pid, argv[1], pid, argv[2])
#define DEBUG_PRINT_WRITE1(pid, melding)    printf(">>> %d <<< SENDER SERVER MELDING TYPE                               : %c\n", pid, melding)
#define DEBUG_PRINT_READ1(pid, bytes_read)  printf(">>> %d <<< LESER JOBBINFO OG TEKSTLENGDE FRA SOCKET, BYTES MOTTAT   : %ld\n", pid, bytes_read)
#define DEBUG_PRINT_READ2(pid, bytes_read)  printf(">>> %d <<< LESER TEKST FRA SOCKET, BYTES MOTTATT                    : %ld\n", pid, bytes_read)
#define DEBUG_PRINT_READ3(pid, retur)       printf(">>> %d <<< LESER JOBBINFO OG TEKSTLENGDE FRA SOCKET, BYTES LEST     : %d\n", pid, retur)

#define DEBUG_PRINT1(pid)                   printf(">>> %d <<< OPPRETTER SOCKET...\n", pid)
#define DEBUG_PRINT2(pid)                   printf(">>> %d <<< KOBLER TIL SERVER...\n", pid)
#define DEBUG_PRINT3(pid)                   printf(">>> %d <<< OPPRETTER BRUKERMENY...\n", pid)
#define DEBUG_PRINT4(pid, type)             printf(">>> %d <<< JOBBTYPE                                                 : %c\n", pid, type);
#define DEBUG_PRINT5(pid, checksum)         printf(">>> %d <<< CHECKSUM FRA SERVER                                      : %d\n", pid, checksum)
#define DEBUG_PRINT6(pid, text_len)         printf(">>> %d <<< LENGDE PAA TEKST                                         : %d\n", pid, text_len)
#define DEBUG_PRINT7(pid, new_checksum)     printf(">>> %d <<< REGNER UT CHECKSUM                                       : %d\n", pid, new_checksum)
#define DEBUG_PRINT8(pid, child_pid)        printf(">>> %d <<< SENDER TEKST TIL BARN %d OG VENTER PAA SIGNAL\n", pid, child_pid)
#define DEBUG_PRINT9(pid)                   printf(">>> %d <<< MOTTAT SIGNAL OM AT SERVER HAR TERMINERT\n", pid)
#define DEBUG_PRINT10(pid)                  printf(">>> %d <<< MOTTAT SIGNAL OM AT JOBBEN ER LEST FERDIG OG BARNEPROSESSENE KAN TEMRINERE\n", pid)
#define DEBUG_PRINT11(pid, text_len)        printf(">>> %d <<< LESER TEKSTLENGDE FRA FORELDER, LENGDE                   : %d\n", pid, text_len)
#define DEBUG_PRINT12(pid, retur)           printf(">>> %d <<< LESER TEKST FRA FORELDER, BYTES MOTTATT                  : %d\n", pid, retur)
#define DEBUG_PRINT13(pid)                  printf(">>> %d <<< SKRIVER UT TEKST:\n", pid)
#define DEBUG_PRINT14(pid)                  printf(">>> %d <<< BARNEPROSESS OPPRETTET\n", pid)
#define DEBUG_PRINT15(pid)                  printf(">>> %d <<< BARNEPROSESS TERMINERER\n", pid)
#define DEBUG_PRINT16(pid, num_jobs)        printf(">>> %d <<< ANTALL JOBBER SOM HENTES                                 : %d\n", pid, num_jobs)

#define DEBUG_PRINT_MENY(pid, input)        printf(">>> %d <<< OPPRETTER BRUKERMENY...\n", pid)

#else

#define DEBUG_PRINT_START(pid)              printf("Klient har startet\n")
#define DEBUG_PRINT_SLUTT(pid)
#define DEBUG_PRINT_ARG(pid, argv)
#define DEBUG_PRINT_WRITE1(pid, melding)
#define DEBUG_PRINT_READ1(pid)
#define DEBUG_PRINT_READ2(pid, bytes_read)
#define DEBUG_PRINT_READ3(pid, retur)
#define DEBUG_PRINT1(pid)
#define DEBUG_PRINT2(pid)
#define DEBUG_PRINT3(pid)
#define DEBUG_PRINT4(pid, type)
#define DEBUG_PRINT5(pid, checksum)
#define DEBUG_PRINT6(pid, text_len)
#define DEBUG_PRINT7(pid, new_checksum)
#define DEBUG_PRINT8(pid, child_pid)
#define DEBUG_PRINT9(pid)
#define DEBUG_PRINT10(pid)
#define DEBUG_PRINT11(pid, text_len)
#define DEBUG_PRINT12(pid, retur)
#define DEBUG_PRINT13(pid)
#define DEBUG_PRINT14(pid)
#define DEBUG_PRINT15(pid)                  printf("Barneprossess <<< %d >>> terminerer...\n", pid)
#define DEBUG_PRINT16(pid, num_jobs)
#define DEBUG_PRINT_MENY(pid, input)

#endif
#endif
