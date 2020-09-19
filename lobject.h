/*
** $Id: lobject.h $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/*
** Extra types for collectable non-values
*/
#define LUA_TUPVAL	LUA_NUMTYPES  /* upvalues */
#define LUA_TPROTO	(LUA_NUMTYPES+1)  /* function prototypes */


/*
** number of all possible types (including LUA_TNONE)
*/
#define LUA_TOTALTYPES		(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* constant)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/

/* add variant bits to a type */
#define makevariant(t,v)	((t) | ((v) << 4))



/*
** Union of all Lua values
*/
typedef union Value {
  struct GCObject *gc;    /* collectable objects */
  void *p;         /* light userdata */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_Number n;    /* float numbers */
} Value;


/*
** Tagged Values. This is the basic representation of values in Lua:
** an actual value plus a tag with its type.
*/

#define TValuefields	Value value_; lu_byte tt_

typedef struct TValue {
  TValuefields;
} TValue;

/*TValue* o; o->value_ */
//#define val_(o)		((o)->value_)
inline Value get_val_(const TValue* o) {
    return o->value_;
}
inline void set_val_n(TValue* o, lua_Number v) {
    o->value_.n = v;
}
inline void set_val_i(TValue* o, lua_Integer v) {
    o->value_.i = v;
}
inline void set_val_f(TValue* o, lua_CFunction v) {
    o->value_.f = v;
}
inline void set_val_p(TValue* o, void* v) {
    o->value_.p = v;
}

inline void set_val_gc(TValue* o, struct GCObject* v) {
    o->value_.gc = v;
}

/*TValue* o; &o->value_ */
//#define valraw(o)	(&val_(o))
inline const Value* get_valraw(const TValue* o) {
    return &o->value_;
}


/*TValue* o; o->tt_: raw type tag of a TValue */
//#define rawtt(o)	((o)->tt_)
inline lu_byte rawtt(const TValue* o) {
    return o->tt_;
}
/* tag with no variants (bits 0-3) */
//#define novariant(t)	((t) & 0x0F)
inline lu_byte novariant(lu_byte t) {
    return (t) & 0x0F;
}
/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
//#define withvariant(t)	((t) & 0x3F)
inline lu_byte withvariant(lu_byte t) {
    return (t) & 0x3F;
}
//#define ttypetag(o)	withvariant(rawtt(o))
inline lu_byte ttypetag(const TValue* o) {
    return withvariant(rawtt(o));
}
/* type of a TValue */
//#define ttype(o)	(novariant(rawtt(o)))
inline lu_byte ttype(const TValue* o) {
    return novariant(rawtt(o));
}

/* Macros to test type */
//#define checktag(o,t)		(rawtt(o) == (t))
inline lu_byte checktag(const TValue* o, lu_byte t) {
    return rawtt(o) == (t);
}
//#define checktype(o,t)		(ttype(o) == (t))
inline lu_byte checktype(const TValue* o, lu_byte t) {
    return ttype(o) == (t);
}



/*
** Any value being manipulated by the program either is non
** collectable, or the collectable object has the right tag
** and it is not dead.
*/
//#define checkliveness(L,obj) \
//	((void)L, lua_longassert(!iscollectable(obj) || \
//		(righttt(obj) && (L == NULL || !isdead(G(L),gcvalue(obj))))))
inline void checkliveness(lua_State* L, const TValue* obj) {
    ((void)L, lua_longassert(!iscollectable(obj) ||
        (righttt(obj) && (L == NULL || !isdead(G(L), gcvalue(obj))))));
}

/* Macros to set values */

/* set a value's tag */
//#define settt_(o,t)	((o)->tt_=(t))
extern int settt_(lua_State* L, const TValue* o, lu_byte t);

/* main macro to copy values (from 'obj1' to 'obj2') */
//#define setobj(L,obj1,obj2) \
//	{ TValue *io1=(obj1); const TValue *io2=(obj2); \
//          io1->value_ = io2->value_; settt_(io1, io2->tt_); \
//	  checkliveness(L,io1); lua_assert(!isnonstrictnil(io1)); }

extern void setobj(lua_State* L, TValue* obj1, const TValue* obj2);
/*
** Different types of assignments, according to source and destination.
** (They are mostly equal now, but may be different in the future.)
*/


/*
** Entries in the Lua stack
*/
typedef union StackValue {
  TValue val;
} StackValue;


/* index to stack elements */
typedef StackValue *StkId;

/* convert a 'StackValue' to a 'TValue' */
//#define s2v(o)	(&(o)->val)
inline TValue* s2v(StkId s) {
    return &(s->val);
}

