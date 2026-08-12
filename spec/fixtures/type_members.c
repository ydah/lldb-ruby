#include <stdint.h>

struct lldb_test_struct {
  int first;
  unsigned int flags : 3;
  double second;
};

int lldb_test_type(struct lldb_test_struct value) {
  return value.first;
}

int main(void) {
  struct lldb_test_struct value = {42, 5, 3.5};
  return lldb_test_type(value);
}
