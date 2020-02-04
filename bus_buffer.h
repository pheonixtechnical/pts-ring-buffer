#ifndef BUS_BUFFER_H
#define	BUS_BUFFER_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "buffer_types.h"

extern void putByteRx(tBufferPair *ptrBufferPair, uint8_t byte, eBufferResponse *ptrStatus);
extern void putByteTx(tBufferPair *ptrBufferPair, uint8_t byte, eBufferResponse *ptrStatus);
extern uint8_t getByteRx(tBufferPair *ptrBufferPair, eBufferResponse *ptrStatus);
extern uint8_t getByteTx(tBufferPair *ptrBufferPair, eBufferResponse *ptrStatus);
extern uint8_t getRxDataByteCount(tBufferPair *ptrBufferPair);
extern uint8_t getTxDataByteCount(tBufferPair *ptrBufferPair);
extern void setupBusBuffer(tBufferPair *ptrBufferPair);
extern void runRxTxStaging(tBufferPair *ptrBufferPair);
extern uint8_t isRxStageFull(tBufferPair *ptrBufferPair);
extern uint8_t isTxStageFull(tBufferPair *ptrBufferPair);
extern uint8_t isRxDataReadyToRead(tBufferPair *ptrBufferPair);
extern uint8_t isTxDataReadyToRead(tBufferPair *ptrBufferPair);

#endif	/* BUS_BUFFER_H */

