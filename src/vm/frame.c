#include <stdio.h>
#include "frame.h"
#include "swap.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "filesys/file.h"

void lru_list_init(){
    list_init(&lru_list);
    lock_init(&lru_list_lock);
    lru_clock = NULL;
}

void add_page_to_lru_list(struct page* page){
    if(page){     //@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        lock_acquire(&lru_list_lock);

        list_push_back(&lru_list, &page->lru_elem);

        lock_release(&lru_list_lock);
    }
}

void del_page_from_lru_list(struct page* page){
    if(page){     //@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        if(lru_clock == page){
            struct list_elem* temp = list_remove(&page->lru_elem);
            lru_clock = list_entry(temp, struct page, lru_elem);
        }
        else{
            list_remove(&page->lru_elem);
        }
    }
}

struct page* alloc_page(enum palloc_flags flags){
    if(flags!=PAL_USER)
        return NULL;

    struct page* rtn = malloc(sizeof(struct page));
    if(rtn == NULL){
        return NULL;
    }

    void *kaddr = palloc_get_page(PAL_USER);
    while(true){
        if(!kaddr){
            try_to_free_pages(flags);
            kaddr = palloc_get_page(PAL_USER);
        }
        else{
            break;
        }
    }
    
    rtn->kaddr = kaddr;
    rtn->pg_thread = thread_current();

    add_page_to_lru_list(rtn)
    return rtn;
}

void free_page(void* kaddr){
    lock_acquire(&lru_list_lock);
    for(struct list_elem* elem = list_begin(&lru_list);
            elem != list_end(&lru_list); elem = list_next(elem)){
        if(list_entry(elem, struct page, lru_elem)->kaddr == kaddr){
            __free_page(list_entry(elem, struct page, lru_elem));
            break;
        }
    }
    lock_release(&lru_list_lock);
}

void __free_page(struct page* page){
    del_page_from_lru_list(page);
    palloc_free_page(page->kaddr);
    free(page);
}

struct list_elem* get_next_lru_clock(){
    if(!lru_clock){
        if(list_begin(&lru_list) == list_end(&lru_list)){
            return NULL;
        }
        else{
            lru_clock = list_entry(list_begin(&lru_list), struct page, lru_elem);
            return list_begin(&lru_list);
        }
    }

    if(list_next(&lru_clock->lru_elem) == list_end(&lru_list)){
        if(&lru_clock->lru_elem == list_begin(&lru_list)){
            return NULL;
        }
        else{
            lru_clock = list_entry(list_begin(&lru_list), struct page, lru_elem);
            return list_begin(&lru_list);
        }
    }
    lru_clock = list_entry(list_next(&lru_clock->lru_elem), struct page, lru_elem);
    return list_next(&lru_clock->lru_elem);
}

void try_to_free_pages(enum palloc_flags flags){
    if(list_empty(&lru_list)){
        return;
    }

    lock_acquire(&lru_list_lock);
    struct list_elem* elem;
    struct page* temp;
    int* dir;

    while(true){
        elem =get_next_lru_clock();

        if(!elem){
            lock_release(&lru_list_lock);
            return;
        }
        temp = list_entry(elem, struct page, lru_elem);
        if(temp->vme->addi){
            continue;
        }
        dir = temp->pg_thread->pagedir;

        if(pagedir_is_accessed(dir, temp->vme->vaddr)){
            pagedir_set_accessed(dir, temp->vme->vaddr, false);
            continue;
        }

        if(pagedir_is_dirty(dir, temp->vme->vaddr) || temp->vme->type == VM_ANON){
            if(temp->vme->type==VM_FILE){
                file_write_at(temp->vme->file, temp->kaddr,
                temp->vme->read_bytes, temp->vme->offset);
            }
            else{
                temp->vme->swap_slot=swap_out(temp->kaddr);
                temp->vme->type = VM_ANON;
            }
        }
        temp->vme->is_loaded=false;
        pagedir_clear_page(dir, temp->vme->vaddr);
        __free_page(temp);
        break;
    }
    lock_release(&lru_list_lock);
}