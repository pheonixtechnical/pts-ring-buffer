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
void runRxStaging(tBufferPair *ptrBufferPair);
void runTxStaging(tBufferPair *ptrBufferPair);
uint8_t isTxDataReadyToRead(tBufferPair *ptrBufferPair);

//                             Common Methods                                 //
uint8_t getByte(eBufferResponse *ptrStatus, tBufferData *ptrBufferData);
void putByte(uint8_t byte, eBufferResponse *ptrStatus, tBufferData *ptrBufferData);
void runStaging(tBufferData *ptrBufferData);
void setupBuffer(tBufferData *ptrBufferData);
void bufferWrite(tRingBuffer *ptrBuffer, uint8_t byte);
uint8_t bufferRead(tRingBuffer *ptrBuffer);

//----------------------------------------------------------------------------//
//                             API Methods                                    //
//----------------------------------------------------------------------------//
void setupBusBuffer(tBufferPair *ptrBufferPair) {
    setupBuffer(&ptrBufferPair->bufferRx);
    setupBuffer(&ptrBufferPair->bufferTx);
}

void runRxStaging(tBufferPair *ptrBufferPair) {
    runStaging(&ptrBufferPair->bufferRx);
}

void runTxStaging(tBufferPair *ptrBufferPair) {
    runStaging(&ptrBufferPair->bufferTx);
}

void putByteRx(tBufferPair *ptrBufferPair, uint8_t byte, eBufferResponse *ptrStatus) {
    putByte(byte, ptrStatus, &(ptrBufferPair->bufferRx));
}

void putByteTx(tBufferPair *ptrBufferPair, uint8_t byte, eBufferResponse *ptrStatus) {
    putByte(byte, ptrStatus, &(ptrBufferPair->bufferTx));
}

uint8_t getByteRx(tBufferPair *ptrBufferPair, eBufferResponse *ptrStatus) {
    return getByte(ptrStatus, &(ptrBufferPair->bufferRx));
}

uint8_t getByteTx(tBufferPair *ptrBufferPair, eBufferResponse *ptrStatus) {
    return getByte(ptrStatus, &(ptrBufferPair->bufferTx));
}

uint8_t getRxDataByteCount(tBufferPair *ptrBufferPair) {
    return ptrBufferPair->bufferRx.buffer.fillLevel;
}

uint8_t getTxDataByteCount(tBufferPair *ptrBufferPair) {
    return ptrBufferPair->bufferTx.buffer.fillLevel;
}

uint8_t isRxDataReadyToRead(tBufferPair *ptrBufferPair) {
    return ptrBufferPair->bufferRx.buffer.fillLevel > 0 && !ptrBufferPair->bufferRx.flags.bufferLocked;
}

uint8_t isTxDataReadyToRead(tBufferPair *ptrBufferPair) {
    return ptrBufferPair->bufferTx.buffer.fillLevel > 0 && !ptrBufferPair->bufferTx.flags.bufferLocked;
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
    
    ptrBuffer = &(ptrBufferData->stagingBuffer);
    
    ptrBuffer -> ptrStart = &(ptrBuffer -> data[0]);
    ptrBuffer -> ptrEnd = &(ptrBuffer -> data[(BUFFER_SIZE - 1)]);
    ptrBuffer -> ptrRead = ptrBuffer -> ptrStart;
    ptrBuffer -> ptrWrite = ptrBuffer -> ptrStart;
    ptrBuffer -> fillLevel = 0;
    ptrBufferData -> flags.byte = 0;
    ptrBuffer -> size = BUFFER_SIZE;
}

void putByte(uint8_t byte, eBufferResponse *ptrStatus, tBufferData *ptrBufferData) {
    tRingBuffer *ptrBuffer = &(ptrBufferData->stagingBuffer);
    
    if(ptrBufferData -> flags.stagingLocked) {
        *ptrStatus = BUFFER_ERROR_LOCKED;
        return;
    }
    
    if(ptrBuffer -> fillLevel >= ptrBuffer -> size) {        
        *ptrStatus = BUFFER_ERROR_FULL;
        return;
    }
    
    ptrBufferData -> flags.stagingLocked = 1;
    
    bufferWrite(ptrBuffer,byte);
    
    ptrBufferData -> flags.stagingLocked = 0;
    
    *ptrStatus = BUFFER_OPERATION_OK;
}

uint8_t getByte(eBufferResponse *ptrStatus, tBufferData *ptrBufferData) {
    tRingBuffer *ptrBuffer = &(ptrBufferData->buffer);
    tBufferFlags *ptrFlags = &(ptrBufferData->flags);
    *ptrStatus = BUFFER_OPERATION_NONE;
    
    if(ptrBuffer -> fillLevel == 0) {
        *ptrStatus = BUFFER_ERROR_EMPTY;
        return 0;
    }
    if(ptrFlags -> bufferLocked) {        
        *ptrStatus = BUFFER_ERROR_LOCKED;
        return 0;
    }
    
    ptrFlags -> bufferLocked = 1;
    uint8_t byte = bufferRead(ptrBuffer);
    
    ptrFlags -> bufferLocked = 0;
    
    *ptrStatus = BUFFER_OPERATION_OK;
    return byte;
}

uint8_t bufferRead(tRingBuffer *ptrBuffer) {
    uint8_t byte = *(ptrBuffer -> ptrRead);
    
    ptrBuffer -> ptrRead++;
    ptrBuffer -> fillLevel--;
    
    if(ptrBuffer -> ptrRead > ptrBuffer -> ptrEnd) {
        ptrBuffer -> ptrRead = ptrBuffer -> ptrStart;
    }
    return byte;
}

void bufferWrite(tRingBuffer *ptrBuffer, uint8_t byte) {
    *(ptrBuffer -> ptrWrite) = byte;

    ptrBuffer -> fillLevel++;
    ptrBuffer -> ptrWrite++;

    if(ptrBuffer -> ptrWrite > ptrBuffer -> ptrEnd) {
    ptrBuffer -> ptrWrite = ptrBuffer -> ptrStart;
    }
}

void runStaging(tBufferData *ptrBufferData) {
    if(!ptrBufferData -> flags.stagingLocked && !ptrBufferData -> flags.bufferLocked) {
        tRingBuffer *ptrStagingBuffer = &(ptrBufferData->stagingBuffer);
        tRingBuffer *ptrBuffer = &(ptrBufferData->buffer);
        if(ptrStagingBuffer -> fillLevel > 0) {
            ptrBufferData -> flags.stagingLocked = 1;
            uint8_t byte = bufferRead(ptrStagingBuffer);
            ptrBufferData -> flags.stagingLocked = 0;
            
            if(ptrBuffer -> fillLevel >= ptrBuffer -> size) {   
                return;
            }

            bufferWrite(ptrBuffer,byte);
        }   
    }
}