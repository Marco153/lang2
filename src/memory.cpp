#pragma once
#include "memory.h"
#include <xmmintrin.h>

mem_chunk* heap_get_unallocated_cache(mem_alloc* alloc);
void heap_insert_unallocated(mem_alloc* alloc, mem_chunk* a);
mem_chunk *GetUnallocatedChunk(mem_alloc *alloc)
{
	mem_chunk* ret = heap_get_unallocated_cache(alloc);

	if (ret)
	{
		heap_insert_unallocated(alloc, ret + 1);
		heap_insert_unallocated(alloc, ret - 1);
		return ret;
	}
	if (!ret)
	{
		for (int i = 0; alloc->chunks_cap; i += 4)
		{


			mem_chunk* cur0 = &alloc->all[i];
			mem_chunk* cur1 = &alloc->all[i + 1];
			mem_chunk* cur2 = &alloc->all[i + 2];
			mem_chunk* cur3 = &alloc->all[i + 3];

			int f1 = IS_FLAG_OFF(cur0->flags, CHUNK_ALLOCATED);
			int f2 = IS_FLAG_OFF(cur1->flags, CHUNK_ALLOCATED);
			int f3 = IS_FLAG_OFF(cur2->flags, CHUNK_ALLOCATED);
			int f4 = IS_FLAG_OFF(cur3->flags, CHUNK_ALLOCATED);

			// Prefetching the next cache lines
			_mm_prefetch((char*)&alloc->all[(i + 4) % alloc->chunks_cap], _MM_HINT_T0);

			if (IS_FLAG_OFF(cur0->flags, CHUNK_ALLOCATED)) {
				ret = cur0;
				heap_insert_unallocated(alloc, &alloc->all[(i + 1) % alloc->chunks_cap]);
				break;
			}
			if (IS_FLAG_OFF(cur1->flags, CHUNK_ALLOCATED)) {
				ret = cur1;
				heap_insert_unallocated(alloc, &alloc->all[(i + 2) % alloc->chunks_cap]);
				break;
			}
			if (IS_FLAG_OFF(cur2->flags, CHUNK_ALLOCATED)) {
				ret = cur2;
				heap_insert_unallocated(alloc, &alloc->all[(i + 3) % alloc->chunks_cap]);
				break;
			}
			if (IS_FLAG_OFF(cur3->flags, CHUNK_ALLOCATED)) {
				ret = cur3;
				heap_insert_unallocated(alloc, &alloc->all[(i + 4) % alloc->chunks_cap]);
				break;
			}
			/*
			mem_chunk* cur = &alloc->all[i];
			_mm_prefetch((char*)&alloc->all[(i + 1) % alloc->chunks_cap], _MM_HINT_T0);

			if (IS_FLAG_OFF(cur->flags, CHUNK_ALLOCATED))
			{
				ret = cur;
				alloc->probable_unallocated = &alloc->all[(i + 1) % alloc->chunks_cap];
				break;
			}
			*/
		}
	}

	ret->flags |= CHUNK_ALLOCATED;

    ASSERT(ret);
	ret->next = 0;
	ret->prev = 0;
    return ret;
}
void heap_assert_next_not_equal_to_cur(mem_alloc* alloc)
{
    auto cur = alloc->head_free;

	auto last = cur;
	while (cur)
	{
		ASSERT(cur != cur->next)
		ASSERT(cur->flags == 4)
		cur = cur->next;

		if (cur)
		{
			ASSERT(cur->prev == last);
			last = cur;
		}

	}
	ASSERT(last->flags == 4)
	//printf("ON ASSERTING HEAP LOOP: \n last node size is was %d, ptr %p\n", last->size, last);
}
struct heap_free_ret_info
{
	bool contiguous_with_cur;
	bool contiguous_with_cur_next;
};
void maybe_join_to_right(mem_chunk *prev, mem_chunk *cur, mem_chunk *next, heap_free_ret_info *hinfo)
{
	bool contiguous_with_cur_next = (cur->addr + cur->size * BYTES_PER_CHUNK) == next->addr;
	if (contiguous_with_cur_next)
	{
		cur->size += next->size;
		auto next_aux = next;

		next->flags &= ~CHUNK_ALLOCATED;

		cur->next = next->next;
		cur->prev = prev;
		if(next->next)
			next->next->prev = cur;
		if(prev)
			prev->next = cur;
		if (hinfo)
			hinfo->contiguous_with_cur_next = true;
	}
}
void heap_clear(mem_alloc* alloc)
{
	alloc->in_use.Clear();
	int total_size = alloc->chunks_cap * BYTES_PER_CHUNK;
	memset(alloc->all, 0, total_size);

    mem_chunk *free = GetUnallocatedChunk(alloc);

	free->size = total_size / BYTES_PER_CHUNK;
    free->addr = alloc->buffer;

	alloc->head_free = free;
}

