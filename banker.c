#define _POSIX_C_SOURCE 200809L
#define CUSTOMER_FILE "customer.txt"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

static int n_resources = 0;
static int *available = NULL;
static int n_customers = 0;
static int **maximum = NULL;
static int **allocation = NULL;
static int **need = NULL;

static void free_all(void){
    for (int i = 0; i < n_customers; i++){
        free(maximum[i]);
        if (allocation){
            free(allocation[i]);
        }
        if (need){
            free(need[i]);
        }
    }
    free(allocation);
    free(need);
    free(maximum);
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

static void read_customers(void){
    FILE *f = fopen(CUSTOMER_FILE, "r");
    if (!f){
        fatal("não foi possível abrir o arquivo %s: %s.", CUSTOMER_FILE, strerror(errno));
    }

    char *line = NULL;
    size_t cap = 0;
    int line_no = 0;
    int capacity = 0;
    while (getline(&line, &cap, f) != -1){
        line_no++;
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0'){
            continue;
        }
        int fields = 1;
        for (char *p = line; *p; p++){
            if (*p == ','){
                fields++;
            }
        }
        if (fields != n_resources){
            free(line);
            fclose(f);
            fatal("%s, linha %d: o cliente possui %d recurso(s), mas foram " "informados %d recurso(s) na linha de comando.", CUSTOMER_FILE, line_no, fields, n_resources);
        }
        if (n_customers == capacity){
            capacity = capacity ? capacity * 2 : 8;
            int **tmp = realloc(maximum, sizeof(int *) * capacity);
            if (!tmp){
                free(line);
                fclose(f);
                fatal("memória insuficiente.");
            }
            maximum = tmp;
        }
        maximum[n_customers] = xmalloc(sizeof(int) * n_resources);
        n_customers++;
        char *p = line;
        for (int j = 0; j < n_resources; j++){
            char *comma = strchr(p, ',');
            if (comma){
                *comma = '\0';
            }
            if (!parse_nonneg_int(p, &maximum[n_customers - 1][j])){
                char copy[64];
                snprintf(copy, sizeof copy, "%s", p);
                free(line);
                fclose(f);
                fatal("%s, linha %d: valor \"%s\" inválido (esperado inteiro " "não negativo; formato: v1,v2,...,vn).", CUSTOMER_FILE, line_no, copy);
            }
            if (comma){
                p = comma + 1;
            }
        }
    }
    free(line);
    fclose(f);
    if (n_customers == 0){
        fatal("o arquivo %s está vazio (nenhum cliente encontrado).", CUSTOMER_FILE);
    }
    allocation = xmalloc(sizeof(int *) * n_customers);
    need = xmalloc(sizeof(int *) * n_customers);
    for (int i = 0; i < n_customers; i++){
        allocation[i] = xmalloc(sizeof(int) * n_resources);
        need[i] = xmalloc(sizeof(int) * n_resources);
        memcpy(need[i], maximum[i], sizeof(int) * n_resources);
    }
}

int main(int argc, char *argv[]){
    parse_arguments(argc, argv);
    parse_arguments(argc, argv);
    read_customers();
    for (int i = 0; i < n_customers; i++){
        for (int j = 0; j < n_resources; j++) printf("%d ", maximum[i][j]);
        printf("| ");
        for (int j = 0; j < n_resources; j++) printf("%d ", allocation[i][j]);
        printf("| ");
        for (int j = 0; j < n_resources; j++) printf("%d ", need[i][j]);
        printf("\n");
    }
    free_all();
    return EXIT_SUCCESS; 
}