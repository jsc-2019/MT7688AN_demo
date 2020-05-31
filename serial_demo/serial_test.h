#ifndef __SERIAL_TEST__H
#define __SERIAL_TEST__H

typedef struct {
    char parity;
	unsigned int baud;
    unsigned int flow_ctrl;
    unsigned int databits;
    unsigned int stopbits;
} SERIAL_ATTR_ST;


#endif