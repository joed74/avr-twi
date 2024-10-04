#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <string.h>

#include "twi.h"

typedef enum {
	IDLE,
	WRITING,
	WRITEOK,
	WRITERETRY,
	READING,
	READOK,
	READRETRY,
	ERROR
} TWI_STATE;

volatile TWI_STATE twi_state;

static struct {
	uint8_t buffer[TWI_BUFFER_LENGTH];
	uint8_t length;
	uint8_t index;
} twi;

void twi_init() {
	TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
	TWSR = 0; // prescaler = 1
	TWCR = _BV(TWEN);	
	twi_state = IDLE;
}

void twi_resetstate() {
	if (twi_state==READRETRY || twi_state==WRITERETRY) twi_state=IDLE;
}

void twi_start(void) {
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
}

void twi_stop(void) {
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
}

void twi_ack() {
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
}

void twi_nack() {
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
}

void twi_send(uint8_t data) {
	TWDR = data;
}

void twi_recv() {
	twi.buffer[twi.index++] = TWDR;
}

void twi_reply() {
	if (twi.index < (twi.length - 1))
	{
		twi_ack();
	}
	else
	{
		twi_nack();
	}
}

void twi_done() {
	if (twi.index==1)
	{
		// error, only address was sent or received
		twi_state=ERROR;
	}
	else
	{
		// ok
		if (twi_state==READING)
		{
			twi_state=READOK;
		}
		else
		{
			twi_state=WRITEOK;
		}
	}
}

TWI_STATUS twi_write(uint8_t address, uint8_t* data, uint8_t length) {

	if (twi_state==WRITING || twi_state==READING || twi_state==READOK || twi_state==READRETRY) return TWI_BUSY;
	if (twi_state==WRITEOK) 
	{
		twi_state=IDLE;
		return TWI_OK;
	}
	if (twi_state==ERROR)
	{
		twi_state=WRITERETRY;
		return TWI_NOK;
	}
	twi_state=WRITING;
	
	if (length + 1 > TWI_BUFFER_LENGTH) length = TWI_BUFFER_LENGTH-2;

	twi.buffer[0] = (address << 1) | TW_WRITE;
	twi.length = length + 1;
	twi.index = 0;
	
	memcpy(&twi.buffer[1], data, length);

	twi_start();
	return TWI_BUSY;
}

TWI_STATUS twi_write1(uint8_t address, uint8_t data) {
	return twi_write(address,&data,1);
}


TWI_STATUS twi_read(uint8_t address, uint8_t *length, uint8_t **data) {
	
	if (twi_state==WRITING || twi_state==READING || twi_state==WRITEOK || twi_state==WRITERETRY) return TWI_BUSY;
	if (twi_state==READOK)
	{
		*length=twi.index-1;
		*data=&twi.buffer[1];
		twi_state=IDLE;
		return TWI_OK;
	}
	if (twi_state==ERROR)
	{
		twi_state=READRETRY;
		return TWI_NOK;
	}
	twi_state = READING;

	if (*length + 1 > TWI_BUFFER_LENGTH) *length = TWI_BUFFER_LENGTH-2;

	twi.buffer[0] = (address << 1) | TW_READ;
	twi.length = *length + 1;
	twi.index = 0;

	twi_start();
	return TWI_BUSY;
}

ISR(TWI_vect) {
	switch (TW_STATUS) {
		case TW_START:
		case TW_REP_START:
		case TW_MT_SLA_ACK:
		case TW_MT_DATA_ACK:
		if (twi.index < twi.length)
		{
			twi_send(twi.buffer[twi.index++]);
			twi_nack();
		}
		else
		{
			twi_stop();
			twi_done();
		}
		break;

		case TW_MR_DATA_ACK:
		twi_recv();
		twi_reply();
		break;

		case TW_MR_SLA_ACK:
		twi_reply();
		break;

		case TW_MR_DATA_NACK:
		twi_recv();
		twi_stop();
		twi_done();
		break;

		case TW_MT_SLA_NACK:
		case TW_MR_SLA_NACK:
		case TW_MT_DATA_NACK:
		case TW_MR_ARB_LOST:
		default:
		twi_stop();
		twi_done();
		break;
	}
}
