/*
	buzzpirathlp
	by Dreg @therealdreg https://github.com/therealdreg/ http://buzzpirat.com/
	-
	WARNING: ugly, buggy and crap code
	The program's design is quite messy, and I've only created it for my own needs
	-
	MIT LICENSE
 */

#include <stdio.h>
#include <windows.h>
#include "buzzpirathlp.h"
#include "log_en.h"

typedef struct
{
	HANDLE handle;
} COMOP_t;

COMOP_t* volatile com_glb = NULL;
unsigned char* volatile i2c_memaux = NULL;
extern unsigned int volatile end_fast;

BOOL CreateCOM(COMOP_t* com, const char* name)
{
	DCB my_dcb = { 0 };
	COMMTIMEOUTS timeouts = { 0 };

	if (com == NULL)
	{
		return FALSE;
	}

	memset(com, 0, sizeof(*com));

	com->handle = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_ATTRIBUTE_NORMAL, 0);
	if (com->handle == INVALID_HANDLE_VALUE) {
		fprintf(LOG_FILE, "Error Opening Serial Port\n");
		return FALSE;
	}

	my_dcb.DCBlength = sizeof(my_dcb);
	GetCommState(com->handle, &my_dcb);
	my_dcb.BaudRate = CBR_115200;
	my_dcb.ByteSize = 8;
	my_dcb.StopBits = ONESTOPBIT;
	my_dcb.Parity = NOPARITY;

	if (!SetCommState(com->handle, &my_dcb)) {
		fprintf(LOG_FILE, "Error setting up serial port\n");
		return FALSE;
	}

	GetCommTimeouts(com->handle, &timeouts);

	timeouts.ReadIntervalTimeout = 100;
	timeouts.ReadTotalTimeoutConstant = 1;
	timeouts.ReadTotalTimeoutMultiplier = 1;

	SetCommTimeouts(com->handle, &timeouts);

	return TRUE;
}



BOOL CloseCOM(COMOP_t* com)
{
	if (com == NULL)
	{
		return FALSE;
	}

	CloseHandle(com->handle);

	memset(com, 0, sizeof(*com));

	return TRUE;
}

size_t WINAPI ComWriteBuff(COMOP_t* com, unsigned char* buffer, size_t bytes_to_write, unsigned int max_tries)
{
	size_t total_bytes_written = 0;
	size_t cur_bytes_written = 0;

	if (max_tries == 0)
	{
		max_tries = 100;
	}

	do
	{
		cur_bytes_written = 0;
		if (WriteFile(com->handle, buffer + total_bytes_written, bytes_to_write - total_bytes_written, &cur_bytes_written, 0))
		{
			if (cur_bytes_written)
			{
				total_bytes_written += cur_bytes_written;
			}
			else
			{
				Sleep(10);
			}
		}
		max_tries--;
	} while (bytes_to_write > total_bytes_written && max_tries > 0);

	return total_bytes_written;
}

size_t WINAPI ComWriteByte(COMOP_t* com, unsigned char byte_to_write, unsigned int max_tries)
{
	unsigned char aux_byte = 0;

	aux_byte = byte_to_write;

	return ComWriteBuff(com, &aux_byte, 1, max_tries);
}


size_t WINAPI ComReadBuff(COMOP_t* com, unsigned char* buffer, size_t bytes_to_read, unsigned int max_tries)
{
	size_t total_bytes_readed = 0;
	size_t cur_bytes_readed = 0;

	if (max_tries == 0)
	{
		max_tries = 10;
	}

	do
	{
		cur_bytes_readed = 0;
		if (ReadFile(com->handle, buffer + total_bytes_readed, bytes_to_read - total_bytes_readed, &cur_bytes_readed, 0))
		{
			if (cur_bytes_readed)
			{
				total_bytes_readed += cur_bytes_readed;
			}
			else
			{
				Sleep(5);
			}
		}
		max_tries--;
	} while (bytes_to_read > total_bytes_readed && max_tries > 0);

	return total_bytes_readed;
}

size_t WINAPI ComReadByte(COMOP_t* com, unsigned char* byte_readed, unsigned int max_tries)
{
	return ComReadBuff(com, byte_readed, 1, max_tries);
}

void FlushCOMIn(COMOP_t* com)
{
	unsigned char byte_readed;

	while (ComReadByte(com, &byte_readed, 0));
}


