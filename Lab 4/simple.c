
#include <stdio.h>

int main() {
    FILE *file = fopen("/home/security/labs-sec/lab2/simple_example/data/simple.txt", "w");
    fprintf(file, "Hello World!");
    fclose(file);

    FILE *file_secret = fopen("/home/security/labs-sec/lab2/simple_example/data/secret.txt", "r");
    char buf[10];
    while (fgets(buf, 10, file_secret) != NULL) {
        printf("%s", buf);
    }
    fclose(file_secret);
}
