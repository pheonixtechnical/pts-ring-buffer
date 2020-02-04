#include "global.h"
#include "buffer_types.h"

//                             API Methods                                    //
void putByteRx(tBufferPair *ptrBufferPair, uint8_t byte, eBufferResponse *ptrStatus);
void putByteTx(tBufferPair *ptrBufferPair, uint8_t byte, eBufferResponse *ptrStatus);
uint8_t getByteRx(tBufferPair *ptrBufferPair, eBufferResponse *ptrStatus);
uint8_t getByteTx(tBufferPair *ptrBufferPair, eBufferResponse *ptrStatus);
uint8_t getRxDataByteCount(tBufferPair *ptrBufferPair);
uint8_t getTxDataByteCount(tBufferPair *ptrBufferPair);
void setupBusBuffer(tBufferPair *ptrBufferPair);
void runRxTxStaging(tBufferPair *ptrBufferPair);
uint8_t isTxDataReadyToRead(tBufferPair *ptrBufferPair);

//                             Common Methods                                 //
uint8_t getByte(eBufferResponse *ptrStatus, tBufferData *ptrBufferData);
void putByte(uint8_t byte, eBufferResponse *ptrStatus, tBufferData *ptrBufferData);
void runStaging(tBufferData *ptrBufferData);
void setupBuffer(tBufferData *ptrBufferData);

//----------------------------------------------------------------------------//
//                             API Methods                                    //
//----------------------------------------------------------------------------//
void setupBusBuffer(tBufferPair *ptrBufferPair) {
    setupBuffer(&(*ptrBufferPair).bufferRx);
    setupBuffer(&(*ptrBufferPair).bufferTx);
}

void runRxTxStaging(tBufferPair *ptrBufferPair) {
    runStaging(&(*ptrBufferPair).bufferRx);
    runStaging(&(*ptrBufferPair).bufferTx);
}

void putByteRx(tBufferPair *ptrBufferPair, uint8_t byte, eBufferResponse *ptrStatus) {
    putByte(byte, ptrStatus, &(*ptrBufferPair).bufferRx);
}

void putByteTx(tBufferPair *ptrBufferPair, uint8_t byte, eBufferResponse *ptrStatus) {
    putByte(byte, ptrStatus, &(*ptrBufferPair).bufferTx);
}

uint8_t getByteRx(tBufferPair *ptrBufferPair, eBufferResponse *ptrStatus) {
    return getByte(ptrStatus, &(*ptrBufferPair).bufferRx);
}

uint8_t getByteTx(tBufferPair *ptrBufferPair, eBufferResponse *ptrStatus) {
    return getByte(ptrStatus, &(*ptrBufferPair).bufferTx);
}

uint8_t getRxDataByteCount(tBufferPair *ptrBufferPair) {
    return (*ptrBufferPair).bufferRx.buffer.fillLevel;
}

uint8_t getTxDataByteCount(tBufferPair *ptrBufferPair) {
    return (*ptrBufferPair).bufferTx.buffer.fillLevel;
}

uint8_t isRxStageFull(tBufferPair *ptrBufferPair){
    return (*ptrBufferPair).bufferRx.flags.stagingFull;
}

uint8_t isTxStageFull(tBufferPair *ptrBufferPair) {
    return (*ptrBufferPair).bufferTx.flags.stagingFull;
}

uint8_t isRxDataReadyToRead(tBufferPair *ptrBufferPair) {
    return (*ptrBufferPair).bufferRx.buffer.fillLevel > 0 && !(*ptrBufferPair).bufferRx.flags.bufferLocked;
}

uint8_t isTxDataReadyToRead(tBufferPair *ptrBufferPair) {
    return (*ptrBufferPair).bufferTx.buffer.fillLevel > 0 && !(*ptrBufferPair).bufferTx.flags.bufferLocked;
}

//----------------------------------------------------------------------------//
//                             Common Methods                                 //
//----------------------------------------------------------------------------//
void setupBuffer(tBufferData *ptrBufferData) {
    tRingBuffer *ptrBuffer = &(ptrBufferData->buffer);
    
    ptrBuffer -> ptrStart = &(ptrBuffer -> data[0]);
    ptrBuffer -> ptrEnd = &(ptrBuffer -> data[(BUFFER_SIZE - 1)]);
    ptrBuffer -> ptrRead = ptrBuffer -> ptrStart;
    ptrBuffer -> ptrWrite = ptrBuffer -> ptrStart;
    ptrBuffer -> fillLevel = 0;
    ptrBufferData -> flags.byte = 0;
    ptrBuffer -> size = BUFFER_SIZE;
    ptrBufferData -> state = BUFFER_NONE;
}

