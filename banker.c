#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static int n_resources = 0;
static int *available = NULL;

static void fatal(const char *fmt, ...){
    va_list ap;
    fprintf(stderr, "Erro: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void parse_arguments(int argc, char *argv[]){
    if (argc < 2){
        fatal("nenhum recurso informado.\nUso: %s <inst_0> <inst_1> ...", argv[0]);
    }

    n_resources = argc - 1;
    available = malloc(sizeof(int) * n_resources);

    for (int j = 0; j < n_resources; j++){
        available[j] = atoi(argv[j + 1]);
    }
}

int main(int argc, char *argv[]){
    parse_arguments(argc, argv);
    for (int j = 0; j < n_resources; j++){
        printf("recurso %d: %d instancias\n", j, available[j]);
    }
    free(available);
    return EXIT_SUCCESS;
}