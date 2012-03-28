import std.stdio;
import std.file;  // for dirEntries
import std.conv;  // for .to!long()
import std.string; // for format
import vjson;

int main(string[] args) {
    if (args.length < 3) {
        writeln("usage: check-reduce dir expected_num");
        return 1;
    }

    auto dir=args[1];
    long expected_num = args[2].to!long();
    auto entries = dirEntries(dir,"*-check.json",SpanMode.shallow);

    long count=0;

    JSONValue out_root;

    out_root.type = JSON_TYPE.ARRAY;

    JSONValue val;
    foreach (entry; entries) {

        count ++;

        auto json_text=readText(entry.name);
        auto this_root = parseJSON(json_text);
        auto dict = this_root.getDict();

        auto estring = dict["error_string"].getString();
        if (estring != "") {
            writeln(entry.name);
            // inefficient
            out_root.array ~= this_root;
        }
    }
    writefln("out_root.array.length: %s", out_root.array.length);
    if (count < expected_num) {
        throw new Exception(format("Expected %s entries, found %s",
                                   expected_num, count));
    }

    string out_json_string = toJSON(&out_root);
    writeln(out_json_string);
    return 0;
}