/* from stack to stack */
//#define setobjs2s(L,o1,o2)	setobj(L,s2v(o1),s2v(o2))
inline void setobjs2s(lua_State* L, StkId o1, StkId o2) {
    setobj(L, s2v(o1), s2v(o2));
}
/* to stack (not from same stack) */
//#define setobj2s(L,o1,o2)	setobj(L,s2v(o1),o2)
inline void setobj2s(lua_State* L, StkId o1, const TValue* o2) {
    setobj(L, s2v(o1), o2);
}

/* from table to same table */
//#define setobjt2t	setobj
inline void setobjt2t(lua_State* L, TValue* obj1, const TValue* obj2) {
    setobj(L, obj1, obj2);
}
/* to new object */
//#define setobj2n	setobj
inline void setobj2n(lua_State* L, TValue* obj1, const TValue* obj2) {
    setobj(L, obj1, obj2);
}
/* to table */
//#define setobj2t	setobj
inline void setobj2t(lua_State* L, TValue* obj1, const TValue* obj2) {
    setobj(L, obj1, obj2);
}


/*
** {==================================================================
** Nil
** ===================================================================
*/

/* Standard nil */
#define LUA_VNIL	makevariant(LUA_TNIL, 0)

/* Empty slot (which might be different from a slot containing nil) */
#define LUA_VEMPTY	makevariant(LUA_TNIL, 1)

/* Value returned for a key not found in a table (absent key) */
#define LUA_VABSTKEY	makevariant(LUA_TNIL, 2)


/* macro to test for (any kind of) nil */
//#define ttisnil(v)		checktype((v), LUA_TNIL)
inline lu_byte ttisnil(const TValue* v) {
    return checktype((v), LUA_TNIL);
}

/* macro to test for a standard nil */
//#define ttisstrictnil(o)	checktag((o), LUA_VNIL)
inline lu_byte ttisstrictnil(const TValue* o) {
    return checktag((o), LUA_TNIL);
}


//#define setnilvalue(obj)  settt_(obj, LUA_VNIL)
inline void setnilvalue(const TValue* obj) {
    settt_(0, obj, LUA_VNIL);
}

//#define isabstkey(v)		checktag((v), LUA_VABSTKEY)
inline lu_byte isabstkey(const TValue* v) {
    return checktag((v), LUA_VABSTKEY);
}

/*
** macro to detect non-standard nils (used only in assertions)
*/
//#define isnonstrictnil(v)	(ttisnil(v) && !ttisstrictnil(v))
inline lu_byte isnonstrictnil(const TValue* v) {
    return (ttisnil(v) && !ttisstrictnil(v));
}

/*
** By default, entries with any kind of nil are considered empty.
** (In any definition, values associated with absent keys must also
** be accepted as empty.)
*/
//#define isempty(v)		ttisnil(v)
inline lu_byte isempty(const TValue* v) {
    return ttisnil(v);
}

/* macro defining a value corresponding to an absent key */
#define ABSTKEYCONSTANT		{NULL}, LUA_VABSTKEY


/* mark an entry as empty */
//#define setempty(v)		settt_(v, LUA_VEMPTY)
inline void setempty(const TValue* v) {
    settt_(0, v, LUA_VEMPTY);
}


/* }================================================================== */


/*
** {==================================================================
** Booleans
** ===================================================================
*/


#define LUA_VFALSE	makevariant(LUA_TBOOLEAN, 0)
#define LUA_VTRUE	makevariant(LUA_TBOOLEAN, 1)

//#define ttisboolean(o)		checktype((o), LUA_TBOOLEAN)
inline lu_byte ttisboolean(const TValue* o) {
    return checktype((o), LUA_TBOOLEAN);
}
//#define ttisfalse(o)		checktag((o), LUA_VFALSE)
inline lu_byte ttisfalse(const TValue* o) {
    return checktype((o), LUA_VFALSE);
}
//#define ttistrue(o)		checktag((o), LUA_VTRUE)
inline lu_byte ttistrue(const TValue* o) {
    return checktype((o), LUA_VTRUE);
}

//#define l_isfalse(o)	(ttisfalse(o) || ttisnil(o))
inline lu_byte l_isfalse(const TValue* o) {
    return (ttisfalse(o) || ttisnil(o));
}

//#define setbfvalue(obj)		settt_(obj, LUA_VFALSE)
inline void setbfvalue(const TValue* obj)
{
    settt_(0, obj, LUA_VFALSE);
}
//#define setbtvalue(obj)		settt_(obj, LUA_VTRUE)
inline void setbtvalue(const TValue* obj)
{
    settt_(0, obj, LUA_VTRUE);
}
/* }================================================================== */
/* Bit mark for collectable types */
#define BIT_ISCOLLECTABLE	(1 << 6)

