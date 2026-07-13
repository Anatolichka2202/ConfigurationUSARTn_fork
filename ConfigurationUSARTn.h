#ifndef ConfigurationUSARTn_H
#define ConfigurationUSARTn_H
/*
#include "ConfigurationUSARTn.h"


create by Sikorsky VI
*/
//uncomment your processor
//#define GD32F303_
#define GD32F450_

#define SENDING_VIA_USART // functions of sending via USART without DMA

#define ARRAYNUM(arr_nanme)      (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))
#define USARTn_DATA_ADDRESS(USARTn)    ((uint32_t)&USART_DATA(USARTn))

enum confInterrupt
{
    non             = 0x00, // without interruptions
    receiveRBNE     = 0x01, // reception interruption
    transmissionTBE = 0x02, // transmitter buffer empty interrupt
    transmissionTC  = 0x04, // transmission complete interrupt
};
enum confInterruptDMAReceiv
{
      iRQn_non_Rx          = 0x00,  //- without interruptions
      iRQn_receiv_Uart     = 0x80,  //- reception interruption Usart
      iRQn_half_receiv_Dma = 0x01,  //- reception interruption thith dma half transfer
      iRQn_full_receiv_Dma = 0x08,  //- reception interruption thith dma full transfer
};
enum confInterruptDMATransmit
{
    iRQn_non_Tx            = 0x00, //- without interruptions
    iRQn_half_transmit_Dma = 0x01, //- transmition interruption thith dma half transfer
    iRQn_full_transmit_Dma = 0x08  //- transmition interruption thith dma full transfer
};

#ifdef GD32F303_
#include "gd32f30x.h" 
/* Example

#define USARTx_PORT         GPIOA
#define USARTx_PIN_TX       GPIO_PIN_9
#define USARTx_PIN_RX       GPIO_PIN_10

#define USARTx_PORT_SWITCH	GPIOA
#define USARTx_PIN_SWITCH   GPIO_PIN_11

#define USARTx_RX           gpio_bit_reset( USARTx_PORT, USARTx_PIN_SWITCH ) // from rs485
#define USARTx_TX           gpio_bit_set  ( USARTx_PORT, USARTx_PIN_SWITCH ) // from rs485

  //// ====================  USART? ====================  
  gpio_init(USART?_CK_PORT, GPIO_MODE_OUT_PP,       GPIO_OSPEED_50MHZ,  USART?_CK_PIN);
  gpio_init(USART?_PORT,    GPIO_MODE_AF_PP,        GPIO_OSPEED_50MHZ,  USART?_TX_PIN);
  gpio_init(USART?_PORT,    GPIO_MODE_IN_FLOATING,  GPIO_OSPEED_50MHZ,  USART?_RX_PIN); 
  
    ConfigUsart       (USART?,BAUDRATE,USART_MSBF_LSB,non,1,1);
    ConfigUsartDMA_Tx (Usart?,&tx_buffer,TX_BUFFER_SIZE,0,DMA_PRIORITY_HIGH,0,4,iRQn_full_transmit_Dma);
    ConfigUsartDMA_Rx (Usart?,&rx_buffer,RX_BUFFER_SIZE,0,DMA_PRIORITY_HIGH,0,3,iRQn_full_receiv_Dma);
*/
enum usartDMA
{
    Usart0 = USART0,
    Usart1 = USART1,
    Uart3 = UART3
};
/*!
 \arg when using dma, you first call ConfigUsart, then ConfigDmaFromReceivUsart

    \brief      config USARTx
    \param[in]  usart_periph: USARTx(x=0,1,2)/UARTx(x=3,4)
    \param[in]  baudrate: USART0 <=5000000 USARTx(x=1,2)/UARTx(x=3,4) <=~2500000
    \param[in]  msbf: LSB/MSB
                only one parameter can be selected which is shown as below:
      \arg       USART_MSBF_LSB: LSB first
      \arg       USART_MSBF_MSB: MSB first
    \param[in]  configIrqn: bitmask from using an interrupt    (enum confInterrupt)
                only one parameter can be selected which is shown as below:
      \arg       non             - without interruptions
      \arg       receivRBNE      - reception interruption
      \arg       transmissionTBE - transmitter buffer empty interrupt
      \arg       transmissionTC  - ransmission complete interrupt
    \param[in]  priority:     using an interrupt
    \param[in]  sub_priority: using an interrupt
    \param[out] none
    \retval     none
*/
void ConfigUsart(uint32_t usart, uint32_t baudrate, uint32_t msbf, uint8_t configIrqn, uint8_t priority, uint8_t sub_priority);

