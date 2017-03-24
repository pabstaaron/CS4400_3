int a;
extern int b;
int do_something(int v) {
  b = a;
  a = v;
  return b;
}

int do_something_else(int v) {
  return v;
}
