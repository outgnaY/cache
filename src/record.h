// physical record 
#ifndef RECORD_H
#define RECORD_H

#include "common_define.h"

/**
 *  32 bits physical record header:
 *  info_bits       n_owned     number of field     short flag      next_record
 *  2 bits          4 bits      9 bits              1 bits          16 bits   
 */           

typedef byte record_t;

// gets a bit field from within 1 byte
ulint record_get_bit_field_1(
    record_t *rec,      // pointer to record origin
    ulint offs,         // offset from the origin down
    ulint mask,         // mask used to filter bits
    ulint shift);       // shift right applied after masking
    
// set a bit field within 1 byte
void record_set_bit_field_1(
    record_t *rec,      // pointer to record origin
    ulint val,          // value to set
    ulint offs,         // offset from the origin down
    ulint mask,         // mask used to filter bits
    ulint shift);       // shift right applied after masking

ulint record_get_bit_field_2(
    record_t *rec,      // pointer to record origin
    ulint offs,         // offset from the origin down
    ulint mask,         // mask used to filter bits
    ulint shift);       // shift right applied after masking

void record_set_bit_field_2(
    record_t *rec,      // pointer to record origin
    ulint val,          // value to set
    ulint offs,         // offset from the origin down
    ulint mask,         // mask used to filter bits
    ulint shift);       // shift right applied after masking


ulint record_get_info_bits(
    record_t *rec);     // pointer to record origin


void record_set_info_bits(
    record_t *rec,      // pointer to record origin
    ulint bits);        // info bits

ulint record_get_n_owned(
    record_t *rec);     // pointer to record origin

void record_set_n_owned(
    record_t *rec,      // pointer to record origin
    ulint n_owned);     // number of owned

ulint record_get_n_fields(
    record_t *rec);     // pointer to record origin

void record_set_n_fields(
    record_t *rec,      // pointer to record origin
    ulint n_fields);    // number of fields

bool record_get_short_flag(
    record_t *rec);     // pointer to record origin

void record_set_short_flag(
    record_t *rec,      // pointer to record origin
    bool short_flag);   // short flag

ulint record_get_next_offs(
    record_t *rec);     // offset of next record

void record_set_next_offs(
    record_t *rec,      // pointer to record origin
    ulint next);        // offset of next record

ulint record_1_get_field_end_info(
    record_t *rec,      // pointer to record origin 
    ulint n);           // field index, start from 0

void record_1

ulint record_2_get_field_end_info(
    record_t *rec,      // pointer to record origin
    ulint n);           // field index, start from 0

bool record_get_nth_field_extern_bit(
    record_t *rec,      // pointer to record origin
    ulint n);           // nth field

ulint record_1_get_prev_field_end_info(
    record_t *rec,      // pointer to record origin
    ulint n);           // nth field

ulint record_2_get_prev_field_end_info(
    record_t *rec,      // pointer to record origin
    ulint n);           // nth field

ulint record_1_get_field_start_offs(
    record_t *rec,      // pointer to record origin 
    ulint n);           // nth field

ulint record_get_field_start_offs(
    record_t *rec,      // pointer to record origin
    ulint n);           // nth field

#define REC_EXTRA_BYTES 4

#define REC_INFO_BITS_MASK 0xc0
#define REC_INFO_BITS_SHIFT 6
#define REC_INFO_BITS_OFFS 4


#define REC_N_OWNED_MASK 0x3c
#define REC_N_OWNED_SHIFT 2
#define REC_N_OWNED_OFFS 4


#define REC_N_FIELDS_MASK 0x3fe
#define REC_N_FIELDS_SHIFT 1
#define REC_N_FIELDS_OFFS 4

#define REC_SHORT_MASK 0x1
#define REC_SHORT_SHIFT 0
#define REC_SHORT_OFFS 3

#define REC_NEXT_MASK 0xffff
#define REC_NEXT_SHIFT 0
#define REC_NEXT_OFFS 2

#endif  // RECORD_H