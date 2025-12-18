#include "calculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // для sleep()
#include <time.h>     // для srand()

// Массив операций для удобства
const char ops[] = { '+', '-', '*', '/' };
const int NUM_OPS = 4;

void calculator_prog_1_auto(char *host)
{
    CLIENT *clnt;
    struct CALCULATOR *result_1;
    struct CALCULATOR args;

    // Инициализация генератора случайных чисел
    srand(time(NULL));

    // Используем TCP — надёжнее для сетевого соединения
    clnt = clnt_create(host, CALCULATOR_PROG, CALCULATOR_VER, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(host);
        exit(1);
    }

    printf("Авто-клиент запущен. Подключено к %s\n", host);
    printf("Бесконечная отправка случайных запросов...\n");
    printf("Нажмите Ctrl+C для остановки.\n\n");

    while (1) {
        // Генерируем случайные числа (от -100 до 100)
        float a = (rand() % 201) - 100.0f;  // [-100, 100]
        float b = (rand() % 201) - 100.0f;

        // Избегаем деления на 0
        if (b == 0.0f) b = 1.0f;

        // Выбираем случайную операцию
        int op_index = rand() % NUM_OPS;
        char op_char = ops[op_index];

        // Заполняем аргументы
        args.arg1 = a;
        args.arg2 = b;

        switch(op_char) {
            case '+': args.op = ADD; break;
            case '-': args.op = SUB; break;
            case '*': args.op = MUL; break;
            case '/': args.op = DIV; break;
        }

        // Отправляем запрос
        result_1 = calculator_proc_1(&args, clnt);

        if (result_1 == NULL) {
            clnt_perror(clnt, "RPC вызов не удался");
            // sleep(2);
            continue; // попробуем снова
        }

        printf("Запрос: %7.2f %c %7.2f = %8.2f\n", a, op_char, b, result_1->result);

        // Пауза 0.5 секунды между запросами
        // usleep(500000); // 500 миллисекунд
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