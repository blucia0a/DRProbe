#include <signal.h>
#include <stdio.h>
#include "drprobe.h"

void handle_trap(int signum){
  fprintf(stderr,"Trap caused by watchpoint %d (a = 0x%016lx)\n",drp_explain(),drp_value(drp_explain()));
}

int main(int argc, char *argv[]){


  drp_init();
  signal(SIGTRAP,handle_trap);

  int i;
  int a[4];
  
  for( i = 0; i < 4; i++){
    a[i] = i;
    fprintf(stderr,"Correct address is 0x%016lx\n",&a);

    drp_watch_wr((unsigned long)&a[i],i);
  
    fprintf(stderr,"Watchpoint %d (a = 0x%016lx)\n",i,drp_value(i));
    fprintf(stderr,"Config: 0x%016lx\n",drp_value(7));
    
  
    fprintf(stderr,"WP%d is %s 0x%016lx\n", i, drp_status(i) ? "watching" : "not watching", drp_value(i) );
  
    /*This should cause a trap*/
    int b = a[i];
  
    drp_unwatch(i);
    fprintf(stderr,"WP%d is %s 0x%016lx\n", i, drp_status(i) ? "watching" : "not watching", drp_value(i) );
    
    /*This should not cause a trap*/
    int c = a[i];
    fprintf(stderr,"%d %d %d\n",a[i],b,c);
    usleep(100);
  }

}
