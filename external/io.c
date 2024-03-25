#include <stdio.h>
#include <stdlib.h>

int write_int(int x) {
    printf("%d", x);
    return 0;
}

int write_double(double x) {
    printf("%.3f", x);
    return 0;
}

int writeln_int(int x) {
    printf("%d\n", x);
    return 0;
}

int writeln_double(double x) {
    printf("%.3f\n", x);
    return 0;
}

int readln_int(int *x) {
    scanf("%d", x);
    return 0;
}

int readln_double(double *x) {
    scanf("%lf", x);
    return 0;
}

int error(char *s) {
    printf("%s", s);
    exit(1);
}
