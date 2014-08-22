
#include <csl_vp.h>
#include <csl_edma.h>
#include <csl_irq.h>


/*******************************************************************/
/* Function : bt656_8bit_ncfc 									   */
/* Input(s) : portNumber, video port number i.e. 0, 1 or 2.        */
/* Description : Configures given video port for 8.bit BT.656 non. */
/* continuos frame capture on channel A.                           */
/*******************************************************************/
VP_Handle bt656_8bit_ncfc( int portNumber);
/*******************************************************************/
/* Function : bt656_capture_start    							   */
/* Input(s) : VP_handle									           */
/* Description : Configures given video port for 8.bit BT.656 non. */
/* continuos frame capture on channel A.                           */
/*******************************************************************/
void bt656_capture_start(VP_Handle videoHandle);
/*................................................................ */
/* Function : VPCapChaAIsr */
/* Description : This capture ISR clears FRMC to continue capture */
/* in this non.continuous mode and also clears other */
/* status bits. */
/*................................................................ */
interrupt void VPCapChaAIsr(void);
/*................................................................ */
/* Function : setupVPCapChaAEDMA(Int32 portNumber) */
/* Input(s) : portNumber, video port number i.e. 0, 1 or 2. */
/* Description : Sets up EDMA channels for Y, U, V events for */
/* channel A capture. */
/*................................................................ */
void setupVPCapChaAEDMA(int portNumber);
/*................................................................ */
/* Function : configVPCapEDMAChannel */
/* */
/* Input(s) : edmaHandle . pointer to EDMA handle. */
/* eventId . EDMA eventId. */
/* tccNum . pointer to transfer complete number. */
/* srcAddr . source address for EDMA transfer. */
/* dstAddr . destination address for EDMA transfer */
/* frameCount . frame count. */
/* elementCount . element count(32.bit element size). */
/* */
/* Output(s): edmaHandle . edma Handle of the given event. */
/* tccNum . transfer complete code for the given */
/* event. */
/* */
/* Description : Configures the given VP capture EDMA channel. */
/* The source address update is fixed address mode */
/* because the captured data is read from the FIFO. */
/* In this example, the destination address mode is */
/* auto.increment. But, in real.time applications */
/* there is lot of flexibility in the way capture */
/* buffers can be managed like ping.pong and round */
/* robin,¡­etc. */
/*................................................................ */
void configVPCapEDMAChannel(EDMA_Handle *edmaHandle, Int32 eventId,
							Int32 *tccNum, Uint32 srcAddr,
							Uint32 dstAddr, Uint32 frameCount,
							Uint32 elementCount);

