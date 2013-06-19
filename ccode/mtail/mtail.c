/*
Usage: 
    mtail [-c ncol] file1 file2 file3 ...

Description:

    Tail multiple files.  The files are shown in a grid of windows, with the
    number of columns specified by the -c option.  By default a single column
    is used.

    e.g. for two columns

        --------n1---------------n4-------
                        |
                        |
                        |
                        |
        --------n2---------------n5-------
                        |
                        |
                        |
                        |
        --------n3---------------n6-------
                        |
                        |
                        |
                        |

    where n1,n2... etc. show where the file names are displayed.  The number of
    rows is automatically adjusted to fit all the files within the specified
    number of columns.  

    If the file names become too long, the are truncated to fit the window with a
    preceding ...

    To exit the program, hit ctrl-c

Dependencies:
    The curses library and headers.  On ubuntu/debian you may have to install the
    development packages:

        sudo apt-get install libncurses5 libncurses5-dev

Copyright (C) 2010  Erin Sheldon (erin dot sheldon at gmail dot com)
                    and Eli Rykoff (erykoff at physics dot ucsb dot edu )

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as 
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <curses.h>
#include <string.h>
#include <fcntl.h>

#define MAXFILELEN 255
// 1 second
#define POLLTIME 1000000

struct tail {

    char *buf;
    char *line_buf;

    char fname[MAXFILELEN];
    int fd;
    WINDOW *win;
    WINDOW *border_win;

    /* these are row,col in characters on the screen, not the file matrix */
    int startrow;
    int numrows;

    int startcol;
    int numcols;

    struct stat sbuf;
};

struct mtail {

    WINDOW *stdscr;

    long poll_time;

    int numfiles;

    int ncol; /* number of columns of files */
    int nrow; 
    int xmax; /* size of whole screen */
    int ymax;

    int ywinsize; /* size of each screen if perfect fit */
    int extra_ychars; /* extra characters to go in first screen */

    int xwinsize;
    int extra_xchars;

    /* one per file + window */
    struct tail* tst;

};

int parse_command_line(int argc, char* argv[], int* ncol, int* numfiles) {

    *ncol = 1;

    int c;
    while ((c = getopt (argc, argv, "c:")) != -1) {
        switch (c) {
            case 'c':
                *ncol = atoi( optarg );
                break;
            default:
                abort();
        }
    }

    if (*ncol <= 0) {
        fprintf(stderr,"number of columns must be an integer > 0\n");
        exit(45);
    }
    *numfiles = argc - optind;
    return optind;

}

struct mtail* mtail_new(int numfiles) {
    struct mtail* mtst;

    mtst = (struct mtail*) calloc(1, sizeof(struct mtail));
    if (mtst == NULL) {
        fprintf(stderr,"could not calloc struct mtail\n");
        exit(45);
    }

    mtst->numfiles = numfiles;

    mtst->tst = (struct tail *) calloc(numfiles, sizeof(struct tail));
    if (mtst == NULL) {
        fprintf(stderr,"could not calloc struct tail\n");
        exit(45);
    }

    return mtst;
}


void open_files(char* argv[], int ind, struct mtail* mtst) {
    /* Open them files */
    int i, j;
    struct tail* tst;

    tst = mtst->tst;
    for (i=0;i<mtst->numfiles;i++) {
        if (strlen(argv[ind]) > MAXFILELEN) {
            fprintf(stderr,"File name too long: %s\n", argv[ind]);
            exit(45);
        }
        strncpy(tst[i].fname, argv[ind],MAXFILELEN);
        if ((tst[i].fd = open(tst[i].fname, O_RDONLY)) < 0) {
            fprintf(stderr,"Failed to open %s\n",tst[i].fname);
            for (j=0;j<i;j++) {
                close(tst[j].fd);
            }
            exit(45);
        }
        if (fstat(tst[i].fd, &(tst[i].sbuf)) < 0) {
            fprintf(stderr,"failed to fstat\n");
            exit(45);
        }
        ind++;
    }
}

void init_screen(struct mtail* mtst) {
    if ((mtst->stdscr = initscr()) == NULL) {
        perror("initscr\n");
        exit(45);
    }
}


int yseparator_position(struct mtail* mtst, int row) {
    int ysep_pos;
    if (row == 0) {
        ysep_pos = 0;
    } else {
        ysep_pos = (mtst->ywinsize+1)*row + mtst->extra_ychars;
    }
    return ysep_pos;
}

