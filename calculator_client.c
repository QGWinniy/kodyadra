#include "calculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

const char ops[] = { '+', '-', '*', '/' };
const int NUM_OPS = 4;

void calculator_prog_1_auto(char *host)
{
    CLIENT *clnt;
    struct CALCULATOR *result_1;
    struct CALCULATOR args;
    unsigned long request_id = 0;

    srand(time(NULL));

    clnt = clnt_create(host, CALCULATOR_PROG, CALCULATOR_VER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    printf("Авто-клиент запущен. Подключено к %s\n", host);
    printf("Бесконечная отправка случайных запросов...\n");
    printf("Нажмите Ctrl+C для остановки.\n\n");

    while (1) {
        float a = (rand() % 201) - 100.0f;
        float b = (rand() % 201) - 100.0f;
        if (b == 0.0f) b = 1.0f;
        int op_index = rand() % NUM_OPS;
        char op_char = ops[op_index];
        args.arg1 = a;
        args.arg2 = b;

        switch(op_char) {
            case '+': args.op = ADD; break;
            case '-': args.op = SUB; break;
            case '*': args.op = MUL; break;
            case '/': args.op = DIV; break;
        }

        result_1 = calculator_proc_1(&args, clnt);
        if (result_1 == NULL) {
            clnt_perror(clnt, "RPC вызов не удался");
            continue;
        }

        request_id++;
        printf("Запрос %lu: %7.2f %c %7.2f = %8.2f\n", request_id, a, op_char, b, result_1->result);
    }
    clnt_destroy(clnt);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("usage: %s server_host\n", argv[0]);
        exit(1);
    }

    calculator_prog_1_auto(argv[1]);
    return 0;
}
