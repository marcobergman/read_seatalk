#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* Marco Bergman 01-OCT-2019
** 9-bit Raspberry Seatalk reader

   Reads bytes from ttyAMA0 and presents in hex, in lines on standard output.
   Each line starts with the byte that has the 9th bit set.
   The 9th bit is in the position of the parity bit.

   Normally, you cannot read 9 bits, but many UARTs have a trick for that.
   The trick is first to enable input parity checking (INPCK).  If enabled, this will
   make the UART evaluate the parity bit.  Normally, when the parity bit has the 'wrong'
   value, the resulting data byte is set to 0x00.  But then you lose your data.
   Therefore, we set PARMRK.  This inserts 0xFF 0x00 sequences before the byte with the 9th
   bit set, and that we can recognise.  Finally, we need to tell the UART that the parity bit
   is not calculated based on the other bits, but that it is fixed.  This is done by setting
   the elusive 'stick' parity with the flag constant CMSPAR.  Now we get 0xFF 0x00 sequences
   in our data stream preceding a new seatalk command.  The rest is trivial.

   This CMSPAR mechanism is not always available.  FTDI chips ('serial-to-USB') are known not 
   to have this 'stick bit' facility, and also in raspberries <= 2016 this was not compiled 
   into the kernel.  But from Raspberry 3+ onwards, this mechanism seems to work with the 
   built-in /dev/ttyAMA0.

   Documentation is in the linux man pages for TERMIOS(3).
        http://man7.org/linux/man-pages/man3/termios.3.html
        https://www.raspberrypi.org/forums/viewtopic.php?t=25516

   Other options would have been do write directly into the UART's Line Control Register,
        https://www.lammertbies.nl/comm/info/serial-uart.html#LCR

   or to use some HAL library
        https://raspberrypi.stackexchange.com/questions/44961/9-bit-serial-communication-and-parity-error-detection
**
*/

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag |= PARENB;      /* enable parity bit */
    tty.c_cflag &= ~PARODD;     /* only 'address' causes exception */

    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IGNPAR);

    /* And now: the trick Raymarine does not want you to know: */
    tty.c_iflag |= INPCK | PARMRK;
    tty.c_cflag |= CMSPAR;      /* use stick parity */

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}



int main()
{
    char *portname = "/dev/ttyAMA0";
    int fd;
    int wlen;

    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    set_interface_attribs(fd, B4800);

    /* simple noncanonical input */
    do {
        unsigned char buf[80];
        int rdlen;

        rdlen = read(fd, buf, sizeof(buf) - 1);

        if (rdlen > 0) {
                unsigned char *p;
                for (p = buf; rdlen-- > 0; p++) {
                        if (*p == 0xFF) {
                                p++;rdlen--;
                                if (*p == 0x00) {
                                        printf ("\n");
                                        p++;rdlen--;
                                } else if (*p == 0xFF) {
					printf ("0x%x ", *p);
					p++;rdlen--;
				} 
                        }
                        printf("0x%x ", *p);
                }
        } else if (rdlen < 0) {
                printf("Error from read: %d: %s\n", rdlen, strerror(errno));
        } else {  /* rdlen == 0 */
                printf("Timeout from read\n");
        }
        /* repeat read to get full message */
    } while (1);
}

