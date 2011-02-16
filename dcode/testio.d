import std.stdio;
import std.random;
import std.string;

struct Sources {
    int index;
    float fdata;
};

void write_header(File f, size_t nrows) {
    // need to use right width here for updating
    f.writef("SIZE = %s\n", nrows);
    f.writeln("{'_DTYPE': [('index', '<i4'),('fdata','f4')],");
    f.writeln(" '_VERSION': '1.0'}");
    f.writeln("END\n");

}

size_t read_header(File f) {
    // for now just skip to END
    char[] line;

    // first row should be NROWS = somenumber
    char[] key;
    char[] junk;
    size_t nrows;

    f.readf("%s %s %d", &key, &junk, &nrows);
    if (key.tolower() != "size") {
        throw new Exception("Expected SIZE = number in first line");
    }

    while (f.readln(line)) {
        if (line == "END\n") {
            // read one more line, which should be blank
            f.readln(line);
            if (line != "\n") {
                throw new Exception("Expected blank line after END");
            }
            break;
        }
    }
    return nrows;
}

void write_row(File f, Sources* s) {
    FILE* fptr = f.getFP();

    size_t nwrite;
    nwrite = fwrite (&s.index, int.sizeof, 1, fptr);
    if (nwrite != 1) {
        writefln("Failed to write one int. nwrite=%d\n", nwrite);
        throw new Exception("io error");
    }
    nwrite = fwrite (&s.fdata, float.sizeof, 1, fptr);
    if (nwrite != 1) {
        writefln("Failed to write one int. nwrite=%d\n", nwrite);
        throw new Exception("io error");
    }
}
void read_row(File f, Sources* s) {
    FILE* fptr = f.getFP();

    // for errors
    size_t nread;
    //int s = int.sizeof;
    nread = fread(&s.index, int.sizeof, 1, fptr);
    if (nread != 1) {
        writefln("Failed to read one int. nread=%d\n", nread);
        throw new Exception("io error");
    }
    nread = fread(&s.fdata, float.sizeof, 1, fptr);
    if (nread != 1) {
        writefln("Failed to read one float. nread=%d\n", nread);
        throw new Exception("io error");
    }
}


int main (char[][] args) 
{
    Random gen;

    Sources[] sout;
    sout.length = 1000000;

    for (size_t i=0; i<sout.length; i++) {
        sout[i].index = i;
        sout[i].fdata = uniform(0.0L, 100.0L, gen);
    }

    auto f = File("testio.rec", "w+");

    write_header(f, sout.length);
    for (size_t i=0; i<sout.length; i++) {
        write_row(f, &sout[i]);
    }


    // now read it back in
    f.rewind();

    size_t nrows = read_header(f);
    Sources[] sin;
    sin.length = nrows;

    for (size_t i=0; i<sout.length; i++) {
        read_row(f, &sin[i]);
    }
    f.close();

    // compare the data
    /*
    for (size_t i=0; i<sout.length; i++) {
        writefln("indexout: %10s  indexin: %10s    fdataout: %10s  fdatain: %10s", 
                 sout[i].index, sin[i].index, sout[i].fdata, sin[i].fdata);
    }
    */

    size_t ind=0;
    writefln("indexout: %10s  indexin: %10s    fdataout: %10s  fdatain: %10s", 
            sout[ind].index, sin[ind].index, sout[ind].fdata, sin[ind].fdata);
    ind=sout.length-1;
    writefln("indexout: %10s  indexin: %10s    fdataout: %10s  fdatain: %10s", 
            sout[ind].index, sin[ind].index, sout[ind].fdata, sin[ind].fdata);
	return 0;
}
