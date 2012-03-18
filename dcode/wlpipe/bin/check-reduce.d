import std.stdio;
import std.file;  // for dirEntries
import std.conv;  // for .to!long()
import std.algorithm; // for filter
import vjson;

int main(string[] args) {
    if (args.length < 3) {
        writeln("usage: check-reduce dir expected_num");
        return 1;
    }

    auto dir=args[1];
    long expected_num = args[2].to!long();
    auto files = 
      filter!`endsWith(a.name,"-check.json")`(dirEntries(".",SpanMode.depth));

    foreach (name; files) {
        writeln("file: ",name);
    }
    return 0;
}
