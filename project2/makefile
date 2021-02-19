COMMON_OBJECTS = 
CPU		= msp430g2553
CFLAGS		= -mmcu=${CPU} -Os -I../h
LDFLAGS		= -L../lib -L/opt/ti/msp430_gcc/include/

CC		= msp430-elf-gcc
AS		= msp430-elf-as -mmcu=${CPU} -c
AR		= msp43--elf-ar

all: pg.elf

pg.elf: ${COMMON_OBJECTS} pg.o wdt_handler.o buzzer.o player.o
	${CC} $(CFLAGS) ${LDFLAGS} -o $@ $^ -lLcd -lp2sw -lTimer -lShape -lCircle

clean:
	rm -f *.a *.o *.elf

load: pg.elf
	msp430loader.sh $^

er:
	mspdebug rf2500 "erase"
