#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include"smpl.h"
#include"cisj.h"

#define test 1
#define fault 2
#define repair 3

typedef struct {
    int id;
    int cluster;
    int *timestamp;
    int *nodos_testados;
    int *latencia;
    int qtd_testes;
}tnodo;

typedef struct {
    int qtd_testes;
    int latencia;
}tmudancas;

tnodo *nodo;

tmudancas *mudancas;

int pos = 0;

Inicializacao_do_Termina_Diagnostico_Nodos( int *termina_diagnostico_nodos, int n){

    int i;

    for (i = 0; i < n; i++){
        termina_diagnostico_nodos[i] = -1;
    }

}

Atualizacao_do_Inicia_Diagnostico_Nodos( int *inicia_diagnostico_nodos, int n){

    int i;

    for (i = 0; i < n; i++){
        inicia_diagnostico_nodos[i] = status( nodo[i].id);
    }

}

Atualizacao_do_Termina_Diagnostico_Nodos( int *termina_diagnostico_nodos, int n){

    int i;
    for (i = 0; i < n; i++){
        termina_diagnostico_nodos[i] = status( nodo[i].id);
    }

}

Calcula_Latencia( int token, int n){

    int i;

    /* verifica mudanças de estado dos nodos (latencia) */
    for ( i = 0; i < n; i++){
        if ( ( (nodo[token].timestamp[i] % 2) != status(nodo[i].id) ) /*&& nodo[token].timestamp[i] != -1*/ ){
            nodo[token].latencia[i]++;
        }
    }

}

int Checa_se_Terminou_o_Diagnostico( int n){

    int i, j,
    terminou_diagnostico = 1; // chuta que terminou

    /* checa se o diagnostico terminou */
    for ( i = 0; i < n; i++ ){ // para cada nodo
        for ( j = 0; j < n; j++ ){ // para cada posição do timestamp do nodo

            /* se i esta sem falha e no timestamp de i na posição j é diferente do estado de j */
            if ( status( nodo[i].id) == 0 && (( nodo[i].timestamp[j] % 2 ) != status( nodo[j].id) ) ){

                terminou_diagnostico = 0;
            }

        }
    }

    return terminou_diagnostico;
}

int Checa_se_Iniciou_o_Diagnostico( int *inicia_diagnostico_nodos, int *termina_diagnostico_nodos, int n){

    int i, iniciou_diagnostico = 0;

    for (i = 0; i < n; i++){

        /* se um nodo mudou de estado se inicia um novo diagnostico */
        if ( inicia_diagnostico_nodos[i] != termina_diagnostico_nodos[i] )
            iniciou_diagnostico = 1;
    }

    return iniciou_diagnostico;
}


Iniciando_Simulacao( int *termina_diagnostico_nodos, int n){

    char fa_name[5];
    int i, j;

    smpl(0, "Simulação");
    reset();
    stream(1);
    nodo = (tnodo *) malloc (sizeof(tnodo)*n);

    for ( i = 0; i < n ; i++) {
        memset(fa_name, '\0', 5);
        sprintf(fa_name, "%d", i);
        nodo[i].id = facility(fa_name, 1);
        nodo[i].timestamp = (int *) malloc (sizeof(int)*n);
        nodo[i].nodos_testados = (int *) malloc (sizeof(int)*n);
        nodo[i].latencia = (int *) malloc(sizeof(int)*n);

        /* inicia no cluster 1 */
        nodo[i].cluster = 1;
        /* inicia quantidade de testes */
        nodo[i].qtd_testes = 0;

        /* seta vetor de teste para -1 */
        for (j = 0; j < n; j++) {
            nodo[i].timestamp[j] = -1;
            nodo[i].nodos_testados[j] = 0;
        }
        /* valor de teste dele mesmo */
        nodo[i].timestamp[i] = 0;

        /* seta vetor de latencia para 0 */
        for (j = 0; j < n; j++){
            nodo[i].latencia[j] = 0;
        }

    }

    /* primeiro escalonamento de todos os nodos */
    for (i = 0; i< n ; i++) {
        schedule(test, 30.0, i);
    }

    /* definição de tempo de erro e reparo */

    schedule(fault, 40.0, 0);
    schedule(fault, 40.0, 1);
    schedule(fault, 60.0, 2);
//    schedule(fault, 60.0, 6);

    schedule(repair, 50.0, 0);
    schedule(repair, 50.0, 1);
    schedule(repair, 50.0, 2);
//    schedule(repair, 70.0, 6);

    Inicializacao_do_Termina_Diagnostico_Nodos( termina_diagnostico_nodos, n);



}

