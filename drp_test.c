#include <signal.h>
#include <stdio.h>
#include "drprobe.h"

void handle_trap(int signum){
  fprintf(stderr,"Trap caused by watchpoint %d (a = 0x%016lx)\n",drp_explain(),drp_value(drp_explain()));
}

int main(int argc, char *argv[]){

  signal(SIGTRAP,handle_trap);

  int a;
  fprintf(stderr,"Correct address is 0x%016lx\n",&a);
  drp_watch((unsigned long)&a,0);

  fprintf(stderr,"WP0 is %s 0x%016lx\n", drp_status(0) ? "watching" : "not watching", drp_value(0) );

  /*This should cause a trap*/
  int b = a;


  drp_unwatch(0);
  fprintf(stderr,"WP0 is %s 0x%016lx\n", drp_status(0) ? "watching" : "not watching", drp_value(0) );
  
  /*This should not cause a trap*/
  int c = a;
  fprintf(stderr,"%d %d %d\n",a,b,c);

}
