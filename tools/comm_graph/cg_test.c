#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN 0
#define MAX 100

#define NUM_TRS 100

#define WITHDRAWAL 0
#define DEPOSIT 1

typedef struct {
  int type;
  int amount;
} transaction;

typedef struct {
  int num;
  transaction *t_array;
} transactions;

int balance;
pthread_mutex_t *balance_lock;

transactions *new_transactions(int num){
  
  int i;
  transactions * trs = (transactions*)malloc(sizeof(transactions));  
  trs->num = num;
  trs->t_array = (transaction*)malloc(sizeof(transaction) * num);

  for(i = 0; i < num; i++){
    int q = rand() % 100;
    trs->t_array[i].amount = q;
    trs->t_array[i].type = q % 2;
  } 
  return trs;

}

void *deposit_repeatedly(void *np){

  transactions * t;
  t = (transactions*)np;
  int i;
  for(i = 0; i < t->num; i++){
    int q;
    
    if(t->t_array[i].type == DEPOSIT){
      int bal;
      
      pthread_mutex_lock(balance_lock);
      bal = balance;
      pthread_mutex_unlock(balance_lock); 
      
      if(bal + t->t_array[i].amount <= MAX) 
      	bal += t->t_array[i].amount;
      
      pthread_mutex_lock(balance_lock);
      balance = bal;
      pthread_mutex_unlock(balance_lock); 
    }
  }

}

void *withdraw_repeatedly(void *np){

  transactions * t;
  t = (transactions*)np;
  int i;
  for(i = 0; i < t->num; i++){
    int q;
    if(t->t_array[i].type == WITHDRAWAL){
      int bal;
      
      pthread_mutex_lock(balance_lock);
      bal = balance;
      pthread_mutex_unlock(balance_lock); 
      
      if(bal - t->t_array[i].amount >= MIN) 
      	bal -= t->t_array[i].amount;
      
      pthread_mutex_lock(balance_lock);
      balance = bal;
      pthread_mutex_unlock(balance_lock); 
    }
  }

}

int main(int argv, char ** argc){

  transactions * trs = new_transactions(NUM_TRS);
  transactions * trs2 = new_transactions(NUM_TRS);
  balance_lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t)); 
  pthread_mutex_init(balance_lock,NULL);

  drp_tool_cg_init();
  drp_watch(&balance);
  
  pthread_t dep, wit;
  pthread_create(&dep,NULL,deposit_repeatedly,(void*)trs);
  pthread_create(&wit,NULL,withdraw_repeatedly,(void*)trs);

  pthread_join(dep,NULL);
  pthread_join(wit,NULL);
  

}
