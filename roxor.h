#ifndef __ROXOR_H_
#define __ROXOR_H_

#if defined(__cplusplus)
extern "C" {
#endif

VALUE rb_vm_run_node(const char *fname, NODE *node);
IMP rb_vm_compile_imp(const char *fname, NODE *node);

bool rb_vm_running(void);
void rb_vm_set_running(bool flag);
VALUE rb_vm_load_path(void);
VALUE rb_vm_loaded_features(void);
int rb_vm_safe_level(void);
void rb_vm_set_safe_level(int level);
VALUE rb_vm_top_self(void);
void rb_vm_const_defined(ID path);
bool rb_vm_lookup_method(Class klass, SEL sel, IMP *pimp, NODE **pnode);
bool rb_vm_lookup_method2(Class klass, ID mid, SEL *psel, IMP *pimp, NODE **pnode);
void rb_vm_define_method(Class klass, SEL sel, IMP imp, NODE *node);
void rb_vm_define_attr(Class klass, const char *name, bool read, bool write, int noex);
void rb_vm_alias(VALUE klass, ID name, ID def);
VALUE rb_vm_call(VALUE self, SEL sel, int argc, const VALUE *args, bool super);
VALUE rb_vm_call_with_cache(void *cache, VALUE self, SEL sel, int argc, const VALUE *argv);
VALUE rb_vm_call_with_cache2(void *cache, VALUE self, VALUE klass, SEL sel, int argc, const VALUE *argv);
void *rb_vm_get_call_cache(SEL sel);
VALUE rb_vm_yield(int argc, const VALUE *argv);
bool rb_vm_respond_to(VALUE obj, SEL sel, bool priv);
VALUE rb_vm_method_missing(VALUE obj, int argc, const VALUE *argv);
int rb_vm_find_class_ivar_slot(VALUE klass, ID name);

static inline void
rb_vm_regrow_robject_slots(struct RObject *obj, int new_num_slot)
{
    VALUE *new_slots = (VALUE *)xrealloc(obj->slots, sizeof(VALUE) * new_num_slot);
    if (obj->slots != new_slots) {
	GC_WB(&obj->slots, new_slots);
    }
    int i;
    for (i = obj->num_slots; i < new_num_slot; i++) {
	obj->slots[i] = Qundef;
    }
    obj->num_slots = new_num_slot;
}

static inline VALUE
rb_vm_get_ivar_from_slot(VALUE obj, int slot) 
{
    struct RObject *robj = (struct RObject *)obj;
    assert(slot >= 0);
    if (robj->num_slots < (unsigned int)slot) {
	return Qnil;
    }
    return robj->slots[slot];
}

static inline void
rb_vm_set_ivar_from_slot(VALUE obj, VALUE val, int slot) 
{
    struct RObject *robj = (struct RObject *)obj;
    assert(slot >= 0);
    if (robj->num_slots < (unsigned int)slot) {
	rb_vm_regrow_robject_slots(robj, slot);
    }
    robj->slots[slot] = val;
}

typedef struct {
    VALUE oclass;
    VALUE rclass;
    VALUE recv;
    SEL sel;
    int arity;
    NODE *node;			// can be NULL (if pure Objective-C)
    void *cache;
} rb_vm_method_t;

rb_vm_method_t *rb_vm_get_method(VALUE klass, VALUE obj, ID mid, int scope);

typedef struct {
    VALUE self;
    NODE *node;
    IMP imp;
    bool is_lambda;
    int dvars_size;
    VALUE *dvars[1];
} rb_vm_block_t;

static inline rb_vm_block_t *
rb_proc_get_block(VALUE proc)
{
   return (rb_vm_block_t *)DATA_PTR(proc);
}

rb_vm_block_t *rb_vm_current_block(void);
VALUE rb_vm_block_eval(rb_vm_block_t *block, int argc, const VALUE *argv);

struct rb_float_cache {
    unsigned int count;
    VALUE obj;
};

struct rb_float_cache *rb_vm_float_cache(double d);

static inline VALUE
rb_robject_allocate_instance(VALUE klass)
{
    struct RObject *obj;
    int num_slots = 10;

    obj = (struct RObject *)xmalloc(sizeof(struct RObject) + (num_slots * sizeof(VALUE)));
    OBJSETUP(obj, klass, T_OBJECT);

    ROBJECT(obj)->tbl = NULL;
    ROBJECT(obj)->num_slots = num_slots;

    int i;
    for (i = 0; i < num_slots; i++) {
	ROBJECT(obj)->slots[i] = Qundef;
    }

    return (VALUE)obj;
}

void rb_vm_raise(VALUE exception);
VALUE rb_vm_backtrace(int level);

VALUE rb_vm_pop_broken_value(void);
#define RETURN_IF_BROKEN() \
    do { \
	VALUE __v = rb_vm_pop_broken_value(); \
	if (__v != 0) { \
	    return __v; \
	} \
    } \
    while (0)

VALUE rb_iseq_compile(VALUE src, VALUE file, VALUE line);
VALUE rb_iseq_eval(VALUE iseq);
VALUE rb_iseq_new(NODE *node, VALUE filename);

#if 0 // TODO
#if ENABLE_DEBUG_LOGGING 
# include <libgen.h>
extern bool ruby_dlog_enabled;
extern FILE *ruby_dlog_file;
# define DLOG(mod, fmt, args...)                                          \
    if (UNLIKELY(ruby_dlog_enabled)) {                                    \
        fprintf(ruby_dlog_file, "%s:%d %s ",                              \
                basename((char *)rb_sourcefile()), rb_sourceline(), mod); \
        fprintf(ruby_dlog_file, fmt, ##args);                             \
        fprintf(ruby_dlog_file, "\n");                                    \
    }
# endif
#endif
#define DLOG(mod, fmt, args...)

#if defined(__cplusplus)
}
#endif

#endif /* __ROXOR_H_ */
