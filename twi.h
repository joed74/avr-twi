#ifndef TWI_H
#define TWI_H

#include <stdint.h>

#ifndef TWI_FREQ
#define TWI_FREQ 100000UL
#endif

#ifndef TWI_BUFFER_LENGTH
#define TWI_BUFFER_LENGTH 32
#endif

typedef enum {
	TWI_BUSY,
	TWI_OK,
	TWI_NOK,
} TWI_STATUS;

#ifndef TWS7
#define TWS7 TWS07
#endif

#ifndef TWS6
#define TWS6 TWS06
#endif

#ifndef TWS5
#define TWS5 TWS05
#endif

#ifndef TWS4
#define TWS4 TWS04
#endif

#ifndef TWS3
#define TWS3 TWS03
#endif

#ifndef TWBR
#define TWBR TWBR0
#endif

#ifndef TWSR
#define TWSR TWSR0
#endif

#ifndef TWCR
#define TWCR TWCR0
#endif

#ifndef TWDR
#define TWDR TWDR0
#endif

#ifndef TWI_vect
#define TWI_vect TWI0_vect
#endif

void twi_init();
void twi_resetstate();
TWI_STATUS twi_write1(uint8_t address, uint8_t data);
TWI_STATUS twi_write(uint8_t address, uint8_t* data, uint8_t length);
TWI_STATUS twi_read(uint8_t address, uint8_t *length, uint8_t **data);
#endif
