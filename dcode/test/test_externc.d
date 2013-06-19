import std.stdio;
import std.string;

// no linking needed for either of this.  Just works for C standard library!
extern (C) int strcmp(const char* string1, const char* string2);
extern (C) double cos(double x);

int myDfunction(char[] s) {
  return strcmp(std.string.toStringz(s), "foo");
}

int main(string[] args) {
    auto test_stuff = myDfunction("stuff".dup);
    auto test_foo = myDfunction("foo".dup);

    writeln("test of 'stuff': ",test_stuff);
    writeln("test of 'foo': ",test_foo);

    auto cos5 = cos(0.5);
    writeln("test of cos(0.5): ",cos5);
    return 0;
}