/*!
 \arg when using dma, you first call ConfigUsart, then ConfigUsartDMA_Tx
    \brief      enable DMAx
    \param[in]  usart: usart_periph
    \param[in]  buf: buffer for received data
    \param[in]  lenBuf: size buffer
    \param[in]  circulationEnable: 1/0
    \param[in]  channelPriorityDMA:
                only one parameter can be selected which is shown as below:
      \arg       DMA_PRIORITY_LOW, DMA_PRIORITY_MEDIUM, DMA_PRIORITY_HIGH, DMA_PRIORITY_ULTRA_HIGH
    \param[in]  priority:     using an interrupt
    \param[in]  sub_priority: using an interrupt
    \param[in]  iRQn: bitmask from using an interrupt    (enum confInterruptTransmit)
      \arg       iRQn_non_Tx            - without interruptions
      \arg       iRQn_half_transmit_Dma - transmition interruption thith dma half transfer
      \arg       iRQn_full_transmit_Dma - transmition interruption thith dma full transfer
    \param[out] none
    \retval     none
*/
void ConfigUsartDMA_Tx(enum usartDMA  usart, uint32_t* buf, uint32_t lenBuf, _Bool circulationEnable,
    uint32_t channelPriorityDMA, uint8_t priority, uint8_t sub_priority, uint8_t iRQn);
/*!
 \arg when using dma, you first call ConfigUsart, then ConfigUsartDMA_Rx
    \brief      enable DMAx
    \param[in]  usart: usart_periph
    \param[in]  buf: buffer for received data
    \param[in]  lenBuf: size buffer
    \param[in]  circulationEnable: 1/0
    \param[in]  channelPriorityDMA:
                only one parameter can be selected which is shown as below:
      \arg       DMA_PRIORITY_LOW, DMA_PRIORITY_MEDIUM, DMA_PRIORITY_HIGH, DMA_PRIORITY_ULTRA_HIGH
    \param[in]  priority:     using an interrupt
    \param[in]  sub_priority: using an interrupt
    \param[in]  iRQn: bitmask from using an interrupt    (enum confInterruptreceiv)
      \arg       iRQn_non_Rx          - without interruptions
      \arg       iRQn_receiv_Uart     - reception interruption Usart
      \arg       iRQn_half_receiv_Dma - reception interruption thith dma half transfer
      \arg       iRQn_full_receiv_Dma - reception interruption thith dma full transfer
    \param[out] none
    \retval     none
*/
void ConfigUsartDMA_Rx(enum usartDMA  usart, uint32_t* buf, uint32_t lenBuf, _Bool circulationEnable,
    uint32_t channelPriorityDMA, uint8_t priority, uint8_t sub_priority, uint8_t iRQn);

void USART0_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void UART3_IRQHandler(void);
void UART4_IRQHandler(void);
/* ex
void USART?_IRQHandler(void) // "?" = You number usart
{
    if (usart_interrupt_flag_get  (USART?, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(USART?, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (USART?, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(USART?, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART?, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART?, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART?, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(USART?, USART_INT_FLAG_TC);
    }
};
void UART?_IRQHandler(void)/ / "?" = You number uart
{
    if (usart_interrupt_flag_get  (UART?, USART_INT_FLAG_RBNE) != RESET) {
        usart_interrupt_flag_clear(UART?, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (UART?, USART_INT_FLAG_TBE)  != RESET) {
        usart_interrupt_flag_clear(UART?, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (UART?, USART_INT_FLAG_TC)   != RESET) {
        usart_interrupt_flag_clear(UART?, USART_INT_FLAG_TC);
    }
};
*/

