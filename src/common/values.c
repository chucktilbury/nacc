
#include <assert.h>

#include "common.h"

Value* createValue(ValueType type, const char* name) {

    Value* val = _alloc_ds(Value);
    val->type = type;
    val->is_assigned = false;
    val->is_const = false;
    if(name != NULL)
        val->name = _copy_str(name);

    return val;
}

void assignNumVal(Value* value, double val) {

    value->data.num = val;
    value->is_assigned = true;
}

void assignStrVal(Value* value, const char* val) {

    value->data.str = (char*)val;
    value->is_assigned = true;
}

void assignBoolVal(Value* value, bool val) {

    value->data.bval = val;
    value->is_assigned = true;
}

void destroyValue(Value* val) {

    assert(val != NULL);
    if(val->name != NULL)
        _free(val->name);
    _free(val);
}

ValueType getValType(Value* val) {

    assert(val != NULL);
    return val->type;
}


ValueStore* createValueStore() {

    ValueStore* vs = _alloc_ds(ValueStore);
    vs->cap = 1 << 3;
    vs->len = 0;
    vs->lst = _alloc_ds_array(Value*, vs->cap);

    return vs;
}

void destroyValueStore(ValueStore* vs) {

    assert(vs != NULL);
    for(size_t i = 0; i < vs->len; i++)
        destroyValue(vs->lst[i]);
    _free(vs->lst);
    _free(vs);
}

Value* getValue(ValueStore* store, size_t idx) {

    assert(store != NULL);

    if(idx < store->len) {
        return store->lst[idx];
    }

    return NULL;
}

size_t addValue(ValueStore* store, Value* val, const char* name) {

    assert(store != NULL);

    if(store->len + 1 > store->cap) {
        store->cap <<= 1;
        store->lst = _realloc_ds_array(store->lst, Value*, store->cap);
    }

    if(name != NULL)
        val->name = _copy_str(name);
    size_t idx = store->len;
    store->lst[store->len] = val;
    store->len++;

    return idx;
}

static const char* val_type_to_str(ValueType type) {

    return (type == VAL_NUM )? "NUMBER":
        (type == VAL_STR )? "STRING":
        (type == VAL_BVAL)? "BOOLEAN":
        (type == VAL_ADDR)? "ADDRESS": "UNKNOWN";
}

void dumpValue(int idx, Value* val) {

    printf("\t%d:%s:%c:%c:\t\"%s\"\t", idx, val_type_to_str(val->type),
            val->is_assigned? 't': 'f', val->is_const? 't':'f',
            val->name? val->name: "");

    switch(val->type) {
        case VAL_NUM: printf("%f ", val->data.num); break;
        case VAL_ADDR: printf("%u ", val->data.addr); break;
        case VAL_STR: printf("\"%s \"", val->data.str); break;
        case VAL_BVAL: printf("%s ", val->data.bval? "true": "false"); break;
        default: printf("UNKNOWN"); break;
    }

    printf("\n");
}

void dumpValueStore(ValueStore* store) {

    assert(store != NULL);

    size_t len = store->len;
    Value** lst = store->lst;
    printf("Value Store:\n");
    for(size_t idx = 0; idx < len; idx++) {
        dumpValue(idx, lst[idx]);
    }
}

static Value* load_value(FILE* fp) {

    Value* val = _alloc_ds(Value);

    fread(&val->type, sizeof(val->type), 1, fp);
    fread(&val->is_assigned, sizeof(val->is_assigned), 1, fp);

    size_t sz;
    fread(&sz, sizeof(sz), 1, fp);
    val->name = _alloc(sz);
    fread((void*)val->name, sizeof(char), sz, fp);

    if(val->is_assigned) {
        switch(val->type) {
            case VAL_NUM:
                fread(&val->data.num, sizeof(val->data.num), 1, fp);
                break;
            case VAL_ADDR:
                fread(&val->data.addr, sizeof(val->data.addr), 1, fp);
                break;
            case VAL_BVAL:
                fread(&val->data.bval, sizeof(val->data.bval), 1, fp);
                break;
            case VAL_STR: {
                    size_t slen;
                    fread(&slen, sizeof(slen), 1, fp);
                    val->data.str = _alloc(slen+1);
                    fread(val->data.str, sizeof(char), slen, fp);
                }
                break;
        }
    }

    return val;
}

void loadValueStore(ValueStore* store, FILE* fp) {

    size_t len;
    fread(&len, sizeof(len), 1, fp);
    for(size_t idx = 0; idx < len; idx++) {
        Value* val = load_value(fp);
        addValue(store, val, NULL);
    }
}

static void save_value(Value* val, FILE* fp) {

    fwrite(&val->type, sizeof(val->type), 1, fp);
    fwrite(&val->is_assigned, sizeof(val->is_assigned), 1, fp);

    if(val->name != NULL) {
        size_t sz = strlen(val->name) + 1;
        fwrite(&sz, sizeof(sz), 1, fp);
        fwrite(val->name, sizeof(char), sz, fp);
    }
    else {
        size_t sz = 1;
        fwrite(&sz, sizeof(sz), 1, fp);
        char* buf[1] = {0};
        fwrite(buf, sizeof(char), sz, fp);
    }

    if(val->is_assigned) {
        switch(val->type) {
            case VAL_NUM:
                fwrite(&val->data.num, sizeof(val->data.num), 1, fp);
                break;
            case VAL_ADDR:
                fwrite(&val->data.addr, sizeof(val->data.addr), 1, fp);
                break;
            case VAL_BVAL:
                fwrite(&val->data.bval, sizeof(val->data.bval), 1, fp);
                break;
            case VAL_STR: {
                    size_t slen = strlen(val->data.str)+1;
                    fwrite(&slen, sizeof(slen), 1, fp);
                    fwrite(val->data.str, sizeof(char), slen, fp);
                }
                break;
        }
    }
}

void saveValueStore(ValueStore* store, FILE* fp) {

    fwrite(&store->len, sizeof(store->len), 1, fp);
    for(size_t idx = 0; idx < store->len; idx++)
        save_value(store->lst[idx], fp);
}


