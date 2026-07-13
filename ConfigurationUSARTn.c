#include "ConfigurationUSARTn.h"
#include <stdlib.h>




#ifdef GD32F303_
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
    
    \example: ConfigUsart       (USART1,BAUDRATE_TU,USART_MSBF_LSB,non,1,4);
*/
void ConfigUsart(uint32_t usart, uint32_t baudrate, uint32_t msbf, uint8_t configIrqn, uint8_t priority, uint8_t sub_priority)
{
    //    if ((receiv & 0x01 == 0x01) && (receivOnDMA & 0x04 == 0x04)) {
    //      #error "You cannot select 2 reception parameters"
    //    }

    usart_deinit(usart);

    usart_word_length_set(usart, USART_WL_8BIT);
    usart_baudrate_set(usart, baudrate);
    usart_data_first_config(usart, msbf);

    usart_stop_bit_set(usart, USART_STB_1BIT);
    usart_parity_config(usart, USART_PM_NONE);

    usart_hardware_flow_rts_config(usart, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(usart, USART_CTS_DISABLE);

    usart_receive_config(usart, USART_RECEIVE_ENABLE);
    usart_transmit_config(usart, USART_TRANSMIT_ENABLE);

    if (receiveRBNE & configIrqn) {
        usart_interrupt_enable(usart, USART_INT_RBNE);
        usart_interrupt_flag_clear(usart, USART_INT_FLAG_RBNE);
    }
    if (transmissionTBE & configIrqn) {
        usart_interrupt_enable(usart, USART_INT_TBE);
        usart_interrupt_flag_clear(usart, USART_INT_FLAG_TBE);
    }
    if (transmissionTC & configIrqn) {
        usart_interrupt_enable(usart, USART_INT_TC);
        usart_interrupt_flag_clear(usart, USART_INT_FLAG_TC);
    }


    usart_enable(usart);

    if (configIrqn != non) {
        uint8_t usartX_irqn = 0;
        switch (usart)
        {
        case USART0:
        {
            usartX_irqn = USART0_IRQn;
            break;
        }
        case USART1:
        {
            usartX_irqn = USART1_IRQn;
            break;
        }
        case USART2:
        {
            usartX_irqn = USART2_IRQn;
            break;
        }
        case UART3:
        {
            usartX_irqn = UART3_IRQn;
            break;
        }
        case UART4:
        {
            usartX_irqn = UART4_IRQn;
            break;
        }
        default:
        {
            //#error Error parametr USART
            while(1){}
            break;
        }
        }

        nvic_irq_enable(usartX_irqn, priority, sub_priority);
    }


}

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
    
    \example: ConfigUsartDMA_Tx (Usart1,tx_buffer,TX_BUFFER_SIZE,0,DMA_PRIORITY_HIGH,1,4,iRQn_full_transmit_Dma);
*/
void ConfigUsartDMA_Tx(enum usartDMA  usart, uint32_t* buf, uint32_t lenBuf, _Bool circulationEnable,
    uint32_t channelPriorityDMA, uint8_t priority, uint8_t sub_priority, uint8_t iRQn)
{
       usart_disable       (usart);
    dma_channel_enum channelTx = 0;
    uint32_t dma_periph = 0;
    uint8_t  TX_irqn = 0;
    switch (usart)
    {
    case USART0:
    {
        dma_periph = DMA0;
        channelTx = DMA_CH3;
        break;
    }
    case USART1:
    {
        dma_periph = DMA0;
        channelTx = DMA_CH6;
        break;
    }
    case UART3:
    {
        dma_periph = DMA1;
        channelTx = DMA_CH4;
        break;
    }
    default:
    {
        while(1){}
        //#error there is no DMA on this USART
        break;
    }
    }


    // USART0 DMA receiving configuration
    dma_parameter_struct dma_init_struct;
    dma_deinit(dma_periph, channelTx);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;		  // Memory to peripheral  
    dma_init_struct.memory_addr = buf;			                  // Set the memory receiving base address
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;	// Memory address increasing
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;		  // 8 -bit memory data
    dma_init_struct.number = lenBuf;

    dma_init_struct.periph_addr = USARTn_DATA_ADDRESS(usart);	// Peripheral address, USART data register address
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;	// The peripheral address does not increase
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;	// 8 -bit external data
    dma_init_struct.priority = channelPriorityDMA;			// The highest DMA channel priority
    dma_init(dma_periph, channelTx, &dma_init_struct); 			// Initialize DMA according to the configuration of the structure

    if (circulationEnable) {
        dma_circulation_enable(dma_periph, channelTx);
    }
    else {
        dma_circulation_disable(dma_periph, channelTx);
    }

    dma_memory_to_memory_disable(dma_periph, channelTx);	        // DMA memory to the memory mode is not turned on

    usart_dma_transmit_config(usart, USART_DENT_ENABLE);
    if (iRQn != iRQn_non_Tx) {
        if (iRQn & iRQn_full_transmit_Dma) {
            dma_interrupt_enable    (dma_periph, channelTx, DMA_INT_FTF);
            dma_interrupt_flag_clear(dma_periph, channelTx, DMA_INT_FTF);
        }
        if (iRQn & iRQn_half_transmit_Dma) {
            dma_interrupt_enable    (dma_periph, channelTx, DMA_INT_HTF);
            dma_interrupt_flag_clear(dma_periph, channelTx, DMA_INT_HTF);
        }
       switch (usart)
       {
        case USART0:
        {
            TX_irqn = DMA0_Channel3_IRQn;
            break;
        }
        case USART1:
        {
            TX_irqn = DMA0_Channel6_IRQn;
            break;
        }
        case UART3:
        {
            TX_irqn = DMA1_Channel3_Channel4_IRQn;
            break;
        }
        default:
        {
            //#error there is no DMAn on this USART
            while(1){}
            break;
        }
       }
    }
    usart_enable(usart);
    dma_channel_enable(dma_periph, channelTx);

    if (iRQn != iRQn_non_Tx) {
        nvic_irq_enable(TX_irqn, priority, sub_priority);
    }
}



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
    
    \example: ConfigUsartDMA_Rx (Usart1,&rx_buffer,RX_BUFFER_SIZE,0,DMA_PRIORITY_HIGH,1,4,iRQn_full_receiv_Dma);
*/
void ConfigUsartDMA_Rx(enum usartDMA  usart, uint32_t* buf, uint32_t lenBuf, _Bool circulationEnable,
    uint32_t channelPriorityDMA, uint8_t priority, uint8_t sub_priority, uint8_t iRQn)
{
       usart_disable       (usart);
    dma_channel_enum channelRx = 0;
    uint32_t dma_periph = 0;
    uint8_t RX_irqn = 0;
    switch (usart)
    {
    case USART0:
    {
        dma_periph = DMA0;
        channelRx = DMA_CH4;
        break;
    }
    case USART1:
    {
        dma_periph = DMA0;
        channelRx = DMA_CH5;
        break;
    }
    case UART3:
    {
        dma_periph = DMA1;
        channelRx = DMA_CH2;
        break;
    }
    default:
    {
        //#error there is no DMA on this USART
        while(1){}
        break;
    }
    }
    dma_parameter_struct dma_init_struct;
    dma_deinit(dma_periph, channelRx);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;		// Peripheral to memory
    dma_init_struct.memory_addr = buf;			                // Set the memory receiving base address
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;	// Memory address increasing
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;		// 8 -bit memory data
    dma_init_struct.number = lenBuf;

    dma_init_struct.periph_addr = USARTn_DATA_ADDRESS(usart);	// Peripheral address, USART data register address
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;	// The peripheral address does not increase
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;	// 8 -bit external data

    dma_init_struct.priority = channelPriorityDMA;			// The highest DMA channel priority
    dma_init(dma_periph, channelRx, &dma_init_struct); 			// Initialize DMA according to the configuration of the structure

    if (circulationEnable) {
        dma_circulation_enable(dma_periph, channelRx);
    }
    else {
        dma_circulation_disable(dma_periph, channelRx);
    }

    dma_memory_to_memory_disable(dma_periph, channelRx);	        // DMA memory to the memory mode is not turned on

    usart_dma_receive_config(usart, USART_DENR_ENABLE);

    if (iRQn_receiv_Uart & iRQn) {
        usart_interrupt_enable(usart, USART_INT_IDLE);
        usart_interrupt_flag_clear(usart, USART_INT_FLAG_IDLE);
        switch (usart)
        {
        case USART0:
        {
            RX_irqn = USART0_IRQn;
            break;
        }
        case USART1:
        {
            RX_irqn = USART1_IRQn;
            break;
        }
        case UART3:
        {
            RX_irqn = UART3_IRQn;
            break;
        }
        default:
        {
            //#error there is no DMAn on this USART
            while(1){}
            break;
        }
        }
    }
	
    if (iRQn != iRQn_receiv_Uart && iRQn != iRQn_non_Rx) {
       switch (usart)
       {
        case USART0:
        {
            RX_irqn = DMA0_Channel4_IRQn;
            break;
        }
        case USART1:
        {
            RX_irqn = DMA0_Channel5_IRQn;
            break;
        }
        case UART3:
        {
            RX_irqn = DMA1_Channel2_IRQn;
            break;
        }
        default:
        {
            //#error there is no DMAn on this USART
            while(1){}
            break;
        }
        }
        if (iRQn & iRQn_full_receiv_Dma) {
            dma_interrupt_enable    (dma_periph, channelRx, DMA_INT_FTF);
            dma_interrupt_flag_clear(dma_periph, channelRx, DMA_INT_FTF);
        }
        if (iRQn & iRQn_half_receiv_Dma) {
            dma_interrupt_enable    (dma_periph, channelRx, DMA_INT_HTF);
            dma_interrupt_flag_clear(dma_periph, channelRx, DMA_INT_HTF);
        }
    }
	
    usart_enable(usart);
    dma_channel_enable(dma_periph, channelRx);
    if (iRQn_non_Rx != iRQn) {
        nvic_irq_enable(RX_irqn, priority, sub_priority);
    }
}


/*
void USART0_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (USART0, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (USART0, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART0, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART0, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TC);
    }

};
void USART1_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (USART1, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (USART1, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART1, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART1, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_TC);
    }
};
void USART2_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (USART2, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART2, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART2, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_TC);
    }
};
void UART3_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (UART3, USART_INT_FLAG_IDLE) != RESET) {
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (UART3, USART_INT_FLAG_RBNE) != RESET) {
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (UART3, USART_INT_FLAG_TBE)  != RESET) {
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (UART3, USART_INT_FLAG_TC)   != RESET) {
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_TC);
    }

};
void UART4_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (UART4, USART_INT_FLAG_RBNE) != RESET) {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (UART4, USART_INT_FLAG_TBE)  != RESET) {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (UART4, USART_INT_FLAG_TC)   != RESET) {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_TC);
    }
};

*/

#endif //end GD32F303_
#ifdef GD32F450_
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

void ConfigUsart(uint32_t usart, uint32_t baudrate, uint32_t msbf, uint32_t oversample, uint8_t configIrqn, uint8_t priority, uint8_t sub_priority)
{

    //    if ((receive & 0x01 == 0x01) && (receivOnDMA & 0x04 == 0x04)) {
    //#error "You cannot select 2 reception parameters"
    //    }
    switch (usart)
    {
    case USART0:
    {
        rcu_periph_clock_enable(RCU_USART0);
        break;
    }
    case USART1:
    {
        rcu_periph_clock_enable(RCU_USART1);
        break;
    }
    case USART2:
    {
        rcu_periph_clock_enable(RCU_USART2);
        break;
    }
    case UART3:
    {
        rcu_periph_clock_enable(RCU_UART3);
        break;
    }
    case UART4:
    {
        rcu_periph_clock_enable(RCU_UART4);
        break;
    }
    case USART5:
    {
        rcu_periph_clock_enable(RCU_USART5);
        break;
    }
    case UART6:
    {
        rcu_periph_clock_enable(RCU_UART6);
        break;
    }
    case UART7:
    {
        rcu_periph_clock_enable(RCU_UART7);
        break;
    }
    default:
    {
        //#error Error parametr USART confInterrupt
      while(1){}
        break;
    }
    }


    usart_deinit(usart);
    usart_data_first_config(usart, msbf);
    usart_oversample_config(usart, oversample);
    usart_baudrate_set(usart, baudrate);

    usart_word_length_set(usart, USART_WL_8BIT);
    usart_stop_bit_set(usart, USART_STB_1BIT);
    usart_parity_config(usart, USART_PM_NONE);

    usart_hardware_flow_rts_config(usart, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(usart, USART_CTS_DISABLE);

    usart_receive_config(usart, USART_RECEIVE_ENABLE);
    usart_transmit_config(usart, USART_TRANSMIT_ENABLE);

    if (receiveRBNE & configIrqn) {
        usart_interrupt_enable(usart, USART_INT_RBNE);
        usart_interrupt_flag_clear(usart, USART_INT_FLAG_RBNE);
    }
    if (transmissionTBE & configIrqn) {
        usart_interrupt_enable(usart, USART_INT_TBE);
        usart_interrupt_flag_clear(usart, USART_INT_FLAG_TBE);
    }
    if (transmissionTC & configIrqn) {
        usart_interrupt_enable(usart, USART_INT_TC);
        usart_interrupt_flag_clear(usart, USART_INT_FLAG_TC);
    }

    if (configIrqn != non) {
        uint8_t usartX_irqn = 0;
        switch (usart)
        {
        case USART0:
        {
            usartX_irqn = USART0_IRQn;
            break;
        }
        case USART1:
        {
            usartX_irqn = USART1_IRQn;
            break;
        }
        case USART2:
        {
            usartX_irqn = USART2_IRQn;
            break;
        }
        case UART3:
        {
            usartX_irqn = UART3_IRQn;
            break;
        }
        case UART4:
        {
            usartX_irqn = UART4_IRQn;
            break;
        }
        case USART5:
        {
            usartX_irqn = USART5_IRQn;
            break;
        }
        case UART6:
        {
            usartX_irqn = UART6_IRQn;
            break;
        }
        case UART7:
        {
            usartX_irqn = UART7_IRQn;
            break;
        }
        default:
        {
            //#error Error parametr USART confInterrupt
          while(1){}
            break;
        }
        }
        nvic_irq_enable(usartX_irqn, priority, sub_priority);
    }
       usart_enable       (usart);
}
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

         dma_channel_disable                  (DMA0, DMA_CH3  );                                // ��������� dma 
         dma_flag_clear                       (DMA0, DMA_CH3, DMA_FLAG_FTF );
         dma_channel_subperipheral_select     (DMA0, DMA_CH3, DMA_SUBPERI4 );
         dma_transfer_direction_config        (DMA0, DMA_CH3, DMA_MEMORY_TO_PERIPH );
         dma_memory_address_generation_config (DMA0, DMA_CH3, DMA_MEMORY_INCREASE_ENABLE );
         dma_priority_config                  (DMA0, DMA_CH3, DMA_PRIORITY_ULTRA_HIGH );
         dma_memory_address_config            (DMA0, DMA_CH3, DMA_MEMORY_0, (uint32_t)usartX_buffer_tx );  // ��������� ����� ������
         dma_periph_address_config            (DMA0, DMA_CH3, USARTn_DATA_ADDRESS(USART2) );
         dma_transfer_number_config           (DMA0, DMA_CH3, USART_BUFFER_SIZE );               // ��������� ���������� ������
         dma_channel_enable                   (DMA0, DMA_CH3  );                                // �������� dma ��� ��������
*/
void ConfigUsartDMA_Tx(uint32_t usart, uint8_t* buf, uint32_t lenBuf, _Bool circulationEnable,
    uint32_t channelPriorityDMA, uint8_t priority, uint8_t sub_priority, uint8_t iRQn)
{
       usart_disable       (usart);
    dma_channel_enum channelTx = 0;
    uint32_t dma_periph = 0;
    dma_subperipheral_enum dma_sub_periph = 0;
    IRQn_Type  TX_irqn = 0;

    switch (usart)
    {
    case USART0:
    {
        dma_periph = DMA1;
        channelTx = DMA_CH7;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case USART1:
    {
        dma_periph = DMA0;
        channelTx = DMA_CH6;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case USART2:
    {
        dma_periph = DMA0;
        channelTx = DMA_CH3;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case UART3:
    {
        dma_periph = DMA0;
        channelTx = DMA_CH4;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case UART4:
    {
        dma_periph = DMA0;
        channelTx = DMA_CH7;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case USART5:
    {
        dma_periph = DMA1;
        channelTx = DMA_CH6;
        dma_sub_periph = DMA_SUBPERI5;
        break;
    }
    case UART6:
    {
        dma_periph = DMA0;
        channelTx = DMA_CH1;
        dma_sub_periph = DMA_SUBPERI5;
        break;
    }
    case UART7:
    {
        dma_periph = DMA0;
        channelTx = DMA_CH0;
        dma_sub_periph = DMA_SUBPERI5;

        break;
    }
    default:
    {
        //throw  "there is no DMAn on thiis USART";
      while(1){}
        break;
    }
    }

    
    dma_single_data_parameter_struct dma_init_struct;
    /* USART0 DMA receiving configuration*/
    dma_channel_disable (dma_periph, channelTx);
    dma_deinit          (dma_periph, channelTx);

    dma_init_struct.direction           = DMA_MEMORY_TO_PERIPH;		    // Memory to peripheral  
    dma_init_struct.memory0_addr        = (uint32_t)buf;			        // Set the memory transmitting base address 
    dma_init_struct.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;	// Memory address increasing 
    dma_init_struct.number              = lenBuf;                     // size of buffer
    dma_init_struct.periph_addr         = USARTn_DATA_ADDRESS(usart); 
    dma_init_struct.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority            = channelPriorityDMA;
    if (circulationEnable){
        dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    }else{
        dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    }
    dma_single_data_mode_init       (dma_periph, channelTx, &dma_init_struct); 					// Initialize DMA */according to the configuration of the structure  
    dma_channel_subperipheral_select(dma_periph, channelTx,  dma_sub_periph);
    if (iRQn != iRQn_non_Tx) {
        if (iRQn & iRQn_full_transmit_Dma) {
            dma_interrupt_enable    (dma_periph, channelTx, DMA_INT_FTF);
            dma_interrupt_flag_clear(dma_periph, channelTx, DMA_INT_FTF);
        }
        if (iRQn & iRQn_half_transmit_Dma) {
            dma_interrupt_enable    (dma_periph, channelTx, DMA_INT_HTF);
            dma_interrupt_flag_clear(dma_periph, channelTx, DMA_INT_HTF);
        }
        switch (usart)
        {
        case USART0:
        {
            TX_irqn = DMA1_Channel7_IRQn;
            break;
        }
        case USART1:
        {
            TX_irqn = DMA0_Channel6_IRQn;
            break;
        }
        case USART2:
        {
            TX_irqn = DMA0_Channel3_IRQn;
            break;
        }
        case UART3:
        {
            TX_irqn = DMA0_Channel4_IRQn;
            break;
        }
        case UART4:
        {
            TX_irqn = DMA0_Channel7_IRQn;
            break;
        }
        case USART5:
        {
            TX_irqn = DMA1_Channel6_IRQn;
            break;
        }
        case UART6:
        {
            TX_irqn = DMA0_Channel1_IRQn;
            break;
        }
        case UART7:
        {
            TX_irqn = DMA0_Channel0_IRQn;
            break;
        }
        default:
        {
          while(1){}
            break;
        }
        }
    }
    dma_channel_enable(dma_periph, channelTx);
    usart_dma_transmit_config(usart, USART_TRANSMIT_DMA_ENABLE);
    if (iRQn_non_Tx != iRQn) {
        nvic_irq_enable(TX_irqn, priority, sub_priority);
    }
     usart_enable       (usart);
}
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
*/
void ConfigUsartDMA_Rx(uint32_t usart, uint8_t* buf, uint32_t lenBuf, _Bool circulationEnable,
    uint32_t channelPriorityDMA, uint8_t priority, uint8_t sub_priority, uint8_t iRQn)
{
       usart_disable       (usart);
    dma_channel_enum channelRx= 0;
    uint32_t dma_periph       = 0;
    dma_subperipheral_enum dma_sub_periph   = 0;
    IRQn_Type RX_irqn         = 0;

    switch (usart)
    {
    case USART0:
    {
        dma_periph = DMA1;
        channelRx = DMA_CH5;// 2 and 5 channels rx
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case USART1:
    {
        dma_periph = DMA0;
        channelRx = DMA_CH5;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case USART2:
    {
        dma_periph = DMA0;
        channelRx = DMA_CH1;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case UART3:
    {
        dma_periph = DMA0;
        channelRx = DMA_CH2;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case UART4:
    {
        dma_periph = DMA0;
        channelRx = DMA_CH0;
        dma_sub_periph = DMA_SUBPERI4;
        break;
    }
    case USART5:
    {
        dma_periph = DMA1;
        channelRx = DMA_CH1;// 1 and 2 channels rx
        dma_sub_periph = DMA_SUBPERI5;
        break;
    }
    case UART6:
    {
        dma_periph = DMA0;
        channelRx = DMA_CH3;
        dma_sub_periph = DMA_SUBPERI5;
        break;
    }
    case UART7:
    {
        dma_periph = DMA0;
        channelRx = DMA_CH6;
        dma_sub_periph = DMA_SUBPERI5;

        break;
    }
    default:
    {
        //throw  "there is no DMAn on thiis USART";
      while(1){}
        break;
    }
    }


    dma_single_data_parameter_struct dma_init_struct;
    /* USART0 DMA receiving configuration*/
    dma_deinit(dma_periph, channelRx);

    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;		// Peripheral to memory */
    dma_init_struct.memory0_addr = (uint32_t)buf;			// Set the memory receiving base address */
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;	// Memory address increasing */
    dma_init_struct.number = lenBuf;
    dma_init_struct.periph_addr = USARTn_DATA_ADDRESS(usart);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.priority = channelPriorityDMA;
    if (circulationEnable) {
        dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE;
    }
    else {
        dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
    }
    dma_single_data_mode_init(dma_periph, channelRx, &dma_init_struct); 					// Initialize DMA */according to the configuration of the structure  
    dma_channel_subperipheral_select(dma_periph, channelRx, dma_sub_periph);
    usart_dma_receive_config(usart, USART_RECEIVE_DMA_ENABLE);

    if (iRQn_receiv_Uart & iRQn) {
        switch (usart)
        {
        case USART0:
        {
            RX_irqn = USART0_IRQn;
            break;
        }
        case USART1:
        {
            RX_irqn = USART1_IRQn;
            break;
        }
        case USART2:
        {
            RX_irqn = USART2_IRQn;
            break;
        }
        case UART3:
        {
            RX_irqn = UART3_IRQn;
            break;
        }
        case UART4:
        {
            RX_irqn = UART4_IRQn;
            break;
        }
        case USART5:
        {
            RX_irqn = USART5_IRQn;
            break;
        }
        case UART6:
        {
            RX_irqn = UART6_IRQn;
            break;
        }
        case UART7:
        {
            RX_irqn = UART7_IRQn;
            break;
        }
        default:
        {
            //#error Error parametr USART confInterrupt
          while(1){}
            break;
        }
        }
        usart_interrupt_enable(usart, USART_INT_IDLE);
        usart_interrupt_flag_clear(usart, USART_INT_FLAG_IDLE);
    }
    if (iRQn != iRQn_receiv_Uart && iRQn != iRQn_non_Rx) {
        switch (usart)
        {
        case USART0:
        {
            RX_irqn = DMA1_Channel5_IRQn;
            break;
        }
        case USART1:
        {
            RX_irqn = DMA0_Channel5_IRQn;
            break;
        }
        case USART2:
        {
            RX_irqn = DMA0_Channel1_IRQn;
            break;
        }
        case UART3:
        {
            RX_irqn = DMA0_Channel2_IRQn;
            break;
        }
        case UART4:
        {
            RX_irqn = DMA0_Channel0_IRQn;
            break;
        }
        case USART5:
        {
            RX_irqn = DMA1_Channel1_IRQn;
            break;
        }
        case UART6:
        {
            RX_irqn = DMA0_Channel3_IRQn;
            break;
        }
        case UART7:
        {
            RX_irqn = DMA0_Channel6_IRQn;
            break;
        }
        default:
        {
          while(1){}
            break;
        }
        }

        if (iRQn & iRQn_full_receiv_Dma) {
            dma_interrupt_enable(dma_periph, channelRx, DMA_INT_FTF);
            dma_interrupt_flag_clear(dma_periph, channelRx, DMA_INT_FTF);
        }
        if (iRQn & iRQn_half_receiv_Dma) {
            dma_interrupt_enable(dma_periph, channelRx, DMA_INT_HTF);
            dma_interrupt_flag_clear(dma_periph, channelRx, DMA_INT_HTF);
        }
    }
    dma_channel_enable(dma_periph, channelRx);
    if (iRQn_non_Rx != iRQn) {
        nvic_irq_enable(RX_irqn, priority, sub_priority);
    }
       usart_enable       (usart);
}
/*
void USART0_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (USART0, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (USART0, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART0, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART0, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TC);
    }

};
void USART1_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (USART1, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (USART1, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART1, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART1, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(USART1, USART_INT_FLAG_TC);
    }
};
void USART2_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (USART2, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (USART2, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART2, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART2, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(USART2, USART_INT_FLAG_TC);
    }
};
void UART3_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (UART3, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (UART3, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (UART3, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (UART3, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(UART3, USART_INT_FLAG_TC);
    }
};
void UART4_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (UART4, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (UART4, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (UART4, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (UART4, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(UART4, USART_INT_FLAG_TC);
    }
};
void USART5_IRQHandler(void)
{
    if (usart_interrupt_flag_get  (USART5, USART_INT_FLAG_IDLE) != RESET) {
        usart_interrupt_flag_clear(USART5, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (USART5, USART_INT_FLAG_RBNE) != RESET) {
        usart_interrupt_flag_clear(USART5, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (USART5, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(USART5, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (USART5, USART_INT_FLAG_TC) != RESET) {
        usart_interrupt_flag_clear(USART5, USART_INT_FLAG_TC);
    }
};
void UART6_IRQHandler(void)
{

    if (usart_interrupt_flag_get  (UART6, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(UART6, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (UART6, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(UART6, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (UART6, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(UART6, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (UART6, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(UART6, USART_INT_FLAG_TC);
    }
};
void UART7_IRQHandler(void)
{

    if (usart_interrupt_flag_get  (UART7, USART_INT_FLAG_IDLE)!= RESET) {
        usart_interrupt_flag_clear(UART7, USART_INT_FLAG_IDLE);
    }
    if (usart_interrupt_flag_get  (UART7, USART_INT_FLAG_RBNE)!= RESET) {
        usart_interrupt_flag_clear(UART7, USART_INT_FLAG_RBNE);
    }
    if (usart_interrupt_flag_get  (UART7, USART_INT_FLAG_TBE) != RESET) {
        usart_interrupt_flag_clear(UART7, USART_INT_FLAG_TBE);
    }
    if (usart_interrupt_flag_get  (UART7, USART_INT_FLAG_TC)  != RESET) {
        usart_interrupt_flag_clear(UART7, USART_INT_FLAG_TC);
    }
};
*/
#endif //end GD32F450_
#if  defined(GD32F303_) && defined(GD32F450_)

#error error ConfigurationUSARTn

#endif //end defined(GD32F303_) && defined(GD32F450_)



#ifdef SENDING_VIA_USART

void Usart_send_byte(const   uint8_t  byte, const uint32_t usart_perith)
{
    // Send a byte data to USART
    usart_data_transmit(usart_perith, byte);

    // Waiting for the sending 
    while (usart_flag_get(usart_perith, USART_FLAG_TBE) == RESET);
}

void Usart_send_buf(const void* const buf, const uint32_t usart_perith, const uint32_t len)
{
    if (buf == 0) {
        return;
    }
    const uint8_t* pbuf = (const uint8_t*)buf;
    unsigned int k = 0;
    while (k < len){
      Usart_send_byte((pbuf[k++]), usart_perith);
    }
}

void Usart_send_string(const void* const str, const uint32_t usart_perith)
{
    if (str == 0) {
        return;
    }
    const uint8_t* pbuf = (const uint8_t*)str;
    uint32_t k = 0;
    while (pbuf[k] != '\0') {
        Usart_send_byte((pbuf[k++]), usart_perith);
    }
}

#endif // SENDING_VIA_USART

