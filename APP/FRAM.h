#ifndef _FRAM_H_
#define _FRAM_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

#define FRAM_SPI                                  SPI1
#define FRAM_SPI_CLK                              RCC_APB2Periph_SPI1
#define FRAM_CS_PIN                               GPIO_Pin_0                 
#define FRAM_CS_GPIO_PORT                         GPIOB                      
#define FRAM_CS_GPIO_CLK                          RCC_APB2Periph_GPIOB
#define FRAM_SPI_SCK_PIN                          GPIO_Pin_5                
#define FRAM_SPI_SCK_GPIO_PORT                    GPIOA                       
#define FRAM_SPI_SCK_GPIO_CLK                     RCC_APB2Periph_GPIOA
#define FRAM_SPI_MISO_PIN                         GPIO_Pin_6                 
#define FRAM_SPI_MISO_GPIO_PORT                   GPIOA                     
#define FRAM_SPI_MISO_GPIO_CLK                    RCC_APB2Periph_GPIOA
#define FRAM_SPI_MOSI_PIN                         GPIO_Pin_7                 
#define FRAM_SPI_MOSI_GPIO_PORT                   GPIOA                     
#define FRAM_SPI_MOSI_GPIO_CLK                    RCC_APB2Periph_GPIOA

#define FRAM_CS_LOW()                             GPIO_ResetBits(FRAM_CS_GPIO_PORT, FRAM_CS_PIN)
#define FRAM_CS_HIGH()                            GPIO_SetBits(FRAM_CS_GPIO_PORT, FRAM_CS_PIN)

void FRAM_init(void);
void set_write_enable_latch(void);
void reset_write_enable_latch(void);
u8 read_status_reg(void);
void write_status_reg(u8 val);
void read_memory_code(u16 addr, u8 *buf, u16 length);
void write_memory_code(u16 addr, u8 *buf, u16 length);
void read_device_ID(u8 *buf);
void fast_read_memory_code(u16 addr, u8 *buf, u16 length);
#endif