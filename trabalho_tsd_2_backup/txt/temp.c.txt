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
}tnodo;

tnodo *nodo;

main ( int argc, char *argv[]) {
    static int n,
    token, aux_token,
    event;
    int r, s,
    i, j, k,
    max_cluster, num_cluster = 0;
    char fa_name[5];
    node_set* nodes,* aux_nodes, *nodes_j;

    if (argc != 2) {
        puts ("Número errado de parametros, uso correto, ./temp |número de nodos|");
        exit(1);
    }

    n = atoi(argv[1]);
    max_cluster =(int) ceil( log2(n) );

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
        /* inicia no cluster 1 */
        nodo[i].cluster = 1;

        /* seta vetor de teste para -1 */
        for (j = 0; j < n; j++) {
            nodo[i].timestamp[j] = -1;
            nodo[i].nodos_testados[j] = 0;
        }
        /* valor de teste dele mesmo */
        nodo[i].timestamp[i] = 0;
    }

    /* primeiro escalonamento de todos os nodos */
    for (i = 0; i< n ; i++) {
        schedule(test, 30.0, i);
    }

    /* definição de tempo de erro e reparo */

    schedule(fault, 40.0, 1);
    schedule(fault, 40.0, 2);

    printf("\n<<<<<<<<<<<<<<<<<<<START SIMULATION>>>>>>>>>>>>>>>>>>>\n\n");

    while (time() < 100.0) {

        cause(&event, &token);

        switch(event){

        case test:

            if ( status(nodo[token].id) != 0) break;

            /* pega todos os nodos de C i,s */
            nodes = cis(token, nodo[token].cluster);

            /* percorre para todo a pertencente a C j,s */
            j = 0;
            while (j < nodes->size){

                /* se o nodo de C i,s existe */
                if (nodes->nodes[j] < n){
                    /* pega todos os nodos de C j,s do cluster de i */
                    nodes_j = cis(nodes->nodes[j], nodo[token].cluster);

                    /* se o primeiro nodo de C j,s for i */
                    if (nodes_j->nodes[0] == token ){

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
                j++;

            }

            /* imprimi log */

            /* checa cluster */
            if (num_cluster != nodo[token].cluster){
                printf("CLUSTER %d\n", nodo[token].cluster);
                num_cluster = nodo[token].cluster;
            }

            /* checa se o nodo testado não é maior que a quantidade de nodos */
            if (nodes->nodes[j] < n) {
                printf("sou o nodo %d no tempo %5.1f \n", token, time());
                printf("testei os nodo(s): ");
                for (k = 0; k < n; k++) {
                    if (nodo[token].nodos_testados[k] == 1)
                        printf(" %d ", k);
                }
                printf("\n");
                printf("timestamp: ");
                for (k = 0; k < n; k++) {
                    printf(" [%d] ", nodo[token].timestamp[k]);
                }
                printf("\n\n");
            }

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

}
