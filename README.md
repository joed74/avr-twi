# avr-twi

Really nonblocking TWI/I2C master driver for Atmel AVR

## API

###### `void twi_init()`

Initializes the driver.  Should be called once before calling any other TWI functions.

###### `TWI_STATUS twi_write(uint8_t address, uint8_t* data, uint8_t length)`

Writes data to the given address.

* `address` - TWI slave address
* `data` - pointer to data buffer
* `length` - numer of bytes to write from the given data buffer

Returns `TWI_STATUS`

* `TWI_BUSY` - still sending
* `TWI_OK` - writing was ok
* `TWI_NOK` - writing was not ok

###### `TWI_STATUS twi_write1(uint8_t address, uint8_t data)`

Writes one byte to the given address.

###### `TWI_STATUS twi_read(uint8_t address, uint8_t *length, uint8_t **data)`

Reads data from the given address.

* `address` - TWI slave address
* `length` - pointer to number of bytes to read
* `data` - address of pointer which points to the first data byte

Returns `TWI_STATUS`, see above - only for reading

On `TWI_OK`

* `length` - contains received count of data
* `data` - points to first data byte

###### `void twi_resetstate()`

Resets the internal state. Normally a write is repeated until its `TWI_OK`, after that we expect a read - which is repeated until its `TWI_OK`. If this is not wanted, you can reset the internal state.

    switch (operation) {
       case 1:
         if (twi_write(address, &data, 2)==TWI_OK) {
            twi_resetstate();
            operation++;
         }
        case 2:
         if (twi_write(address, &data, 2)==TWI_OK) {
            operation++;
         }
        case 3:
         if (twi_read(address, &length, &data)==TWI_OK) {
            operation++;
         }
     }

## Definitions

* `F_CPU` - you should define this before including this library
* `TWI_FREQ` - defaults to 100kHz if left undefined
* `TWI_BUFFER_LENGTH` - defaults to 32 if left undefined

## Example

This example reads temperature and humidity from an SHT45 sensor. First we send the command `0xFD` to the address `0x44`, than we read 6 bytes from the sensor. The sensor replies with NACK if the conversion is not done. The read is performed until we get an ACK. After reading 6 Bytes temperature and humidity gets calculated.

     if (readtemp==1) {
        twi_write(0x44,0xFD);
        uint8_t length=6;
        uint8_t *buffer=NULL;
        if (twi_read(0x44,&length,&buffer)==TWI_OK) {
           if (length==6) {
                uint16_t t=buffer[0]*256+buffer[1];
                uint16_t h=buffer[3]*256+buffer[4];
                reg.temperature=-45.0f+175.0f*((float) t/65535.0f);
                reg.humidity=-6.0f+125.0f*((float) h/65535.0f);
                if (reg.humidity<0) reg.humidity=0;
                if (reg.humidity>100) reg.humidity=100;
           }
           readtemp++;
        }
     }
