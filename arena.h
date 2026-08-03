#ifndef ARENA_
#define ARENA_


#if defined(__clang__)
#define COMPILER_CLANG 1
#define COMPILER_MSVC 0
#define COMPILER_GCC 0

#elif defined(_MSC_VER)
#define COMPILER_CLANG 0
#define COMPILER_MSVC 1
#define COMPILER_GCC 0

#elif defined(__GNUC__)
#define COMPILER_CLANG 0
#define COMPILER_MSVC 0
#define COMPILER_GCC 1

#else
#define COMPILER_CLANG 0
#define COMPILER_MSVC 0
#define COMPILER_GCC 0
#error "Unknown compiler!"

#endif

#if defined(__cplusplus)
#define LANG_CPP 1
#define LANG_C 0
#else
#define LANG_CPP 0
#define LANG_C 1
#endif


#if defined(_WIN32)
#define OS_WINDOWS 1
#else
#define OS_WINDOWS 0
#error "Currently only for Windows!"
#endif

#if OS_WINDOWS
#define NOGDI
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#define global static


// to suppress unused function warning

#if COMPILER_CLANG || COMPILER_GCC
#define internal static __attribute__((unused))
#elif COMPILER_MSVC
#define internal __pragma(warning(suppress : 4505)) static
#else
#define internal static
#endif

#if COMPILER_MSVC
#define force_inline __forceinline
#elif COMPILER_CLANG || COMPILER_GCC
#define force_inline __attribute__((always_inline))
#else
#define force_inline
#endif

#if COMPILER_MSVC
#define no_inline __declspec(noinline)
#elif COMPILER_CLANG || COMPILER_GCC
#define no_inline __attribute__((noinline))
#else
#define no_inline
#endif


/* Units */
#define KB(n) (((u64)(n)) << 10)
#define MB(n) (((u64)(n)) << 20)
#define GB(n) (((u64)(n)) << 30)
#define TB(n) (((u64)(n)) << 40)

/* pow2 math */
#define is_pow2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))
#define is_pow2_or_zero(x) (((x) & ((x) - 1)) == 0)

#define align_up_pow2(x, p) (((x) + ((p) - 1)) & ~((p) - 1))
#define align_down_pow2(x, p) ((x) & ~((p) - 1))

/* Clamp */
#define clamp_top(val, high) (((val) < (high)) ? (val) : (high))
#define clamp_bottom(val, low) (((val) > (low)) ? (val) : (low))
#define clamp(val, low, high) (clamp_bottom(low, clamp_top(val, high)))

/* Alignment */
#if COMPILER_MSVC
#define align_of(T) __alignof(T)
#elif COMPILER_CLANG
#define align_of(T) __alignof(T)
#elif COMPILER_GCC
#define align_of(T) __alignof__(T)
#else
#error align_of not defined for this compiler.
#endif

#if COMPILER_MSVC
#define align_to(x) __declspec(align(x))
#elif COMPILER_CLANG || COMPILER_GCC
#define align_to(x) __attribute__((aligned(x)))
#else
#error align_to not defined for this compiler.
#endif

/* Scope Wrappers */
#define scope(it, begin, end) for ((it) = ((begin), 1); (it); (it) = 0, (end)) /* `it` is used internaly. */

#define OPTIONS(T, ...)                                                                                                                                   \
	typedef struct T {                                                                                                                                    \
		__VA_ARGS__                                                                                                                                       \
	} T


/* LinkList */
#define dll_push_back_np(head, tail, node, next, prev)                                                                                                    \
	((head) == 0 ? ((head) = (tail) = (node), (node)->next = (node)->prev = 0)                                                                            \
				 : ((node)->prev = (tail), (tail)->next = (node), (tail) = (node), (node)->next = 0))

#define dll_remove_np(head, tail, node, next, prev)                                                                                                       \
	((head) == (node)                                                                                                                                     \
		 ? ((head) == (tail) ? ((head) = (tail) = 0) : ((head) = (head)->next, (head)->prev = 0))                                                         \
		 : ((tail) == (node) ? ((tail) = (tail)->prev, (tail)->next = 0) : ((node)->next->prev = (node)->prev, (node)->prev->next = (node)->next)))

