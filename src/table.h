#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

/* a variable is essentially a string, right? */
typedef struct {
    obj_string* key;
    Val value;
    /* key/value pair */
} entry;

/* define the hash table */
typedef struct {
    int count;
    int capacity;
    entry *entries;
} table;

void init_table(table *tab);
void free_table(table *tab);
/* add to the table */
bool get_table(table *tab, obj_string *key, Val *value);
bool set_table(table *tab, obj_string *key, Val value);
bool delete_table(table *tab, obj_string *key);
void copy_table(table *from, table *to); //needed for inheritance support
obj_string *table_find(table *tab, const char *chars, int length, uint32_t hash);
#endif

