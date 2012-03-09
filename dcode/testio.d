import std.stdio;
import std.random;
import std.string;

struct Sources {
    ptrdiff_t index;
    float fdata;
};

void write_header(File f, ptrdiff_t nrows) {
    // need to use right width here for updating
    f.writef("SIZE = %s\n", nrows);
    f.writeln("{'_DTYPE': [('index', '<i4'),('fdata','f4')],");
    f.writeln(" '_VERSION': '1.0'}");
    f.writeln("END\n");

}

ptrdiff_t read_header(File f) {
    // for now just skip to END
    char[] line;

    // first row should be NROWS = somenumber
    char[] key;
    char[] junk;
    ptrdiff_t nrows;

    f.readf("%s %s %d", &key, &junk, &nrows);
    if (key != "SIZE") {
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
    ptrdiff_t nwrite;
    f.writef("%s,%.16g\n",s.index,s.fdata);
}
void read_row(File f, Sources* s) {
    auto nread = f.readf("%d,%f\n",&s.index,&s.fdata);
    if (nread != 2) {
        throw new Exception("failed to read source");
    }
}


int main (char[][] args) 
{
    Random gen;

    Sources[] sout;
    sout.length = 1000000;

    for (ptrdiff_t i=0; i<sout.length; i++) {
        sout[i].index = i;
        sout[i].fdata = uniform(0.0L, 100.0L, gen);
    }

    auto f = File("testio.rec", "w+");

    writefln("writing header");
    write_header(f, sout.length);
    writefln("writing rows");
    for (ptrdiff_t i=0; i<sout.length; i++) {
        write_row(f, &sout[i]);
    }


    // now read it back in
    f.rewind();

    ptrdiff_t nrows = read_header(f);
    Sources[] sin;
    sin.length = nrows;

    writefln("reading %s rows",nrows);
    for (ptrdiff_t i=0; i<sout.length; i++) {
        read_row(f, &sin[i]);
    }
    f.close();

    ptrdiff_t ind=0;
    writefln("indexout: %10s  indexin: %10s    fdataout: %.16g  fdatain: %.16g", 
            sout[ind].index, sin[ind].index, sout[ind].fdata, sin[ind].fdata);
    ind=sout.length-1;
    writefln("indexout: %10s  indexin: %10s    fdataout: %.16g  fdatain: %.16g", 
            sout[ind].index, sin[ind].index, sout[ind].fdata, sin[ind].fdata);
	return 0;
}
