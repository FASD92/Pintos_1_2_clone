#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "filesys/file.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);


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
	uint64_t arg3 =	f->R.r10;
	uint64_t arg4 = f->R.r8;
	uint64_t arg5 = f->R.r9;
	switch (syscall_num) {
		case SYS_EXIT:
		syscall_exit((int) arg0);
		break;
		case SYS_WRITE:
		f->R.rax = syscall_write((int) arg0,(void *) arg1, (unsigned) arg2);
		break;
  	}
	// printf ("system call!\n");
	// thread_exit ();
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

