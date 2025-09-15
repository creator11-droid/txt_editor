#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include<sys/ioctl.h>

#define CTRL_KEY(k) ((k) & 0x1F)

struct editorConfig {
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editorConfig E;

// error handler
void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J" , 4);
	write(STDOUT_FILENO, "\x1b[H" , 3);
	perror(s);
	exit(1);
}
void disableRawMode()
{
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode(){
	/* tcgetattr takes a file descriptor(fd) and a termios struct address
 STDIN_FILENO represent the file descriptor number for standard input on POSIX compliant systems */
	if(tcgetattr(STDIN_FILENO, &E.orig_termios)== -1) die("tcgetattr");
	atexit(disableRawMode);

	struct termios raw = E.orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	/* tcsetattr takes a fd, optional options in this case TCSAFLUSH to remove queued input from the terminal and then the address of a termios struct */
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

// to watch if user is typing
char editorReadKey(){
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1){
	if (nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

int getCursorPosition(int *rows, int *cols){
	char buf[32];
	unsigned int i = 0;
	if (write(STDOUT_FILENO, "\x1b[6n",4) !=4) return -1;
	while (i< sizeof(buf) -1) {
		if (read (STDIN_FILENO, &buf[i], 1) != 1)break;
		if ( buf[i] == 'R') break;
		i++;
	}
	buf[i] ='\0';
	if(buf[0] != '\x1b' || buf[1] != '[')return -1;
	if (sscanf( &buf[2], "%d;%d", rows, cols)!=2)return -1;
	
	
	
	return 0;
}

int getWindowSize(int *rows, int *cols){
	struct winsize ws;
	if ( ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0 ) {
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12)!= 12) return -1;
		return getCursorPosition(rows, cols);
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

void editorProcessKeypress(){
	char c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}
void editorDrawRows(){
	int y;
	for(y= 0; y < E.screenrows; y++){
	write(STDOUT_FILENO, "~", 1);

	if (y < E.screenrows -1 ){
		write(STDOUT_FILENO, "\r\n", 2);
	}
	}
}

void editorRefreshScreen(){
	/* the hex 1b defines the escape sequence 27 and the next character is the [ which follows an escape sequence and it can take an arguement in the first case its 2 which clears the entire screen and the j command to clear the screen and the H command takes two argument arguments are separated by ;*/
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	
	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[H", 3);
}
void initEditor(){
	if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main(){
	enableRawMode();
	initEditor();
	while(1){
		editorRefreshScreen();
		editorProcessKeypress();
	
	};
	return 0;
}