mem_chunk *heap_get_unallocated_cache(mem_alloc* alloc)
{
	for (int i = 0; i < UNALLOCATED_BUFFER_ITEMS; i++)
	{
		auto cur = (mem_chunk **) & alloc->probable_unallocated[i];
		if (*cur)
		{
			mem_chunk* ret = *cur;
			(*cur)->flags |= CHUNK_ALLOCATED;
			*cur = nullptr;
			ret->next = nullptr;
			ret->prev = nullptr;
			return ret;
		}
	}
	return nullptr;
}
void heap_insert_unallocated(mem_alloc* alloc, mem_chunk *a)
{
	if (a < alloc->all || a > (alloc->all + alloc->chunks_cap) || IS_FLAG_ON(a->flags, CHUNK_ALLOCATED))
		return;

	for (int i = 0; i < UNALLOCATED_BUFFER_ITEMS; i++)
	{
		auto cur = (mem_chunk **) & alloc->probable_unallocated[i];
		if (*cur == nullptr)
		{
			*cur = a;
			break;
		}
	}
}
void heap_free(mem_alloc *alloc, char *ptr)//, heap_free_ret_info *ret = nullptr)
{
	heap_free_ret_info* ret = nullptr;
	//free(ptr);
	//return;
	//if(ret)
		//*ret = {};
    mem_chunk *chunk_to_free = (mem_chunk *)alloc->in_use.Get(ptr);
    alloc->in_use.Remove(ptr);
    auto cur = alloc->head_free;
	memset(ptr, 0xcc, 1);
	
	
	// in case the chunk to free is in a higher address want
	// we want to find in front of which chunk are we in
	// and increment its size
	if (chunk_to_free->addr > cur->addr)
	{
		while (cur->next && cur->next->addr < chunk_to_free->addr)
		{
			cur = cur->next;
		}

		ASSERT(cur && cur->next->addr >= chunk_to_free->addr);
		
		//TODO: create a var put these two vals in each bit, and create a swich statetement so that we 
		// wont need if and elses
		bool contiguous_with_cur = (cur->addr + cur->size * BYTES_PER_CHUNK) == chunk_to_free->addr;
		bool contiguous_with_cur_next = (chunk_to_free->addr + chunk_to_free->size * BYTES_PER_CHUNK) == cur->next->addr;

		if (ret)
		{
			ret->contiguous_with_cur = contiguous_with_cur;
			ret->contiguous_with_cur_next = contiguous_with_cur_next;
		}

		// to join with cur, the chunk_free and cur needs to be contiguous
		if (contiguous_with_cur && !contiguous_with_cur_next)
		{
			cur->size += chunk_to_free->size;
			chunk_to_free->flags &= ~CHUNK_ALLOCATED;
			heap_insert_unallocated(alloc, chunk_to_free);
		}
		// we migth also want also to join with the cur->next
		else if (contiguous_with_cur_next && !contiguous_with_cur)
		{
			maybe_join_to_right(cur, chunk_to_free, cur->next, ret);
			/*
			chunk_to_free->size += cur->next->size;
			auto next_aux = cur->next;

			cur->next->flags &= ~CHUNK_ALLOCATED;

			chunk_to_free->next = cur->next->next;
			chunk_to_free->prev = cur;
			cur->next = chunk_to_free;

			//ASSERT(cur->next != chunk_to_free)
			*/


		}
		// if we're contiguous with both
		else if (contiguous_with_cur_next && contiguous_with_cur)
		{
			cur->size += chunk_to_free->size + cur->next->size;

			cur->next->flags &= ~CHUNK_ALLOCATED;
			chunk_to_free->flags &= ~CHUNK_ALLOCATED;
			heap_insert_unallocated(alloc, cur->next);
			heap_insert_unallocated(alloc, chunk_to_free);



			if (cur->next->next)
			{
				auto next_next = cur->next->next;
				cur->next = cur->next->next;
				next_next->prev = cur;
			}
			else
				cur->next = nullptr;

		}
		// if we're not contiguos with anything
		else if(!contiguous_with_cur_next && !contiguous_with_cur)
		{
			auto next_aux = cur->next;

			cur->next = chunk_to_free;
			chunk_to_free->prev = cur;
			chunk_to_free->next = next_aux;
			next_aux->prev = chunk_to_free;
			//chunk_to_free->flags &= ~CHUNK_IN_USE;
		}
	}
	else
	{
		chunk_to_free->prev = 0;
		// making the chunk to be the head
		chunk_to_free->next = cur;
		cur->prev = chunk_to_free;
		alloc->head_free = chunk_to_free;

		maybe_join_to_right(nullptr, chunk_to_free, cur, ret);
	}
	heap_assert_next_not_equal_to_cur(alloc);
	//memset(chunk_to_free->addr, 'x', chunk_to_free->size * BYTES_PER_CHUNK);

}
char *heap_alloc(mem_alloc *alloc, int size)//, mem_chunk **out = nullptr)
{
	if (size == 0)
		return nullptr;
	//return (char *)malloc(size);
    mem_chunk *cur = alloc->head_free;

    while(cur && (cur->size * BYTES_PER_CHUNK) < size)
    {
        cur = cur->next;
    }
    ASSERT(cur && (cur->size * BYTES_PER_CHUNK) >= size);

	bool got_all_bytes_from_chunk = ((cur->size * BYTES_PER_CHUNK) - size) == 0;
	
	int min_alloc_size = size;

	if(min_alloc_size  == 0)
		min_alloc_size = 1 * BYTES_PER_CHUNK;

	if (got_all_bytes_from_chunk)
	{
		if (cur->prev)
			cur->prev->next = cur->next;
		if (cur->next)
			cur->next->prev = cur->prev;

		if (alloc->head_free == cur)
			alloc->head_free = cur->next;
		//cur->flags &= ~CHUNK_ALLOCATED;
		alloc->head_free = cur->next;
	}
	else
	{
		// fragmenting the original chunk
		mem_chunk *other_chunk = GetUnallocatedChunk(alloc);
		ASSERT(other_chunk != cur)
		

		int align_to_bytes_per_chunk = (min_alloc_size % BYTES_PER_CHUNK);
		// if we're not aligned yet
		if (align_to_bytes_per_chunk != 0)
			align_to_bytes_per_chunk = BYTES_PER_CHUNK - align_to_bytes_per_chunk;

		min_alloc_size += align_to_bytes_per_chunk;

		other_chunk->addr = cur->addr + min_alloc_size;
		other_chunk->size = cur->size - min_alloc_size / BYTES_PER_CHUNK;
		if (cur->prev)
		{
			auto prev = cur->prev;
			prev->next = other_chunk;
			other_chunk->prev = prev;

		}
		if (cur->next)
		{
			auto next = cur->next;
			next->prev = other_chunk;
			other_chunk->next = next;
		}

		if(cur == alloc->head_free)
			alloc->head_free = other_chunk;
	}


    //cur->flags |= CHUNK_IN_USE;
	alloc->in_use.Store(cur->addr, cur);
	cur->size = min_alloc_size / BYTES_PER_CHUNK;

	ASSERT(cur->next != cur);
	//if (out)
		//*out = cur;
	heap_assert_next_not_equal_to_cur(alloc);
    return cur->addr;
}

