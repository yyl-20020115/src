#include "cstl/inc/c_lib.h"
#include "cstl/inc/c_map.h"
#include "cstl/inc/c_set.h"

#include "cstl/inc/c_array.h"
#include "lobject.h"
#include "lstate.h"
#include "ltable.h"

int luaC_process_unlink_object(lua_State* L, GCObject* o, struct cstl_array* dup_array, struct cstl_set* collecting);
int luaC_process_unlink_upval(lua_State* L, UpVal* uv, struct cstl_array* dup_array, struct cstl_set* collecting);

extern void freeobj(lua_State* L, GCObject* o);

static struct cstl_map* map = 0;

void luaC_set_destroy(void* value) {
    //
}
int luaC_set_compare(const void* a, const void* b)
{
    return (int)(((char*)a) - ((char*)b));
}

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

void luaC_ensure_init_rc() {
    if (map == 0)
    {
        map = cstl_map_new(luaC_map_compare, luaC_map_destroy_key, luaC_map_destroy_value);
    }
}


int luaC_relref_internal(lua_State* L, GCObject* o)
{
    int c = 0;
    luaC_ensure_init_rc();
    if (cstl_map_exists(map, o)) {
        c = (int)cstl_map_find(map, o);
        c = c >= 0 ? c : 0;
        if (c == 0) {
            //put into remove list
            cstl_map_remove(map, (void*)o);
        }
        else {
            c--;
            //sub one to the ref
            cstl_map_replace(map, o, (void*)c, sizeof(c));
        }
    }
    return c;
}
void luaC_process_unlink_gc(lua_State* L, GCObject* gc, struct cstl_array* dup_array, struct cstl_set* collecting) {
    if (gc != 0 && 0 == luaC_relref_internal(L, gc))
    {
        if (!cstl_set_exists(collecting, (void*)gc)) {
            cstl_array_push_back(dup_array, (void*)gc, sizeof(gc));
        }
    }
}

int luaC_process_unlink_string(lua_State* L, TString* s, struct cstl_array* dup_array, struct cstl_set* collecting) {
    int mc = 0;
    if (s != 0 && CSTL_ERROR_SUCCESS == cstl_set_insert(collecting, (void*)s, sizeof(s))) {
        mc = 1;
    }
    return mc;
}
int luaC_process_unlink_udata(lua_State* L, Udata* u, struct cstl_array* dup_array, struct cstl_set* collecting) {
    int mc = 0;
    if (u != 0 && CSTL_ERROR_SUCCESS == cstl_set_insert(collecting, (void*)u, sizeof(u))) {
        mc = 1;
        for (int i = 0; i < u->nuvalue; i++) {
            UValue* pv = (u->uv + i);
            if (luaC_should_do_rc(pv->uv.tt_)) {
                pv->uv.tt_ = LUA_TNIL;
                luaC_process_unlink_gc(L, (GCObject*)pv, dup_array, collecting);
            }
            else {
                //not rc, just set to 0
                pv->uv.value_.p = 0;
            }
        }
    }
    return mc;

}
extern unsigned int luaH_realasize(const Table* t);