#define sll_push_front_n(head, tail, node, next) ((node)->next = (head), ((head) == 0 ? (tail) = (node) : 0), (head) = (node))

#define sll_push_back_n(head, tail, node, next) ((node)->next = 0, ((head) == 0 ? ((head) = (node)) : ((tail)->next = (node))), (tail) = (node))

#define sll_pop_front_n(head, tail, next) ((head) == (tail) ? ((head) = (tail) = 0) : ((head) = (head)->next))

#define dll_push_front_np(head, tail, node, next, prev) dll_push_back_np(tail, head, node, prev, next)

#define dll_push_back(head, tail, node) dll_push_back_np(head, tail, node, next, prev)
#define dll_push_front(head, tail, node) dll_push_front_np(head, tail, node, next, prev)
#define dll_remove(head, tail, node) dll_remove_np(head, tail, node, next, prev)

#define sll_push_front(head, tail, node) sll_push_front_n(head, tail, node, next)
#define sll_push_back(head, tail, node) sll_push_back_n(head, tail, node, next)
#define sll_pop_front(head, tail) sll_pop_front_n(head, tail, next)
#define sll_stack_push(head, tail, node) sll_push_front(head, tail, node)
#define sll_stack_pop(head, tail) sll_pop_front(head, tail)
#define sll_queue_push(head, tail, node) sll_push_back(head, tail, node)
#define sll_queue_pop(head, tail) sll_pop_front(head, tail)


/*  Base Type  */
typedef signed char i8;
typedef short		i16;
typedef int			i32;
typedef long long	i64;

typedef unsigned char	   u8;
typedef unsigned short	   u16;
typedef unsigned int	   u32;
typedef unsigned long long u64;

typedef float  f32;
typedef double f64;


#include <stdio.h>


/* ======================================================================= */
/*                              decelaration                               */
/* ======================================================================= */
#ifdef LANG_CPP
extern "C" {
#endif
/* .... structs .... */

typedef struct ArenaTempNode {
	struct ArenaTempNode* next;
	u64					  start_cursor;
} ArenaTempNode;

typedef struct Arena {
	u64			   cursor;
	u64			   committed;
	u64			   reserved;
	ArenaTempNode* temp_stack_tail;
	ArenaTempNode* temp_stack_head;
} Arena;

#define ARENA_HEADER_SIZE align_up_pow2(sizeof(Arena), 64)

/* .... global var .... */
global u64 global_page_size;


/* .... function .... */

OPTIONS(ArenaOpt, u64 commit_size;);
internal Arena* _arena_alloc(u64 reserve_size, ArenaOpt opt);
internal void*	arena_push(Arena* arena, u64 size, u64 alignment);
internal void	arena_release(Arena* arena);
internal void	arena_reset(Arena* arena);

#define arena_alloc(reserve_size, ...) _arena_alloc(reserve_size, (ArenaOpt){__VA_ARGS__})

#define arena_push_type(arena, T) arena_push(arena, sizeof(T), align_of(T))
#define arena_push_array(arena, T, count) arena_push(arena, (count * sizeof(T)), align_of(T))

internal void arena_temp_begin(Arena* arena);
internal void arena_temp_end(Arena* arena);
internal void arena_temp_end_all(Arena* arena);

#ifdef LANG_CPP
}
#endif

/* ======================================================================= */
/*                             implementation                              */
/* ======================================================================= */


#ifdef ARENA_IMPLEMENTATION

#if OS_WINDOWS

internal Arena* _arena_alloc(u64 reserve_size, ArenaOpt opt) {
#if LANG_C
	SYSTEM_INFO sys_info = {0};
#else
	SYSTEM_INFO sys_info = {};
#endif
	Arena* arena;
	void*  memory;
	u32	   error_code;
	if (global_page_size == 0) {
		GetSystemInfo(&sys_info);
		global_page_size = sys_info.dwPageSize;
	}

	reserve_size	= align_up_pow2(reserve_size, global_page_size);
	// by default commit just one page
	opt.commit_size = (opt.commit_size) ? align_up_pow2(opt.commit_size, global_page_size) : global_page_size;

	if (opt.commit_size > reserve_size) {
		opt.commit_size = reserve_size;
	}

	memory = VirtualAlloc(0, reserve_size, MEM_RESERVE, PAGE_READWRITE);
	if (!memory) {
		error_code = GetLastError();
		fprintf(stderr, "ERROR: VirtualAlloc Failed While Reserving: 0x%X\n", error_code);
		return NULL;
	}

	if (!VirtualAlloc(memory, opt.commit_size, MEM_COMMIT, PAGE_READWRITE)) {
		error_code = GetLastError();
		fprintf(stderr, "ERROR: VirtualAlloc Failed While Committing: 0x%X\n", error_code);
		VirtualFree(memory, 0, MEM_RELEASE);
		return NULL;
	}

	arena = (Arena*)memory;

	arena->cursor		   = ARENA_HEADER_SIZE;
	arena->committed	   = opt.commit_size;
	arena->reserved		   = reserve_size;
	arena->temp_stack_tail = 0;
	arena->temp_stack_head = 0;

	return arena;
}

