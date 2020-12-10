#include "swap.h"
#include "page.h"
#include "frame.h"
#include "lib/kernel/bitmap.h"
#include "devices/block.h"
#include "threads/synch.h"

struct bitmap* swap_bitmap;
struct block* swap_block;
struct lock* swap_lock;

void swap_init(){
    lock_init(&swap_lock);

    swap_block = block_get_role(BLOCK_SWAP);
    if(!swap_block)
        return;

    swap_bitmap = bitmap_create(block_size(swap_block)/8);
    if(!swap_bitmap)
        return;
    
    bitmap_set_all(swap_bitmap, 0);
}

void swap_in(int used_index, void* kaddr){
    lock_acquire(&swap_lock);
    if(!bitmap_test(swap_bitmap, used_index))
        return;
    
    for(int i = 0; i < 8; i++)
		block_read(swap_block, used_index * 8 + i, (int *)kaddr + i * 8);
    
    bitmap_set(swap_bitmap, used_index, false);
    lock_release(&swap_lock);
}

int swap_out(void* kaddr){
    lock_acquire(&swap_lock);
    
    int idx = bitmap_scan_and_flip(swap_bitmap, 0, 1, 0);
    for(int i = 0; i < 8; i++)
		block_write(swap_block, idx * 8 + i, (int *)kaddr + i * 8);

    lock_release(&swap_lock);
    return idx;
}