void DMA0_Channel3_IRQHandler(void);//USART0 TX
void DMA0_Channel4_IRQHandler(void);//USART0 RX

void DMA0_Channel6_IRQHandler(void);//USART1 TX
void DMA0_Channel5_IRQHandler(void);//USART1 RX

void DMA1_Channel3_4_IRQHandler(void);//UART3 TX 
void DMA1_Channel2_IRQHandler(void);  //UART3 RX
/* ex
void DMA-_Channel?_IRQHandler(void) // "?" = You number dma "^" = You dma channel
{
  if (dma_flag_get  (DMA?, DMA_CH^,DMA_FLAG_HTF)!= RESET) {
      dma_flag_clear(DMA?, DMA_CH^,DMA_FLAG_HTF);
    
  }
  if (dma_flag_get  (DMA?, DMA_CH^,DMA_FLAG_FTF)!= RESET) {
      dma_flag_clear(DMA?, DMA_CH^,DMA_FLAG_FTF);
    
  }
};

 reset DMA channel to start transmit or reciev
{
		// Channel disable 
		dma_channel_disable(DMA?, DMA_CH?);
		
		dma_memory_address_config(DMA?, DMA_CH?,(uint32_t)data_buffer);
		dma_transfer_number_config(DMA?,DMA_CH?,length);
		
		// enable DMA channel to start tx or rx
		dma_channel_enable(DMA?, DMA_CH?);
}
*/
#endif
#ifdef GD32F450_
#include "gd32f4xx.h"
/* Example
// ==================== USART7 ====================
#define UART7_PORT                     GPIOE
#define UART7_TX_PIN                   GPIO_PIN_1
#define UART7_RX_PIN                   GPIO_PIN_0
#define UART7_CK_PIN                   GPIO_PIN_2
#define UART7_AF                       GPIO_AF_8

  //// ====================  USART? ====================  
  gpio_mode_set           (USART?_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,  USART?_CK_PIN);
  gpio_output_options_set (USART?_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  USART?_CK_PIN);

  gpio_af_set             (USART?_PORT, USART?_AF,                         USART?_TX_PIN);
  gpio_mode_set           (USART?_PORT, GPIO_MODE_AF,   GPIO_PUPD_PULLUP,  USART?_TX_PIN);
  gpio_output_options_set (USART?_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  USART?_TX_PIN);
  
  gpio_af_set             (USART?_PORT, USART?_AF,                         USART?_RX_PIN);
  gpio_mode_set           (USART?_PORT, GPIO_MODE_AF,     GPIO_PUPD_NONE,  USART?_RX_PIN); 
  gpio_output_options_set (USART?_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  USART?_RX_PIN);  
  
  ConfigUsart       (USART?,USART_BAUDRATE,USART_MSBF_MSB,USART_OVSMOD_8,non,0,5);
  ConfigUsartDMA_Tx (USART?,buffer_tx,BUFFER_SIZE,0,DMA_PRIORITY_HIGH,0,6,iRQn_full_transmit_Dma);
*/
/*!
 \arg when using dma, you first call ConfigUsart, then ConfigDmaFromreceivUsart

    \brief      config USARTx
    \param[in]  usart: USARTx(x=0,1,2)/UARTx(x=3,4)
    \param[in]  oversample:
      \arg      USART_OVSMOD_8
      \arg      USART_OVSMOD_16
    \param[in]  baudrate:
    \param[in]  msbf: LSB/MSB
                only one parameter can be selected which is shown as below:
      \arg       USART_MSBF_LSB: LSB first
      \arg       USART_MSBF_MSB: MSB first
    \param[in]  oversamp: oversample value
                only one parameter can be selected which is shown as below:
      \arg        USART_OVSMOD_8:  8 bits
      \arg        USART_OVSMOD_16: 16 bits
    \param[out] none
    \retval     none
    \param[in]  configIrqn: bitmask from using an interrupt    (enum confInterrupt)
                only one parameter can be selected which is shown as below:
      \arg       non             - without interruptions
      \arg       receiveRBNE     - reception interruption
      \arg       transmissionTBE - transmitter buffer empty interrupt
      \arg       transmissionTC  - ransmission complete interrupt
      \arg       receivOnDMA     - reception interruption thith dma (only USARTx(x=0,1)/UART3)
    \param[in]  priority:     using an interrupt
    \param[in]  sub_priority: using an interrupt
    \param[out] none
    \retval     none

    \example: ConfigUsart       (UART7, USART_BAUDRATE,USART_MSBF_LSB,USART_OVSMOD_8,receiveRBNE,0,5); 
*/
void ConfigUsart(uint32_t usart, uint32_t baudrate, uint32_t msbf, uint32_t oversample, uint8_t configIrqn, uint8_t priority, uint8_t sub_priority);
/*!
 \arg when using dma, you first call ConfigUsart, then ConfigUsartDMA_Tx
    \brief      enable DMAx
    \param[in]  usart: usart_periph
    \param[in]  buf: buffer for received data
    \param[in]  lenBuf: size buffer
    \param[in]  circulationEnable: 1/0
    \param[in]  channelPriorityDMA:
                only one parameter can be selected which is shown as below:
      \arg       DMA_PRIORITY_LOW, DMA_PRIORITY_MEDIUM, DMA_PRIORITY_HIGH, DMA_PRIORITY_ULTRA_HIGH
    \param[in]  priority:     using an interrupt
    \param[in]  sub_priority: using an interrupt
    \param[in]  iRQn: bitmask from using an interrupt    (enum confInterruptTransmit)
      \arg       iRQn_non_Tx            - without interruptions
      \arg       iRQn_half_transmit_Dma - transmition interruption thith dma half transfer
      \arg       iRQn_full_transmit_Dma - transmition interruption thith dma full transfer
    \param[out] none
    \retval     none

    \example:     ConfigUsartDMA_Tx (UART7, usartX_buffer_tx,USART_BUFFER_SIZE,0,DMA_PRIORITY_HIGH,0,6,iRQn_full_transmit_Dma);
*/
void ConfigUsartDMA_Tx(uint32_t usart, uint8_t* buf, uint32_t lenBuf, _Bool circulationEnable,
    uint32_t channelPriorityDMA, uint8_t priority, uint8_t sub_priority, uint8_t iRQn);
