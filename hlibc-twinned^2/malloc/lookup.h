/*
 * Copyright (C) 2016  SRI International
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __SRI_GLIBC_LOOKUP_H__
#define __SRI_GLIBC_LOOKUP_H__


#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>


/* Compile-time constants.  (once lived in arena.c) */

#define HEAP_MIN_SIZE (32 * 1024)
#ifndef HEAP_MAX_SIZE
# ifdef DEFAULT_MMAP_THRESHOLD_MAX
#  define HEAP_MAX_SIZE (2 * DEFAULT_MMAP_THRESHOLD_MAX)
# else
#  define HEAP_MAX_SIZE (1024 * 1024) /* must be a power of two */
# endif
#endif

/*
 * HEAP_MIN_SIZE and HEAP_MAX_SIZE limit the size of mmap()ed heaps
 *  that are dynamically created for multi-threaded programs.  The
 *  maximum size must be a power of two, for fast determination of which
 *  heap belongs to a chunk.  It should be much larger than the mmap
 *  threshold, so that requests with a size just below that threshold
 *  can be fulfilled without creating too many heaps.
 */



extern void lookup_init(void);

extern void lookup_delete(void);

/*
  Returns true if it figures out the arena the ptr belongs to,
  and places that value in it's second argument.

  0: means the main_arena and the ptr is mmapped
  1: means the main_arena and the ptr is sbrked (or 
  more accurately cleaved from top).

  N: means the ptr belongs to arena N where N > 1

  Returns false otherwise.

*/
extern bool lookup_arena_index(void* ptr, size_t* arena_indexp);

extern bool lookup_set_sbrk_lo(void* ptr);
extern bool lookup_incr_sbrk_hi(size_t incr);
extern bool lookup_decr_sbrk_hi(size_t incr);
extern bool lookup_add_sbrk_region(void* lo, void* hi);
extern bool lookup_add_heap(void* ptr, size_t index);
extern bool lookup_delete_heap(void* ptr);

extern bool lookup_add_mmap(void* ptr, size_t sz);
extern bool lookup_delete_mmap(void* ptr);

extern void lookup_dump(FILE*, bool dumptables);

#endif