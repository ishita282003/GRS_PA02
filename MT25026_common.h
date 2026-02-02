#ifndef COMMON_H
#define COMMON_H

/* Number of fields in a message */
#define NUM_FIELDS 8

/* 
 * message_t:
 * Represents an application-level message.
 * - fields: array of pointers, each pointing to a message field
 * - field_size: size (in bytes) of each field
 */
typedef struct {
    char *fields[NUM_FIELDS];  /* Pointers to individual message fields */
    int field_size;            /* Size of each field in bytes */
} message_t;

#endif