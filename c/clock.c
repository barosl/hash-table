#include "clock.h"
#include <sys/time.h>
#include <stdio.h>
#ifdef __linux__
#include <unistd.h>
#elif !defined(WITHOUT_SDL)
#include <SDL.h>
#endif

static struct timeval clock_initiated;

void clock_init() {
	if (clock_initiated.tv_sec || clock_initiated.tv_usec) return;

	gettimeofday(&clock_initiated, NULL);

	clock_initiated.tv_sec--;
}

int clock_msec() {
	if (!clock_initiated.tv_sec && !clock_initiated.tv_usec) clock_init();

	struct timeval cur;
	gettimeofday(&cur, NULL);
	return (cur.tv_sec - clock_initiated.tv_sec)*1000 + (cur.tv_usec - clock_initiated.tv_usec)/1000;
}

void clock_msleep(unsigned int msec) {
#ifdef __linux__
	usleep(1000*msec);
#elif !defined(WITHOUT_SDL)
	SDL_Delay(msec);
#else
#error invalid platform
#endif
}
