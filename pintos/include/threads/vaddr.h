#ifndef THREADS_VADDR_H
#define THREADS_VADDR_H

#include <debug.h>
#include <stdint.h>
#include <stdbool.h>

#include "threads/loader.h"

/* Functions and macros for working with virtual addresses.
 *
 * See pte.h for functions and macros specifically for x86
 * hardware page tables. */

#define BITMASK(SHIFT, CNT) (((1ul << (CNT)) - 1) << (SHIFT))

/* Page offset (bits 0:12). */
#define PGSHIFT 0                          /* Index of first offset bit. */
#define PGBITS  12                         /* Number of offset bits. */
#define PGSIZE  (1 << PGBITS)              /* Bytes in a page. */
#define PGMASK  BITMASK(PGSHIFT, PGBITS)   /* Page offset bits (0:12). */

/* Offset within a page. */
#define pg_ofs(va) ((uint64_t) (va) & PGMASK)

#define pg_no(va) ((uint64_t) (va) >> PGBITS)

/* Round up to nearest page boundary. */
#define pg_round_up(va) ((void *) (((uint64_t) (va) + PGSIZE - 1) & ~PGMASK))

/* Round down to nearest page boundary. */
#define pg_round_down(va) (void *) ((uint64_t) (va) & ~PGMASK)

/* Kernel virtual address start */
#define KERN_BASE LOADER_KERN_BASE

/* 유저 스택의 시작 주소
	User stack start */
#define USER_STACK 0x47480000

/* VADDR이 유저 가상 주소면 true를 반환
	Returns true if VADDR is a user virtual address. */
#define is_user_vaddr(vaddr) (!is_kernel_vaddr((vaddr)))

/* VADDR이 커널 가상 주소면 true를 반환
	Returns true if VADDR is a kernel virtual address. */
#define is_kernel_vaddr(vaddr) ((uint64_t)(vaddr) >= KERN_BASE)

// FIXME: add checking
/* 물리 주소 PADDR이 매핑된 커널 가상 주소를 반환합니다.
	Returns kernel virtual address at which physical address PADDR is mapped. */
#define ptov(paddr) ((void *) (((uint64_t) paddr) + KERN_BASE))

/* 커널 가상 주소 VADDR가 매핑된 물리 주소를 반환합니다.
	Returns physical address at which kernel virtual address VADDR is mapped. */
#define vtop(vaddr) \
({ \
	ASSERT(is_kernel_vaddr(vaddr)); \
	((uint64_t) (vaddr) - (uint64_t) KERN_BASE);\
})

#endif /* threads/vaddr.h */