internal void* arena_push(Arena* arena, u64 size, u64 alignment) {
	u64	  begin;
	u64	  end;
	u64	  error_code;
	void* user_ptr;

	if (!arena) {
		fprintf(stderr, "ERROR: Arena is NULL\n");
		return NULL;
	}

	begin = align_up_pow2(arena->cursor, alignment);
	end	  = begin + size;

	if (end > arena->reserved) {
		fprintf(stderr,
				"ERROR: Arena overflow:\n"
				"           Reserved: %lld\n"
				"           Required: %lld\n",
				arena->reserved, end);
		return NULL;
	}
	if (end > arena->committed) {
		u64 required_size = end - arena->committed;
		required_size	  = align_up_pow2(required_size, global_page_size);

		if (!VirtualAlloc((u8*)arena + arena->committed, required_size, MEM_COMMIT, PAGE_READWRITE)) {
			error_code = GetLastError();
			fprintf(stderr, "ERROR: VirtualAlloc Failed during arena_push: 0x%llX\n", error_code);
			return NULL;
		}
		arena->committed += required_size;
	}

	arena->cursor = end;
	user_ptr	  = (u8*)arena + begin;
	return user_ptr;
}

internal void arena_reset(Arena* arena) {
	if (!arena) {
		fprintf(stderr, "ERROR: Arena is NULL\n");
		return;
	}

	arena->cursor		   = ARENA_HEADER_SIZE;
	arena->temp_stack_head = 0;
	arena->temp_stack_tail = 0;
}

internal void arena_release(Arena* arena) {
	u32 error_code;
	if (!arena) {
		fprintf(stderr, "ERROR: Arena is NULL\n");
		return;
	}

	if (!VirtualFree(arena, 0, MEM_RELEASE)) {
		error_code = GetLastError();
		fprintf(stderr, "ERROR: VirtualFree Failed: 0x%X\n", error_code);
	}
}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ */

internal void arena_temp_begin(Arena* arena) {
	ArenaTempNode* node;
	u64			   cursor;

	if (!arena) {
		fprintf(stderr, "ERROR: Arena is NULL\n");
		return;
	}
	cursor = arena->cursor;

	node = (ArenaTempNode*)arena_push_type(arena, ArenaTempNode);
	if (!node) {
		fprintf(stderr, "ERROR: arena_temp_begin failed\n");
		return;
	}

	node->start_cursor = cursor;

	sll_stack_push(arena->temp_stack_head, arena->temp_stack_tail, node);
}

internal void arena_temp_end(Arena* arena) {
	if (!arena) {
		fprintf(stderr, "ERROR: Arena is NULL\n");
		return;
	}

	if (arena->temp_stack_head) {
		arena->cursor = arena->temp_stack_head->start_cursor;
		sll_stack_pop(arena->temp_stack_head, arena->temp_stack_tail);
	} else {
		fprintf(stderr, "NOTE: arena_temp_end: temp stack was empty\n");
	}
}

internal void arena_temp_end_all(Arena* arena) {
	if (!arena) {
		fprintf(stderr, "ERROR: Arena is NULL\n");
		return;
	}

	if (arena->temp_stack_tail) {
		arena->cursor = arena->temp_stack_tail->start_cursor;
	} else {
		fprintf(stderr, "NOTE: arena_temp_end_all: temp stack was empty\n");
	}

	arena->temp_stack_tail = 0;
	arena->temp_stack_head = 0;
}

#endif

#endif
#endif