/*!
 \arg when using dma, you first call ConfigUsart, then ConfigUsartDMA_Rx
    \brief      enable DMAx
    \param[in]  usart: usart_periph
    \param[in]  buf: buffer for received data
    \param[in]  lenBuf: size buffer
    \param[in]  circulationEnable: 1/0
    \param[in]  channelPriorityDMA:
                only one parameter can be selected which is shown as below:
      \arg        DMA_PRIORITY_LOW, DMA_PRIORITY_MEDIUM, DMA_PRIORITY_HIGH, DMA_PRIORITY_ULTRA_HIGH
    \param[in]  priority:     using an interrupt
    \param[in]  sub_priority: using an interrupt
    \param[in]  iRQn: bitmask from using an interrupt    (enum confInterruptreceiv)
      \arg       iRQn_non_Rx          - without interruptions
      \arg       iRQn_receiv_Uart     - reception interruption Usart
      \arg       iRQn_half_receiv_Dma - reception interruption thith dma half transfer
      \arg       iRQn_full_receiv_Dma - reception interruption thith dma full transfer
    \param[out] none
    \retval     none

    \example:     ConfigUsartDMA_Rx (UART7, buffer_Rx,USART_BUFFER_SIZE,0,DMA_PRIORITY_HIGH,0,6,iRQn_full_receiv_Dma);
*/
void ConfigUsartDMA_Rx(uint32_t usart, uint8_t* buf, uint32_t lenBuf, _Bool circulationEnable,
    uint32_t channelPriorityDMA, uint8_t priority, uint8_t sub_priority, uint8_t iRQn);