int luaC_process_unlink_table(lua_State* L, Table* t, struct cstl_array* dup_array, struct cstl_set* collecting) {
    int mc = 0;
    if (t != 0 && CSTL_ERROR_SUCCESS == cstl_set_insert(collecting, (void*)t, sizeof(t))) {
        mc = 1;
        
        Node* limit = gnodelast(t);

        for (Node* n = gnode(t, 0); n < limit; n++) {
            if (luaC_should_do_rc(n->u.key_tt)) {
                n->u.key_tt = LUA_TNIL;
                luaC_process_unlink_gc(L, n->u.key_val.gc, dup_array, collecting);
            }
            if (luaC_should_do_rc(n->i_val.tt_)) {
                n->i_val.tt_ = LUA_TNIL;
                luaC_process_unlink_gc(L, n->i_val.value_.gc, dup_array, collecting);
            }
        }

        int array_size = luaH_realasize(t);
        for (int i = 0; i < array_size; i++) {
            TValue* tv = t->array + i;
            if (luaC_should_do_rc(tv->tt_)) {
                tv->tt_ = LUA_TNIL;
                luaC_process_unlink_gc(L, tv->value_.gc, dup_array, collecting);
            }
        }
    }
    return mc;

}
int luaC_process_unlink_thread(lua_State* L, lua_State* t, struct cstl_array* dup_array, struct cstl_set* collecting) {
    int mc = 0;
    if (t != 0 && CSTL_ERROR_SUCCESS == cstl_set_insert(collecting, (void*)t, sizeof(t))) {
        mc = 1;
        //stack
        for (int i = 0; i < t->stacksize; i++) {
            StkId sid = t->stack + i;
            if (luaC_should_do_rc(sid->val.tt_)) {
                //unlink stack members!
                luaC_process_unlink_gc(L, sid->val.value_.gc, dup_array, collecting);
            }
        }
        //upval
        if (t->openupval != 0) {
            for (UpVal* uv = t->openupval; uv != 0; uv = uv->u.open.next)
            {
                mc += luaC_process_unlink_upval(L, uv, dup_array, collecting);
            }
            t->openupval = 0;
        }
        
    }
    return mc;

}
int luaC_process_unlink_proto(lua_State* L, Proto* p, struct cstl_array* dup_array, struct cstl_set* collecting) {
    int mc = 0;
    if (p != 0 && CSTL_ERROR_SUCCESS == cstl_set_insert(collecting, (void*)p, sizeof(p))) {
        mc = 1;
        if (p->p != 0) {
            for (int i = 0; i < p->sizep; i++) {
                luaC_process_unlink_gc(L, (GCObject*)p->p[i], dup_array, collecting);
            }
        }
        if (p->k != 0) {
            for (int i = 0; i < p->sizek; i++) {
                TValue* tv = (p->k + i);
                if (tv != 0 && luaC_should_do_rc(tv->tt_)) {
                    tv->tt_ = LUA_TNIL;
                    luaC_process_unlink_gc(L, tv->value_.gc, dup_array, collecting);
                }
            }
        }
        if (p->source != 0) {
            luaC_process_unlink_gc(L, (GCObject*)p->source, dup_array, collecting);
        }
    }
    return mc;

}

int luaC_process_unlink_upval(lua_State* L, UpVal* uv, struct cstl_array* dup_array, struct cstl_set* collecting) {
    int mc = 0;
    if (uv != 0 && CSTL_ERROR_SUCCESS == cstl_set_insert(collecting, (void*)uv, sizeof(uv))) {
        mc = 1;
        //if uv->tbc(to-be-closed=open) value is on stack:uv->v
        //if closed, value is uv->u.value;
        TValue* v = uv->tbc ? uv->v : &uv->u.value;

        if (luaC_should_do_rc(v->tt_)) {
            v->tt_ = LUA_TNIL;
            luaC_process_unlink_gc(L, v->value_.gc, dup_array, collecting);
        }
    }
    return mc;
}
int luaC_process_unlink_lcl(lua_State* L, LClosure* lcl, struct cstl_array* dup_array, struct cstl_set* collecting) 
{
    int mc = 0;
    if (lcl != 0 && CSTL_ERROR_SUCCESS == cstl_set_insert(collecting, (void*)lcl, sizeof(lcl)))
    {
        mc = 1;
        if (lcl->p != 0) {
            luaC_process_unlink_gc(L, (GCObject*)lcl->p, dup_array, collecting);
        }
        for (int i = 0; i < lcl->nupvalues; i++) {
            UpVal* uv = lcl->upvals[i];
            if (luaC_should_do_rc(uv->tt)) {
                uv->tt = LUA_TNIL;
                luaC_process_unlink_gc(L, uv->v->value_.gc, dup_array, collecting);
            }
        }
    }

    return mc;

}
int luaC_process_unlink_ccl(lua_State* L, CClosure* ccl, struct cstl_array* dup_array, struct cstl_set* collecting)
{
    int mc = 0;
    if (ccl != 0 && CSTL_ERROR_SUCCESS == cstl_set_insert(collecting, (void*)ccl, sizeof(ccl)))
    {
        mc = 1;
        for (int i = 0; i < ccl->nupvalues;i++) {
            TValue* tv = &ccl->upvalue[i];
            if (luaC_should_do_rc(tv->tt_)) {
                luaC_process_unlink_gc(L, tv->value_.gc, dup_array, collecting);
            }
        }
    }

    return mc;
}

