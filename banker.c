#define _POSIX_C_SOURCE 200809L
#define CUSTOMER_FILE "customer.txt"
#define COMMANDS_FILE "commands.txt"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

typedef enum { CMD_REQUEST, CMD_RELEASE, CMD_PRINT } cmd_type_t;

typedef struct {
    cmd_type_t type;
    int customer;
    int *values;
} command_t;

static int n_resources = 0;
static int *available = NULL;
static int n_customers = 0;
static int **maximum = NULL;
static int **allocation = NULL;
static int **need = NULL;
static command_t *commands = NULL;
static int n_commands = 0;

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
    for (int k = 0; k < n_commands; k++){
        free(commands[k].values);
    }
    free(commands);
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
            if (capacity == 0){
                capacity = 8;
            } else {
                capacity = capacity * 2;
            }
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

static void read_commands(void){
    FILE *f = fopen(COMMANDS_FILE, "r");
    if (!f){
        fatal("não foi possível abrir o arquivo %s: %s.", COMMANDS_FILE, strerror(errno));
    }

    char *line = NULL;
    size_t cap = 0;
    int line_no = 0;
    int capacity = 0;

    while (getline(&line, &cap, f) != -1){
        line_no++;
        line[strcspn(line, "\r\n")] = '\0';
        char *tok = strtok(line, " \t");
        if (tok == NULL){
            continue;
        }
        if (n_commands == capacity){
            if (capacity == 0){
                capacity = 16;
            } else {
                capacity = capacity * 2;
            }
            command_t *tmp = realloc(commands, sizeof(command_t) * capacity);
            if (!tmp){
                free(line);
                fclose(f);
                fatal("memória insuficiente.");
            }
            commands = tmp;
        }
        command_t *cmd = &commands[n_commands];
        cmd->values = NULL;
        cmd->customer = -1;
        n_commands++;
        if (strcmp(tok, "*") == 0){
            cmd->type = CMD_PRINT;
            continue;
        }
        if (strcmp(tok, "RQ") == 0){
            cmd->type = CMD_REQUEST;
        } else if (strcmp(tok, "RL") == 0){
            cmd->type = CMD_RELEASE;
        } else {
            char copy[32];
            snprintf(copy, sizeof copy, "%s", tok);
            free(line);
            fclose(f);
            fatal("%s, linha %d: comando \"%s\" desconhecido (esperado RQ, RL ou *).", COMMANDS_FILE, line_no, copy);
        }
        tok = strtok(NULL, " \t");
        if (!parse_nonneg_int(tok, &cmd->customer)){
            free(line);
            fclose(f);
            fatal("%s, linha %d: número do cliente ausente ou inválido.", COMMANDS_FILE, line_no);
        }

        cmd->values = xmalloc(sizeof(int) * n_resources);
        for (int j = 0; j < n_resources; j++){
            tok = strtok(NULL, " \t");
            if (!parse_nonneg_int(tok, &cmd->values[j])){
                free(line);
                fclose(f);
                fatal("%s, linha %d: quantidade de recurso ausente ou inválida.", COMMANDS_FILE, line_no);
            }
        }
    }
    free(line);
    fclose(f);
}

int main(int argc, char *argv[]){
    parse_arguments(argc, argv);
    parse_arguments(argc, argv);
    read_customers();
    read_commands();
    for (int k = 0; k < n_commands; k++){
        if (commands[k].type == CMD_PRINT){
            printf("*\n");
            continue;
        }
        if (commands[k].type == CMD_REQUEST){
            printf("RQ %d", commands[k].customer);
        } else {
            printf("RL %d", commands[k].customer);
        }        
        for (int j = 0; j < n_resources; j++){
            printf(" %d", commands[k].values[j]);
        }
        printf("\n");
    }
    free_all();
    return EXIT_SUCCESS; 
}