#ifndef __DEBUG_SERVER_H
#define __DEBUG_SERVER_H

#ifdef DEBUG

#define DEBUG_PRINT_START(pid)              printf(">>> %d <<< STARTER PROGRAM\n", pid)
#define DEBUG_PRINT_SLUTT(pid)              printf(">>> %d <<< AVSLUTTER PROGRAM\n", pid)

#define DEBUG_PRINT_ARG(pid, argv)          printf(">>> %d <<< filename: %s \n>>> %d <<< port: %s\n", pid, argv[1], pid, argv[2])
#define DEBUG_PRINT_WRITE1(pid, melding)    printf(">>> %d <<< SENDER SEVRER MELDING TYPE           : %c\n", pid, melding)

#define DEBUG_PRINT1(pid, filename)         printf(">>> %d <<< AAPNER %s FOR LESING\n", pid, filename)
#define DEBUG_PRINT2(pid)                   printf(">>> %d <<< KOBLER TIL KLIENT...\n", pid)
#define DEBUG_PRINT3(pid)                   printf(">>> %d <<< VENTER MELDING FRA KLIENT...\n", pid)
#define DEBUG_PRINT4(pid, msg_client)       printf(">>> %d <<< MELDING FRA KLIENT                   : %c\n", pid, msg_client)
#define DEBUG_PRINT5(pid)                   printf("\n>>> %d <<< LESER JOBB FRA FIL...\n", pid)
#define DEBUG_PRINT6(pid)                   printf(">>> %d <<< FIL ER FERDIG LEST\n", pid)
#define DEBUG_PRINT7(pid, jobtype)          printf(">>> %d <<< JOBBTYPE                             : %c\n", pid, jobtype)
#define DEBUG_PRINT8(pid, text_len)         printf(">>> %d <<< LESER TEKSTLENGDE                    : %d\n", pid, text_len)
#define DEBUG_PRINT9(pid, checksum)         printf(">>> %d <<< CHECKSUM                             : %d\n", pid, checksum)
#define DEBUG_PRINT10(pid, msg_len)         printf(">>> %d <<< MELDINGSLENGDE                       : %d\n", pid, msg_len)
#define DEBUG_PRINT11(pid)                  printf("\n>>> %d <<< SKRIVER MELDING TIL SOCKET...\n", pid)
#define DEBUG_PRINT12(pid, bytes_written)   printf(">>> %d <<< ANTALL BYTES SKREVET TIL SOCKET      : %ld\n", pid, bytes_written)
#define DEBUG_PRINT13(pid, bytes_read)      printf(">>> %d <<< LESER TEKST FRA FIL, BYTES LEST      : %ld\n", pid, bytes_read)
#define DEBUG_PRINT14(pid)                  printf(">>> %d <<< HENTER MAKS ANTALL JOBBER...\n", pid)
#define DEBUG_PRINT15(pid, num_jobs)        printf(">>> %d <<< ANTALL JOBBER SOM HENTES             : %d\n", pid, num_jobs)
#define DEBUG_PRINT16(pid, type)            printf(">>> %d <<< SKRIVER MELDING TIL KLIENT, TYPE     : %d\n", pid, type)

#else

#define DEBUG_PRINT_START(pid)              printf("Server har startet\n");
#define DEBUG_PRINT_ARG(pid, argv)
#define DEBUG_PRINT_SLUTT(pid)
#define DEBUG_PRINT_WRITE1(pid, melding)
#define DEBUG_PRINT1(pid, filename)
#define DEBUG_PRINT2(pid)
#define DEBUG_PRINT3(pid)                   printf("Venter paa melding fra klient...\n");
#define DEBUG_PRINT4(pid, msg_client)
#define DEBUG_PRINT5(pid)
#define DEBUG_PRINT6(pid)
#define DEBUG_PRINT7(pid, jobtype)
#define DEBUG_PRINT8(pid, text_len)
#define DEBUG_PRINT9(pid, checksum)
#define DEBUG_PRINT10(pid, msg_len)
#define DEBUG_PRINT11(pid)                  printf("Skriver melding til socket...\n")
#define DEBUG_PRINT12(pid, bytes_written)
#define DEBUG_PRINT13(pid, bytes_read)
#define DEBUG_PRINT14(pid)
#define DEBUG_PRINT15(pid, num_jobs)
#define DEBUG_PRINT16(pid, type)

#endif
#endif
