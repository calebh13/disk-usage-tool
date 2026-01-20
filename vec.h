#include <stdlib.h>

#define DECL_VECTOR(T, name) \
    typedef struct { \
        T* data; \
        size_t len; \
        size_t cap; \
        void (*free_elem)(T*); /* optional destructor */ \
    } name; \
    \
    static inline void name##_init(name* v, void (*free_elem)(T*)) { \
        v->data = NULL; \
        v->len = 0; \
        v->cap = 0; \
        v->free_elem = free_elem; \
    } \
    \
    static inline int name##_push(name* v, T val) { \
        if (v->len == v->cap) { \
            size_t new_cap = v->cap ? v->cap * 2 : 4; \
            T* tmp = realloc(v->data, new_cap * sizeof(T)); \
            if (!tmp) { \
                return -1; \
            } \
            v->data = tmp; \
            v->cap = new_cap; \
        } \
        v->data[v->len++] = val; \
        return 0; \
    } \
    \
    static inline T* name##_get(name* v, size_t i) { \
        if (i >= v->cap) return NULL; \
        return &v->data[i]; \
    } \
    \
    static inline void name##_free(name* v) { \
        if (v->free_elem) { \
            for (size_t i = 0; i < v->len; i++) \
                v->free_elem(&v->data[i]); \
        } \
        free(v->data); \
        v->data = NULL; \
        v->len = v->cap = 0; \
    } \
    \
    static inline int name##_pop(name* v) { \
        if (v->len > 0) { \
            v->len--; \
            if (v->free_elem) { \
                v->free_elem(&v->data[v->len]); \
            } \
        } else return -1; \
        return 0; \
    }
