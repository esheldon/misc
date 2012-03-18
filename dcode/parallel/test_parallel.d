import std.concurrency, std.stdio;

void main() {
   auto low = 0, high = 100;
   spawn(&fun, low, high);
   foreach (i; low .. high) {
      writeln("Main thread: ", i);
   }
}

void fun(int low, int high) {
   foreach (i; low .. high) {
      writeln("Secondary thread: ", i);
   }
}
