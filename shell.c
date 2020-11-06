
/* Inclusão das bibliotecas necessárias para o funcionamento do programa */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>

#define MAXARGS 128                                                                 /* Número máximo de argumentos que poderão ser passados */
#define MAXLINE 500                                                                 /* tamanho máximo da linha que será passada como comando */

#define KGRN  "\x1B[32m"
#define KBLU  "\x1B[34m"
#define KWHT  "\x1B[37m"
#define KYEL  "\x1B[33m"
#define KCYN  "\x1B[36m"
#define BB    "\033[1;34m"
#define BG    "\033[1;32m"
#define RESET "\033[0m"

char *dir;
extern char **environ;                                                              /* lista de ponteiros das variáveis de ambiente */

typedef struct{
    pid_t pid;
    char path[100];
} Processo;

/* Protótipo das funções que serão futuramente utilizadas */
void eval(char *cmdline);                                                           /* Analisa a linha de comando passada pelo usuário */
int parseline(char *buf, char **argv);                                              /* Recorta o input para a sua análise, construindo o vetor de argumentos que foram passados */
int builtin_command(char **argv);                                                   /* Comandos definidos que a Shell executa */
void unix_error(const char *msg);                                                   /* Mensagem de erro que será usada apenas para a função waitpid */
int waitpid(pid_t pid, int *statusp, int options);                                  /* Função waitpid na qual o processo pai espera pelo término/parada do processo filho */
void handler(int sig);
void printShellName(void);


char shell_name[MAXLINE] = "mabshell> ";

void handler(int sig){
    
}

void printShellName (void){
    char buf[MAXLINE];
    char *shell_name = "mabshell";
    
    dir = getcwd(buf, sizeof(buf));

    char check[MAXLINE] = "/home/";
    int i, equals = 0;

    for (i = 0; check[i] != '\0'; i++){
        if (*(dir + i) == check[i]){
            equals = 1;
        } else {
            equals = 0;
            break;
        }
    }
    if (equals){
        int slashes = 0;
        while (slashes < 2){
            if (*dir == '/') slashes ++;
            dir++;
        }
        while (*dir != '/' && *dir != '\0') dir++;
        if (*dir == '/') {
            dir--;
            *dir = '~';
        } else {
            dir--;
            *dir = '~';
        }
    }
    printf("%s%s%s:%s%s%s> ", KYEL, shell_name, KWHT, KCYN, dir, KWHT);
}


void unix_error(const char *msg)    /* Função para o print de erros */
{
    int errnum = errno;                                                             /* em errno temos o tipo de erro recebido. Então, criamos uma variável chamada errnum que armazenará esse valor */
    fprintf(stderr, "%s (%d: %s)\n", msg, errnum, strerror(errnum));                /* fprintf para o print da mensagem de erro, de acordo com o erro na variável errnum */
    return;                                                                         /* Como tivemos um erro, damos um return encerrando o processo */
}

int main(int argc, char *argv[]) {    /* Função main */

    char cmdline[MAXLINE];                                                          /* String que armazenará o input do usuário na linha de comando da nossa Shell */
    while (1) {                                                                     /* Loop infinito enquanto o usuário estiver na Shell */
    
        printShellName();                                                           /* Printamos o nome da shell e damos um espaço para o usuário poder distinguir */
        fgets(cmdline, MAXLINE, stdin);                                             /* Pegamos o input dado pelo usuário e colocamos na string cmdline definida anteriormente, e com o tamanho máximo de MAXLINE, sendo a entrada padrão definida no C */
        if (feof(stdin)){                                                           /* Se for detectado o símbolo de EOF no input dado pelo usuário, terminamos o loop */
            exit(0);                                                                /* Dando exit(0) */

        }
        eval(cmdline);                                                              /* Enquanto o usuário entrar com o input e não detectarmos E0F, avaliamos a string cmdline com a função eval */
    }
}

