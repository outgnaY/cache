// logical record 
#ifndef LOGICAL_H
#define LOGICAL_H


// data types
#define DATA_VARCHAR 1
#define DATA_CHAR 2
#define DATA_BINARY 4
#define DATA_BLOB 5
#define DATA_INT 6
#define DATA_FLOAT 7
#define DATA_DOUBLE 8

#define DATA_OBJECT 128

#define DATA_MTYPE_MAX 255

typedef struct logical_rec_type {
    ulint mtype;    // main data type
    ulint prtype;   // precise type; 
    
    ulint len;      // length
    ulint prec;     // precision
} logical_rec_type_t;

void logical_rec_type_set(
    logical_rec_type_t *type, 
    ulint mtype,
    ulint prtype,
    ulint len,
    ulint prec);

void logical_rec_type_copy(
    logical_rec_type_t *dest,  // type struct to copy to
    logical_rec_type_t *src); // type struct to copy from

ulint logical_rec_type_get_mtype(logical_rec_type_t *type);

ulint logical_rec_type_get_prtype(logical_rec_type_t *type);

ulint logical_rec_type_get_len(logical_rec_type_t *type);

ulint logical_rec_type_get_prec(logical_rec_type_t *type);

ulint logical_rec_type_get_fixed_size(logical_rec_type_t *type);

bool logical_rec_type_is_fixed_size(logical_rec_type_t *type);


// data field of logical record
typedef struct logical_rec_field {
    void *data;                 // pointer to data
    ulint len;                  // data length
    logical_rec_type_t *type;   // type of data
    ulint col_no;               // 
} logical_rec_field_t;

// logical record 
typedef struct logical_rec {
    ulint info_bits;                // info bits
    ulint n_fields;                 // number of fields
    logical_rec_field_t *fields;    // fields
    struct logical_rec *prev;
    struct logical_rec *next;
} logical_rec_t;


// big vector
typedef struct big_rec_field {
    ulint field_no;     // field number in record
    ulint len;          // stored data len
    void *data;         // stored data
} big_rec_field_t;

// big record
typedef struct big_rec {
    ulint n_fields;             // number of stored fields
    big_rec_field_t *fields;    // fields
} big_rec_t;


#endif  // LOGICAL_H