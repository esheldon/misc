import std.stdio;

class Blah {
    int i=0;
    this (int i) {
        this.i=i;
    }
    void say() {
        writeln("my val is ",this.i);
    }
}
int main (char[][] args) 
{

    Blah[] b;
    b.length = 10;
    b[1]=new Blah(1);
    b[5]=new Blah(5);

    foreach (bb; b) {
        if (bb) {
            bb.say();
        }
        if (!bb) {
            writeln("bad");
        }
    }

    return 0;
}
