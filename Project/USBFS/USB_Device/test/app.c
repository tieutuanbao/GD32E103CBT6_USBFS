/*!
    \file  app.c
    \brief USB main routine for HID device(USB keyboard)
    
    \version 2017-12-26, V1.0.0, firmware for GD32E10x
*/

/*
    Copyright (c) 2017, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include  "custom_hid_core.h"
#include  "delay.h"

uint8_t timer_prescaler = 0;
uint32_t usbfs_prescaler = 0;

usb_core_handle_struct usbhs_core_dev = 
{
    .dev = 
    {
        .dev_desc = (uint8_t *)&device_descripter,
        .config_desc = (uint8_t *)&configuration_descriptor,
        .strings = usbd_strings,
        .class_init = custom_hid_init,
        .class_deinit = custom_hid_deinit,
        .class_req_handler = custom_hid_req_handler,
        .class_data_handler = custom_hid_data_handler
    },

    .udelay = delayMicroseconds,
    .mdelay = delay
};

void usb_clock_config(void);
void usb_gpio_config(void);
void usb_interrupt_config(void);

void led_config(void);

/*!
    \brief      main routine will construct a USB keyboard
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	
	/* configure led */
	led_config();
	gpio_bit_set(GPIOC, GPIO_PIN_13);
	
	/* configure USB clock */
    usb_clock_config();
  
    /* timer nvic initialization */
    // timer_nvic_init();

    /* USB device stack configure */
    usbd_init(&usbhs_core_dev, USB_FS_CORE_ID);

    /* USB interrupt configure */
    usb_interrupt_config();

    /* check if USB device is enumerated successfully */
    while (usbhs_core_dev.dev.status != USB_STATUS_CONFIGURED) {
    }

    while (1) {
    }
}

/*!
    \brief      configure USB clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usb_clock_config(void)
{
#if 1
    uint32_t system_clock = rcu_clock_freq_get(CK_SYS);
    if (48000000U == system_clock) {
        usbfs_prescaler = RCU_CKUSB_CKPLL_DIV1;
    } else if (72000000U == system_clock) {
        usbfs_prescaler = RCU_CKUSB_CKPLL_DIV1_5;
    } else if (96000000U == system_clock) {
        usbfs_prescaler = RCU_CKUSB_CKPLL_DIV2;
    } else if (120000000U == system_clock) {
        usbfs_prescaler = RCU_CKUSB_CKPLL_DIV2_5;
    }  else {
        /*  reserved  */
    }
    rcu_usb_clock_config(usbfs_prescaler);
#else	
    //rcu_usb_clock_config(usbfs_prescaler);
	/* cau hinh su dung dao dong noi USB */
	RCU_ADDCTL |= RCU_ADDCTL_IRC48MEN;
	
	while ((RCU_ADDCTL & RCU_ADDCTL_IRC48MSTB) != RCU_ADDCTL_IRC48MSTB) {
	}
	
	rcu_periph_clock_enable(RCU_CTC);
	ctc_hardware_trim_mode_config(CTC_HARDWARE_TRIM_MODE_ENABLE);
	ctc_counter_enable();
	rcu_ck48m_clock_config(RCU_CK48MSRC_IRC48M);
#endif
    rcu_periph_clock_enable(RCU_USBFS);
}

/*!
    \brief      configure USB interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usb_interrupt_config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);

    nvic_irq_enable((uint8_t)USBFS_IRQn, 2U, 0U);

#ifdef USBFS_LOW_PWR_MGMT_SUPPORT

    /* enable the power module clock */
    rcu_periph_clock_enable(RCU_PMU);

    /* USB wakeup EXTI line configuration */
    exti_interrupt_flag_clear(EXTI_18);
    exti_init(EXTI_18, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_enable(EXTI_18);

    nvic_irq_enable((uint8_t)USBFS_WKUP_IRQn, 1U, 0U);

#elif defined(USBHS_LOW_PWR_MGMT_SUPPORT)

    /* enable the power module clock */
    rcu_periph_clock_enable(RCU_PMU);

    /* USB wakeup EXTI line configuration */
    exti_interrupt_flag_clear(EXTI_20);
    exti_init(EXTI_20, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_enable(EXTI_20);

    nvic_irq_enable((uint8_t)USBHS_WKUP_IRQn, 1U, 0U);

#endif /* USBHS_LOW_PWR_MGMT_SUPPORT */
}

/*!
    \brief      configure key
    \param[in]  none
    \param[out] none
    \retval     none
*/

/*!
    \brief      configure the leds
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
	gpio_bit_set(GPIOC, GPIO_PIN_13);
}
