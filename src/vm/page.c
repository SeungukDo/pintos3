#include "vm/page.h"
#include <string.h>

static unsigned vm_hash_func(const struct hash_elem *e, void *aux);
static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b);

void vm_init(struct hash *vm)
{
    hash_init(vm, vm_hash_func, vm_less_func, NULL);
}

static unsigned vm_hash_func(const struct hash_elem *e, void *aux)
{
    struct vm_entry *entry = hash_entry(e, struct vm_entry, elem);
    return hash_int((int)entry->vaddr);
}

static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b)
{
    struct vm_entry *a_entry = hash_entry(a, struct vm_entry, elem);
    struct vm_entry *b_entry = hash_entry(b, struct vm_entry, elem);

    if (pg_no(b_entry->vaddr) > pg_no(a_entry->vaddr))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool insert_vme(struct hash *vm, struct vm_entry *vme)
{
    if (hash_insert(vm, &vme->elem) == NULL)
        return false;
    return true;
}

bool delete_vme(struct hash *vm, struct vm_entry *vme)
{
    if (hash_delete(vm, &vme->elem) == NULL)
        return false;
    return true;
}

struct vm_entry *find_vme(void *vaddr)
{
    struct vm_entry vme;
    struct hash_elem *e;
    struct thread *t = thread_current();
    vme.vaddr = (void *)(pg_round_down(vaddr));

    e = hash_find(&(t->vm), &vme.elem);
    if (e == NULL)
    {
        printf("find_vme not found: %x\n", pg_round_down(vaddr));
        return NULL;
    }
    else
    {
        return hash_entry(e, struct vm_entry, elem);
    }
}

void vm_destroy(struct hash *vm)
{
    hash_destroy(vm, NULL); //hash action function 추가 예정
}
/*
void page_destructor (struct hash_elem *e, void *aux UNUSED)
{
  struct vm_entry *p = hash_entry (e, struct vm_entry, elem);
  if (p->is_loaded == true) {
    palloc_free_page(p);
    pagedir_clear_page();
    p->is_loaded = false;
    free_frame (f);
  }
}
*/

bool load_file(void *kaddr, struct vm_entry *vme)
{
    //printf("load file %x\n", vme->vaddr);
    file_seek(vme->file, vme->offset);
    if (file_read(vme->file, kaddr, vme->read_bytes) != vme->read_bytes)
    {
        return false;
    }
    memset(kaddr + vme->read_bytes, 0, vme->zero_bytes);

    return true;
}
