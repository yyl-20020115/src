#include "cstl/inc/c_lib.h"
#include "cstl/inc/c_map.h"
#include "cstl/inc/c_list.h"
#include "lobject.h"

static struct cstl_list* list = 0;

static struct cstl_map* map = 0;

void luaC_map_destroy_key(void* key) {
    //
}
void luaC_map_destroy_value(void* value) {
    //
}
int luaC_map_compare(const void* a, const void* b)
{
    return (int)(((char*)a) - ((char*)b));
}
void luaC_ensure_rc() {
    if (map == 0) {
        map = cstl_map_new(luaC_map_compare, luaC_map_destroy_key, luaC_map_destroy_value);
    }
}


long long luaC_addref(GCObject* o)
{
    luaC_ensure_rc();
    long long c = 0LL;
    
    if (cstl_map_exists(map, o)) {
        c = (long long)cstl_map_find(map, o);
        c++;
        //add one to the ref
        cstl_map_replace(map, o, (void*)c,sizeof(c));
    }
    else {
        c++;
        cstl_map_insert(map, o, sizeof(void*), (void*)c, sizeof(long long));
    }
    return c;
}
long long luaC_relref(GCObject* o)
{
    long long c = 0LL;
    luaC_ensure_rc();
    if (cstl_map_exists(map, o)) {
        c = (long long)cstl_map_find(map, o);
        c--;
        if (c <= 0) {
            //put into remove list
        }
        else {
            //sub one to the ref
            cstl_map_replace(map, o,(void*) c, sizeof(c));
        }
    }
    else {
        cstl_map_insert(map, o, sizeof(void*), 0LL, sizeof(long long));
    }
    return c;
}