/* mark a tag as collectable */
//#define ctb(t)			((t) | BIT_ISCOLLECTABLE)
inline lu_byte ctb(lu_byte t) {
    return ((t) | BIT_ISCOLLECTABLE);
}
/*
** {==================================================================
** Threads
** ===================================================================
*/

#define LUA_VTHREAD		makevariant(LUA_TTHREAD, 0)

//#define ttisthread(o)		checktag((o), ctb(LUA_VTHREAD))
inline lu_byte ttisthread(const TValue* o) {
    return checktag((o), ctb(LUA_VTHREAD));
}

//#define thvalue(o)	check_exp(ttisthread(o), gco2th(get_val_(o).gc))
extern struct lua_State* thvalue(const TValue* o);

//#define setthvalue(L,obj,x) \
//  { TValue *io = (obj); lua_State *x_ = (x); \
//    set_val_gc(io,obj2gco(x_)); settt_(io, ctb(LUA_VTHREAD)); \
//    checkliveness(L,io); }

extern void setthvalue(lua_State* L, TValue* obj, lua_State* x);

//#define setthvalue2s(L,o,t)	setthvalue(L,s2v(o),t)
inline void setthvalue2s(lua_State* L, StkId o, lua_State* t)
{
    setthvalue(L, s2v(o), t);
}
/* }================================================================== */


/*
** {==================================================================
** Collectable Objects
** ===================================================================
*/

/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	struct GCObject *next; lu_byte tt; lu_byte marked


/* Common type for all collectable objects */
typedef struct GCObject {
  CommonHeader;
} GCObject;

LUAI_FUNC int luaC_addref(lua_State* L, GCObject* o);
LUAI_FUNC int luaC_relref(lua_State* L, GCObject* o);

//#define iscollectable(o)	(rawtt(o) & BIT_ISCOLLECTABLE)
inline lu_byte iscollectable(const TValue* o) {
    return rawtt(o) & BIT_ISCOLLECTABLE;
}
//#define gcvalue(o)	check_exp(iscollectable(o), get_val_(o).gc)
inline struct GCObject* gcvalue(const TValue* o) {
    return check_exp(iscollectable(o), get_val_(o).gc);
}
//#define gcvalueraw(v)	((v).gc)
inline struct GCObject* gcvalueraw(const Value v){
    return v.gc;
}
//#define setgcovalue(L,obj,x) \
//  { TValue *io = (obj); GCObject *i_g=(x); \
//    set_val_gc(io,i_g); settt_(io, ctb(i_g->tt)); }

inline void setgcovalue(lua_State* L, TValue* obj, GCObject* x)
{
    TValue* io = (obj);
    GCObject* i_g = (x);
    set_val_gc(io, i_g); 
    settt_(L, io, ctb(i_g->tt));
}
/* Macros for internal tests */
/* collectable object has the same tag as the original value */
//#define righttt(obj)		(ttypetag(obj) == gcvalue(obj)->tt)
inline int righttt(const TValue* obj) {
    return (ttypetag(obj) == (gcvalue(obj)->tt));
}

/* }================================================================== */


/*
** {==================================================================
** Numbers
** ===================================================================
*/

/* Variant tags for numbers */
#define LUA_VNUMINT	makevariant(LUA_TNUMBER, 0)  /* integer numbers */
#define LUA_VNUMFLT	makevariant(LUA_TNUMBER, 1)  /* float numbers */

//#define ttisnumber(o)		checktype((o), LUA_TNUMBER)
inline lu_byte ttisnumber(const TValue* o) {
    return checktype((o), LUA_TNUMBER);
}
//#define ttisfloat(o)		checktag((o), LUA_VNUMFLT)
inline lu_byte ttisfloat(const TValue* o) {
    return checktype((o), LUA_VNUMFLT);
}
//#define ttisinteger(o)		checktag((o), LUA_VNUMINT)
inline lu_byte ttisinteger(const TValue* o) {
    return checktype((o), LUA_VNUMINT);
}

//#define fltvalue(o)	check_exp(ttisfloat(o), get_val_(o).n)
inline lua_Number fltvalue(const TValue* o) {
    return check_exp(ttisfloat(o), get_val_(o).n);
}
//#define ivalue(o)	check_exp(ttisinteger(o), get_val_(o).i)
inline lua_Integer ivalue(const TValue* o) {
    return check_exp(ttisinteger(o), get_val_(o).i);
}
//#define nvalue(o)	check_exp(ttisnumber(o), (ttisinteger(o) ? cast_num(ivalue(o)) : fltvalue(o)))
inline lua_Number nvalue(const TValue* o) {
    return check_exp(ttisnumber(o), (ttisinteger(o) ? cast_num(ivalue(o)) : fltvalue(o)));
}