int xseparator_position(struct mtail* mtst, int col) {
    int xsep_pos;
    if (col == 0) {
        fprintf(stderr,"No separator in column 0\n");
        exit(45);
    } else {
        xsep_pos = mtst->extra_xchars + mtst->xwinsize*col + (col -1);
    }
    return xsep_pos;
}
void set_geometry(struct mtail* mtst) {

    struct tail* tst;
    int i, row, col;

    tst = mtst->tst;

    mtst->nrow = mtst->numfiles/mtst->ncol;
    if ((mtst->numfiles % mtst->ncol) != 0) {
        mtst->nrow += 1;
    }
    getmaxyx(mtst->stdscr, mtst->ymax, mtst->xmax);

    /* don't use the last line, helps with terminals that don't clear upon
     * quitting like within screen */
    //mtst->ymax -= 1;

    /* is right here because of our arithmetic for regions sizes */
    if (mtst->ymax < (mtst->numfiles * 4)) {
        fprintf(stderr,"Screen too small!\n");
        endwin();
        exit(45);
    }

    /* There will be a separator between rows, so subtract nrows */
    mtst->ywinsize     = (mtst->ymax - mtst->nrow)/mtst->nrow;
    mtst->extra_ychars = mtst->ymax - mtst->ywinsize*mtst->nrow - mtst->nrow;

    /* only separator *between* columns, so only subtract ncol-1 */
    mtst->xwinsize     = (mtst->xmax-(mtst->ncol-1))/mtst->ncol;
    mtst->extra_xchars = mtst->xmax - mtst->xwinsize*mtst->ncol - (mtst->ncol-1);

    col = -1;
    row = 0;

    for (i=0; i<mtst->numfiles; i++) {
        if ((i % mtst->nrow) == 0) {
            row = 0;
            col += 1;
        }

        if (row == 0) {
            /* size of window in y */
            tst[i].numrows = mtst->ywinsize + mtst->extra_ychars;
        } else {
            tst[i].numrows = mtst->ywinsize;
        }
        tst[i].startrow = yseparator_position(mtst, row)+1;


        if (col == 0) {
            tst[i].startcol = 0;
            tst[i].numcols = mtst->xwinsize + mtst->extra_xchars;
        } else {
            /* get separator for x: not first column... */
            tst[i].startcol = xseparator_position(mtst, col) + 1;
            tst[i].numcols = mtst->xwinsize;
        }

        row += 1;
    }

    return;

}

void set_line_bufs(struct mtail* mtst) {
    int i;
    struct tail* tst;

    tst = mtst->tst;
    for (i=0; i<mtst->numfiles; i++) {

        /* each window get's a line for reading from the file.  This we will
         * not free, but it will be used later as well */
        if ((tst[i].line_buf = (char *) calloc(tst[i].numcols, sizeof(char))) == NULL) {
            fprintf(stderr,"shit\n");
            exit(45);
        }
    }
}


void draw_borders(struct mtail* mtst) {
    int i, row, col, x, y;
    struct tail* tst;
    chtype linechar=0;

    tst = mtst->tst;

    for (col=1; col<mtst->ncol; col++) {
        x = xseparator_position(mtst,col);
        mvvline(0, x, linechar, mtst->ymax);
    }

    for (row=0; row<mtst->nrow; row++) {
        y = yseparator_position(mtst, row);
        mvhline(y,0,linechar,mtst->xmax);
    }


    /* fill in the gaps created by the lines */
    col = -1;
    row = 0;

    for (i=0; i<mtst->numfiles; i++) {
        if ((i % mtst->nrow) == 0) {
            row = 0;
            col += 1;
        }

        if (col != (mtst->ncol-1)) {
            y = tst[i].startrow-1;
            x = tst[i].startcol + tst[i].numcols;
            if (row == 0) {
                mvaddch(y, x, ACS_TTEE); /* T shape so we don't protrude above line */
            } else {
                mvaddch(y, x, ACS_PLUS); /* plus to fill in the gap */
            }
        }
        /* Determine if we need to draw a plus in the upper right */
        row += 1;
    }



    wrefresh(mtst->stdscr);
}

char *get_basename(char *path)
{
    char *base = strrchr(path, '/');
    return base ? base+1 : path;
}

