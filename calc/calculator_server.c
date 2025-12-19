#include "calculator.h"

float *
calculator_proc_1_svc(struct CALCULATOR *argp, struct svc_req *rqstp)
{
	static __thread float result;

	(void)rqstp;
	switch (argp->op) {
	case ADD:
		result = argp->arg1 + argp->arg2;
		break;
	case SUB:
		result = argp->arg1 - argp->arg2;
		break;
	case MUL:
		result = argp->arg1 * argp->arg2;
		break;
	case DIV:
		result = (argp->arg2 != 0.0f) ? (argp->arg1 / argp->arg2) : 0.0f;
		break;
	default:
		result = 0.0f;
		break;
	}
	return &result;
}
