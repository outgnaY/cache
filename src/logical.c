// logical record 
#include "cache.h"

void logical_rec_type_set(
    logical_rec_type_t *type, 
    ulint mtype,
    ulint prtype,
    ulint len,
    ulint prec) {

    assert(type != NULL);
    type->mtype = mtype;
    type->prtype = prtype;
    type->len = len;
    type->prec = prec;
}

void logical_rec_type_copy(
    logical_rec_type_t *dest,  // type struct to copy to
    logical_rec_type_t *src) {
    
    *dest = *src;
}

ulint logical_rec_type_get_mtype(logical_rec_type_t *type) {
    assert(type != NULL);
    return type->mtype;
}

ulint logical_rec_type_get_prtype(logical_rec_type_t *type) {
    assert(type != NULL);
    return type->prtype;
}

ulint logical_rec_type_get_len(logical_rec_type_t *type) {
    assert(type != NULL);
    return type->len;
}

ulint logical_rec_type_get_prec(logical_rec_type_t *type) {
    assert(type != NULL);
    return type->prec;
}

ulint logical_rec_type_get_fixed_size(logical_rec_type_t *type) {
    ulint mtype;
    mtype = logical_rec_type_get_mtype(type);
    switch (mtype) {
    case DATA_CHAR:
    case DATA_INT:
    case DATA_FLOAT:
    case DATA_DOUBLE:
        return logical_rec_type_get_len(type);
    case DATA_VARCHAR:
    case DATA_BINARY:
    case DATA_BLOB:
        return 0;
    default:
        assert(0);
    }
}

bool logical_rec_type_is_fixed_size(logical_rec_type_t *type) {
    ulint size;
    size = logical_rec_type_get_fixed_size(type);
    if (size) {
        return true;
    }
    return false;
}