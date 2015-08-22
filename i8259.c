/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
#define PIC1_DATA	(MASTER_8259_PORT+1)
#define PIC2_DATA	(SLAVE_8259_PORT+1)
/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

void
i8259_init(void)
{
	/*INITIALIZE*/
	master_mask = 0xff;
	slave_mask = 0xff;
	
	outb(master_mask,PIC1_DATA);
	outb(slave_mask,PIC2_DATA);
	
	outb(ICW1,MASTER_8259_PORT);
	/* ICW2: Master PIC vector offset */
	outb(ICW2_MASTER,PIC1_DATA);
	/* ICW3: tell Master PIC that there is a slave PIC at IRQ2 */
	outb(ICW3_MASTER,PIC1_DATA);
	outb(ICW4,PIC1_DATA);
	
	outb(ICW1,SLAVE_8259_PORT);
	/* ICW2: Slave PIC vector offset */
	outb(ICW2_SLAVE,PIC2_DATA);
	/* ICW3: tell Slave PIC its cascade identity */
	outb(ICW3_SLAVE,PIC2_DATA);
	outb(ICW4,PIC2_DATA);
	
	/*restore the IRQ mask*/
	outb(master_mask,PIC1_DATA);
	outb(slave_mask,PIC2_DATA);

}

/* Enable (unmask) the specified IRQ */
void
enable_irq(uint32_t irq_num)
{
    uint16_t port;
    uint8_t value;
    if(irq_num < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }
    value = inb(port) & ~ (1 << irq_num); 
    outb(value,port); 

}

/* Disable (mask) the specified IRQ */
void
disable_irq(uint32_t irq_num)
{
    uint16_t port;
    uint8_t value;
    if(irq_num < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_num -= 8;
    }
    value = inb(port) | (1 << irq_num); 
    outb(value,port);  

}

/* Send end-of-interrupt signal for the specified IRQ */
void
send_eoi(uint32_t irq_num)
{
     unsigned long flags;
	 cli_and_save(flags);
	if(irq_num >= 8) {
		outb((0x02 | EOI),MASTER_8259_PORT);
		outb((uint16_t) ((irq_num-8) | EOI),SLAVE_8259_PORT);
		}
	else 
		outb((irq_num | EOI),MASTER_8259_PORT);
	 restore_flags(flags);
}

