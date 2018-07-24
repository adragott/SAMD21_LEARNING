/**
 * \file
 *
 * \brief SAM USART Quick Start
 *
 * Copyright (C) 2012-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <stdio.h>

void usart_read_callback(struct usart_module *const usart_module);
void usart_write_callback(struct usart_module *const usart_module);

void configure_usart(void);
void configure_usart_callbacks(void);


struct usart_module usart_instance;
struct usart_module gps_mod;

#define USART_INIT_MSSG "USART INITIALIZED\n"

#define SUPERBUFF_LENGTH 1024

//this defines how many characters your temp buffer can receive/send at a time
#define MAX_RX_BUFFER_LENGTH   1
// NOTE:
// this isn't actually how many your rx pin gets at a time. That is still only 1, but it decides how many to
// 	store in its buffer until it fires an interrupt flag letting you know you have data. It's safest to keep this at one so it interrupts
// 	at every character, but you can optimize your code using these chunks instead of every character


volatile uint8_t debug_buff[MAX_RX_BUFFER_LENGTH];
volatile uint8_t gps_buff[MAX_RX_BUFFER_LENGTH];

uint8_t superBuff[SUPERBUFF_LENGTH];
uint16_t ctr = 0;

//this is useless until we set up other things. not important
//void usart_read_debugcb(struct usart_module *const usart_module)
//{
	//usart_write_buffer_wait(&usart_instance, (uint8_t *)debug_buff, MAX_RX_BUFFER_LENGTH);
//}

//This handles the reception of data for the gps usart module
void usart_read_gpscb(struct usart_module *const usart_module)
{
	/*this code writes the whole temp buffer
		Note: casting this down to uint8_t is only necessary if using 9 bits. We aren't, so it's not necessary (but still recommended by atmel)
		
		usart_write_buffer_job(&usart_instance, (uint8_t*)rx_buffer2, MAX_RX_BUFFER_LENGTH) <==> usart_write_buffer_job(&usart_instance, rx_buffer2, MAX_RX_BUFFER_LENGTH)

																				^^ These are the same thing ^^															
	*/
	
	//!If you just wanted a simple way to print a character as it came in, here would be the best way:
	//usart_write_job(&usart_instance, &gps_buff[0]);
	
	//Here we simply pass the address of the 0th element of the array. This is the same as ==> usart_write_job(&usart_instance, gps_buff);
	//but ONLY in this instance because it's only looking for a single element anyways and our buffer can only have 1 element inside of it at once.
	//It's better that we explicitly say we're passing the address of the 0th element to avoid confusion
	//This is my preferred way of doing this. Reading character by character is the easiest way to parse in C
	
															

	//here we store a single character inside of superBuff, an array declared outside of our isr
	superBuff[ctr] = gps_buff[0];
	//this writes a single character
	usart_write_job(&usart_instance, &superBuff[ctr++]);	//This will work but only up to 1024 characters because this is the max size I have made the array.
															//It's up to you to figure out how to go about storing the data without allocating memory and while handling
															//a seemingly endless stream of data.
															
															//The point of this example was to show you how to take the character (or characters if you choose to go by buffer packers)
															//out of the temporary buffer and into a more permanent buffer for use outside of the isr
															
	//if you want to ensure that this code works, here's a super botched way of using a giant buffer forever (comment out above code or else this won't work)
	//superBuff[ctr >= (SUPERBUFF_LENGTH - 1) ? ctr=0 : ctr] = gps_buff[0];
	//usart_write_job(&usart_instance, &superBuff[ctr++]);
}

void configure_usart(void)
{

	struct usart_config config_usart;
	struct usart_config gps_conf;
	
	usart_get_config_defaults(&gps_conf);
	usart_get_config_defaults(&config_usart);

	config_usart.baudrate    = 9600;
	config_usart.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	config_usart.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	config_usart.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	config_usart.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	config_usart.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
	
	gps_conf.baudrate    = 9600;
	gps_conf.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	gps_conf.pinmux_pad0 = PINMUX_PB08D_SERCOM4_PAD0;
	gps_conf.pinmux_pad1 = PINMUX_PB09D_SERCOM4_PAD1;
	gps_conf.pinmux_pad2 = PINMUX_UNUSED;
	gps_conf.pinmux_pad3 = PINMUX_UNUSED;
	
	//init and enable Debug usart
	usart_init(&usart_instance, EDBG_CDC_MODULE, &config_usart);
	usart_enable(&usart_instance);
	usart_write_buffer_wait(&usart_instance, (uint8_t*)USART_INIT_MSSG, sizeof(USART_INIT_MSSG));
	//init and enable gps usart
	usart_init(&gps_mod, SERCOM4, &gps_conf);
	usart_enable(&gps_mod);

}

void configure_usart_callbacks(void)
{
	//not important yet
	//usart_register_callback(&usart_instance, usart_read_debugcb, USART_CALLBACK_BUFFER_RECEIVED);
	//usart_enable_callback(&gps_mod, USART_CALLBACK_BUFFER_RECEIVED);
	
	usart_register_callback(&gps_mod, usart_read_gpscb, USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&gps_mod, USART_CALLBACK_BUFFER_RECEIVED);
	

}

int main(void)
{
	system_init();

	configure_usart();
	configure_usart_callbacks();

	system_interrupt_enable_global();

	for(;;)
	{
		//This checks our usart buffer for received chars I'm PRETTY SURE. I could be wrong. It has an assert for rx_data so if rx_data is empty nothing happens. So no data == no interrupt trigger
		usart_read_buffer_job(&gps_mod, (uint8_t*)gps_buff, MAX_RX_BUFFER_LENGTH);
	}
}