int luaC_process_unlink_object(lua_State* L, GCObject* o, struct cstl_array* dup_array, struct cstl_set* collecting)
{
    int mc = 0;

    if (o != 0 && luaC_should_do_rc(o->tt))
    {
        switch (o->tt & 0x0f) {
        case LUA_TSTRING:
        {
            //free string object directly
            mc += luaC_process_unlink_string(L, (TString*)o, dup_array, collecting);
        }
        break;
        case LUA_TUSERDATA:
        {
            //free user data in depth
            mc += luaC_process_unlink_udata(L, (Udata*)o, dup_array, collecting);
        }
        break;
        case LUA_TUPVAL:
        {
            //free upvalue in depth
            mc += luaC_process_unlink_upval(L, (UpVal*)o, dup_array, collecting);
        }
        break;
        case LUA_TTABLE:
        {
            //free table
            mc += luaC_process_unlink_table(L, (Table*)o, dup_array, collecting);
        }
        break;
        case LUA_TTHREAD:
        {
            //free thread
            mc += luaC_process_unlink_thread(L, (lua_State*)o, dup_array, collecting);
        }
        break;
        case LUA_TPROTO:
        {
            //free proto
            mc += luaC_process_unlink_proto(L, (Proto*)o, dup_array, collecting);
        }
        break;
        case LUA_TFUNCTION:
        {
            //lua closure
            if (o->tt == LUA_VLCL)
            {
                mc += luaC_process_unlink_lcl(L, (LClosure*)o, dup_array, collecting);
            }
            //C closure
            else if (o->tt == LUA_VCCL)
            {
                mc += luaC_process_unlink_ccl(L, (CClosure*)o, dup_array, collecting);
            }
        }
        break;
        }
    }
    return mc;
}

int luaC_process_unlink_step(lua_State* L, struct cstl_iterator* i,struct cstl_array* dup_array, struct cstl_set* collecting)
{
    return i != 0 ? luaC_process_unlink_object(L, (GCObject*)(i->current_element), dup_array, collecting) : 0 ;
}

int luaC_process_unlink(lua_State* L, GCObject* m, struct cstl_set* collecting)
{
    int nc = 0;
    //when we are here: o.rc == 0
    if (m != 0) 
    {
        struct cstl_array* rcs_array = cstl_array_new(16, 0, 0);
        if (rcs_array != 0)
        {
            cstl_array_push_back(rcs_array, m, sizeof(m));
            
            struct cstl_array* dup_array = cstl_array_new(cstl_array_size(rcs_array), 0, 0);
            
            if (dup_array != 0) 
            {
                do
                {
                    int mc = 0;
                    struct cstl_iterator* i = cstl_array_new_iterator(rcs_array);                
                    if (i == 0) break;

                    while (i->next(i) != 0)
                    {
                        mc += luaC_process_unlink_step(L, i, dup_array, collecting);
                    }
                    cstl_array_delete_iterator(i);
                    cstl_array_delete(rcs_array);
                    i = 0;
                    rcs_array = 0;
                    
                    if (cstl_array_size(dup_array) == 0) {
                        cstl_array_delete(dup_array);
                        dup_array = 0;
                    }
                    else {
                        nc += mc;
                        rcs_array = dup_array;
                        dup_array = cstl_array_new(16, 0, 0);
                    }
                } while (dup_array !=0);
            }
        }
    }
    return nc;
}

int luaC_addref(lua_State* L, GCObject* o)
{
    luaC_ensure_init_rc();
    int c = 0;
    
    if (cstl_map_exists(map, o)) {
        c = (int)cstl_map_find(map, o);
        c++;
        //add one to the ref
        cstl_map_replace(map, o, (void*)c,sizeof(c));
    }
    else 
    {
        c++;
        cstl_map_insert(map, o, sizeof(void*), (void*)c, sizeof(c));
    }
    return c;
}
int luaC_collect(lua_State* L, struct cstl_set* collecting) {

    if (collecting != 0) {
        struct cstl_iterator* i = cstl_set_new_iterator(collecting);
        if (i != 0) {
            while (i->next(i) != 0) {
                GCObject* o = (GCObject*)(i->current_key(i));
                if (o != 0) {
                    freeobj(L, o);
                }
            }
        }
        cstl_set_delete_iterator(i);
    }
    return 0;
}
int luaC_relref(lua_State* L, GCObject* o)
{
    int c = 0;
    if (o != 0)
    {
        c = luaC_relref_internal(L, o);

        if (c == 0) 
        {
            struct cstl_set* collecting = cstl_set_new(luaC_set_compare, luaC_set_destroy);
            
            if (collecting != 0) 
            {
                luaC_process_unlink(L, o, collecting);
                //collect everything that has reference count of 0
                luaC_collect(L, collecting);

                cstl_set_delete(collecting);
            }
        }
    }
    return c;
}
void luaC_ensure_deinit_rc() {
    if (map != 0)
    {
        struct cstl_iterator* i = cstl_map_new_iterator(map);
        if (i != 0) {
            while (i->next(i) != 0) {
                GCObject* o = (GCObject*)i->current_key(i);
                if (o != 0) {
                    freeobj(0, o);
                }
            }
            cstl_map_delete_iterator(i);
        }
        cstl_map_delete(map);
        map = 0;
    }
}
