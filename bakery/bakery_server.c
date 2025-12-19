#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "bakery.h"

#define MAX_CLIENTS 100
#define MAX_TICKETS 1000

// Структура для хранения информации о заявке
typedef struct {
    int ticket_number;
    long long processing_time_ns;
    int client_id;
    int is_processed;
    long long start_time_ns;
} TicketInfo;

// Глобальные переменные
TicketInfo tickets[MAX_TICKETS];
int ticket_counter = 0;
int next_ticket_number = 1;

// Функция для получения текущего времени в наносекундах
long long get_nanotime() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

// Алгоритм булочной - получение номера
int get_ticket(int client_id) {
    int ticket_number = next_ticket_number++;
    
    // Находим свободный слот для хранения информации о заявке
    int slot = -1;
    for (int i = 0; i < MAX_TICKETS; i++) {
        if (tickets[i].ticket_number == 0) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        slot = ticket_counter % MAX_TICKETS;
    }
    
    // Сохраняем информацию о заявке
    tickets[slot].ticket_number = ticket_number;
    tickets[slot].client_id = client_id;
    tickets[slot].is_processed = 0;
    tickets[slot].processing_time_ns = 0;
    tickets[slot].start_time_ns = get_nanotime();
    
    printf("Ticket %d issued for client %d at time %lld\n", 
           ticket_number, client_id, tickets[slot].start_time_ns);
    
    return ticket_number;
}


void process_ticket(int ticket_number) {
    long long start_time = get_nanotime();
    
    // Ищем заявку
    for (int i = 0; i < MAX_TICKETS; i++) {
        if (tickets[i].ticket_number == ticket_number) {
            usleep(10000);
			
			long long end_time = get_nanotime();
            long long processing_time = end_time;
            // long long processing_time = end_time - tickets[i].start_time_ns;
            
            tickets[i].processing_time_ns = processing_time;
            tickets[i].is_processed = 1;
            
            printf("Ticket %d processed in %lld ns\n", ticket_number, processing_time);
            break;
        }
    }
}

// Первый удаленный вызов: получение номера в очереди
int *get_ticket_1_svc(int *client_id, struct svc_req *rqstp) {
    (void)rqstp;
    static int result;
    
    int id = *client_id;
    
    if (id <= 0) {
        result = -1; // Неверный ID клиента
        return &result;
    }
    
    printf("\n=== Processing request from client %d ===\n", id);
    
    // 1. Получаем номерок
    int ticket = get_ticket(id);
    printf("Client %d received ticket: %d\n", id, ticket);
    
    // 2. Немедленно обрабатываем заявку (однопоточный режим)
    printf("Processing ticket %d immediately...\n", ticket);
    process_ticket(ticket);
    
    printf("Ticket %d completed for client %d\n", ticket, id);
    
    result = ticket;
    return &result;
}

// Второй удаленный вызов: получение времени обработки заявки
int *get_processing_time_1_svc(int *ticket_number, struct svc_req *rqstp) {
    (void)rqstp;
    static long long result;
    
    int ticket = *ticket_number;
    
    if (ticket <= 0) {
        result = -1; // Неверный номер заявки
        return &result;
    }
    
    // Ищем заявку
    int found = 0;
    for (int i = 0; i < MAX_TICKETS; i++) {
        if (tickets[i].ticket_number == ticket) {
            found = 1;
            if (tickets[i].is_processed) {
                result = tickets[i].processing_time_ns;
                printf("Returning processing time for ticket %d: %lld ns\n", 
                       ticket, result);
            } else {
                result = 0; // Заявка еще не обработана
                printf("Ticket %d is still processing\n", ticket);
            }
            break;
        }
    }
    
    if (!found) {
        result = -1; // Заявка не найдена
        printf("Ticket %d not found\n", ticket);
    }
    
    return &result;
}

// Обязательная функция
int bakery_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result) {
    (void)transp;
    xdr_free(xdr_result, result);
    return 1;
}