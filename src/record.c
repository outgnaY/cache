// physical record 
#include "cache.h"

ulint record_get_bit_field_1(
    record_t *rec,  
    ulint offs,     
    ulint mask,     
    ulint shift) {

    assert(rec != NULL);
    return ((read_from_1(rec - offs) & mask) >> shift);
} 

void record_set_bit_field_1(
    record_t *rec,
    ulint val,
    ulint offs,
    ulint mask,
    ulint shift) {

    assert(rec != NULL);
    write_to_1(rec - offs, (read_from_1(rec - offs) & ~mask) | (val << shift));
}

ulint record_get_bit_field_2(
    record_t *rec,
    ulint offs,
    ulint mask,
    ulint shift) {

    assert(rec != NULL);
    return ((read_from_2(rec - offs) & mask) >> shift);
}

void record_set_bit_field_2(
    record_t *rec,
    ulint val,
    ulint offs,
    ulint mask,
    ulint shift) {

    write_to_2(rec - offs, (read_from_2(rec - offs) & ~mask) | (val << shift));
}

ulint record_get_info_bits(record_t *rec) {
    ulint ret;
    assert(rec != NULL);

}


ulint record_get_info_bits(record_t *rec) {
    assert(rec != NULL);
    ulint ret;
    ret = record_get_bit_field_1(rec, REC_INFO_BITS_OFFS, REC_INFO_BITS_MASK, REC_INFO_BITS_SHIFT);
    return ret;
}

void record_set_info_bits(record_t *rec, ulint bits) {
    record_set_bit_field_1(rec, bits, REC_INFO_BITS_OFFS, REC_INFO_BITS_MASK, REC_INFO_BITS_SHIFT);
}


ulint record_get_n_owned(record_t *rec) {
    assert(rec != NULL);
    ulint ret;
    ret = record_get_bit_field_1(rec, REC_N_OWNED_OFFS, REC_N_OWNED_MASK, REC_N_OWNED_SHIFT);
    return ret;
}

void record_set_n_owned(record_t *rec, ulint n_owned) {
    assert(rec != NULL);
    record_set_bit_field_1(rec, n_owned, REC_N_OWNED_OFFS, REC_N_OWNED_MASK, REC_N_OWNED_SHIFT);
}

ulint record_get_n_fields(record_t *rec) {
    assert(rec != NULL);
    ulint ret;
    ret = record_get_bit_field_2(rec, REC_N_FIELDS_OFFS, REC_N_FIELDS_MASK, REC_N_FIELDS_SHIFT);
    return ret;
}

void record_set_n_fields(record_t *rec, ulint n_fields) {
    assert(rec != NULL);
    record_set_bit_field_2(rec, n_fields, REC_N_FIELDS_OFFS, REC_N_FIELDS_MASK, REC_N_FIELDS_SHIFT);
}

bool record_get_short_flag(record_t *rec) {
    assert(rec != NULL);
    ulint flag;
    flag = record_get_bit_field_1(rec, REC_SHORT_OFFS, REC_SHORT_MASK, REC_SHORT_SHIFT);
    return flag == 1 ? true : false;
}

void record_set_short_flag(record_t *rec, bool short_flag) {
    assert(rec != NULL);
    ulint flag = short_flag ? 1 : 0;
    record_set_bit_field_1(rec, flag, REC_SHORT_OFFS, REC_SHORT_MASK, REC_SHORT_SHIFT);
}

ulint record_get_next_offs(record_t *rec) {
    assert(rec != NULL);
    ulint ret;
    ret = record_get_bit_field_2(rec, REC_NEXT_OFFS, REC_NEXT_MASK, REC_NEXT_SHIFT);
    return ret;
}

void record_set_next_offs(record_t *rec, ulint next) {
    assert(rec != NULL);
    record_set_bit_field_2(rec, next, REC_NEXT_OFFS, REC_NEXT_MASK, REC_NEXT_SHIFT);
}

ulint record_1_get_field_end_info(record_t *rec, ulint n) {
    return read_from_1(rec - (REC_EXTRA_BYTES + n + 1));
}

ulint record_2_get_field_end_info(record_t *rec, ulint n) {
    return read_from_2(rec - (REC_EXTRA_BYTES + 2 * n + 2));
}

bool record_get_nth_field_extern_bit(record_t *rec, ulint n) {
    ulint info;
    if (record_get)
}

ulint record_1_get_prev_field_end_info(record_t *rec, ulint n) {
    return read_from_1(rec - (REC_EXTRA_BYTES + n));
}

ulint record_2_get_prev_field_end_info(record_t *rec, ulint n) {
    return read_from_2(rec - (REC_EXTRA_BYTES + 2 * n));
}

ulint record_1_get_field_start_offs(record_t *rec, ulint n) {
    if (n == 0) {
        return 0;
    }

}

ulint record_get_field_start_offs(record_t *rec, ulint n) {
    if (n == 0) {
        return 0;
    }
    if (record_get_short_flag(rec)) {
        return record_1_get_field_end_info
    }
}


