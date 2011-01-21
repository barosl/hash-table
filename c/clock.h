#ifndef CLOCK_H
#define CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

extern void clock_init(void);
extern int clock_msec(void);
extern void clock_msleep(unsigned int msec);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CLOCK_H */