//#define fltvalueraw(v)	((v).n)
inline lua_Number fltvalueraw(Value v)
{
    return v.n;
}
//#define ivalueraw(v)	((v).i)
inline lua_Integer ivalueraw(Value v)
{
    return v.i;
}

//#define setfltvalue(obj,x) \
//  { TValue *io=(obj); set_val_n(io,x); settt_(io, LUA_VNUMFLT); }
inline void setfltvalue(TValue* obj, lua_Number x) {
    TValue* io = (obj); 
    set_val_n(io, x); 
    settt_(0, io, LUA_VNUMFLT);
}
//#define chgfltvalue(obj,x) \
//  { TValue *io=(obj); lua_assert(ttisfloat(io)); set_val_n(io,x); }
inline void chgfltvalue(TValue* obj, lua_Number x)
{
    TValue* io = (obj); 
    lua_assert(ttisfloat(io)); 
    set_val_n(io, x);
}

//#define setivalue(obj,x) \
//  { TValue *io=(obj); set_val_i(io,x); settt_(io, LUA_VNUMINT); }
inline void setivalue(TValue* obj, lua_Integer x)
{
    TValue* io = (obj); 
    set_val_i(io, x); 
    settt_(0,io, LUA_VNUMINT);
}

//#define chgivalue(obj,x) \
//  { TValue *io=(obj); lua_assert(ttisinteger(io)); set_val_i(io,x); }
inline void chgivalue(TValue* obj, lua_Integer x) 
{ 
    TValue* io = (obj); 
    lua_assert(ttisinteger(io)); 
    set_val_i(io, x); 
}

/* }================================================================== */


/*
** {==================================================================
** Strings
** ===================================================================
*/

/* Variant tags for strings */
#define LUA_VSHRSTR	makevariant(LUA_TSTRING, 0)  /* short strings */
#define LUA_VLNGSTR	makevariant(LUA_TSTRING, 1)  /* long strings */

//#define ttisstring(o)		checktype((o), LUA_TSTRING)
inline lu_byte ttisstring(const TValue* o) {
    return checktype((o), LUA_TSTRING);
}
//#define ttisshrstring(o)	checktag((o), ctb(LUA_VSHRSTR))
inline lu_byte ttisshrstring(const TValue* o) {
    return checktype((o), LUA_VSHRSTR);
}
//#define ttislngstring(o)	checktag((o), ctb(LUA_VLNGSTR))
inline lu_byte ttislngstring(const TValue* o) {
    return checktype((o), LUA_VLNGSTR);
}

//#define tsvalueraw(v)	(gco2ts((v).gc))
extern struct TString* tsvalueraw(Value v);

//#define tsvalue(o)	check_exp(ttisstring(o), gco2ts(get_val_(o).gc))
extern struct TString* tsvalue(const TValue* o);

//#define setsvalue(L,obj,x) \
//  { TValue *io = (obj); TString *x_ = (x); \
//    set_val_gc(io,obj2gco(x_)); settt_(io, ctb(x_->tt)); \
//    checkliveness(L,io); }

extern void setsvalue(lua_State* L, TValue* obj, struct TString* x);

/* set a string to the stack */
//#define setsvalue2s(L,o,s)	setsvalue(L,s2v(o),s)
inline void setsvalue2s(lua_State* L, StkId o, struct TString* s)
{
    setsvalue(L, s2v(o), s);
}
/* set a string to a new object */
//#define setsvalue2n	setsvalue
inline void setsvalue2n(lua_State* L, TValue* o, struct TString* s)
{
    setsvalue(L, o, s);
}

/*
** Header for a string value.
*/
typedef struct TString {
  CommonHeader;
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  lu_byte shrlen;  /* length for short strings */
  unsigned int hash;
  union {
    size_t lnglen;  /* length for long strings */
    struct TString *hnext;  /* linked list for hash table */
  } u;
  char contents[1];
} TString;



/*
** Get the actual string (array of bytes) from a 'TString'.
*/
//#define getstr(ts)  ((ts)->contents)
inline char* getstr(const TString* ts) {
    return (char*)ts->contents;
}

/* get the actual string (array of bytes) from a Lua value */
//#define svalue(o)       getstr(tsvalue(o))
inline char* svalue(const TValue* o) {
    return getstr(tsvalue(o));
}
/* get string length from 'TString *s' */
//#define tsslen(s)	((s)->tt == LUA_VSHRSTR ? (s)->shrlen : (s)->u.lnglen)
inline size_t tsslen(const TString* s) {
    return ((s)->tt == LUA_VSHRSTR ? (s)->shrlen : (s)->u.lnglen);
}
/* get string length from 'TValue *o' */
//#define vslen(o)	tsslen(tsvalue(o))
inline size_t vslen(const TValue* o) {
    return tsslen(tsvalue(o));
}
/* }================================================================== */


