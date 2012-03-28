import std.stdio;
//import std.json;
import vjson;
import std.file;
import std.string;
import std.conv;

import std.variant;

int main(string[] args)
{
    //string json_file=args[1];
    string json_file="/home/esheldon/tmp/decam--25--39-i-4-35-check.json";
    //string json_file="/home/esheldon/tmp/impyp001i-goodlist.json";
    string json_text=readText(json_file);

    auto root = parseJSON(json_text);

    //auto array = root.getArray();

    auto dict = root.getDict();


    real rval;
    long lval;
    string sval;

    rval = dict["flt"].getReal();
    writefln("got flt getReal: %.19g",rval);
    sval = dict["flt"].coerce!string();
    writefln("got flt coerce string: '%s'",sval);

    long status = dict["exit_status"].coerce!long();
    string run = dict["run"].getString();
    bool tbool = dict["tbool"].getBool();
    writefln("got exit_status: %s", status);
    writefln("got run: '%s'", run);
    writefln("got tbool: %s", tbool);

    writefln("type of 'input_files': %s",dict["input_files"].type);
    auto fdict = dict["input_files"].getDict();
    foreach (name,member; fdict) {
        writefln("    input_file['%s']: '%s'",name,member.getString());
    }
    writefln("type of 'tarray': %s",dict["tarray"].type);
    auto tarray = dict["tarray"].getArray();
    foreach (i, jval; tarray) {
        writefln("    tarray[%s]: %s", i, jval.getLong());
    }

    return 0;
}