Imprime_Log( node_set *nodes, int *num_cluster, int token, int max_cluster, int j, int n){

    int i;

    /* imprimi log */

    /* checa cluster */
    if ( *num_cluster != nodo[token].cluster){
        printf("CLUSTER %d\n", nodo[token].cluster);
        *num_cluster = nodo[token].cluster;
    }

    /* checa se o nodo testado não é maior que a quantidade de nodos */
    if (nodes->nodes[j] < n) {
        printf("sou o nodo %d no tempo %5.1f \n", token, time());
        printf("testei os nodo(s): ");
        for (i = 0; i < n; i++) {
            if (nodo[token].nodos_testados[i] == 1)
                printf(" %d ", i);
        }
        printf("\n");
        printf("timestamp: ");
        for (i = 0; i < n; i++) {
            printf(" [%d] ", nodo[token].timestamp[i]);
        }
        printf("\n\n");
    }

}

Algoritmo_de_Testes( int *num_cluster, int token, int max_cluster, int n){

    int i, j, k, l,
    precisa_testar;

    node_set *nodes, *nodes_j, *nodes_k;

    /* pega todos os nodos de C i,s */
    nodes = cis(token, nodo[token].cluster);

    /* percorre para todo a pertencente a C j,s */

    j = 0;
    while (j < nodes->size){

        /* se o nodo de C i,s existe */
        if (nodes->nodes[j] < n){

            /* pega todos os nodos de C j,s do cluster de i */
            nodes_j = cis(nodes->nodes[j], nodo[token].cluster);

            /* se o primeiro nodo vindo de C j,s existe */
            if (nodes_j->nodes[0] < n){

                /* se o primeiro nodo de C j,s for i ou se o primeiro nodo de C j,s estiver falho */
                if ( nodes_j->nodes[0] == token || ( nodo[token].timestamp[ nodes_j->nodes[0] ] % 2 ) == 1 ){
                    precisa_testar = 1; // chuta que precisa

                    /* se o primeiro nodo de C j,s não for i, mas estiver falho */
                    if ( ( nodo[token].timestamp[ nodes_j->nodes[0] ] % 2 ) == 1 ){

                        /* para cada nodo pertencente a clusters anteriores */
                        for(k = 1; k < nodo[token].cluster; k++){
                            nodes_k = cis( nodes->nodes[j], k);
                            for(l = 0; l < nodes->size; l++){
                                // se não for o token e se um desses nodos não estiver falho
                                if( token != nodes_k->nodes[l] &&( nodo[token].timestamp[ nodes_k->nodes[l] ] ) % 2 == 0 )
                                    precisa_testar = 0;
                            }

                        }

                    }

                    if ( precisa_testar == 1){
                        /* nodo esta realizando um teste */
                        nodo[token].qtd_testes++;

                        /* se primeira vez que seta o timestamp de i * posição j */
                        if (nodo[token].timestamp[nodes->nodes[j]] == -1 ){
                            /* se j sem falha */
                            if (status(nodo[nodes->nodes[j]].id) == 0)
                                nodo[token].timestamp[nodes->nodes[j]] = 0;
                            else
                                nodo[token].timestamp[nodes->nodes[j]] = 1;
                        }

                        /* nodo j vai ser testado */
                        nodo[token].nodos_testados[nodes->nodes[j]] = 1;

                        /* incrementa timestamp de i se o nodo mudou de estado */
                        if (status(nodo[nodes->nodes[j]].id) != nodo[token].timestamp[nodes->nodes[j]] % 2)
                            nodo[token].timestamp[nodes->nodes[j]]++;

                        /* se o nodo j esta sem falha */
                        if (status(nodo[nodes->nodes[j]].id) == 0){

                            /* atualiza o timestamp de i com base no timestamp de j */
                            for (i=0;i<n;i++){

                                /* so nao atualizao o timestamp da posicao de i e j */
                                if (i != token && i != nodes->nodes[j]){

                                    /* se o timestamp do i for menor que o timestamp de j para todos os nodos */
                                    if (nodo[token].timestamp[i] < nodo[nodes->nodes[j]].timestamp[i]){

                                        nodo[token].timestamp[i] = nodo[nodes->nodes[j]].timestamp[i];

                                    }
                                }
                            }

                        }
                    }
                }
            }

        }

        j++;

    }

    Imprime_Log( nodes, num_cluster, token, max_cluster, j, n);

}

