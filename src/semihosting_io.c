/* src/semihosting_io.c */
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"

#define SYS_WRITEC   0x03

// main.c에 선언할 뮤텍스 핸들 참조
extern SemaphoreHandle_t xPrintfMutex;

static inline int semihosting_call(int reason, void *arg)
{
    register int r0 __asm__("r0") = reason;
    register void *r1 __asm__("r1") = arg;
    __asm__ volatile ("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");
    return r0;
}

int _write(int fd, const char *buf, int len)
{
    (void) fd;
    for (int i = 0; i < len; i++) {
        char c = buf[i];
        semihosting_call(SYS_WRITEC, &c);
    }
    return len;
}

/* printf 링커 에러 방지용 필수 더미 스텁들 */
int _close(int fd) { (void) fd; return -1; }
int _fstat(int fd, struct stat *st) { (void) fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd) { (void) fd; return 1; }
int _lseek(int fd, int offset, int whence) { (void) fd; (void) offset; (void) whence; return 0; }
int _read(int fd, char *buf, int len) { (void) fd; (void) buf; (void) len; return 0; }