void print_filenames(struct mtail* mtst) {
    int i, x, y;
    int maxlen, len;
    char name[MAXFILELEN];
    char ellipses[] = "...";
    struct tail* tst;

    tst = mtst->tst;
    char *bname;

    for (i=0; i<mtst->numfiles; i++) {

        // not a copy, just pointer
        bname = get_basename(tst[i].fname);
        //len = strlen(tst[i].fname);
        len = strlen(bname);
        maxlen = tst[i].numcols-2;

        if (len > maxlen) {
            //strncpy(name, &tst[i].fname[len-maxlen], MAXFILELEN);
            strncpy(name, &bname[len-maxlen], MAXFILELEN);
            memcpy(name, ellipses, 3);
            len = maxlen;
        } else {
            //strncpy(name, tst[i].fname, MAXFILELEN);
            strncpy(name, bname, MAXFILELEN);
        }

        y = tst[i].startrow-1;
        x = tst[i].startcol + (tst[i].numcols - len)/2;
        if (x < 0) x=0;

        mvwaddstr(mtst->stdscr, y, x, name);
    }
    wrefresh(mtst->stdscr);
}

/* set up each window.  Set the dividers and names */
void create_windows(struct mtail* mtst) {
    int i;
    struct tail* tst;

    tst = mtst->tst;

    for (i=0; i<mtst->numfiles; i++) {

        /* eli didn't use the last column.. */
        tst[i].win = subwin(mtst->stdscr, 
                tst[i].numrows, 
                /*tst[i].numcols-1,*/
                tst[i].numcols,
                tst[i].startrow,
                tst[i].startcol);

        scrollok(tst[i].win,1);

    }
    wrefresh(mtst->stdscr);

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



/* load the initial data */
void load_initial_file_data(struct mtail* mtst) {
    int i, ncols, retval;
    int j,ctr;
    char* line;

    struct tail* tst;
    tst = mtst->tst;

    /* Read in files and print out last number of lines */
    for (i=0; i<mtst->numfiles; i++) {

        ncols = tst[i].numcols;
        line = tst[i].line_buf;

        lseek(tst[i].fd, tst[i].sbuf.st_size - (tst[i].numrows+2)*ncols, SEEK_SET);

        if ((tst[i].buf = (char *) calloc(tst[i].numrows * ncols, sizeof(char))) == NULL) {
            fprintf(stderr,"Calloc buf failed\n");
            exit(45);
        }
        ctr = 0;
        do {
            retval = getline(tst[i].fd, ncols, line);
            if (ctr < (tst[i].numrows - 1)) {
                memcpy((tst[i].buf + ctr * ncols), line, ncols);
                ctr++;
            } else {
                memmove(tst[i].buf, tst[i].buf + ncols, ncols * (tst[i].numrows - 1));
                memcpy((tst[i].buf + ctr * ncols), line, ncols);
            }
        } while (retval != -1);

        for (j=0;j<tst[i].numrows;j++) {
            waddstr(tst[i].win,tst[i].buf + j * ncols);
        }
        wrefresh(tst[i].win);

        free(tst[i].buf);

    }

}
void tail_files(struct mtail* mtst) {
    int i, ncol, retval;
    struct stat sbuf_new;
    char* line;
    struct tail* tst;

    set_line_bufs(mtst);
    load_initial_file_data(mtst);

    tst = mtst->tst;

    while(1) {
        for(i=0;i<mtst->numfiles;i++) {

            line = tst[i].line_buf;
            ncol = tst[i].numcols;

            if (fstat(tst[i].fd, &sbuf_new) < 0) {
                exit(45);
            }
            if (sbuf_new.st_size > tst[i].sbuf.st_size) {
                //wrefresh(tst[i].win);
                memcpy(&(tst[i].sbuf), &sbuf_new, sizeof(sbuf_new));
                do {

                    retval = getline(tst[i].fd, ncol, line);
                    waddstr(tst[i].win,line);
                    //wrefresh(tst[i].win);

                } while (retval != -1);
                wrefresh(tst[i].win);
            }
        }
        usleep(mtst->poll_time);
    }

}


int main(int argc, char *argv[])
{

    struct mtail* mtst;

    int numfiles, ncol, ind;

    ind = parse_command_line(argc, argv, &ncol, &numfiles);

    if (numfiles == 0) {
        printf("Usage: mtail [-c ncol] file1 [file2 file3 ...]\n");
        exit(0);
    }

    mtst = mtail_new(numfiles);
    mtst->ncol = ncol;
    mtst->poll_time = POLLTIME;

    open_files(argv, ind, mtst);

    init_screen(mtst);
    set_geometry(mtst);
    draw_borders(mtst);
    create_windows(mtst);
    print_filenames(mtst);

    tail_files(mtst);

    sleep(2);
    endwin();

    return(0);
}


