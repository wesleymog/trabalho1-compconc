#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

//numero de threads
int nthreads;
//Variável do pthread_mutex
pthread_mutex_t mutex;

double somatorioConcorrente = 0;
//argumentos para serem utilizados no metodo concorrente
typedef struct {
    double limiteSuperior;
    double limiteInferior;
    long long int inicioSomatorio;
    long long int fimSomatorio;
    double width;
}StructConcorrent;
//Integração com retângulos sequencial
double IntRetSequencial(double limiteSuperior, double limiteInferior, long long int quantidadeRetangulo){
    double width = (limiteSuperior - limiteInferior)/quantidadeRetangulo;
    double somatorio, x_i = 0;
    for (long long int i = 1; i <= quantidadeRetangulo ; i++) {
        x_i = limiteInferior + (i-1)*width;
        somatorio = somatorio + (width * (x_i * x_i * x_i));
    }
    return somatorio;
}

void definirQuantidadeRetangulosParaCadaThread(StructConcorrent *structConcorrent, long long int quantidadeRetangulo, double limiteInferior, double limiteSuperior){
    double width = (limiteSuperior - limiteInferior)/quantidadeRetangulo;
    long long int quantidadeRetanguloDivido = quantidadeRetangulo/nthreads;
    long long int restoDivisao = quantidadeRetangulo%nthreads;
    long long int quantidadeRetanguloComRestoDestribuido = 1; //somatorio começa em 1
    for (int i = 0; i < nthreads; i++) {
        long long int len = quantidadeRetanguloDivido;
        structConcorrent[i].inicioSomatorio = quantidadeRetanguloComRestoDestribuido;
        if (restoDivisao > 0){
            len++;
            restoDivisao--;
        }
        quantidadeRetanguloComRestoDestribuido += len;
        structConcorrent[i].fimSomatorio = structConcorrent[i].inicioSomatorio + len;
        structConcorrent[i].limiteSuperior = limiteSuperior;
        structConcorrent[i].limiteInferior = limiteInferior;
        structConcorrent[i].width = width;
    }
}

//Integração com retângulos concorrente
void * rectangleMethodConcorrente(void * arg){
    double x_i = 0.0;
    double *somatorioConcorrenteLocal;
    double somaLocal;
    somatorioConcorrenteLocal = (double *) malloc(sizeof (double ));
    if (somatorioConcorrenteLocal == NULL){
        printf("erro-Malloc\n");
        exit(1);
    }
    *somatorioConcorrenteLocal = 0;
    StructConcorrent *structConcorrent = (StructConcorrent *) arg;
    for (int i = structConcorrent->inicioSomatorio; i < structConcorrent->fimSomatorio; i++) {
        x_i = structConcorrent->limiteInferior + (i-1)*structConcorrent->width;
        somaLocal = somaLocal + (structConcorrent->width * (x_i * x_i * x_i));
    }
    pthread_mutex_lock(&mutex);
    somatorioConcorrente+=somaLocal;
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

int main(int argc, char* argv[] ){

    double retornoSomatorioSequencial;

    //parametros da integral
    double limiteInferior;
    double limiteSuperior;
    long long int quantidadeRetangulo;

    //parametros para marcar o tempo
    double inicio;
    double fim;

    //identificadores das threads no sistema
    pthread_t *tid;

    //recebe e valida os parametros de entrada(Limite Inferior, Limite Superior, quantidade de retangulos e numero de threads)
    if (argc < 5){
        fprintf(stderr, "Digite: %s <Limite Inferior> <Limite Superior> <Quantidade de retangulos> < numero de threads> \n", argv[0]);
        return 1;
    }
    limiteInferior = atoll(argv[1]);
    limiteSuperior = atoll(argv[2]);
    quantidadeRetangulo = atoll(argv[3]);
    nthreads = atoi(argv[4]);

    //resultado do valor sequencial do rectangleMethod
    GET_TIME(inicio);
    retornoSomatorioSequencial = IntRetSequencial(limiteSuperior,limiteInferior,quantidadeRetangulo);
    GET_TIME(fim);
    printf("O metodo sequencial demorou %.7lf e obteve o valor aproximado igual a %.15lf  \n", (fim - inicio),retornoSomatorioSequencial);

    //rectangleMethod concorrente
    GET_TIME(inicio);
    //--inicilaiza o mutex (lock de exclusao mutua)
    pthread_mutex_init(&mutex, NULL);
    tid = (pthread_t *) malloc(sizeof (pthread_t) * nthreads);
    if (tid == NULL){
        fprintf(stderr, "ERRO --malloc\n");
        return 2;
    }
    StructConcorrent *structConcorrent = (StructConcorrent *) malloc(sizeof(StructConcorrent) * nthreads);
    if (structConcorrent == NULL){
        printf("ERRO--Malloc \n");
        return 2;
    }
    definirQuantidadeRetangulosParaCadaThread(structConcorrent, quantidadeRetangulo, limiteInferior, limiteSuperior);
    //criar as threads
    for (long int i = 0; i < nthreads; i++) {
        if (pthread_create(tid + i, NULL, rectangleMethodConcorrente, (void *) &structConcorrent[i] )){
            fprintf(stderr, "ERRO --pthread_Create\n");
            return 3;
        }
    }
    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(*(tid + i), NULL)){
            fprintf(stderr, "ERRO --pthread_Join\n");
            return 4;
        }
    }

    GET_TIME(fim)
    printf("o metodo concorrente demorou %.7lf e obteve o valor aproximado igual a %.15lf \n", (fim - inicio), somatorioConcorrente);
    free(tid);
    free(structConcorrent);
    pthread_mutex_destroy(&mutex);
    return 0;
}