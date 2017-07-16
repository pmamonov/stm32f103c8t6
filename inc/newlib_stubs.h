#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

/*
#define STDIN_BUFFER_SIZE 1024
#define STDOUT_BUFFER_SIZE 1024

#ifdef __NEWLIB_STUBS
volatile uint8_t stdin_buffer[STDIN_BUFFER_SIZE];
volatile uint16_t stdin_buffer_in=0;
volatile uint16_t stdin_buffer_len=0;
volatile uint16_t stdin_buffer_out=0;
volatile uint8_t stdout_buffer[STDOUT_BUFFER_SIZE];
volatile uint16_t stdout_buffer_in=0;
volatile uint16_t stdout_buffer_len=0;
volatile uint16_t stdout_buffer_out=0;
#else
extern volatile uint8_t stdin_buffer[];
extern volatile uint16_t stdin_buffer_in;
extern volatile uint16_t stdin_buffer_out;
extern volatile uint16_t stdin_buffer_len;
extern volatile uint8_t stdout_buffer[];
extern volatile uint16_t stdout_buffer_in;
extern volatile uint16_t stdout_buffer_out;
extern volatile uint16_t stdout_buffer_len;
#endif

int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);
*/
