#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include "common.h"

typedef enum data_type_e {
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_INT_8,
    TYPE_INT_16,
    TYPE_INT_32,
    TYPE_INT_64,   
    TYPE_CHAR,
    TYPE_STRING
} data_type_t;


typedef struct node_s {
    char *key;
    void *value;
    data_type_t type;
    node_t *next;
    node_t *prev;
} node_t;


typedef struct dict_s {
    size_t size, used;
    node_t **table;
} dict_t;


dict_t *dict_create(void);
void dict_delete(dict_t *dict);
void dict_insert(dict_t *dict, const char *key, void *value, data_type_t type);
node_t *dict_search(dict_t *dict, const char *key);
void dict_remove_entry(dict_t *dict, const char *key);
void dict_update_entry(dict_t *dict, const char* key, void *value, data_type_t type);

#endif