/* eval - Avalia a linha de comando */
void eval(char *cmdline) {                                                          /* recebe como parâmetro a string da linha de comando passada pelo usuário */

    char *argv[MAXARGS];                                                            /* Lista de argumentos que será passada na função execve criando a execução do processo filho */
    char buf[MAXLINE];                                                              /* Conforme formos avaliando a linha de comando passada, faremos alterações que ficarão guardadas em buf */
    int bg;                                                                         /* Variável que armazenará a escolha feita pelo usário de realizar o processo em foregroud ou em background  */
    pid_t pid;                                                                      /* id do processo para a sua identificação. Id do pai != 0, id do filho = 0 */

    strcpy(buf, cmdline);                                                           /* buf recebe o input na linha de comando passado pelo usuário, para alterarmos o input sem o perder */
    bg = parseline(buf, argv);                                                      /* Como Parseline analisa o input e constrói a lista de argumentos, ela retorna se o processo será em bg ou fg visto que essa decisão é feita no próprio input pelo usuário */
    
    if (argv[0] == NULL)                                                            /* Caso o usuário não tenha passado argumentos, não há linha de comando para ser analisada */
        return;                                                                     /* E então podemos dar retorno 0, encerrando a função */

    if (!builtin_command(argv)) {                                                   /* Verifica se é um comando existente, se não for o if é satisfeito. Pois, ou o comando não existe, ou é um objeto executável */
        if ((pid = fork()) == 0) {                                                  /* Criamos então um processo filho, e o if é satisfeito somente no processo filho */
            if (execve(argv[0], argv, environ) < 0) {                               /* Nesse caso, tentamos executar o primeiro argumento passado esperando que seja um objeto executável. Caso não seja, recebemos um erro e então o if é satisfeito */
                printf("%s: Command not found.\n", argv[0]);                        /* Não é um comando conhecido e não é um objeto executável, então não o comando não foi achado */
                exit(0);                                                            /* Encerramos o processo */
            }
         }

        if (!bg) {                                                                  /* Se !bg, ou seja, se o usuário quer o processo rodando em foregroud */
            int status;                                                             /* definimos a variável de status para receber o estado do processo filho */
            if (waitpid(pid, &status, WUNTRACED) < 0)                               /* O processo pai espera então o processo filho terminar, já que temos o pid do filho e o inteiro de espera bloqueante passados como parâmetros de waitpid */
                unix_error("waitfg: waitpid error");                                /* Caso waitpid seja menor que 0, houve um erro e então o sinalizamos */                                                                 
        } else {
            printf("%d %s", pid, cmdline);                                          /* Se for em bg, printamos o pid do filho e a linha de comando passada */
        }                 
    }
    return;                                                                         /* retorno 0 de eval para o término do processo visto que eval retorna void */
}

/* Na função builtin_command, temos os comandos dos quais sabemos executar */
int builtin_command(char **argv) {                                                  /* retorna um inteiro (0 para false, 1 para true) e recebe a lista dos argumentos como parâmetro */
    if (!strcmp(argv[0], "quit"))                                                   /* Comparamos o primeiro argumento com a string 'quit', ou seja, com o comando quit. Se os dois forem iguais, o if será satisfeito */
        exit(0);                                                                    /* Então, queremos sair do processo e damos exit(0) */
    if (!strcmp(argv[0], "&"))                                                      /* No caso em que o primeiro argumento passado for o caractere '&', não há o que executar em bg */
        return 1;                                                                   /* E então retornamos o valor 1, indicando o encerramento do processo ao dar return em eval */
    if (!strcmp(argv[0], "jobs")){
    
    }

    if (!strcmp(argv[0], "cd")) {
        int success;
        success = chdir(argv[1]);
        if (success < 0){
            unix_error("Directory error: ");
        }
        return 1;
        
    }

    if (!strcmp(argv[0], "fg")){
        
    }
    if (!strcmp(argv[0], "ls")){
        // Falta ordenar os nomes ???
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        if (d){
            while ((dir = readdir(d)) != NULL){
                if (dir->d_type == 4){
                    printf(BB);
                }
                else{
                    if (strchr(dir->d_name, '.') == NULL)
                        printf(BG);
                }
                if(dir->d_name[0] != '.')
                    printf("%s  ", dir->d_name);
                printf(RESET);
            }
            printf("\n");
            closedir(d);
        }
        return 1;
    }

    return 0;                                                                       /* Retornamos 0 por default caso o comando não exista na nossa função e então eval printará mensagem de erro */
}



