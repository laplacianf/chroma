#ifndef __MAP_H__
#define __MAP_H__

#define MAP_SIZE 127

struct KeyValuePair {
    char* key;
    void* value;
};

struct Map {
    struct Vector** buckets;
};

struct Map* map();
void set(struct Map* map, char* key, void* value);
void* get(struct Map* map, char* key);

#endif
