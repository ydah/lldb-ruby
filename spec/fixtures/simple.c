#include <stdio.h>

int lldb_test_add(int a, int b) {
    int result = a + b;
    return result;
}

int main(int argc, char** argv) {
    int x = 10;
    int y = 20;
    int sum = lldb_test_add(x, y);
    printf("Sum: %d\n", sum);
    return 0;
}
