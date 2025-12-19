#include "bakery.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void bakery_prog_1(char *host, int client_id)
{
    CLIENT *clnt;
    quad_t *result_time;
    int *result_number;
    int number;

#ifndef DEBUG
    clnt = clnt_create(host, BAKERY_PROG, BAKERY_VER, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }
#endif

    printf("Запуск клиента ID: %d\n", client_id);
    for (int i = 1;; i++) {
		usleep(100 * 1000);
        fflush(stdout);

        result_number = get_ticket_number_1(&client_id, clnt);
        if (result_number == (int *)NULL) {
            clnt_perror(clnt, "Ошибка RPC при получении номера");
            break;
        }

        number = *result_number;
        printf("Заявка: %5d - номер: %5d ", i, number);
        fflush(stdout);

        int attempts = 0;
        for (;;) {
            result_time = get_service_time_1(&number, clnt);
            if (result_time == (quad_t *)NULL) {
                clnt_perror(clnt, "Ошибка RPC при ожидании времени");
                break;
            }

            if (*result_time >= 0) {
                long long total = (long long)*result_time;
                long long sec = total / 1000000LL;
                long long usec = total % 1000000LL;

                printf("Время: %02lld.%06lld\n", sec, usec);
                break;
            }

            if (++attempts % 20 == 0) { printf("."); fflush(stdout); }
            usleep(50000);
        }
    }

#ifndef DEBUG
    clnt_destroy(clnt);
#endif
}

int main(int argc, char *argv[])
{
    char *host;
    int client_id;

    if (argc < 2) {
        exit(1);
    }
    host = argv[1];
    client_id = (argc >= 3) ? atoi(argv[2]) : (int)getpid();
    bakery_prog_1(host, client_id);
    exit(0);
}