void putByte(uint8_t byte, eBufferResponse *ptrStatus, tBufferData *ptrBufferData) {
    if(!ptrBufferData -> flags.stagingFull) {
        uint8_t count = (ptrBufferData -> stageWriteCount)++;
        ptrBufferData -> stage[count] = byte;
        if(count >= (MAX_STAGING_SIZE - 1)) {
            ptrBufferData -> flags.stagingFull = 1;
        } else {
            ptrBufferData -> stagingActivity = 20;
        }
        *ptrStatus = BUFFER_OPERATION_OK;
    } else {
        *ptrStatus = BUFFER_ERROR_FULL;
    }
}

uint8_t getByte(eBufferResponse *ptrStatus, tBufferData *ptrBufferData) {
    tRingBuffer *ptrBuffer = &(ptrBufferData->buffer);
    tBufferFlags *ptrFlags = &(ptrBufferData->flags);
    *ptrStatus = BUFFER_NONE;
    
    if(ptrBuffer -> fillLevel == 0) {
        *ptrStatus = BUFFER_ERROR_EMPTY;
        return 0;
    }
    if(ptrFlags -> bufferLocked) {
        *ptrStatus = BUFFER_ERROR_LOCKED;
        return 0;
    }
    ptrFlags -> bufferLocked = 1;
    uint8_t byte = *(ptrBuffer -> ptrRead);
    
    ptrBuffer -> ptrRead++;
    ptrBuffer -> fillLevel--;
    
    if(ptrBuffer -> ptrRead > ptrBuffer -> ptrEnd) {
        ptrBuffer -> ptrRead = ptrBuffer -> ptrStart;
    }
    
    ptrFlags -> bufferLocked = 0;
    
    *ptrStatus = BUFFER_OPERATION_OK;
    return byte;
}

void runStaging(tBufferData *ptrBufferData) {
    eBufferLoopState *ptrState = &(ptrBufferData->state);
    tBufferFlags *ptrFlags = &(ptrBufferData->flags);
    tRingBuffer *ptrBuffer = &(ptrBufferData->buffer);
    
    switch (*ptrState) {
        case BUFFER_NONE:
            *ptrState = BUFFER_WAIT_FOR_STAGE;
        case BUFFER_WAIT_FOR_STAGE:
            if(ptrBufferData -> stagingActivity > 0) {
                (ptrBufferData -> stagingActivity)--;
                break;
            }
            if(ptrBufferData -> stageWriteCount > 0) {
                *ptrState = BUFFER_CHECK_FILL;
            }
            break;
            
        case BUFFER_CHECK_FILL:
            if((ptrBuffer -> fillLevel + ptrBufferData -> stageWriteCount) >= ptrBuffer -> size){
                *ptrState = BUFFER_WAIT_FOR_STAGE;
                break;
            }
            *ptrState = BUFFER_LOCK;
            break;
            
        case BUFFER_LOCK:
            if(ptrFlags -> bufferLocked) {
                break;
            }
            ptrFlags -> bufferLocked = 1;
            ptrBufferData -> stageReadCount = 0;
            *ptrState = BUFFER_READ_FROM_STAGE;
            break;
            
        case BUFFER_READ_FROM_STAGE:
            *(ptrBuffer -> ptrWrite) = ptrBufferData -> stage[ptrBufferData -> stageReadCount];
            ptrBuffer -> fillLevel++;
            ptrBuffer -> ptrWrite++;
            
            if(ptrBuffer -> ptrWrite > ptrBuffer -> ptrEnd) {
                ptrBuffer -> ptrWrite = ptrBuffer -> ptrStart;
            }
            
            ptrBufferData -> stage[ptrBufferData -> stageReadCount] = 0;
            ptrBufferData -> stageReadCount++;
            *ptrState = BUFFER_MORE_IN_STAGE;
            break;
            
        case BUFFER_MORE_IN_STAGE:
            if(ptrBufferData -> stageReadCount > ptrBufferData -> stageWriteCount) {
                *ptrState = BUFFER_UNLOCK;
            } else {
                *ptrState = BUFFER_READ_FROM_STAGE;
            }
            break;
            
        case BUFFER_UNLOCK:
            ptrBufferData-> stageReadCount = 0;
            ptrBufferData-> stageWriteCount = 0;
            ptrFlags -> stagingFull = 0;
            ptrFlags -> bufferLocked = 0;
            *ptrState = BUFFER_WAIT_FOR_STAGE;
            break;
    }
}