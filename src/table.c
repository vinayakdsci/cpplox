#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

/* grow the table when 75 percent full */
#define MAX_LOAD 0.75

void init_table(table *tab) {
    //Initialise the init values for the hash table
    tab->capacity = 0;
    tab->count = 0;
    tab->entries = NULL;
}

void free_table(table *tab) {
    FREE_ARRAY(entry, tab->entries, tab->capacity);
    init_table(tab);
}

static entry *find_entry(entry *entries, int capacity, obj_string *key) {
    uint32_t index = key->hash & (capacity - 1);
    /* printf("hash ind = %d\n, capacity = %d\n", index, capacity); */
    /* first time a stone is passed. store it */
    entry *stone = NULL;
    for(;;) {
        entry *ent = &entries[index];  //the variable and its value at that index
        if(ent->key == NULL) {
            if(IS_NIL(ent->value))
                return stone != NULL ? stone : ent;
            else {
                if(stone == NULL)
                    stone = ent;
            }
        }
        else if(key == ent->key) {
            return ent;
        }
        index = (index + 1) % capacity;
    }
}

bool get_table(table *tab, obj_string *key, Val *value) {
    if(tab->count == 0) return false;

    entry *ent = find_entry(tab->entries, tab->capacity, key);
    if(ent->key == NULL) return false; //key does not exist
    *value = ent->value;
    return true;
}

static void adjust_table(table *tab, int capacity) {
    /* In case of a simple reallocate, the entries might 
     * get distributed in different places than their original ones.
     * The best way to deal with that is to rebuild the entire table every time.
     * */
    entry *entries = ALLOCATE(entry, capacity);
    for(int i = 0; i < capacity; ++i) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    tab->count = 0;
    for(int i = 0; i < tab->capacity; i++) {
        entry *ent = &tab->entries[i];
        if(ent->key == NULL) continue;

        entry *fin = find_entry(entries, capacity, ent->key);
        fin->key = ent->key;
        fin->value = ent->value;
        /* found a non-stone entry, increment table count */
        tab->count++;
    }
    //(TODO) document later
    FREE_ARRAY(entry, tab->entries, tab->capacity);
    tab->entries = entries;
    tab->capacity = capacity;
}

bool set_table(table *tab, obj_string *key, Val value) {
    /* allocate the table */
    if(tab->count + 1 > tab->capacity * MAX_LOAD) {
        int capacity = GROW_CAPACITY(tab->capacity);
        adjust_table(tab, capacity);
    }

    entry *ent = find_entry(tab->entries, tab->capacity, key);
    bool new = ent->key == NULL;  //check if key already exists
    /* (TODO) */
    if(new && IS_NIL(ent->value)) tab->count++;

    ent->key = key;
    ent->value = value;
    return new;
}

bool delete_table(table *tab, obj_string *key) {
    if(tab->count == 0) return false;

    entry *ent = find_entry(tab->entries, tab->capacity, key);
    if(ent->key == NULL) return false;

    //place the dummy here
    ent->key = NULL;
    ent->value = BOOL_VAL(true);
    return true;
}

void table_copy(table *from, table *to) {
    for(int i = 0; i < from->capacity; ++i) {
        entry *ent = &from->entries[i];
        if(ent->key != NULL) {
            set_table(to, ent->key, ent->value);
        }
    }
}
/* interned all strings, faster equality operator in string case */
obj_string *table_find(table *tab, const char *chars, int length, uint32_t hash) {
    if(tab->count == 0) return NULL;

    uint32_t index = hash & (tab->capacity - 1);
    for(;;) {
        entry *ent = &tab->entries[index];
        if(ent->key == NULL) {
            if(IS_NIL(ent->value)) return NULL;
        }
        else if(ent->key->length == length &&
                ent->key->hash == hash &&
                (memcmp(ent->key->chars, chars, length) == 0)) {
            return ent->key;
        }
        index = (index + 1) % tab->capacity;
    }
}


