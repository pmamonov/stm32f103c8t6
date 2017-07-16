/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "cdcio.h"

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5

uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
uint8_t USB_Tx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
uint8_t  USB_Tx_State = 0;

void stdout_to_usb();

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback (void)
{
  if (USB_Tx_State == 1) stdout_to_usb();
}

/*******************************************************************************
* Function Name  : EP3_OUT_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/

void EP3_OUT_Callback(void)
{
  uint16_t USB_Rx_Cnt;
  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = USB_SIL_Read(EP3_OUT, USB_Rx_Buffer);
  cdc_write_buf(&cdc_in, USB_Rx_Buffer, USB_Rx_Cnt, 0);
  SetEPRxValid(ENDP3);
}

/*******************************************************************************
* Function Name  : Handle_USBAsynchXfer.
* Description    : send data to USB.
* Input          : None.
* Return         : none.
*******************************************************************************/
void Handle_USBAsynchXfer (void) {
  if(USB_Tx_State != 1){
    USB_Tx_State = 1;
    stdout_to_usb();
  }
}

void stdout_to_usb(){
  uint16_t USB_Tx_length;
  USB_Tx_length = cdc_read_buf(&cdc_out, USB_Tx_Buffer, VIRTUAL_COM_PORT_DATA_SIZE);
  if (USB_Tx_length){
    UserToPMABufferCopy(USB_Tx_Buffer, ENDP1_TXADDR, USB_Tx_length);
    SetEPTxCount(ENDP1, USB_Tx_length);
    SetEPTxValid(ENDP1);
  }
  else{
    USB_Tx_State = 0;
  }
}

/*******************************************************************************
* Function Name  : SOF_Callback / INTR_SOFINTR_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
#ifdef STM32F10X_CL
void INTR_SOFINTR_Callback(void)
#else
void SOF_Callback(void)
#endif /* STM32F10X_CL */
{
  static uint32_t FrameCount = 0;
  
  if(bDeviceState == CONFIGURED)
  {
    if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL)
    {
      /* Reset the frame counter */
      FrameCount = 0;
      
      /* Check the data to be sent through IN pipe */
      Handle_USBAsynchXfer();
    }
  }  
}
