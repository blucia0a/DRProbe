#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ucontext.h>
#include <assert.h>

#include "drprobe.h"
#include "cg.h"

void drp_tool_cg_dump( void ){
  int i = 0;
  for( i = 0; i < CTSIZE; i++ ){
    if( drp_tool_cg_ct->ct[i].src || 
        drp_tool_cg_ct->ct[i].snk ){
      fprintf(stderr,"%p->%p\n", drp_tool_cg_ct->ct[i].src, drp_tool_cg_ct->ct[i].snk);
    }
  }
}

void drp_tool_cg(int signo, siginfo_t *info, void* context){

  ucontext_t *ctx = (ucontext_t *)context;
  unsigned long curPC = ctx->uc_mcontext.gregs[16];

  int which = drp_explain(); 

  assert( which >= 0 && which <= 3 );
  last_writer_record *w = &( drp_tool_cg_lwt[which] );
  if(  !pthread_equal( w->thd, pthread_self() )  ){
    int i;
    for( i = 0; i < CTSIZE; i++ ){

      if( drp_tool_cg_ct->ct[i].src == (unsigned long)w->pc && 
          drp_tool_cg_ct->ct[i].snk == (unsigned long)curPC ){
        w->pc = (void*)curPC;
        w->thd = pthread_self();
        return; 
      }
      
      if( drp_tool_cg_ct->ct[i].src == 0 && 
          drp_tool_cg_ct->ct[i].snk == 0 ){

        drp_tool_cg_ct->ct[i].src = (unsigned long)w->pc;
        drp_tool_cg_ct->ct[i].snk = (unsigned long)curPC;
        w->pc = (void*)curPC;
        w->thd = pthread_self();
        return;

      }

    }    
  }

  w->pc = (void*)curPC;
}

void drp_tool_cg_init(){

  drp_tool_cg_ct = malloc( sizeof( comm_tab ) );
  memset( drp_tool_cg_ct, 0, sizeof( comm_tab ) );

  drp_tool_cg_ct->ct = malloc( sizeof( comm_tab_entry ) * CTSIZE );
  memset( drp_tool_cg_ct->ct, 0, sizeof( comm_tab_entry ) * CTSIZE );

  int i;
  for( i = 0; i < NUM_WATCHPOINTS; i++ ){

    drp_tool_cg_lwt[i].pc = (void*)0x0;
    drp_tool_cg_lwt[i].thd= (pthread_t)0;

  }
  atexit( drp_tool_cg_dump );
  
  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));

  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  act.sa_sigaction = &drp_tool_cg;

  sigaction(SIGTRAP, &act, 0);

}
