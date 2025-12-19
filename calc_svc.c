/* calculator_svc.c (Modified for Multi-threading) */
#include "calculator.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> /* Подключаем потоки */
#include <unistd.h>  /* Для sleep, если захотите эмулировать долгие вычисления */

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

/* Структура для передачи данных в поток */
struct thread_data {
    SVCXPRT *transp;              /* Транспортный дескриптор для ответа */
    struct CALCULATOR args;       /* Аргументы запроса */
};

/* Функция рабочего потока */
void *processing_thread(void *ptr) {
    struct thread_data *data = (struct thread_data *)ptr;
    struct CALCULATOR result;

    /* Инициализируем результат нулями */
    memset(&result, 0, sizeof(result));

    /* Логика калькулятора (раньше была в server.c) */
    /* Копируем входные данные в результат для удобства или логирования */
    result.op = data->args.op;
    result.arg1 = data->args.arg1;
    result.arg2 = data->args.arg2;

    switch (data->args.op) {
        case ADD:
            result.result = data->args.arg1 + data->args.arg2;
            break;
        case SUB:
            result.result = data->args.arg1 - data->args.arg2;
            break;
        case MUL:
            result.result = data->args.arg1 * data->args.arg2;
            break;
        case DIV:
            if (data->args.arg2 != 0)
                result.result = data->args.arg1 / data->args.arg2;
            else
                result.result = 0; // Обработка деления на 0
            break;
        default:
            result.result = 0;
            break;
    }

    /* Эмуляция долгой работы, чтобы проверить многопоточность (опционально) */
    /* sleep(5); */

    /* Отправляем ответ клиенту */
    if (!svc_sendreply(data->transp, (xdrproc_t) xdr_CALCULATOR, (char *)&result)) {
        svcerr_systemerr(data->transp);
    }
    if (!svc_freeargs(data->transp, (xdrproc_t) xdr_CALCULATOR, (char *)&data->args)) {
        fprintf(stderr, "unable to free arguments\n");
    }

    free(data);
    pthread_exit(NULL);
    return NULL;
}

static void
calculator_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
    struct thread_data *t_data;
    pthread_t th;
    pthread_attr_t attr;
    int ret;

    switch (rqstp->rq_proc) {
    case NULLPROC:
        (void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
        return;

    case CALCULATOR_PROC:
        t_data = (struct thread_data *) malloc(sizeof(struct thread_data));
        if (t_data == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return;
        }

        t_data->transp = transp;

        memset ((char *)&t_data->args, 0, sizeof (t_data->args));
        if (!svc_getargs (transp, (xdrproc_t) xdr_CALCULATOR, (caddr_t) &t_data->args)) {
            svcerr_decode (transp);
            free(t_data);
            return;
        }

        pthread_attr_init(&attr);
        ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if (ret != 0) {
            fprintf(stderr, "Error setting detach state\n");
            free(t_data);
            return;
        }

        if (pthread_create(&th, &attr, processing_thread, (void *)t_data) != 0) {
            fprintf(stderr, "Error creating thread\n");
            free(t_data);
            return;
        }

        pthread_attr_destroy(&attr);


        break;

    default:
        svcerr_noproc (transp);
        return;
    }
}

int
main (int argc, char **argv)
{
    register SVCXPRT *transp;

    pmap_unset (CALCULATOR_PROG, CALCULATOR_VER);

    transp = svcudp_create(RPC_ANYSOCK);
    if (transp == NULL) {
        fprintf (stderr, "%s", "cannot create udp service.");
        exit(1);
    }
    if (!svc_register(transp, CALCULATOR_PROG, CALCULATOR_VER, calculator_prog_1, IPPROTO_UDP)) {
        fprintf (stderr, "%s", "unable to register (CALCULATOR_PROG, CALCULATOR_VER, udp).");
        exit(1);
    }

    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (transp == NULL) {
        fprintf (stderr, "%s", "cannot create tcp service.");
        exit(1);
    }
    if (!svc_register(transp, CALCULATOR_PROG, CALCULATOR_VER, calculator_prog_1, IPPROTO_TCP)) {
        fprintf (stderr, "%s", "unable to register (CALCULATOR_PROG, CALCULATOR_VER, tcp).");
        exit(1);
    }

    svc_run ();
    fprintf (stderr, "%s", "svc_run returned");
    exit (1);
    /* NOTREACHED */
}
