# Algoritmo do Banqueiro

Implementação em C do Algoritmo do Banqueiro (*Banker's Algorithm*), apresentado em
*Operating System Concepts* (Silberschatz et al., 10ª edição), para evitar deadlocks
na alocação de recursos.

O número de clientes e de tipos de recurso **não é fixo**: o número de recursos é dado
pelos argumentos da linha de comando, e o número de clientes, pelas linhas do `customer.txt`.

## Compilação e execução

Requer `gcc` e `make` em Linux, Unix ou macOS.

```bash
make                 # compila e gera o executável banker
./banker 10 5 7      # executa com 10, 5 e 7 instâncias dos recursos 0, 1 e 2
make run             # mesmo que ./banker 10 5 7
make clean           # remove o executável
```

## Arquivos de entrada

Os arquivos `customer.txt` e `commands.txt` não fazem parte deste repositório (são fornecidos
pelo professor) e devem estar na mesma pasta do executável.

**`customer.txt`**: uma linha por cliente, com o máximo de cada recurso que ele pode pedir, separado por vírgulas:
```
7,5,3
3,2,2
```

**`commands.txt`**: um comando por linha:
| Comando | Significado |
|---|---|
| `RQ c r0 r1 ...` | cliente `c` pede recursos |
| `RL c r0 r1 ...` | cliente `c` devolve recursos |
| `*` | grava o estado atual (MAXIMUM, ALLOCATION, NEED e AVAILABLE) |

## Saída

O resultado de cada comando é gravado em **`result.txt`**. Um pedido (RQ) é negado quando:
1. excede o que o cliente ainda pode pedir (`exceed its maximum allocation`);
2. não há recursos disponíveis suficientes (`are not enough`);
3. deixaria o sistema em estado inseguro (`result in an unsafe state`).

Uma devolução (RL) maior do que o cliente tem alocado também é negada.

## Tratamento de erros

Todas as entradas são validadas **antes** de qualquer processamento. O programa mostra uma
mensagem de erro e encerra, sem gerar `result.txt`, quando:
- o número de argumentos é diferente do número de recursos em `customer.txt` ou `commands.txt`;
- algum argumento não é um inteiro não negativo;
- `customer.txt` ou `commands.txt` não existe, não pode ser aberto ou está vazio;
- há valores inválidos, campos vazios, comandos desconhecidos ou clientes inexistentes.

Arquivos com quebra de linha do Windows (CRLF) são aceitos.

## Evidências

O arquivo `evidencias.log` registra as execuções do programa no terminal, com a entrada do
enunciado e com os casos de erro tratados.