#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vector.h"

#include "map.h"

struct Map* map() {
    struct Map* newmap = malloc(sizeof(struct Map));
    newmap->buckets = malloc(MAP_SIZE * sizeof(struct Vector*));

    for (int i = 0; i < MAP_SIZE; i++) {
        newmap->buckets[i] = vector();
    }

    return newmap;
}

static int hash(char* value) {
    int hash = 5381;
    int c;

    while (c = *value++) {
        hash = (hash * 33 + c) % MAP_SIZE; 
    }

    return hash;
}

void set(struct Map* map, char* key, void* value) {
    int keyhash = hash(key);
    int keyexist = 0;
    
    for (int i = 0; i <= map->buckets[keyhash]->last; i++) {
        struct KeyValuePair* kvpair = (struct KeyValuePair*)map->buckets[keyhash]->body[i];
        if (!strcmp(kvpair->key, key)) {
            kvpair->value = value;
            keyexist = 1;
            break;
        }
    }

    if (!keyexist) {
        struct KeyValuePair* newkvpair = malloc(sizeof(struct KeyValuePair));
        newkvpair->key = key;
        newkvpair->value = value;

        push(map->buckets[keyhash], newkvpair);
    }
}

void* get(struct Map* map, char* key) {
    int keyhash = hash(key);
    
    for (int i = 0; i <= map->buckets[keyhash]->last; i++) {
        struct KeyValuePair* kvpair = (struct KeyValuePair*)map->buckets[keyhash]->body[i];
        if (!strcmp(kvpair->key, key)) {
            return kvpair->value;
        }
    }

    return NULL;
}
