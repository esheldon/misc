#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <curses.h>
#include <string.h>
#include <fcntl.h>

#define MAXFILELEN 255

struct tail_st {

	char *buf;
	char fname[MAXFILELEN];
	int fd;
	WINDOW *win;
	int startrow;
	int numrows;
	struct stat sbuf;
};


int getline(int fd, int line_len, char *line);

int main(int argc, char *argv[])
{
	WINDOW *stdscr;

	struct tail_st *tst;

	long poll_time = 100000;
	int xmax,ymax;
	int numfiles;
	int i,j;
	int ctr;

	int extrarows,rowperfile;
	char *line;
	int retval;
	struct stat sbuf_new;

	int test;

	numfiles = argc - 1;

	if ((tst = (struct tail_st *) calloc(numfiles, sizeof(struct tail_st))) == NULL) {
		fprintf(stderr,"could not calloc\n");
		return(-1);
	}

	if (numfiles == 0) {
		printf("Usage: mtail file1 [file2] [file3] ...\n");
		exit(0);
	}

	/* Open them files */
	for (i=0;i<numfiles;i++) {
		strncpy(tst[i].fname, argv[i+1],MAXFILELEN);
		if ((tst[i].fd = open(tst[i].fname, O_RDONLY)) < 0) {
			fprintf(stderr,"Failed to open %s\n",tst[i].fname);
			for (j=0;j<i;j++) {
				close(tst[j].fd);
			}
			return(-1);
		}
		if (fstat(tst[i].fd, &(tst[i].sbuf)) < 0) {
			fprintf(stderr,"failed to fstat\n");
			return(-1);
		}
	}

	if ((stdscr = initscr()) == NULL) {
		perror("initscr\n");
		return(-1);
	}

	getmaxyx(stdscr, ymax, xmax);

	if (ymax < (numfiles * 4)) {
		fprintf(stderr,"Screen too small!\n");
		endwin();
		return(0);
	}

	/* Set up the screen...*/
	rowperfile = (ymax - numfiles) / numfiles;
	extrarows = ymax - numfiles - rowperfile * numfiles;


	tst[0].startrow = 0;
	tst[0].numrows = rowperfile + extrarows;

	for (i=1;i<numfiles;i++) {
		tst[i].startrow = tst[i-1].startrow + tst[i-1].numrows+1;
		tst[i].numrows = rowperfile;
	}

	for (i=0;i<numfiles;i++) {
		mvhline(tst[i].startrow,0,0,xmax);
		tst[i].win = subwin(stdscr, tst[i].numrows,xmax-1,tst[i].startrow+1,0);
		mvwaddstr(stdscr,tst[i].startrow,(xmax - strlen(tst[i].fname))/2,
				tst[i].fname);
		scrollok(tst[i].win,1);
	}
	wrefresh(stdscr);

	/* Read in files and print out last number of lines */
	if ((line = (char *) calloc(xmax, sizeof(char))) == NULL) {
		fprintf(stderr,"shit\n");
		return(-1);
	}

	for (i=0;i<numfiles;i++) {
		/*while ((retval = getline(tst[i].fd, xmax, line)) != -1);*/

		if ((tst[i].buf = (char *) calloc(tst[i].numrows * xmax, sizeof(char))) == NULL) {
			fprintf(stderr,"Calloc failed\n");
			return(-1);
		}

		test = lseek(tst[i].fd, tst[i].sbuf.st_size - (tst[i].numrows+2)*xmax, SEEK_SET);

		ctr = 0;
		do {
			retval = getline(tst[i].fd, xmax, line);
			if (ctr < (tst[i].numrows - 1)) {
				memcpy((tst[i].buf + ctr * xmax), line, xmax);
				ctr++;
			} else {
				memmove(tst[i].buf, tst[i].buf + xmax, xmax * (tst[i].numrows - 1));
				memcpy((tst[i].buf + ctr * xmax), line, xmax);
			}
		} while (retval != -1);

		for (j=0;j<tst[i].numrows;j++) {
			waddstr(tst[i].win,tst[i].buf + j * xmax);
		}
		wrefresh(tst[i].win);
		free(tst[i].buf);
	}

	while(1) {
		for(i=0;i<numfiles;i++) {
			if (fstat(tst[i].fd, &sbuf_new) < 0) {
				return(-1);
			}
			if (sbuf_new.st_size > tst[i].sbuf.st_size) {
				wrefresh(tst[i].win);
				memcpy(&(tst[i].sbuf), &sbuf_new, sizeof(sbuf_new));
				do {
					retval = getline(tst[i].fd, xmax, line);
					waddstr(tst[i].win,line);
					wrefresh(tst[i].win);
				} while (retval != -1);
			}
		}
		usleep(poll_time);
	}

	sleep(2);
	endwin();
	printf("ymax = %d, rowper = %d, extra = %d\n",ymax,rowperfile,extrarows);

	for (i=0;i<numfiles;i++) {
		printf("st: %d, num:%d\n",tst[i].startrow,tst[i].numrows);
	}

	return(0);
}

int getline(int fd, int line_len, char *line)
{
	int i=0;
	char the_char = 0;
	int count = 1;

	bzero(line, line_len);
	while ((i<(line_len-1)) && (count > 0) && (the_char != '\n')) {
		count = read(fd, &the_char, 1);
		if ((count > 0) && (the_char != '\n')) {
			line[i] = the_char;
		} else if (the_char == '\n') {
            /* why this extra if? */
			line[i] = '\n';
		} else {
			return(-1);
		}
		i++;
	}

    /* never true */
	if (i == line_len) {
		line[i] = '\n';
	}
	return(0);
}

