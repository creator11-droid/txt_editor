#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

struct termios orig_termios;
// error handler
void die(const char *s) {
	perror(s);
	exit(1);
}
void disableRawMode()
{
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode(){
	/* tcgetattr takes a file descriptor(fd) and a termios struct address
 STDIN_FILENO represent the file descriptor number for standard input on POSIX compliant systems */
	if(tcgetattr(STDIN_FILENO, &orig_termios)== -1) die("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	/* tcsetattr takes a fd, optional options in this case TCSAFLUSH to remove queued input from the terminal and then the address of a termios struct */
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int main(){
	enableRawMode();
	while(1){
	char c = '\0';
	if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
	 if (iscntrl(c)){
	 printf("%d\r\n", c);
	} else {
	       printf("%d ('%c')\r\n", c, c);
		}
	if (c == CTRL_KEY('q')) break;	 
	
	};
	return 0;
}
