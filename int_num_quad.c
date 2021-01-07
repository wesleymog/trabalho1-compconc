
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include "timer.h"
int nthreads, i_global;
double *somatorio;
double a,b,qtd;
pthread_mutex_t mutex; //variavel de lock para exclusao mutua

//cria a estrutura de dados para armazenar os argumentos da thread


void * tarefa (void *arg) {
  double w;
  int i_inter=0;
  w = (b-a)/qtd;
  while(i_global < qtd){
    double x_i,soma;
    pthread_mutex_lock(&mutex);
    i_inter = i_global;
    i_global++;
    pthread_mutex_unlock(&mutex);
    x_i = a + (i_inter-1)*w;
    soma = w*((x_i*x_i)/((x_i*x_i*x_i)+3));
    somatorio[i_inter] = soma;
  }
  pthread_exit(NULL);
}

//funcao principal do programa
int main(int argc, char* argv[] ) {

  pthread_t *tid;
  double inicio, fim, delta,soma;
  
  if(argc<4){
      printf("Digite: %s <qtde de retangulos> <limite inferior> <limite superior> <nthreads>\n", argv[0]);
      return 1;
  }

  qtd = strtod(argv[1], NULL);
  a = strtod(argv[2], NULL);
  b = strtod(argv[3], NULL);
  nthreads = atoi(argv[4]);
  somatorio = (double *) malloc( qtd * sizeof(double) );
  if(somatorio ==NULL){puts("ERRO MALLOC");return 2;}

  // (a) Inicialização da estrutura de dados
  GET_TIME(inicio);
  //--inicilaiza o mutex (lock de exclusao mutua)
  pthread_mutex_init(&mutex, NULL);
  tid = (pthread_t*) malloc(sizeof(pthread_t)*nthreads);
  if(tid ==NULL){puts("ERRO MALLOC");return 2;}

  GET_TIME(fim);

  delta = fim - inicio;
  printf("tempo de Inicialização da estrutura de dados %lf\n", delta);
  
  GET_TIME(inicio);
  for(int i=0;i<nthreads; i++){
      if(pthread_create(tid+i, NULL, tarefa, (void*)(NULL))){
        puts("ERRO --pthread create\n");
        return 3;
      }
  }

  //--espera todas as threads terminarem
  for (int thread=0; thread<nthreads; thread++) {
    if (pthread_join(tid[thread], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
  }

  pthread_mutex_destroy(&mutex);
  for (int i=0; i<i_global; i++) {
    soma= soma+ somatorio[i];
  }
  GET_TIME(fim);
  delta = fim - inicio;
  printf("tempo de criação das threads, execução da integração e termino de threads %lf\n", delta);

  GET_TIME(inicio);
  printf("resultado %lf", soma);
  GET_TIME(fim);
  delta = fim - inicio;
  printf("tempo de finalização do programa %lf\n", delta);

  pthread_exit(NULL);
  
}
