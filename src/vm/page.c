#include "vm/page.h"

static unsigned vm_hash_func(const struct hash_elem *e, void *aux);
static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b);

void vm_init(struct hash *vm)
{
    hash_init(vm, vm_hash_func, vm_less_func, NULL);
}

static unsigned vm_hash_func(const struct hash_elem *e, void *aux)
{
    struct vm_entry *entry = hash_entry(e, struct vm_entry, elem);
    return hash_int(entry->vaddr);
}

static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b)
{
    struct vm_entry *a_entry = hash_entry(a, struct vm_entry, elem);
    struct vm_entry *b_entry = hash_entry(b, struct vm_entry, elem);

    return b_entry->vaddr > a_entry->vaddr;
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

    vme.vaddr = (void *)(pg_no(vaddr) << PGBITS);

    e = hash_find(&(thread_current()->vm), &vme.elem);

    if (e == NULL)
        return NULL;
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