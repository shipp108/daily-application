#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <execinfo.h>
#include <err.h>

#include <assert.h>

#define MAX_STACK_FRAMES	(64)
static void *stack_traces[MAX_STACK_FRAMES];

static char const *icky_global_program_name;

/* Resolve symbol name and source location given the path to 
   the executable and an address */
static int addr2line(char const *const program_name, void const * const addr)
{
	char addr2line_cmd[512] = {0};
	/* have adde2line map the address to the relent line in the
	   code */
	sprintf(addr2line_cmd, "addr2line -f -p -e %.256s %p", program_name, addr);
	
	/* This will print a nicely formatted string specifying the
	   function and source line of the address */
	return system(addr2line_cmd);
}

static void posix_print_stack_trace()
{
	int i, trace_size = 0;
	char **messages = (char **)NULL;
	
	trace_size = backtrace((void**)stack_traces, MAX_STACK_FRAMES);
	messages = backtrace_symbols(stack_traces, trace_size);

	/* skip the first couple stack frames (as they are this 
	   function and our handler) and also skip the last frame
	   as it's a (always?) junk. */
	for (i = 3; i < (trace_size - 1); ++i) {
		if (addr2line(icky_global_program_name, stack_traces[i]) != 0) {
			printf("    error determining line # for %s\n", messages[i]);
		}
	}
	if (messages) {
		free(messages);
	}
}

void posix_signal_handler(int sig, siginfo_t *siginfo, void *context)
{
	(void)context;
	switch (sig) {
	case SIGSEGV:
		fprintf(stderr, "Caught SIGSEGV: Segmentation Fault\n");
		break;
	case SIGINT:
		fprintf(stderr, "Caught SIGINT: Interactive attention signal, (usually ctrl+c)\n");
		break;
	case SIGFPE:
		switch (siginfo->si_code) {
		case FPE_INTDIV:
			fprintf(stderr, "Caught SIGFPE: (integer divide by zero)\n");
			break;
		case FPE_INTOVF:
			fprintf(stderr, "Caught SIGFPE: (integer overflow)\n");
			break;
		case FPE_FLTDIV:
			fprintf(stderr, "Caught SIGFPE: (floatint-point divide by zero)\n");
			break;
		case FPE_FLTOVF:
			fprintf(stderr, "Caught SIGFPE: (floating-point overflow)\n");
			break;
		case FPE_FLTUND:
			fprintf(stderr, "Caught SIGFPE: (floating-point underflow)\n");
			break;
		case FPE_FLTRES:
			fprintf(stderr, "Caught SIGFPE: (floating-point inexact result)\n");
			break;
		case FPE_FLTINV:
			fprintf(stderr, "Caught SIGFPE: (floating-point invalid operation)\n");
			break;
		case FPE_FLTSUB:
			fprintf(stderr, "Caught SIGFPE: (subscript out of range)\n");
			break;
		default:
			fprintf(stderr, "Caught SIGFPE: Arithmetic Exception\n");
			break;
		}
	case SIGILL:
		switch(siginfo->si_code) {
		case ILL_ILLOPC:
			fprintf(stderr, "Caught SIGILL: (illegal opcode)\n");
			break;
		case ILL_ILLOPN:
			fprintf(stderr, "Caught SIGILL: (illegal operand)\n");
			break;
		case ILL_ILLADR:
			fprintf(stderr, "Caught SIGILL: (illegal addressing mode)\n");
			break;
		case ILL_ILLTRP:
			fprintf(stderr, "Caught SIGILL: (illegal trap)\n");
			break;
		case ILL_PRVOPC:
			fprintf(stderr, "Caught SIGILL: (privileged opcode)\n");
			break;
		case ILL_PRVREG:
			fprintf(stderr, "Caught SIGILL: (privileged register)\n");
			break;
		case ILL_COPROC:
			fprintf(stderr, "Caught SIGILL: (coprocessor error)\n");
			break;
		case ILL_BADSTK:
			fprintf(stderr, "Caught SIGILL: (internal stack error)\n");
		break;
		default:
			fprintf(stderr, "Caught SIGILL: Illegal Instruction\n");
			break;
		}
	case SIGTERM:
		fprintf(stderr, "Caught SIGTERM: a termination request was sent to the program\n");
		break;
	case SIGABRT:
		fprintf(stderr, "Caught SIGABRT: usually caused by an abort() or assert()\n");
		break;
	default:
		break;
	}
	posix_print_stack_trace();
	exit(1);
}

static unsigned char alternate_stack[SIGSTKSZ];
void set_signal_handler()
{
	/* setup alternate size */
	{
		stack_t ss = {};
		ss.ss_sp = (void*)alternate_stack;
		ss.ss_size = SIGSTKSZ;
		ss.ss_flags = 0;
		if (sigaltstack(&ss, NULL) != 0) {
			err(1, "signalstack");
		}
	}

	/* register our signal handlers */
	{
		struct sigaction sig_action = {};
		sig_action.sa_sigaction = posix_signal_handler;
		sigemptyset(&sig_action.sa_mask);
		sig_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
		
		if (sigaction(SIGSEGV, &sig_action, NULL) != 0) {
			err(1, "sigaction");
		}
		if (sigaction(SIGFPE, &sig_action, NULL) != 0) {
			err(1, "sigaction");
		}
		if (sigaction(SIGINT, &sig_action, NULL) != 0) {
			err(1, "sigaction");
		}
		if (sigaction(SIGILL, &sig_action, NULL) != 0) {
			err(1, "sigaction");
		}
		if (sigaction(SIGTERM, &sig_action, NULL) != 0) {
			err(1, "sigaction");
		}
		if (sigaction(SIGABRT, &sig_action, NULL) != 0) {
			err(1, "sigaction");
		}
	}
}

void cause_calamity();

int main(int argc, char **argv)
{
	icky_global_program_name = argv[0];	

	set_signal_handler();

	cause_calamity();

	return 0;
}

/* error test function */

int divide_by_zero();
void cause_segfault();
void infinite_loop();
void illegal_instruction();
void stack_overflow();

void cause_calamity()
{
	printf("\033[34mtest: cause_segfault\033[0m\n");
	cause_segfault();

	printf("\033[34mtest: divide_by_zero\033[0m\n");
	divide_by_zero();

	printf("\033[34mtest: assert\033[0m\n");
	assert(false);

	printf("\033[34mtest: infinite_loop\033[0m\n");
	infinite_loop();

	printf("\033[34mtest: illegal_instruction\033[0m\n");
	illegal_instruction();

	printf("\033[34mtest: stack_overflow\033[0m\n");
	stack_overflow();
}

int divide_by_zero()
{
	int a = 1;
	int b = 0;
	return  a / b;
}

void cause_segfault()
{
	int *p = (int *)0x12345678;
	*p = 0;
}

void infinite_loop() 
{
	while (1) {
		;
	}
}

void illegal_instruction()
{
	raise(SIGILL);
}
void stack_overflow()
{
	int foo[100];
	stack_overflow();
}