/*
** {==================================================================
** Userdata
** ===================================================================
*/


/*
** Light userdata should be a variant of userdata, but for compatibility
** reasons they are also different types.
*/
#define LUA_VLIGHTUSERDATA	makevariant(LUA_TLIGHTUSERDATA, 0)

#define LUA_VUSERDATA		makevariant(LUA_TUSERDATA, 0)

//#define ttislightuserdata(o)	checktag((o), LUA_VLIGHTUSERDATA)
inline lu_byte ttislightuserdata(const TValue* o) {
    return checktag((o), LUA_VLIGHTUSERDATA);
}
//#define ttisfulluserdata(o)	checktag((o), ctb(LUA_VUSERDATA))
inline lu_byte ttisfulluserdata(const TValue* o) {
    return checktag((o), ctb(LUA_VUSERDATA));
}
//#define pvalue(o)	check_exp(ttislightuserdata(o), get_val_(o).p)
inline void* pvalue(const TValue* o) {
    return check_exp(ttislightuserdata(o), get_val_(o).p);
}
//#define uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(get_val_(o).gc))
extern struct Udata* uvalue(const TValue* o);

//#define pvalueraw(v)	((v).p)
inline void* pvalueraw(Value v) {
    return v.p;
}
//#define setpvalue(obj,x) \
//  { TValue *io=(obj); set_val_p(io,x); settt_(io, LUA_VLIGHTUSERDATA); }
inline void setpvalue(TValue* obj, void* x) {
    TValue* io = (obj); 
    set_val_p(io, x); 
    settt_(0, io, LUA_VLIGHTUSERDATA);
}

//#define setuvalue(L,obj,x) \
//  { TValue *io = (obj); Udata *x_ = (x); \
//    set_val_gc(io,obj2gco(x_)); settt_(io, ctb(LUA_VUSERDATA)); \
//    checkliveness(L,io); }

extern void setuvalue(struct lua_State* L, TValue* obj, struct Udata* x);

/* Ensures that addresses after this type are always fully aligned. */
typedef union UValue {
  TValue uv;
  LUAI_MAXALIGN;  /* ensures maximum alignment for udata bytes */
} UValue;


/*
** Header for userdata with user values;
** memory area follows the end of this structure.
*/
typedef struct Udata {
  CommonHeader;
  unsigned short nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  struct Table *metatable;
  GCObject *gclist;
  UValue uv[1];  /* user values */
} Udata;


/*
** Header for userdata with no user values. These userdata do not need
** to be gray during GC, and therefore do not need a 'gclist' field.
** To simplify, the code always use 'Udata' for both kinds of userdata,
** making sure it never accesses 'gclist' on userdata with no user values.
** This structure here is used only to compute the correct size for
** this representation. (The 'bindata' field in its end ensures correct
** alignment for binary data following this header.)
*/
typedef struct Udata0 {
  CommonHeader;
  unsigned short nuvalue;  /* number of user values */
  size_t len;  /* number of bytes */
  struct Table *metatable;
  union {LUAI_MAXALIGN;} bindata;
} Udata0;


/* compute the offset of the memory area of a userdata */
//#define udatamemoffset(nuv) \
//	((nuv) == 0 ? offsetof(Udata0, bindata)  \
//                    : offsetof(Udata, uv) + (sizeof(UValue) * (nuv)))
inline size_t udatamemoffset(size_t nuv) {
    return 	((nuv) == 0 
        ? offsetof(Udata0, bindata) 
        : offsetof(Udata, uv) + (sizeof(UValue) * (nuv)));
}
/* get the address of the memory block inside 'Udata' */
//#define getudatamem(u)	(cast_charp(u) + udatamemoffset((u)->nuvalue))
inline void* getudatamem(Udata* u) {
    return (cast_charp(u) + udatamemoffset((u)->nuvalue));
}
/* compute the size of a userdata */
//#define sizeudata(nuv,nb)	(udatamemoffset(nuv) + (nb))
inline size_t sizeudata(size_t nuv, size_t nb) {
    return (udatamemoffset(nuv) + (nb));
}
/* }================================================================== */


/*
** {==================================================================
** Prototypes
** ===================================================================
*/

#define LUA_VPROTO	makevariant(LUA_TPROTO, 0)


