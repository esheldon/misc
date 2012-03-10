import std.stdio;
import std.string;
import std.conv;
import Polygon;

class MangleMask {
    string filename;
    File* file;

    bool snapped;
    bool balkanized;

    long npoly;
    Polygon[] polygons;

    int pixelres=-1; // -1 for not pixelized
    char pixeltype='u';
    long maxpix;

    // for each pixel number, an array of poly_ids contained
    long[][] pixlist;

    int verbose=0;

    char[] buff;
    char[][] lsplit;

    this(in string filename, in int verbose) {
        this.verbose=verbose;
        this(filename);
    }
    this(in string filename) {
        this.load_mask(filename);
    }

    void load_mask(in string filename) {
        this.filename=filename;
        this.file = new File(filename,"r");
        if (this.verbose) {
            stderr.writeln("Loading mask: ",filename);
        }

        this.process_header();
        if (this.verbose) {
            stderr.write(cast(string)this);
        }
        this.read_polygons();
    }

    /*
     * We expect this.lsplit to have the last split line
     */
    private void read_polygons() {
        this.polygons.length = this.npoly;
        for (long i=0; i<this.npoly; i++) {
            if (this.lsplit[0] != "polygon") {
                throw new Exception(format(
                  "Expected polygon keyword for poly %s, got %s", 
                  i, this.lsplit[0]));
            }
            this.polygons[i].read(this.file, this.lsplit);
            if (this.verbose > 1) {
                stderr.writeln(cast(string)this.polygons[i]);
                if (this.verbose > 2) {
                    this.polygons[i].print_caps();
                }
            }

            this.file.readln(this.buff);
            this.lsplit=this.buff.split();
        }
    }

    /*
     * Expect something like this, with all optional except 
     * for the polygons line.  We stop processing when we get
     * to the first line starting with "polygon"
     *
     * We leave last split line in this.split, which should be the first polygon
     * line
     *
        207684 polygons
        pixelization 9s
        snapped
        balkanized
        polygon 0 ( 4 caps, 1 weight, 87381 pixel, 0.000000000129178 str):
    */
    private void process_header() {
        //long pos;
        string mess;

        //pos = this.file.tell();
        this.file.readln(this.buff);
        this.lsplit=this.buff.split();
        while (this.lsplit[0] != "polygon") {
            if (this.lsplit.length == 2) {
                if (this.lsplit[1] == "polygons") {
                    this.npoly = this.extract_npoly(this.lsplit[0]);
                } else if (this.lsplit[0] == "pixelization") {
                    this.extract_pixelization(this.lsplit[1]);
                }
            } else if (this.lsplit.length == 1) {
                if (this.lsplit[0] == "snapped") {
                    this.snapped = true;
                } else if (this.lsplit[0] == "balkanized") {
                    this.balkanized = true;
                } else {
                    mess="unexpected header keyword: " ~ 
                        this.lsplit[0].to!string();
                    throw new Exception(mess);
                }
            } else {
                mess = format("Found too many words in header line: '%s'",
                              this.buff);
                throw new Exception(mess);
            }

            //pos = this.file.tell();
            this.file.readln(this.buff);
            this.lsplit=this.buff.split();
        }

        //this.file.seek(pos);

    }


    private void extract_pixelization(char[] pixspec) {
        char ptype = pixspec[$-1];
        if (ptype == 's' || ptype == 'u') {
            this.pixeltype = ptype;
        } else {
            throw new Exception(
                    format("pixel scheme must be 'u' or 's', got '%s'", ptype));
        }
        this.pixelres = pixspec[0..$-1].to!int();
    }

    // expect first line as {npoly} polygons
    private long extract_npoly(char[] npoly_str) {
        string mess;
        long npoly = npoly_str.to!long();
        if (npoly <= 0) {
            mess=format("got npoly=%s from header", npoly);
            throw new Exception(mess);
        }
        return npoly;
    }

    string opCast() {
        string rep;
        rep = format(q"EOS
MangleMask:
    file: %s
    npoly:         %s
    snapped:       %s
    balkanized     %s
    pixelization: '%s'
    pixelres:      %s
EOS", this.filename, npoly, this.snapped, this.balkanized, 
            this.pixeltype, this.pixelres);

        return rep;
    }

}
