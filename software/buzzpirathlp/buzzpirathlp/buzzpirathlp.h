#pragma once 

/*
	buzzpirathlp
	by Dreg @therealdreg https://github.com/therealdreg/ http://buzzpirat.com/
	-
	WARNING: ugly, buggy and crap code
	The program's design is quite messy, and I've only created it for my own needs
	-
	MIT LICENSE
 */

#include <windows.h>

// http://dangerousprototypes.com/docs/Bitbang

#define BHL_PERIPHERAL_DISABLE_ALL   0x40
#define BHL_PERIPHERAL_CS            0x41
#define BHL_PERIPHERAL_AUX           0x42
#define BHL_PERIPHERAL_PULLUPS       0x44
#define BHL_PERIPHERAL_PWR           0x48
#define BHL_I2C_SEND_START           0x02
#define BHL_I2C_SEND_STOP            0x03 
#define BHL_I2C_READ_BYTE            0x04
#define BHL_I2C_SEND_ACK             0x06
#define BHL_I2C_SEND_NACK            0x07
#define BHL_I2C_BULK_WRITE           0x10
#define BHL_I2C_400KHZ               0x63
#define BHL_I2C_100KHZ               0x62
#define BHL_I2C_50KHZ                0x61
#define BHL_I2C_5KHZ                 0x60
#define BHL_ENTER_RAW_BITBANG        0x00
#define BHL_ENTER_BIN_I2C            0x02
#define BHL_HARD_RESET               0x0F


#ifdef BUZZPIRATHLP_EXPORTS
#define BUZZPIRATHLP_API __declspec(dllexport)
#else
#define BUZZPIRATHLP_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif




BUZZPIRATHLP_API BOOL __stdcall bhl_com_create(const char* com_name);
BUZZPIRATHLP_API void __stdcall bhl_com_close(void);
BUZZPIRATHLP_API void __stdcall bhl_hard_reset(void);
BUZZPIRATHLP_API BOOL __stdcall bhl_set_peripherals(unsigned int peripherals_mask);
BUZZPIRATHLP_API BOOL __stdcall bhl_enter_raw_bitbang(void);
BUZZPIRATHLP_API BOOL __stdcall bhl_enter_bin_i2c(void);
BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_speed(unsigned int speed);
BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_eepr_set_2byteaddr(unsigned int chip_addr, unsigned int mem_addr);
BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_eepr_start_bulk(unsigned int chip_addr);
BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_eepr_bulk_read_byte(unsigned char* recv_buff, unsigned int ack_or_nack);
BUZZPIRATHLP_API unsigned int __stdcall bhl_i2c_eepr_stop_bulk(void);
BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_eepr_write_byte_2byteaddr(unsigned int chip_addr, unsigned int mem_addr, unsigned char byte_val, unsigned char* recv_buff);
BUZZPIRATHLP_API unsigned int __stdcall bhl_i2c_scan(unsigned char* detected_array);

BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_init(const char* com_name, unsigned int power, unsigned int pullups, unsigned int khz, unsigned int just_i2c_scanner);
BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_close(void);
BUZZPIRATHLP_API unsigned char* __stdcall bhl_asprog_i2c_get_memaux(void);
BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_readwrite(unsigned int devaddr, unsigned int size, unsigned char* buffwr, unsigned int size_buffwr);
BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_start(void);
BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_stop(void);
BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_read_byte(void);
BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_write_byte(unsigned int byte);


#ifdef __cplusplus
}
#endif