#include "cstl/inc/c_lib.h"
#include "cstl/inc/c_map.h"
#include "cstl/inc/c_list.h"
#include "lobject.h"

extern void freeobj(lua_State* L, GCObject* o);

void luaC_list_destroy(void* value) {
    //
}
int luaC_list_compare(const void* a, const void* b)
{
    return (int)(((char*)a) - ((char*)b));
}

void luaC_process_unlink(lua_State* L, GCObject* o)
{
    //here o.rc == 0

    struct cstl_list* list = cstl_list_new(luaC_list_destroy, luaC_list_compare);

    if (o != 0) {
        switch (o->tt & 0x0f) {
        case LUA_TLIGHTUSERDATA:
            break;
        case LUA_TSTRING:
            freeobj(L, o);
            break;
        case LUA_TTABLE:
            break;
        case LUA_TUSERDATA:
            break;
        case LUA_TTHREAD:
            break;
        case LUA_TFUNCTION:
            break;
        case LUA_TUPVAL:
            break;
        case LUA_TPROTO:
            break;
        }



    }

}

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


int luaC_addref(lua_State* L, GCObject* o)
{
    luaC_ensure_rc();
    int c = 0;
    
    if (cstl_map_exists(map, o)) {
        c = (int)cstl_map_find(map, o);
        c++;
        //add one to the ref
        cstl_map_replace(map, o, (void*)c,sizeof(c));
    }
    else {
        c++;
        cstl_map_insert(map, o, sizeof(void*), (void*)c, sizeof(c));
    }
    return c;
}
int luaC_relref(lua_State* L, GCObject* o)
{
    int c = 0;
    luaC_ensure_rc();
    if (cstl_map_exists(map, o)) {
        c = (int)cstl_map_find(map, o);
        c--;
        if (c <= 0) {
            //put into remove list
            cstl_map_remove(map, (void*)o);
            luaC_process_unlink(L, o);
        }
        else {
            //sub one to the ref
            cstl_map_replace(map, o,(void*) c, sizeof(c));
        }
    }
    return c;
}
