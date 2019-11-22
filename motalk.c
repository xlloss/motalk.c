/* ************************************************************
 *  MoTalk - A "C" program for modem setup.                   *
 *           This program is meant as an aid only and is      *
 *           not supported by IBM.                            *
 *                  compile:  cc -o motalk motalk.c           *
 *                  Usage:  motalk /dev/tty? [speed]          *
 **************************************************************/
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <termio.h>

FILE *fdr, *fdw;
int fd;
struct termio term_save, stdin_save;
void Exit(int sig)
{
   if (fdr) fclose(fdr);
   if (fdw) fclose(fdw);
   ioctl(fd, TCSETA, &term_save);
   close(fd);
   ioctl(fileno(stdin), TCSETA, &stdin_save);
   exit(sig);
}
main(int argc, char *argv[])
{
   char *b, buffer[80];
   int baud=0, num;
   struct termio term, tstdin;
   if (argc < 2 || !strcmp(argv[1], "-?"))
   {
      fprintf(stderr, "Usage: motalk /dev/tty? [speed]\n");
      exit(1);
   }
   if ((fd = open(argv[1], O_RDWR | O_NDELAY)) < 0)
   {
      perror(argv[1]);
      exit(errno);
   }
   if (argc > 2)
   {
      switch(atoi(argv[2]))
      {
         case   300: baud = B300;
                     break;
         case  1200: baud = B1200;
                     break;
         case  2400: baud = B2400;
                     break;
         case  4800: baud = B4800;
                     break;
         case  9600: baud = B9600;
                     break;
         case 19200: baud = B19200;
                     break;
         case 38400: baud = B38400;
                     break;
         default:    baud = 0;
                     fprintf(stderr, "%s: %s is an unsupported baud\n", argv[0],argv[2]);
                     exit(1);
         }
      }
   /* Save stdin and tty state and trap some signals */
   ioctl(fd, TCGETA, &term_save);
   ioctl(fileno(stdin), TCGETA, &stdin_save);
   signal(SIGHUP, Exit);
   signal(SIGINT, Exit);
   signal(SIGQUIT, Exit);
   signal(SIGTERM, Exit);
   /*  Set stdin to raw mode, no echo */
   ioctl(fileno(stdin), TCGETA, &tstdin);
   tstdin.c_iflag = 0;
   tstdin.c_lflag &= ~(ICANON | ECHO);
   tstdin.c_cc[VMIN] = 0;
   tstdin.c_cc[VTIME] = 0;
   ioctl(fileno(stdin), TCSETA, &tstdin);
   /*  Set tty state */
   ioctl(fd, TCGETA, &term);
   term.c_cflag |= CLOCAL|HUPCL;
   if (baud > 0)
   {
      term.c_cflag &= ~CBAUD;
      term.c_cflag |= baud;
   }
   term.c_lflag &= ~(ICANON | ECHO); /* to force raw mode */
   term.c_iflag &= ~ICRNL; /* to avoid non-needed blank lines */
   term.c_cc[VMIN] = 0;
   term.c_cc[VTIME] = 10;
   ioctl(fd, TCSETA, &term);
   fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NDELAY);
   /*  Open tty for read and write */
   if ((fdr = fopen(argv[1], "r")) == NULL )
   {
      perror(argv[1]);
      exit(errno);
   }
   if ((fdw = fopen(argv[1], "w")) == NULL )
   {
      perror(argv[1]);
      exit(errno);
   }
   /*  Talk to the modem */
   puts("Ready... ^C to exit");
   while (1)
   {
      if ((num = read(fileno(stdin), buffer, 80)) > 0)
         write(fileno(fdw), buffer, num);
      if ((num = read(fileno(fdr), buffer, 80)) > 0)
         write(fileno(stdout), buffer, num);
      Exit (0);
   }
} 
