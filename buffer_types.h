#ifndef BUFFER_TYPES_H
#define	BUFFER_TYPES_H

#include <xc.h>

#define BUFFER_SIZE 60
#define MAX_STAGING_SIZE 30

typedef struct {
    uint8_t *ptrWrite;
    uint8_t *ptrRead;
    uint8_t *ptrStart;
    uint8_t *ptrEnd;
    uint8_t fillLevel;
    uint8_t size;
    uint8_t data[BUFFER_SIZE];
} tRingBuffer;

typedef enum {
    BUFFER_NONE,
    BUFFER_WAIT_FOR_STAGE,
    BUFFER_CHECK_FILL,
    BUFFER_LOCK,
    BUFFER_READ_FROM_STAGE,
    BUFFER_MORE_IN_STAGE,
    BUFFER_UNLOCK
} eBufferLoopState;

typedef union {
    struct {
        unsigned stagingFull:1;
        unsigned bufferLocked:1;       
    };
    uint8_t byte;
} tBufferFlags;

typedef struct {
    tRingBuffer buffer;
    eBufferLoopState state;
    tBufferFlags flags;
    volatile uint8_t stage[MAX_STAGING_SIZE];
    volatile uint8_t stageWriteCount;
    volatile uint16_t stagingActivity;
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