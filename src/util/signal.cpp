#include "signal.h"
#include "stdio.h"

namespace smsc {
namespace util {

bool setSignalHandler(int signo,
											void (*handler)(int))
{
	if (signo == SIGALRM)
		fprintf(stderr, "SIGALRM  handler setted\n");

	struct sigaction act, oact;
	
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo != SIGALRM)
		act.sa_flags |= SA_RESTART;
	
	return sigaction(signo,  &act, &oact) == 0;
}

bool setExtendedSignalHandler(int signo,
															void (*handler)(int, siginfo_t*, void*))
{
	if (signo == SIGALRM)
		fprintf(stderr, "SIGALRM extended handler setted\n");

	struct sigaction act, oact;

	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo != SIGALRM)
		act.sa_flags |= SA_RESTART;
	act.sa_flags |= SA_SIGINFO;
	act.sa_sigaction = handler;
																																								
	return sigaction(signo,  &act, &oact) == 0;
}

}
}
