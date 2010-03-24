#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

static sig_atomic_t _libx52_inited = 0;
static sig_atomic_t _init_called = 0;

static void
_handle_signal(int signal, siginfo_t* info, void* context)
{
	switch (signal)
	{
		case SIGHUP:
		case SIGTERM:
		case SIGINT:
			exit(1);
		default:
			exit(2);
	}
}

static void
_add_signal(int s)
{
	struct sigaction action = {};
	sigset_t to_block;

	sigemptyset(&to_block);
	sigaddset(&to_block, SIGINT);
	sigaddset(&to_block, SIGTERM);
	sigaddset(&to_block, SIGHUP);

	sigemptyset(&action.sa_mask);
	action.sa_sigaction = _handle_signal;
	action.sa_mask = to_block;
	action.sa_flags = SA_SIGINFO;
	sigaction(s, &action, 0);
}

static void
_setup_signalhandlers(void)
{
	_add_signal(SIGINT);
	_add_signal(SIGTERM);
	_add_signal(SIGHUP);
}

static void _cleanup(void)
{

}

 __attribute__((constructor)) static void
 _init_libx52(void)
{
	_setup_signalhandlers();
	atexit(_cleanup);
	_libx52_inited = 1;
}

void
init_ctor(void(*fn)(void))
{
	if (_init_called) {
		abort();
	}
	_init_called = 1;

	if (!_libx52_inited) {
		abort();
	}
	if (fn) atexit(fn);
}
