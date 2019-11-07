#ifndef __WIRE_H__
#define __WIRE_H__

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#define DDR_USI DDRA
#define PORT_USI PORTA
#define PIN_USI PINA
#define USI_SDA PA6
#define USI_SCL PA4

#define DELAY_T2TWI (_delay_us(2)) // >1.3us
#define DELAY_T4TWI (_delay_us(1)) // >0.6us


#define TWI_NACK_BIT 0 // Bit position for (N)ACK bit.

// Constants
// Prepare register value to: Clear flags, and set USI to shift 8 bits i.e. count 16 clock edges.
const unsigned char USISR_8bit = 1 << USISIF | 1 << USIOIF | 1 << USIPF | 1 << USIDC | 0x0 << USICNT0;
// Prepare register value to: Clear flags, and set USI to shift 1 bit i.e. count 2 clock edges.
const unsigned char USISR_1bit = 1 << USISIF | 1 << USIOIF | 1 << USIPF | 1 << USIDC | 0xE << USICNT0;

class Wire
{

public:
    static void init(void)
    {
        PORT_USI |= (1 << USI_SDA); // Enable pullup on SDA.
        PORT_USI |= (1 << USI_SCL); // Enable pullup on SCL.

        DDR_USI |= (1 << USI_SCL); // Enable SCL as output.
        DDR_USI |= (1 << USI_SDA); // Enable SDA as output.

        USIDR = 0xFF;                                     // Preload data register with "released level" data.
        USICR = 0 << USISIE | 0 << USIOIE |               // Disable Interrupts.
                1 << USIWM1 | 0 << USIWM0 |               // Set USI in Two-wire mode.
                1 << USICS1 | 0 << USICS0 | 1 << USICLK | // Software stobe as counter clock source
                0 << USITC;
        USISR = 1 << USISIF | 1 << USIOIF | 1 << USIPF | 1 << USIDC | // Clear flags,
                0x0 << USICNT0;                                       // and reset counter.
    }
    
    static uint8_t read(void)
    {
        if ((numReadBytes != 0) && (numReadBytes != -1)){
            numReadBytes--;
        }

        /* Read a byte */
        DDR_USI &= ~(1 << USI_SDA); // Enable SDA as input.
        uint8_t data = Wire::transfer(USISR_8bit);

        /* Prepare to generate ACK (or NACK in case of End Of Transmission) */
        if (numReadBytes == 0){
            USIDR = 0xFF;
        }else{
            USIDR = 0x00;
        }
        Wire::transfer(USISR_1bit); // Generate ACK/NACK.

        return data; // Read successfully completed
    }

    static bool write(uint8_t data)
    {
        /* Write a byte */
        PORT_USI &= ~(1 << USI_SCL); // Pull SCL LOW.
        USIDR = data;                // Setup data.
        Wire::transfer(USISR_8bit);  // Send 8 bits on bus.

        /* Clock and verify (N)ACK from slave */
        DDR_USI &= ~(1 << USI_SDA); // Enable SDA as input.
        if (Wire::transfer(USISR_1bit) & 1 << TWI_NACK_BIT){
            return false;
        }

        return true; // Write successfully completed
    }
    static bool start(uint8_t address, int readcount)
    {
        if (readcount != 0)
        {
            numReadBytes = readcount;
            readcount = 1;
        }
        uint8_t addressRW = address << 1 | readcount;

        /* Release SCL to ensure that (repeated) Start can be performed */
        PORT_USI |= (1 << USI_SCL); // Release SCL.
        while (!(PIN_USI & 1 << USI_SCL));
        DELAY_T4TWI;

        /* Generate Start Condition */
        PORT_USI &= ~(1 << USI_SDA); // Force SDA LOW.
        DELAY_T4TWI;
        PORT_USI &= ~(1 << USI_SCL); // Pull SCL LOW.
        PORT_USI |= (1 << USI_SDA);    // Release SDA.

        if (!(USISR & 1 << USISIF)){
            return false;
        }

        /*Write address */
        PORT_USI &= ~(1 << USI_SCL); // Pull SCL LOW.
        USIDR = addressRW;           // Setup data.
        Wire::transfer(USISR_8bit);  // Send 8 bits on bus.

        /* Clock and verify (N)ACK from slave */
        DDR_USI &= ~(1 << USI_SDA); // Enable SDA as input.
        if (Wire::transfer(USISR_1bit) & 1 << TWI_NACK_BIT){
            return false; // No ACK
        }

        return true;
    }
    static bool restart(uint8_t address, int readcount)
    {
        return Wire::start(address, readcount);
    }
    static void stop()
    {
        PORT_USI &= ~(1 << USI_SDA); // Pull SDA low.
        PORT_USI |= (1 << USI_SCL);    // Release SCL.
        while (!(PIN_USI & 1 << USI_SCL))
            ; // Wait for SCL to go high.
        DELAY_T4TWI;
        PORT_USI |= (1 << USI_SDA); // Release SDA.
        DELAY_T2TWI;
    }

private:
    Wire();
    static int numReadBytes;

    static uint8_t transfer(uint8_t data)
    {
        USISR = data;                                    // Set USISR according to data.
                                                         // Prepare clocking.
        data = 0 << USISIE | 0 << USIOIE |               // Interrupts disabled
               1 << USIWM1 | 0 << USIWM0 |               // Set USI in Two-wire mode.
               1 << USICS1 | 0 << USICS0 | 1 << USICLK | // Software clock strobe as source.
               1 << USITC;                               // Toggle Clock Port.
        do
        {
            DELAY_T2TWI;
            USICR = data; // Generate positive SCL edge.
            while (!(PIN_USI & 1 << USI_SCL))
                ; // Wait for SCL to go high.
            DELAY_T4TWI;
            USICR = data;                 // Generate negative SCL edge.
        } while (!(USISR & 1 << USIOIF)); // Check for transfer complete.

        DELAY_T2TWI;
        data = USIDR;              // Read out data.
        USIDR = 0xFF;              // Release SDA.
        DDR_USI |= (1 << USI_SDA); // Enable SDA as output.

        return data; // Return the data from the USIDR
    }
};

int Wire::numReadBytes;

#endif
