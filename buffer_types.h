#ifndef BUFFER_TYPES_H
#define	BUFFER_TYPES_H

#include <xc.h>

#define BUFFER_SIZE 150
#define MAX_STAGING_SIZE 60

typedef struct {
    uint8_t *ptrWrite;
    uint8_t *ptrRead;
    uint8_t *ptrStart;
    uint8_t *ptrEnd;
    uint8_t fillLevel;
    uint8_t size;
    uint8_t data[BUFFER_SIZE];
} tRingBuffer;

typedef union {
    struct {
        unsigned bufferLocked:1;
        unsigned stagingLocked:1;
    };
    uint8_t byte;
} tBufferFlags;

typedef struct {
    tRingBuffer buffer;
    tRingBuffer stagingBuffer;
    volatile tBufferFlags flags;
    uint8_t stageReadCount; 
} tBufferData;

typedef struct {
    tBufferData bufferRx;
    tBufferData bufferTx;
} tBufferPair;

typedef enum {
    BUFFER_OPERATION_NONE,
    BUFFER_OPERATION_OK,
    BUFFER_ERROR_EMPTY,
    BUFFER_ERROR_FULL,
    BUFFER_ERROR_LOCKED,
} eBufferResponse;
#endif	/* BUFFER_TYPES_H */