/*
** Description of an upvalue for function prototypes
*/
typedef struct Upvaldesc {
  TString *name;  /* upvalue name (for debug information) */
  lu_byte instack;  /* whether it is in stack (register) */
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
  lu_byte kind;  /* kind of corresponding variable */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
typedef struct LocVar {
  TString *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} LocVar;


/*
** Associates the absolute line source for a given instruction ('pc').
** The array 'lineinfo' gives, for each instruction, the difference in
** lines from the previous instruction. When that difference does not
** fit into a byte, Lua saves the absolute line for that instruction.
** (Lua also saves the absolute line periodically, to speed up the
** computation of a line number: we can use binary search in the
** absolute-line array, but we must traverse the 'lineinfo' array
** linearly to compute a line.)
*/
typedef struct AbsLineInfo {
  int pc;
  int line;
} AbsLineInfo;

/*
** Function Prototypes
*/
typedef struct Proto {
  CommonHeader;
  lu_byte numparams;  /* number of fixed (named) parameters */
  lu_byte is_vararg;
  lu_byte maxstacksize;  /* number of registers needed by this function */
  int sizeupvalues;  /* size of 'upvalues' */
  int sizek;  /* size of 'k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of 'p' */
  int sizelocvars;
  int sizeabslineinfo;  /* size of 'abslineinfo' */
  int linedefined;  /* debug information  */
  int lastlinedefined;  /* debug information  */
  TValue *k;  /* constants used by the function */
  Instruction *code;  /* opcodes */
  struct Proto **p;  /* functions defined inside the function */
  Upvaldesc *upvalues;  /* upvalue information */
  ls_byte *lineinfo;  /* information about source lines (debug information) */
  AbsLineInfo *abslineinfo;  /* idem */
  LocVar *locvars;  /* information about local variables (debug information) */
  TString  *source;  /* used for debug information */
  GCObject *gclist;
} Proto;

/* }================================================================== */


/*
** {==================================================================
** Closures
** ===================================================================
*/

#define LUA_VUPVAL	makevariant(LUA_TUPVAL, 0)


/* Variant tags for functions */
#define LUA_VLCL	makevariant(LUA_TFUNCTION, 0)  /* Lua closure */
#define LUA_VLCF	makevariant(LUA_TFUNCTION, 1)  /* light C function */
#define LUA_VCCL	makevariant(LUA_TFUNCTION, 2)  /* C closure */

//#define ttisfunction(o)		checktype(o, LUA_TFUNCTION)
inline lu_byte ttisfunction(const TValue* o) {
    return checktype(o, LUA_TFUNCTION);
}
//#define ttisclosure(o)		((rawtt(o) & 0x1F) == LUA_VLCL)
inline lu_byte ttisclosure(const TValue* o) {
    return ((rawtt(o) & 0x1F) == LUA_VLCL);
}
//#define ttisLclosure(o)		checktag((o), ctb(LUA_VLCL))
inline lu_byte ttisLclosure(const TValue* o) {
    return checktag((o), ctb(LUA_VLCL));
}

//#define ttislcf(o)		checktag((o), LUA_VLCF)
inline lu_byte ttislcf(const TValue* o) {
    return checktag((o), LUA_VLCF);
}

//#define ttisCclosure(o)		checktag((o), ctb(LUA_VCCL))
inline lu_byte ttisCclosure(const TValue* o) {
    return checktag((o), ctb(LUA_VCCL));
}
//#define isLfunction(o)	ttisLclosure(o)
inline lu_byte isLfunction(const TValue* o) {
    return ttisLclosure(o);
}


//#define setclLvalue(L,obj,x) \
//  { TValue *io = (obj); LClosure *x_ = (x); \
//    set_val_gc(io,obj2gco(x_)); settt_(io, ctb(LUA_VLCL)); \
//    checkliveness(L,io); }

extern void setclLvalue(lua_State* L, TValue* obj, struct LClosure* x);

//#define setclLvalue2s(L,o,cl)	setclLvalue(L,s2v(o),cl)
inline void setclLvalue2s(lua_State* L, StkId o, struct LClosure* cl) {
    setclLvalue(L, s2v(o), cl);
}

//#define setfvalue(obj,x) \
//  { TValue *io=(obj); set_val_f(io,x); settt_(io, LUA_VLCF); }
inline void setfvalue(lua_State* L, TValue* obj, lua_CFunction x)
{ 
    TValue* io = (obj); 
    set_val_f(io, x); 
    settt_(L, io, LUA_VLCF); 
}

//#define setclCvalue(L,obj,x) \
//  { TValue *io = (obj); CClosure *x_ = (x); \
//    set_val_gc(io,obj2gco(x_)) ; settt_(io, ctb(LUA_VCCL)); \
//    checkliveness(L,io); }

extern void setclCvalue(lua_State* L, TValue* obj, struct CClosure* x);

/*
** Upvalues for Lua closures
*/
typedef struct UpVal {
  CommonHeader;
  lu_byte tbc;  /* true if it represents a to-be-closed variable */
  TValue *v;  /* points to stack or to its own value */
  union {
    struct {  /* (when open) */
      struct UpVal *next;  /* linked list */
      struct UpVal **previous;
    } open;
    TValue value;  /* the value (when closed) */
  } u;
} UpVal;



#define ClosureHeader \
	CommonHeader; lu_byte nupvalues; GCObject *gclist

typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;
  UpVal *upvals[1];  /* list of upvalues */
} LClosure;


typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;

//#define clvalue(o)	check_exp(ttisclosure(o), gco2cl(get_val_(o).gc))
extern Closure* clvalue(TValue* o);
//#define clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(get_val_(o).gc))
extern LClosure* clLvalue(TValue* o);

//#define fvalue(o)	check_exp(ttislcf(o), get_val_(o).f)
extern lua_CFunction fvalue(const TValue* o);
//#define clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(get_val_(o).gc))
extern CClosure* clCvalue(const TValue* o);

//#define fvalueraw(v)	((v).f)
inline lua_CFunction fvalueraw(Value v) {
    return v.f;
}

//#define getproto(o)	(clLvalue(o)->p)
inline Proto* getproto(TValue* o) {
    return (clLvalue(o)->p);
}

/* }================================================================== */


/*
** {==================================================================
** Tables
** ===================================================================
*/

#define LUA_VTABLE	makevariant(LUA_TTABLE, 0)

//#define ttistable(o)		checktag((o), ctb(LUA_VTABLE))
inline lu_byte ttistable(const TValue* o) {
    return checktag((o), ctb(LUA_VTABLE));
}
//#define hvalue(o)	check_exp(ttistable(o), gco2t(get_val_(o).gc))
extern struct Table* hvalue(const TValue* o);

//#define sethvalue(L,obj,x) \
//  { TValue *io = (obj); Table *x_ = (x); \
//    set_val_gc(io,obj2gco(x_)); settt_(io, ctb(LUA_VTABLE)); \
//    checkliveness(L,io); }
extern void sethvalue(lua_State* L, TValue* obj, struct Table* x);

//#define sethvalue2s(L,o,h)	sethvalue(L,s2v(o),h)
inline void sethvalue2s(lua_State* L, StkId o, struct Table* h) {
    sethvalue(L, s2v(o), h);
}

/*
** Nodes for Hash tables: A pack of two TValue's (key-value pairs)
** plus a 'next' field to link colliding entries. The distribution
** of the key's fields ('key_tt' and 'key_val') not forming a proper
** 'TValue' allows for a smaller size for 'Node' both in 4-byte
** and 8-byte alignments.
*/
typedef union Node {
  struct NodeKey {
    TValuefields;  /* fields for value */
    lu_byte key_tt;  /* key type */
    int next;  /* for chaining */
    Value key_val;  /* key value */
  } u;
  TValue i_val;  /* direct access to node's value as a proper 'TValue' */
} Node;


/* copy a value into a key */
//#define setnodekey(L,node,obj) \
//	{ Node *n_=(node); const TValue *io_=(obj); \
//	  n_->u.key_val = io_->value_; n_->u.key_tt = io_->tt_; \
//	  checkliveness(L,io_); }
inline void setnodekey(lua_State* L, Node* node, const TValue* obj) {
    Node* n_ = (node); 
    const TValue* io_ = (obj);
    n_->u.key_val = io_->value_; 
    n_->u.key_tt = io_->tt_;
    checkliveness(L, io_);
}

/* copy a value from a key */
//#define getnodekey(L,obj,node) \
//	{ TValue *io_=(obj); const Node *n_=(node); \
//	  io_->value_ = n_->u.key_val; io_->tt_ = n_->u.key_tt; \
//	  checkliveness(L,io_); }
inline void getnodekey(lua_State* L, TValue* obj, Node* node) 
{
    TValue* io_ = (obj); 
    const Node* n_ = (node);
    io_->value_ = n_->u.key_val; 
    io_->tt_ = n_->u.key_tt;
    checkliveness(L, io_);
}


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of size of 'node' array */
  unsigned int alimit;  /* "limit" of 'array' array */
  TValue *array;  /* array part */
  Node *node;
  Node *lastfree;  /* any free position is before this position */
  struct Table *metatable;
  GCObject *gclist; /*list of tables for gc to collect*/
} Table;

/*
** About 'alimit': if 'isrealasize(t)' is true, then 'alimit' is the
** real size of 'array'. Otherwise, the real size of 'array' is the
** smallest power of two not smaller than 'alimit' (or zero iff 'alimit'
** is zero); 'alimit' is then used as a hint for #t.
*/

