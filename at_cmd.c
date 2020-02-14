/***********************************************************
*  Copy form MoTalk                                        *
*  MoTalk - A "C" program for modem setup.                 *
*    This program is meant as an aid only and is           *
*    not supported by IBM.                                 *
*    Usage:  ./at_cmd -d /dev/ttyUSB2 -b SPEED -c AT+CMD   *
*            ./at_cmd -d /dev/ttyUSB2 -b 9600 -c at+gmm    *
*                                                          *
***********************************************************/
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <termio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

FILE *fdr, *fdw;
int fd;
struct termio term_save, stdin_save;
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define ATCMD_BUUER 100
#define DEFAULT_DEV_NAME "/dev/ttyUSB2"

void at_exit(int sig)
{
	if (fdr)
		fclose(fdr);

	if (fdw)
		fclose(fdw);

	ioctl(fd, TCSETA, &term_save);
	close(fd);

	ioctl(fileno(stdin), TCSETA, &stdin_save);
	exit(sig);
}

int main(int argc, char *argv[])
{
	char *b, buffer[ATCMD_BUUER], buffer2[ATCMD_BUUER] = {0};
	int baud = B9600, num, i, j = 0, a = 0;
	struct termio term, tstdin;
	char dev_name[20] = DEFAULT_DEV_NAME;
	char at_cmd[ATCMD_BUUER];
	int flags, opt;
	int nsecs, tfnd;

	nsecs = 0;
	tfnd = 0;
	flags = 0;

	while ((opt = getopt(argc, argv, "d:b:c:")) != -1) {
		switch (opt) {
		case 'd':
			sprintf(dev_name, "%s", optarg);
			break;

		case 'c':
			sprintf(at_cmd, "%s\r\n", optarg);
			/* printf("send at_cmd : %s\r\n", at_cmd); */
			break;

		case 'b':
			switch (atoi(optarg)) {
				case 300: baud = B300;
				break;

				case 1200: baud = B1200;
				break;

				case 2400: baud = B2400;
				break;

				case 4800: baud = B4800;
				break;

				case 9600: baud = B9600;
				break;

				case 19200: baud = B19200;
				break;

				case 38400: baud = B38400;
				break;

				default:
					baud = B9600;
					break;
			}
			break;

		default: /* '?' */
			printf("Usage: ./at_cmd -d /dev/ttyUSB2 -b SPEED -c AT+CMD\n");
			exit(EXIT_FAILURE);
		}
	}

	if ((fd = open(dev_name, O_RDWR | O_NDELAY)) < 0) {
		printf("open device %s fail\r\n", dev_name);
		perror(dev_name);
		exit(errno);
	}

	/* Save stdin and tty state and trap some signals */
	ioctl(fd, TCGETA, &term_save);
	ioctl(fileno(stdin), TCGETA, &stdin_save);
	signal(SIGHUP, at_exit);
	signal(SIGINT, at_exit);
	signal(SIGQUIT, at_exit);
	signal(SIGTERM, at_exit);

	/* Set stdin to raw mode, no echo */
	ioctl(fileno(stdin), TCGETA, &tstdin);
	tstdin.c_iflag = 0;
	tstdin.c_lflag &= ~(ICANON | ECHO);
	tstdin.c_cc[VMIN] = 0;
	tstdin.c_cc[VTIME] = 0;
	ioctl(fileno(stdin), TCSETA, &tstdin);

	/* Set tty state */
	ioctl(fd, TCGETA, &term);
	term.c_cflag |= CLOCAL|HUPCL;

	if (baud > 0) {
		term.c_cflag &= ~CBAUD;
		term.c_cflag |= baud;
	}

	term.c_lflag &= ~(ICANON | ECHO); /* to force raw mode */
	term.c_iflag &= ~ICRNL; /* to avoid non-needed blank lines */
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 10;
	ioctl(fd, TCSETA, &term);
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NDELAY);

	/* Open tty for read and write */
	if ((fdr = fopen(dev_name, "r")) == NULL) {
		perror(dev_name);
		exit(errno);
	}

	if ((fdw = fopen(dev_name, "w")) == NULL) {
		perror(dev_name);
		exit(errno);
	}

	/* talk to the modem */
	memcpy(buffer, at_cmd, strlen(at_cmd));
	write(fileno(fdw), buffer, strlen(buffer));


	/* wait model response */
	usleep(1000);

	num = read(fileno(fdr), buffer, ARRAY_SIZE(buffer));
	if (num <= 0)
		at_exit(0);

	/*
	 * the modem will ans "AT+GMMBG96OK" so.
	 * For script paser convenience,
	 * we will change to "AT+GMM-BG96-OK"
	 */

	for (i = 0; i < num; i++) {
		if (i > 0 && buffer[i - 1] != 0x0a && buffer[i - 1] != 0x0d) {
			if (buffer[i] == 0x0a || buffer[i] == 0x0d) {
				buffer2[j] = '-';
				j = j + 1;
			}
		}

		if (buffer[i] != 0x0d && buffer[i] != 0x0a) {
			buffer2[j] = buffer[i];
			j = j + 1;
		}
	}
	printf("%s", buffer2);

	at_exit(0);
}
