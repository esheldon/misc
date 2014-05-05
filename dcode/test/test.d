import std.stdio;

enum ulong MAXDIM=1024;

struct Image(T) {
    // backing storage
    T[MAXDIM][MAXDIM] data;

    // used size
    long _nrow, _ncol;

    /*
    this(T)(this) {
        _nrow=MAXDIM;
        _ncol=MAXDIM;
    }
    */
    this(long nrow, long ncol) {

        assert(nrow >= 0
               && nrow <= MAXDIM
               && ncol >= 0
               && ncol <= MAXDIM);

        _nrow=nrow;
        _ncol=ncol;
    }

    @property long nrow() const {
        return _nrow;
    }
    @property long ncol() const {
        return _ncol;
    }

    @property void nrow(long nr) {
        assert(nr >= 0 && nr <= MAXDIM);
        _nrow=nr;
    }
    @property void ncol(long nc) {
        assert(nc >= 0 && nc <= MAXDIM);
        _ncol=nc;
    }

}

int main() {
    long nrow=3, ncol=5;

    writefln("hello");
    auto image = Image!double(nrow, ncol);
    /*

    writefln("nrow: %s ncol: %s",image.nrow, image.ncol);

    image.nrow=200;
    image.ncol=80;

    writefln("nrow: %s ncol: %s",image.nrow, image.ncol);
    */

    return 0;
}
