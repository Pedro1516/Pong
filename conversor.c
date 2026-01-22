#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 3) return printf("Uso: %s arquivo_entrada.png nome_da_variavel\n", argv[0]);
    FILE *in = fopen(argv[1], "rb");
    printf("unsigned char %s[] = {", argv[2]);
    unsigned char b;
    int count = 0;
    while (fread(&b, 1, 1, in)) {
        printf("0x%02x, ", b);
        if (++count % 12 == 0) printf("\n");
    }
    printf("};\nunsigned int %s_size = %d;\n", argv[2], count); 
    fclose(in);
    return 0;
}