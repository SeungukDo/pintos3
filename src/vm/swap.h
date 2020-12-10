#ifndef SWAP_H
#define SWAP_H

void swap_init();
void swap_in(int used_index, void* kaddr);
int swap_out(void* kaddr);
#endif
