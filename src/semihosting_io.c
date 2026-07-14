/* src/semihosting_io.c
 *
 * ARM 세미호스팅을 통해 printf()가 QEMU 콘솔로 출력되게 하는 최소 구현.
 * newlib(-specs=nosys.specs)의 기본 _write()는 항상 -1을 반환하는 빈
 * 스텁이라 여기서 강한 심볼로 재정의해 덮어씀.
 *
 * SYS_WRITEC(0x03): 세미호스팅 콜로 문자 1개 출력. 프레임 단위로 매번
 * BKPT 명령을 트랩하니 느리지만, 디버그 로그 용도로는 충분함.
 */

#include <sys/stat.h>
#include <errno.h>

#define SYS_WRITEC   0x03

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

/* printf가 요구하는 나머지 syscall 스텁들 (nosys.specs 기본값과 동일하게 */
int _close(int fd) { (void) fd; return -1; }
int _fstat(int fd, struct stat *st) { (void) fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd) { (void) fd; return 1; }
int _lseek(int fd, int offset, int whence) { (void) fd; (void) offset; (void) whence; return 0; }
int _read(int fd, char *buf, int len) { (void) fd; (void) buf; (void) len; return 0; }