void FreeMemAlloc(mem_alloc* alloc)
{
	//VirtualFree(alloc->main_buffer, 0, MEM_RELEASE);
}
unsigned char *PlatformGetMem(unsigned int sz, int flags)
{
	printf("size is %d\n", sz);
#ifdef LINUX 
	return (unsigned char *)mmap(0, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
	return (unsigned char *)VirtualAlloc(0, sz, MEM_COMMIT, PAGE_READWRITE);
	
#endif
}

void InitMemAlloc(mem_alloc *alloc)
{
    long long all_chunks_sz = alloc->chunks_cap * sizeof(mem_chunk);
    long long chunks_total_size = alloc->chunks_cap * BYTES_PER_CHUNK;
	long long in_use_hash_sz = alloc->in_use.hash_table_size * sizeof(heap_hash::inner);
	long long unallocated_sz = UNALLOCATED_BUFFER_ITEMS * 8;

	long long total_size = all_chunks_sz + chunks_total_size + in_use_hash_sz + unallocated_sz;

	char* start = (char *)PlatformGetMem(total_size, 0);
//	char* start = (char *)VirtualAlloc(0, total_size, MEM_COMMIT, PAGE_READWRITE);
	char* buffer = start;
	memset(start, 0, total_size);

	alloc->main_buffer = start;


	alloc->all = (mem_chunk*)buffer;

	buffer += all_chunks_sz;

	alloc->probable_unallocated = (mem_chunk**)(buffer);

	buffer += unallocated_sz;

	alloc->buffer = (char*)buffer;
	buffer += chunks_total_size;
	
	alloc->in_use.data = (heap_hash::inner*)buffer;

	buffer += in_use_hash_sz;

    mem_chunk *free = GetUnallocatedChunk(alloc);
	//alloc->in_use.reserve(HASH_TABLE_SIZE);

	free->size = chunks_total_size / BYTES_PER_CHUNK;
    free->addr = alloc->buffer;


	alloc->head_free = free;
}
