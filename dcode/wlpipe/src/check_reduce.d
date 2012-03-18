import std.stdio;
import std.file;  // for dirEntries
import std.conv;  // for .to!long()
import std.algorithm; // for filter
import std.string; // for format
import vjson;

int main(string[] args) {
    if (args.length < 3) {
        writeln("usage: check-reduce dir expected_num");
        return 1;
    }

    auto dir=args[1];
    long expected_num = args[2].to!long();
    //auto entries = 
    //  filter!`endsWith(a.name,"-check.json")`(dirEntries(dir,SpanMode.shallow));
    auto entries = dirEntries(dir,"*-check.json",SpanMode.shallow);

    long count=0;
    foreach (entry; entries) {
        count ++;

        auto json_text=readText(entry.name);
        auto root = parseJSON(json_text);
        auto dict = root.getDict();

        auto estring = dict["error_string"].getString();
        if (estring != "") {
            writeln(entry.name);
        }
    }
    if (count < expected_num) {
        throw new Exception(format("Expected %s entries, found %s",
                                   expected_num, count));
    }
    return 0;
}