void USART0_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void UART3_IRQHandler(void);
void UART4_IRQHandler(void);
void USART5_IRQHandler(void);
void UART6_IRQHandler(void);
void UART7_IRQHandler(void);
/* ex
void USART?_IRQHandler(void) // "?" = You number usart
{
    if (usart_interrupt_flag_get  (USART?, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(USART?, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (USART?, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(USART?, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART?, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART?, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART?, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(USART?, USART_INT_FLAG_TC);
    }
};
void UART?_IRQHandler(void)/ / "?" = You number uart
{
    if (usart_interrupt_flag_get  (UART?, USART_INT_FLAG_RBNE) != RESET) {
        usart_interrupt_flag_clear(UART?, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (UART?, USART_INT_FLAG_TBE)  != RESET) {
        usart_interrupt_flag_clear(UART?, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (UART?, USART_INT_FLAG_TC)   != RESET) {
        usart_interrupt_flag_clear(UART?, USART_INT_FLAG_TC);
    }
};
*/

//void DMA0_Channel0_IRQHandler(void);// rx uart4    tx uart7
//void DMA0_Channel1_IRQHandler(void);// rx usart2   tx uart6
//void DMA0_Channel2_IRQHandler(void);// rx uart3
//void DMA0_Channel3_IRQHandler(void);// rx uart6    tx usart2
//void DMA0_Channel5_IRQHandler(void);// rx usart1
//void DMA0_Channel6_IRQHandler(void);// rx uart7    tx usart1
//void DMA0_Channel7_IRQHandler(void);//             tx uart4
//void DMA0_Channel4_IRQHandler(void);//             tx uart3

//void DMA1_Channel1_IRQHandler(void);// rx usart5
//void DMA1_Channel5_IRQHandler(void);// rx usart0
//void DMA1_Channel7_IRQHandler(void);//             tx usart0
//void DMA1_Channel6_IRQHandler(void);//             tx usart5
/* ex

void DMA-_Channel?_IRQHandler(void) // "?" = You number dma "^" = You dma channel
{
  if (dma_flag_get  (DMA?, DMA_CH^,DMA_FLAG_HTF)!= RESET) {
      dma_flag_clear(DMA?, DMA_CH^,DMA_FLAG_HTF);
    
  }
  if (dma_flag_get  (DMA?, DMA_CH^,DMA_FLAG_FTF)!= RESET) {
      dma_flag_clear(DMA?, DMA_CH^,DMA_FLAG_FTF);
    
  }
};

 reset DMA channel to start transmit or reciev
{
		//  Channel disable 
		dma_channel_disable       (DMA?, DMA_CH?);
		
    dma_flag_clear            (DMA?, DMA_CH?, DMA_FLAG_FTF);
    
    dma_memory_address_config (DMA?, DMA_CH?, 0, (uint32_t)buffer_tx);
    
    dma_transfer_number_config(DMA?, DMA_CH?, BUFFER_TX_SIZE);
    
    //  Channel enable 
    dma_channel_enable        (DMA?, DMA_CH?);
}


*/
#endif 

#if  defined(GD32F303_) && defined(GD32F450_)
#error Error ConfigurationUSARTn
#endif
#endif


#ifdef SENDING_VIA_USART
void Usart_send_byte  (const uint8_t byte, const uint32_t usart_perith);
void Usart_send_buf   (const void* const buf,    const uint32_t usart_perith, const uint32_t len);
void Usart_send_string(const void* const str,    const uint32_t usart_perith);
#endif
