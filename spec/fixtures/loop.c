#include <stdio.h>

int main(int argc, char** argv) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += i;
        printf("i = %d, sum = %d\n", i, sum);
    }
    printf("Final sum: %d\n", sum);
    return 0;
}