#define BITRAS		(1 << 7)
//#define isrealasize(t)		(!((t)->marked & BITRAS))
inline lu_byte isrealasize(const Table* t) {
    return (!((t)->marked & BITRAS));
}
//#define setrealasize(t)		((t)->marked &= cast_byte(~BITRAS))
inline void setrealasize(Table* t) {
    ((t)->marked &= cast_byte(~BITRAS));
}
//#define setnorealasize(t)	((t)->marked |= BITRAS)
inline void setnorealasize(Table* t) {
    ((t)->marked |= BITRAS);
}

/*
** Macros to manipulate keys inserted in nodes
*/
//#define keytt(node)		((node)->u.key_tt)
inline lu_byte keytt(const Node* node) {
    return ((node)->u.key_tt);
}
inline lu_byte set_keytt(Node* node, lu_byte tt) {
    return node->u.key_tt = tt;
}
//#define keyval(node)		((node)->u.key_val)
inline Value keyval(const Node* node) {
    return node->u.key_val;
}
inline Value* keyval_ptr(const Node* node) {
    return (Value*)&(node->u.key_val);
}

//#define keyisnil(node)		(keytt(node) == LUA_TNIL)
inline lu_byte keyisnil(const Node* node) {
    return (keytt(node) == LUA_TNIL);
}
//#define keyisinteger(node)	(keytt(node) == LUA_VNUMINT)
inline lu_byte keyisinteger(const Node* node) {
    return (keytt(node) == LUA_VNUMINT);
}

//#define keyival(node)		(keyval(node).i)
inline lua_Integer keyival(const Node* node) {
    return (keyval(node).i);
}

//#define keyisshrstr(node)	(keytt(node) == ctb(LUA_VSHRSTR))
inline lu_byte keyisshrstr(const Node* node) {
    return 	(keytt(node) == ctb(LUA_VSHRSTR));
}
//#define keystrval(node)		(gco2ts(keyval(node).gc))
extern TString* keystrval(const Node* node);

//#define setnilkey(node)		(keytt(node) = LUA_TNIL)
inline void setnilkey(const Node* node) {
    set_keytt((Node*)node, LUA_TNIL);
}
//#define keyiscollectable(n)	(keytt(n) & BIT_ISCOLLECTABLE)
inline lu_byte keyiscollectable(const Node* n) {
    return (keytt(n) & BIT_ISCOLLECTABLE);
}
//#define gckey(n)	(keyval_ptr(n)->gc)
inline struct GCObject* gckey(const Node* n) {
    return (keyval(n).gc);
}
inline void set_gckey(Node* n, struct GCObject* o)
{
    n->u.key_val.gc = o;
}
//#define gckeyN(n)	(keyiscollectable(n) ? gckey(n) : NULL)
inline struct GCObject* gckeyN(const Node* n) {
    return (keyiscollectable(n) ? gckey(n) : NULL);
}

/*
** Use a "nil table" to mark dead keys in a table. Those keys serve
** to keep space for removed entries, which may still be part of
** chains. Note that the 'keytt' does not have the BIT_ISCOLLECTABLE
** set, so these values are considered not collectable and are different
** from any valid value.
*/
//#define setdeadkey(n)	(keytt(n) = LUA_TTABLE, gckey(n) = NULL)
inline void setdeadkey(Node* n) {
    set_keytt(n, LUA_TTABLE);
    set_gckey(n, NULL);
}
/* }================================================================== */



/*
** 'module' operation for hashing (size is always a power of 2)
*/
//#define lmod(s,size) \
//	(check_exp((size&(size-1))==0, (cast_int((s) & ((size)-1)))))
inline unsigned int lmod(lua_Integer s, int size) {
    return (check_exp((size & (size - 1)) == 0, (cast_int((s) & ((size)-1)))));
}
//#define twoto(x)	(1<<(x))
inline size_t twoto(lu_byte x) {
    return (1 << (x));
}
//#define sizenode(t)	(twoto((t)->lsizenode))
inline size_t sizenode(const Table* t) {
    return twoto(t->lsizenode);
}

/* size of buffer for 'luaO_utf8esc' function */
#define UTF8BUFFSZ	8

LUAI_FUNC int luaO_utf8esc (char *buff, unsigned long x);
LUAI_FUNC int luaO_ceillog2 (unsigned int x);
LUAI_FUNC int luaO_rawarith (lua_State *L, int op, const TValue *p1,
                             const TValue *p2, TValue *res);
LUAI_FUNC void luaO_arith (lua_State *L, int op, const TValue *p1,
                           const TValue *p2, StkId res);
LUAI_FUNC size_t luaO_str2num (const char *s, TValue *o);
LUAI_FUNC int luaO_hexavalue (int c);
LUAI_FUNC void luaO_tostring (lua_State *L, TValue *obj);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t srclen);


extern int luaC_should_do_rc(lu_byte tt);

#endif

