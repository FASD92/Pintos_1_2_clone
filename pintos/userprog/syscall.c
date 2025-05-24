#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "threads/init.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "filesys/file.h"
#include "userprog/process.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);
static int64_t get_user (const uint8_t *uaddr);
static bool put_user (uint8_t *udst, uint8_t byte);
bool is_valid_user_ptr(const void *uaddr);



/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	int syscall_num = f->R.rax; //인자 다 받아오고.
	uint64_t arg0 = f->R.rdi;
	uint64_t arg1 = f->R.rsi;
	uint64_t arg2 = f->R.rdx;
	uint64_t arg3 =	f->R.r10;	/* 네번째 인자가 r10이네...*/
	uint64_t arg4 = f->R.r8;
	uint64_t arg5 = f->R.r9;
	switch (syscall_num) {
		case SYS_HALT:
		syscall_halt();
		break;

		case SYS_EXIT:
		syscall_exit((int) arg0);
		break;

		case SYS_WAIT:
		f->R.rax = process_wait((tid_t) arg0);
		break;

		case SYS_WRITE:
		f->R.rax = syscall_write((int) arg0,(void *) arg1, (unsigned) arg2);
		break;
  	}
	// printf ("system call!\n");
	// thread_exit ();
}
void syscall_halt(void) {
	power_off();
}

int syscall_exit(int status){
	struct thread *cur = thread_current(); //프로세스의 커널 스레드.
    cur->exit_status = status; // 부모에게 전달할 종료 상태
    //process_exit();            // 종료 처리
    thread_exit(); 
}

int syscall_write(int fd,void * buffer, unsigned size){
	//fd1 -> stdout ->  FDT -> innode table->dev/tty에 출력
	if (fd == 1) {  // STDOUT
        putbuf(buffer, size);
        return size;
    }
	return -1;
}

/* 사용자 가상 주소 uaddr에서 1바이트 읽기 (UADDR은 KERN_BASE 미만)
 * 성공 시 바이트 값, 세그폴트 시 -1 반환
 * Reads a byte at user virtual address UADDR.
 * UADDR must be below KERN_BASE.
 * Returns the byte value if successful, -1 if a segfault
 * occurred. */
static int64_t
get_user (const uint8_t *uaddr) {
    int64_t result;
    __asm __volatile (
    "movabsq $done_get, %0\n"
    "movzbq %1, %0\n"
    "done_get:\n"
    : "=&a" (result) : "m" (*uaddr));
    return result;
}

/* 사용자 주소 udst에 BYTE 쓰기 (udst는 KERN_BASE 미만)
 * 성공 시 true, 세그폴트 시 false 반환
 * Writes BYTE to user address UDST.
 * UDST must be below KERN_BASE.
 * Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte) {
    int64_t error_code;
    __asm __volatile (
    "movabsq $done_put, %0\n"
    "movb %b2, %1\n"
    "done_put:\n"
    : "=&a" (error_code), "=m" (*udst) : "q" (byte));
    return error_code != -1;
}

bool is_valid_user_ptr(const void *uaddr) {
	if (uaddr == NULL || !is_user_vaddr(uaddr)) {
		return false;
	}
	if (get_user((const uint8_t *)uaddr) == -1){
		return false;
	}
	return true;
}