BOOL EnterBINMode(COMOP_t* com)
{
	unsigned char recv_byte = 0;
	size_t bytes_received = 0;

	ComWriteByte(com, BHL_HARD_RESET, 0);
	Sleep(100);
	FlushCOMIn(com);
	for (int i = 0; i < 10; i++)
	{
		bytes_received = ComWriteByte(com, 0x0d, 0);
		bytes_received = ComReadByte(com, &recv_byte, 0);
		bytes_received = ComWriteByte(com, 0x0a, 0);
		bytes_received = ComReadByte(com, &recv_byte, 0);
	}

	for (int i = 0; i < 3; i++)
	{
		bytes_received = ComWriteByte(com, '#', 0);
		Sleep(100);
	}
	FlushCOMIn(com);

	while (1)
	{
		if (bytes_received == 0)
		{
			bytes_received = ComWriteByte(com, BHL_ENTER_RAW_BITBANG, 0);
		}
		if (recv_byte != 'B')
		{
			bytes_received = ComReadByte(com, &recv_byte, 0);
		}
		if (recv_byte == 'B')
		{
			bytes_received = ComReadByte(com, &recv_byte, 0);
			if (recv_byte == 'B')
			{
				bytes_received = ComReadByte(com, &recv_byte, 0);
				if (recv_byte == 'I')
				{
					bytes_received = ComReadByte(com, &recv_byte, 0);
					if (recv_byte == 'O')
					{
						bytes_received = ComReadByte(com, &recv_byte, 0);
						if (recv_byte == '1')
						{
							return TRUE;
						}
					}
				}
			}
		}
	}

	return FALSE;
}

BOOL EnterI2CMode(COMOP_t* com)
{
	unsigned char recv_byte = 0;
	size_t bytes_received = 0;

	FlushCOMIn(com);

	while (1)
	{
		if (bytes_received == 0)
		{
			bytes_received = ComWriteByte(com, BHL_ENTER_BIN_I2C, 0);
		}
		if (recv_byte != 'I')
		{
			bytes_received = ComReadByte(com, &recv_byte, 0);
		}
		if (recv_byte == 'I')
		{
			bytes_received = ComReadByte(com, &recv_byte, 0);
			if (recv_byte == '2')
			{
				bytes_received = ComReadByte(com, &recv_byte, 0);
				if (recv_byte == 'C')
				{
					bytes_received = ComReadByte(com, &recv_byte, 0);
					if (recv_byte == '1')
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}


void ResetBusPirate(COMOP_t* com)
{
	EnterBINMode(com);
	ComWriteByte(com, BHL_HARD_RESET, 0);
	Sleep(100);
}

BUZZPIRATHLP_API void __stdcall bhl_hard_reset(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	ResetBusPirate(com_glb);
}

BUZZPIRATHLP_API void __stdcall bhl_com_close(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	COMMTIMEOUTS timeouts = { 0 };

	if (com_glb)
	{
		SetCommTimeouts(com_glb->handle, &timeouts);
		CloseCOM(com_glb);
		free(com_glb);
		com_glb = NULL;
	}
}

BUZZPIRATHLP_API BOOL __stdcall bhl_com_create(const char* com_name)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	if (com_glb)
	{
		bhl_com_close();
	}

	com_glb = calloc(1, sizeof(*com_glb));
	if (com_glb)
	{
		if (CreateCOM(com_glb, com_name))
		{
			return TRUE;
		}
		bhl_com_close();
	}

    return FALSE;
}

BUZZPIRATHLP_API BOOL __stdcall bhl_enter_raw_bitbang(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	fprintf(LOG_FILE, "Resetting bus pirate\n");
	ResetBusPirate(com_glb);
	Sleep(500);
	fprintf(LOG_FILE, "Entering bin mode\n");
	EnterBINMode(com_glb);
	Sleep(500);
	FlushCOMIn(com_glb);

	return TRUE;
}

BUZZPIRATHLP_API BOOL __stdcall bhl_enter_bin_i2c(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	EnterI2CMode(com_glb);
	Sleep(500);
	fprintf(LOG_FILE, "Flushing Com In\n");
	FlushCOMIn(com_glb);

	return TRUE;
}

BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_speed(unsigned int speed)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	ComWriteByte(com_glb, BHL_I2C_50KHZ, 0);
	FlushCOMIn(com_glb);

	return TRUE;
}


BUZZPIRATHLP_API BOOL __stdcall bhl_set_peripherals(unsigned int peripherals_mask)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	ComWriteByte(com_glb, peripherals_mask, 0);
	FlushCOMIn(com_glb);

	return TRUE;
}


BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_eepr_set_2byteaddr(unsigned int chip_addr, unsigned int mem_addr)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char buff[] = { BHL_I2C_SEND_START, BHL_I2C_BULK_WRITE | 2, chip_addr & 0xFFFFFFFE, (mem_addr >> 8) & 0x000000FF, mem_addr & 0x000000FF, BHL_I2C_SEND_STOP };

	fprintf(LOG_FILE, "chip addr: 0x%X(%d) setting eppaddr mem_addr_1byte: 0x%X(%d) mem_addr_2byte: 0x%X(%d)\n", buff[2], buff[2], buff[3], buff[3], buff[4], buff[4]);

	ComWriteBuff(com_glb, buff, 6, 0);
	FlushCOMIn(com_glb);

	return TRUE;
}


BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_eepr_start_bulk(unsigned int chip_addr)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char buff[] = { BHL_I2C_SEND_START, BHL_I2C_BULK_WRITE, chip_addr | 1 };

	fprintf(LOG_FILE, "chip addr for bulk: 0x%X(%d)\n", buff[2], buff[2]);

	ComWriteBuff(com_glb, buff, 3, 0);
	FlushCOMIn(com_glb);

	return TRUE;
}

BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_eepr_bulk_read_byte(unsigned char* recv_buff, unsigned int ack_or_nack)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char buff[] = { BHL_I2C_READ_BYTE, ack_or_nack };
	
	recv_buff[0] = 0;
	recv_buff[1] = 0;

	ComWriteBuff(com_glb, buff, 2, 0);
	ComReadBuff(com_glb, recv_buff, 2, 0);

	return TRUE;
}

BUZZPIRATHLP_API unsigned int __stdcall bhl_i2c_eepr_stop_bulk(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char buff[] = { BHL_I2C_READ_BYTE, BHL_I2C_SEND_NACK, BHL_I2C_SEND_STOP };
	unsigned char recv_buff[10] = { 0 };

	ComWriteBuff(com_glb, buff, 3, 0);
	ComReadBuff(com_glb, recv_buff, 4, 0);

	return recv_buff[0];
}


BUZZPIRATHLP_API BOOL __stdcall bhl_i2c_eepr_write_byte_2byteaddr(unsigned int chip_addr, unsigned int mem_addr, unsigned char byte_val, unsigned char* recv_buff)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char buff[] = { BHL_I2C_SEND_START, BHL_I2C_BULK_WRITE | 3, chip_addr & 0xFFFFFFFE, (mem_addr >> 8) & 0x000000FF, mem_addr & 0x000000FF, byte_val, BHL_I2C_SEND_STOP };

	/*
	fprintf(LOG_FILE, "chip addr for eepr_write_byte: 0x%X(%d) mem_addr_1byte: 0x%X(%d) mem_addr_2byte: 0x%X(%d)\n",
		buff[2], buff[2],
		buff[3], buff[3],
		buff[4], buff[4]);
		*/

	ComWriteBuff(com_glb, buff, 7, 0);
	memset(recv_buff, 0, 7);
	ComReadBuff(com_glb, recv_buff, 7, 0);

	return TRUE;
}

BUZZPIRATHLP_API unsigned int __stdcall bhl_i2c_scan(unsigned char* detected_array)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned int i = 0;
	unsigned char buff[] = { BHL_I2C_SEND_START, BHL_I2C_BULK_WRITE, 0, BHL_I2C_SEND_STOP };
	unsigned char recv_buff[5] = { 0 };
	unsigned int founds = 0;

	fprintf(LOG_FILE, "I2C SCANNER\n");

	FlushCOMIn(com_glb);

	for (i = 1; i < 0xFE; i++)
	{
		buff[2] = i;
		ComWriteBuff(com_glb, buff, 4, 0);
		memset(recv_buff, 0xcc, sizeof(recv_buff));
		ComReadBuff(com_glb, recv_buff, 4, 0);
		if (recv_buff[2] == 0)
		{
			detected_array[founds++] = i;
			fprintf(LOG_FILE, "addr found: 0x%X(%d) -> 0x%X(%d) %s\n", i >> 1, i >> 1, i, i, (i & 0x01) ? "R" : "W");
		}
	}

	FlushCOMIn(com_glb);

	return founds;
}

BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_init(const char* com_name, unsigned int power, unsigned int pullups, unsigned int khz, unsigned int just_i2c_scanner)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned int speed = 0;
	unsigned int phs = 0;
	unsigned i2c_detected = 0;
	unsigned int retf = 0;
	unsigned int i = 0;
	unsigned char* curr = NULL;
	static const char* as_msg = "keep pressing ESC key to cancel... keep pressing F1 to relaunch this console... ASProgrammer GUI will be unresponsive while BUS PIRATE is operating. BUS PIRATE is slow, please be (very) patient";

	end_fast = 0;

	fprintf(LOG_FILE, "\n\n%s\n\ncom_name: %s power: %d pullups: %d khz: %d just_i2c_scanner: %d\n", as_msg, com_name, power, pullups, khz, just_i2c_scanner);

	if (pullups)
	{
		phs |= BHL_PERIPHERAL_PULLUPS;
	}

	if (power)
	{
		phs |= BHL_PERIPHERAL_PWR;
	}

	switch (khz)
	{
	case 5:
		speed = BHL_I2C_5KHZ;
		break;

	case 50:
		speed = BHL_I2C_50KHZ;
		break;

	case 100:
		speed = BHL_I2C_100KHZ;
		break;

	default:
		return 0;
		break;
	}

	if (!bhl_com_create(com_name))
	{
		return 0;
	}

	if (NULL == i2c_memaux)
	{
		i2c_memaux = calloc(0x1000, 30000);
		if (NULL == i2c_memaux)
		{
			MessageBoxA(NULL, "ERROR MEMAUX ALLOC", "ERROR MEMAUX ALLOC", MB_OK | MB_TOPMOST | MB_ICONWARNING);
			bhl_com_close();
			return 0;
		}
	}

	Sleep(500);
	bhl_enter_raw_bitbang();
	fprintf(LOG_FILE, "Entering i2c mode\n");
	bhl_enter_bin_i2c();
	fprintf(LOG_FILE, "Setting I2C speed %d khz\n", khz);
	bhl_i2c_speed(speed);
	fprintf(LOG_FILE, "enabling PULL-UPS and POWER...\n");
	bhl_set_peripherals(phs);
	Sleep(200);

	if (!just_i2c_scanner)
	{
		return 1;
	}

	i2c_detected = bhl_i2c_scan(i2c_memaux + 0x200);
	if (i2c_detected)
	{
		retf = 1;

		curr = i2c_memaux;
		for (i = 0; i < i2c_detected; i++)
		{
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

			curr += sprintf(curr, BYTE_TO_BINARY_PATTERN " 0x%X(%d) -> 0x%X(%d) %s ----  ",
				BYTE_TO_BINARY(i2c_memaux[0x200 + i] >> 1),
				i2c_memaux[0x200 + i] >> 1, 
				i2c_memaux[0x200 + i] >> 1, 
				i2c_memaux[0x200 + i], 
				i2c_memaux[0x200 + i], 
				(i2c_memaux[0x200 + i] & 0x01) ? "R" : "W");
		}
		curr[0] = ' ';
		curr[1] = 0;
	}
	bhl_asprog_i2c_close();

	return 1;
}

BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_close(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	fprintf(LOG_FILE, "bhl_asprog_i2c_close\n");

	if (NULL != com_glb)
	{
		bhl_hard_reset();
		bhl_com_close();
		com_glb = NULL;
	}

	return 1;
}

BUZZPIRATHLP_API unsigned char* __stdcall bhl_asprog_i2c_get_memaux(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	return i2c_memaux;
}


BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_readwrite(unsigned int devaddr, unsigned int size, unsigned char* buffwr, unsigned int size_buffwr)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char receive_buf[10] = { 0 };
	unsigned int i = 0;
	unsigned int j = 0;

	fprintf(LOG_FILE, "\ndevaddr: 0x%X(%d) size: 0x%X(%d) buffwr: 0x%X(%d) size_buffwr: 0x%X(%d)\n", 
		devaddr, devaddr, 
		size, size, 
		buffwr, buffwr, 
		size_buffwr, size_buffwr);

	if (end_fast)
	{
		return 0;
	}

	if (size_buffwr > 0)
	{ 
		for (i = 0; i < size_buffwr; i++)
		{
			fprintf(LOG_FILE, "0x%02X ", buffwr[i]);
		}
		fprintf(LOG_FILE, "-\n\n");


		unsigned char buff[] = { BHL_I2C_SEND_START, BHL_I2C_BULK_WRITE, devaddr & 0xFFFFFFFE };
	

		ComWriteBuff(com_glb, buff, 3, 0);
		ComReadBuff(com_glb, receive_buf, 4, 0);

		unsigned char buff2[] = { BHL_I2C_BULK_WRITE, 0 };


		for (i = 0; i < size_buffwr; i++)
		{
			if (end_fast)
			{
				return 0;
			}
			buff2[1] = buffwr[i];
			ComWriteBuff(com_glb, buff2, 2, 0);
			ComReadBuff(com_glb, receive_buf, 3, 0);
		}

		if (size == 0)
		{
			ComWriteByte(com_glb, BHL_I2C_SEND_STOP, 0);
			ComReadBuff(com_glb, receive_buf, 2, 0);
		}
		else
		{
			ComWriteByte(com_glb, BHL_I2C_SEND_START, 0);
			ComReadBuff(com_glb, receive_buf, 2, 0);
		}
	}

	if (size > 0)
	{
		unsigned char buff4[] = { BHL_I2C_BULK_WRITE, devaddr | 1 };
		ComWriteBuff(com_glb, buff4, 2, 0);
		ComReadBuff(com_glb, receive_buf, 3, 0);
		fprintf(LOG_FILE, "\n");
		for (i = 0; i < size; i++)
		{
			if (end_fast)
			{
				return 0;
			}
			if (i == size - 1)
			{
				receive_buf[0] = bhl_i2c_eepr_stop_bulk();
			}
			else
			{
				bhl_i2c_eepr_bulk_read_byte(receive_buf, BHL_I2C_SEND_ACK);
			}
			i2c_memaux[i] = receive_buf[0];
			if (i % 8 == 0)
			{
				if (i > 0)
				{
					fprintf(LOG_FILE, "  ");
					for (j = i - 8; j < i; j++) // craaaap
					{
						fprintf(LOG_FILE, "%c", (i2c_memaux[j] >= 0x20 && i2c_memaux[j] <= 0x7E) ? i2c_memaux[j] : '.');
					}
				}
				fprintf(LOG_FILE, "\n0x%08X: ", i);
			}
			fprintf(LOG_FILE, "%02X ", receive_buf[0]);
		}
		if (size > 7)
		{ // wtf craaaap
			fprintf(LOG_FILE, "  ");
			for (j = i - 8; j < i; j++)
			{
				fprintf(LOG_FILE, "%c", (i2c_memaux[j] >= 0x20 && i2c_memaux[j] <= 0x7E) ? i2c_memaux[j] : '.');
			}
		}
		fprintf(LOG_FILE, "\n");

	}

	return 1;
}

BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_start(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char receive_buf[10] = { 0 };

	fprintf(LOG_FILE, "bhl_asprog_i2c_start\n");

	if (end_fast)
	{
		return 0;
	}

	ComWriteByte(com_glb, BHL_I2C_SEND_START, 0);
	ComReadBuff(com_glb, receive_buf, 2, 0);

	return 1;
}


BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_stop(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char receive_buf[10] = { 0 };

	fprintf(LOG_FILE, "bhl_asprog_i2c_stop\n");

	if (end_fast)
	{
		return 0;
	}

	ComWriteByte(com_glb, BHL_I2C_SEND_STOP, 0);
	ComReadBuff(com_glb, receive_buf, 2, 0);

	return 1;
}

BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_read_byte(void)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char receive_buf[10] = { 0 };
	unsigned char buff[] = { BHL_I2C_READ_BYTE , BHL_I2C_SEND_ACK };

	fprintf(LOG_FILE, "bhl_asprog_i2c_read_byte\n");

	if (end_fast)
	{
		return 0;
	}

	ComWriteBuff(com_glb, buff, 2, 0);
	ComReadBuff(com_glb, receive_buf, 3, 0);

	*i2c_memaux = receive_buf[0];

	return 1;
}

BUZZPIRATHLP_API unsigned int __stdcall bhl_asprog_i2c_write_byte(unsigned int byte)
{
#pragma comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
	unsigned char receive_buf[10] = { 0 };
	unsigned char buff[] = { BHL_I2C_BULK_WRITE, byte };

	fprintf(LOG_FILE, "bhl_asprog_i2c_write_byte\n");

	if (end_fast)
	{
		return 0;
	}

	ComWriteBuff(com_glb, buff, 2, 0);
	ComReadBuff(com_glb, receive_buf, 3, 0);

	return 1;
}