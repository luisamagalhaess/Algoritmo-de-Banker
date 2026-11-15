#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

static int n_resources = 0;
static int *available = NULL;

static void free_all(void){
    free(available);
}

static void fatal(const char *fmt, ...){
    va_list ap;
    fprintf(stderr, "Erro: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    free_all();
    exit(EXIT_FAILURE);
}

static void *xmalloc(size_t size){
    void *p = calloc(1, size);
    if (!p){
        fatal("memória insuficiente.");
    }
    return p;
}

static bool parse_nonneg_int(const char *s, int *out){
    if (s == NULL || *s == '\0') return false;
    for (const char *p = s; *p; p++){
        if (!isdigit((unsigned char)*p)) return false;
    }

    errno = 0;
    char *end;
    long v = strtol(s, &end, 10);
    if (errno == ERANGE || v > INT_MAX) return false;
    *out = (int)v;
    return true;
}

static void parse_arguments(int argc, char *argv[]){
    if (argc < 2){
        fatal("nenhum recurso informado.\nUso: %s <inst_0> <inst_1> ...", argv[0]);
    }

    n_resources = argc - 1;
    available = xmalloc(sizeof(int) * n_resources);

    for (int j = 0; j < n_resources; j++){
        if (!parse_nonneg_int(argv[j + 1], &available[j])){
            fatal("argumento %d (\"%s\") inválido: o número de instâncias de " "cada recurso deve ser um inteiro não negativo.", j + 1, argv[j + 1]);
        }
    }
}

int main(int argc, char *argv[]){
    parse_arguments(argc, argv);
    for (int j = 0; j < n_resources; j++){
        printf("recurso %d: %d instancias\n", j, available[j]);
    }
    free_all();
    return EXIT_SUCCESS; 
}