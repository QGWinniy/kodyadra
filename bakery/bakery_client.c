#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "bakery.h"

// Функция для получения случайного ID клиента
int get_random_client_id() {
    return (rand() % 10) + 1; // Клиенты с ID от 1 до 10
}

// Функция для случайной задержки
void random_delay() {
    int delay_ms = (rand() % 500) + 100; // Задержка от 100 до 600 мс
    usleep(delay_ms * 1000);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s server_host [num_requests]\n", argv[0]);
        exit(1);
    }

    char *host = argv[1];
    int num_requests = 5; // По умолчанию 5 запросов
    
    if (argc > 2) {
        num_requests = atoi(argv[2]);
        if (num_requests <= 0) {
            num_requests = 5;
        }
    }

    CLIENT *clnt;
    
    // Создаем клиента
    clnt = clnt_create(host, BAKERY_PROG, BAKERY_VER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    srand(time(NULL));
    
    printf("=== Bakery Client: Automatic Requests ===\n");
    printf("Server: %s\n", host);
    printf("Number of requests: %d\n\n", num_requests);
    
    int successful_requests = 0;
    int tickets[num_requests];
    
    // Фаза 1: Получаем заявки
    printf("--- Phase 1: Requesting tickets ---\n");
    for (int i = 0; i < num_requests; i++) {
        int client_id = get_random_client_id();
        
        printf("Request %d: Client %d requesting ticket... ", i+1, client_id);
        
        int *ticket_result = get_ticket_1(&client_id, clnt);
        if (ticket_result == NULL) {
            printf("FAILED (RPC error)\n");
            tickets[i] = -1;
            continue;
        }
        
        int ticket_number = *ticket_result;
        if (ticket_number == -1) {
            printf("FAILED (server error)\n");
            tickets[i] = -1;
        } else {
            printf("SUCCESS - Ticket %d\n", ticket_number);
            tickets[i] = ticket_number;
            successful_requests++;
        }
        
        // Случайная задержка между запросами
        if (i < num_requests - 1) {
            random_delay();
        }
    }
    
    // Небольшая пауза перед проверкой времени
    printf("\nWaiting for processing to complete...\n");
    sleep(2);
    
    // Фаза 2: Проверяем время обработки
    // printf("\n--- Phase 2: Checking processing times ---\n");
    int checked_tickets = 0;
    
    for (int i = 0; i < num_requests; i++) {
        if (tickets[i] != -1) {
            // printf("Checking ticket %d... ", tickets[i]);
            
            long long *time_result = get_processing_time_1(&tickets[i], clnt);
            if (time_result == NULL) {
                printf("FAILED (RPC error)\n");
                continue;
            }
            
            long long processing_time = *time_result;
            if (processing_time == -1) {
                printf("NOT FOUND\n");
            } else if (processing_time == 0) {
                printf("STILL PROCESSING\n");
            } else {
                // printf("%lld ns (%.3f ms)\n", 
                //        processing_time, processing_time / 1000000.0);
                checked_tickets++;
            }
            
            // Случайная задержка между проверками
            if (i < num_requests - 1) {
                random_delay();
            }
        }
    }
    
    // printf("\n=== Summary ===\n");
    // printf("Total requests: %d\n", num_requests);
    // printf("Successful ticket requests: %d\n", successful_requests);
    // printf("Processing times checked: %d\n", checked_tickets);
    
    clnt_destroy(clnt);
    return 0;
}