Verifica_Diagnostico( int *inicia_diagnostico_nodos, int *termina_diagnostico_nodos, int *maior, int n){

    int i, j,
    latencia_maior, latencia_testes;

    /* Verifica se um novo diagnostico foi reiniado para termina-lo */
    if (Checa_se_Iniciou_o_Diagnostico( inicia_diagnostico_nodos, termina_diagnostico_nodos, n)){
        if ( Checa_se_Terminou_o_Diagnostico( n ) ){

            Atualizacao_do_Termina_Diagnostico_Nodos( termina_diagnostico_nodos, n);

            for (i = 0; i < n; i++){ // para cada posição do timestamp do vetor
                maior[i] = nodo[0].latencia[i];
                for (j = 1; j < n; j++){ // para cada nodo
                    /* se o nodo não esta falho */
                    if ( status( nodo[j].id ) == 0 ){

                        if ( maior[i] < nodo[j].latencia[i]){
                            maior[i] = nodo[j].latencia[i];
                        }

                    }
                }
            }

            latencia_maior = maior[0];
            for (i = 1; i < n; i++){
                if ( latencia_maior < maior[i] ){
                    latencia_maior = maior[i];
                }
            }

            latencia_testes = 0;
            for (i = 0; i < n; i++){
                latencia_testes = latencia_testes + nodo[i].qtd_testes;
            }

            printf("latencia_maior = %d latencia_testes %d\n\n", latencia_maior, latencia_testes);

            mudancas[pos].latencia = latencia_maior;
            mudancas[pos].qtd_testes = latencia_testes;
            pos++;

            /* zera todo mundo */
            for (i = 0; i < n; i++){
                for (j = 0; j < n; j++){
                    nodo[i].latencia[j] = 0;
                }
            }
            for (i = 0; i < n; i++){
                maior[i] = 0;
            }
            for (i = 0; i < n; i++){
                nodo[i].qtd_testes = 0;
            }

        }
    }
}

main ( int argc, char *argv[]) {
    static int n,
    token, aux_token,
    event;
    int r, s,
    i, j, k, l, m,
    max_cluster, num_cluster = 0,
    *inicia_diagnostico_nodos, *termina_diagnostico_nodos, *maior;

    if (argc != 2) {
        puts ("Número errado de parametros, uso correto, ./temp |número de nodos|");
        exit(1);
    }

    n = atoi(argv[1]);

    max_cluster = (int) ceil( log2(n) );

    /* inicio da alocação de ponteiros */
    maior = malloc(sizeof(int)*n);
    inicia_diagnostico_nodos = malloc(sizeof(int)*n);
    termina_diagnostico_nodos = malloc(sizeof(int)*n);
    mudancas =(tmudancas *) malloc(sizeof(tmudancas)*n);
    /* final da alocação de ponteiros */

    Iniciando_Simulacao( termina_diagnostico_nodos, n);

    printf("\n<<<<<<<<<<<<<<<<<<<START SIMULATION>>>>>>>>>>>>>>>>>>>\n\n");

    while (time() < 100.0) {

        cause(&event, &token);

        switch(event){

        case test:

            if ( status(nodo[token].id) != 0) break;

            Atualizacao_do_Inicia_Diagnostico_Nodos( inicia_diagnostico_nodos, n);

            Calcula_Latencia( token, n);

            Algoritmo_de_Testes( &num_cluster, token, max_cluster, n);

            /* apaga os nodos testados */
            for (k = 0; k < n; k++) {
                nodo[token].nodos_testados[k] = 0;
            }

            /* checa se esta no último cluster ou não */
            if ( nodo[token].cluster < max_cluster ) {
                nodo[token].cluster++;
                schedule(test, 0.0, token);
            }
            else {
                nodo[token].cluster = 1;
                schedule(test, 30.0, token);
            }

            Verifica_Diagnostico( inicia_diagnostico_nodos, termina_diagnostico_nodos, maior, n);

            break;

        case fault:
            r = request( nodo[token].id, token, 0);
            if (r != 0) {
                printf ("Não consegui falhar nodo %d\n", token);
                exit(1);
            }
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("O nodo %d falha no tempo %5.1f \n", token, time());
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
            break;

        case repair:
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            printf("o nodo %d reparou no tempo %5.1f \n", token, time());
            printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");
            release(nodo[token].id, token);
            schedule(test, 30.0, token);
            break;

        }

    }

    printf("<<<<<<<<<<<<<<<<<<<FINISH SIMULATION>>>>>>>>>>>>>>>>>>>\n\n");


    printf("<<<<<<<<<<<<<<INICIO LATENCIA E QTD TESTES>>>>>>>>>>>>>>\n\n");
    printf("Considerando a sequencia de eventos de diagnosticos as\n");
    printf("respectivas latencia e quantidade de testes foram:\n\n");
    for (i = 0; i < max_cluster; i++){
        printf("latencia %d qtd_testes %d\n", mudancas[i].latencia, mudancas[i].qtd_testes);
    }
    printf("\n");
    printf("<<<<<<<<<<<<<<<FIM LATENCIA E QTD TESTES>>>>>>>>>>>>>>>\n\n");

}
