import std.stdio;
//import std.json;
import vjson;
import std.file;
import std.string;
import std.conv;

import std.variant;

/*
@property string toString(in JSONValue jval) @safe pure nothrow
{
    return "hello";
}
*/

class Blah {
    int x;
}

//@property string myToString(in Blah)  @safe pure nothrow {
//    return "hello";
//}

string myToString(in int[] arr) {
    return "hello";
}



string toString(in JSONValue jval) @safe pure nothrow
{
    return "hello";
}

//string myToString(in int val) @safe pure nothrow
//{
//    return "hello";
//}


T getVal(T)() {
    T val=24;
    return val;
}

T getJsonVal(T)(JSONValue jval) {
    T val;
    if (is(T == long)) {
        switch (jval.type) {
            case JSON_TYPE.STRING: val = jval.str.to!T();         break;
            case JSON_TYPE.INTEGER: val = cast(T)jval.integer; break;
            case JSON_TYPE.FLOAT: val = cast(T)jval.floating;  break;
            default: throw new Exception(format(
                  "Cannot convert json type '%s' to %s", jval.type, T.stringof));
                break;
        }
    } else if (is(T == real)) {
        switch (jval.type) {
            case JSON_TYPE.STRING: val = jval.str.to!T();break;
            case JSON_TYPE.INTEGER: val = cast(T)jval.integer;break;
            case JSON_TYPE.FLOAT: val = cast(T)jval.floating;break;
            default: throw new Exception(format(
                  "Cannot convert json type '%s' to real", jval.type));
                break;

        }
    } else {
    }

    return val;

}
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
