/*
 * 	wserdefs.h
 * 		Serial definitions
 */

typedef struct
{
	char com_name[20];
	unsigned baud;
	int bit_num;
	int parity;
	int stop_num;
	int is_modem;
	void *com;
} SERIAL_T;

#ifdef __cplusplus
extern "C" {
#endif
extern SERIAL_T serials[ NUM_CHANNELS ];
#ifdef __cplusplus
}
#endif
		