/* parseline - Analisa a linha de comando passado pelo usuário e constrói o vetor de argumentos */
int parseline(char *buf, char **argv) {                                             /* Recebe como parâmetros o buf e a lista de argumentos definidos em eval */
    char *delim;                                                                    /* É um ponteiro de caractere, servirá para delimitar a cadeia passada em linha de comando */
    int argc;                                                                       /* De acordo com a construção do vetor de argumentos, saberemos o número de argumentos que foram passados */
    int bg;                                                                         /* como eval recebe a resposta de parseline para dizer se o processo é em bg ou fg, usaremos uma variável chamada bg como retorno de parseline */

    buf[strlen(buf)-1] = ' ';                                                       /* Em strlen(buf) - 1 temos o último caractere passado na linha de comando, o enter (\n), o substituímos pelo caractere de espaço */
    while (*buf && (*buf == ' '))                                                   /* Pula os espaços em brancos iniciais que podem ter sido deixados pelo usuário */
        buf++;                                                                      /* incrementa o ponteiro do buffer até encontrar algum carectere diferente de ' ' no qual será o primeiro argumento */

    /* Começaremos o processo de construção da lista de argumentos */
    argc = 0;                                                                       /* De início, não analisamos nenhum argumento, então argc = 0 */
    while ((delim = strchr(buf, ' '))) {                                            /* strchr fará com que delim, um ponteiro de caractere, aponte para a primeira vez que o caractere ' ' aparecer. Caso não há esse caractere na string, delim apontará para NULL */
        argv[argc++] = buf;                                                         /* Fazemos o argumento na posição agrc apontar onde buf aponta, e acrescentamos o número de argumentos em 1 */
        *delim = '\0';                                                              /* E então, substituímos o espaço entre os argumentos pelo caractere final de linha, dessa forma, separamos os argumentos e há uma forma de delimitá-los */
        buf = delim + 1;                                                            /* Movemos então para onde buf aponta, indo para o primeiro caractere do próximo argumento */
        while (*buf && (*buf == ' '))                                               /* Caso tenhamos mais espaços em brancos entre os argumentos, precisamos pulá-los */
            buf++;                                                                  /* Pula os espaços em brancos */
    }
    argv[argc] = NULL;                                                              /* Como padrão, o último elemento da cadeia de argumentos aponta para NULL, dessa forma podemos percorrer essas cadeia e parar quando acabarem os argumentos */

    if (argc == 0)                                                                  /* Se nenhum argumento foi passado, temos apenas espaço em branco, e assim podemos retornar */
        return 1;                                                                   /* Passamos então o valor 1, indicando que o processo será executado em backgroud */

    /* Agora, caso tenhamos algum argumento, precisamos definir se o processo executará em bg ou fg. Por padrão, ele executa em fg.*/
    if ((bg = (*argv[argc-1] == '&')) != 0)                                         /* Para executar em bg, o usuário deverá passar como argumento o caractere '&'. Caso tenha passado, bg = 1 (TRUE) */
        argv[--argc] = NULL;                                                        /* E então removemos esse argumento, pois ele não nos interessa mais, sendo o novo NULL da cadeia de argumentos passados */

    return bg;                                                                      /* Retornamos o valor de bg definido */
}
