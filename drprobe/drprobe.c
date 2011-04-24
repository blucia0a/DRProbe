#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "drprobe.h"

void drp_init(){
  drp_set(0x0,0);
  drp_set(0x0,1);
  drp_set(0x0,2);
  drp_set(0x0,3);

  drp_disable(0);
  drp_disable(1);
  drp_disable(2);
  drp_disable(3);

  drp_set(0x0,6);
}

void drp_disable(int which){

  unsigned long dr7 = 0;
  unsigned long olddr7 = 0;

  if( which > 3 || which < 0 ){

    fprintf(stderr,"[DRPROBE] Invalid Debug Register %d\n", which);

  }

  if(which == 0) dr7 |= 0x000F0003;
  else if(which == 1) dr7 |= 0x00F0000C;
  else if(which == 2) dr7 |= 0x0F000030;
  else if(which == 3) dr7 |= 0xF00000C0;

  dr7 = ~dr7;

  int fd = open("/proc/dr/dr7", O_RDWR);
  read(fd, &olddr7, sizeof olddr7);

  /*Mask off all but the enable bit for this wp*/
  dr7 = olddr7 & dr7;
  
  if( fd == -1 ){

    fprintf(stderr,"[DRPROBE] Couldn't open debug register enable register (dr7)\nMake sure debug_mod.ko has been loaded\n"); 
    return;

  }

  write(fd, &dr7, sizeof dr7);
  close(fd);

}

void drp_enable(int which, drp_trigger_type how){

  unsigned long dr7 = 0;
  unsigned long olddr7 = 0;

  if( which > 3 || which < 0 ){

    fprintf(stderr,"[DRPROBE] Invalid Debug Register %d\n", which);

  }
  if(how == DRP_HOW_RDWR){

    if(which == 0) dr7 |= 0x000F0303;
    else if(which == 1) dr7 |= 0x00F0030C;
    else if(which == 2) dr7 |= 0x0F000330;
    else if(which == 3) dr7 |= 0xF00003C0;

  }else if(how == DRP_HOW_WRONLY){

    if(which == 0) dr7 |= 0x000D0302;
    else if(which == 1) dr7 |= 0x00D00308;
    else if(which == 2) dr7 |= 0x0D000320;
    else if(which == 3) dr7 |= 0xD0000380;

  }

  int fd = open("/proc/dr/dr7", O_RDWR);
  read(fd, &olddr7, sizeof olddr7);

  dr7 |= olddr7;
  
  if( fd == -1 ){

    fprintf(stderr,"[DRPROBE] Couldn't open debug register enable register (dr7)\nMake sure debug_mod.ko has been loaded\n"); 
    return;

  }

  write(fd, &dr7, sizeof dr7);
  close(fd);

}

void drp_set(unsigned long addr, int which){
  
  char buffer[32];
  int fd = -1;

  snprintf(buffer, sizeof buffer, "/proc/dr/dr%d", which);
  fd = open(buffer, O_RDWR);

  if( fd == -1 ){

    fprintf(stderr,"[DRPROBE] Couldn't open debug register %d\n",which); 
    return;

  }

  write(fd, &addr, sizeof(addr));
  unsigned long foo;
  read(fd, &foo, sizeof(foo));
  close(fd);

}

void drp_watch_wr(unsigned long addr, int which){

  drp_enable(which, DRP_HOW_WRONLY);
  drp_set(addr,which);

}

void drp_watch(unsigned long addr, int which){

  drp_enable(which, DRP_HOW_RDWR);
  drp_set(addr,which);

}

void drp_unwatch(int which){

  drp_set(0x0,which);
  drp_disable(which);

}

int drp_status(int which){
  
  int fd = -1;
  unsigned long dr7 = 0;
  fd = open("/proc/dr/dr7", O_RDWR);
  read(fd, &dr7, sizeof dr7);
  close(fd);

  
  if(which == 0) return (dr7 & 0x00000001);
  else if(which == 1) return (dr7 & 0x00000004);
  else if(which == 2) return (dr7 & 0x00000010);
  else if(which == 3) return (dr7 & 0x00000040);

}

unsigned long drp_value(int which){

  char buffer[32];
  int fd = -1;
  unsigned long val = 0;

  snprintf(buffer, sizeof buffer, "/proc/dr/dr%d", which);
  fd = open(buffer, O_RDWR);
  read(fd, &val, sizeof(unsigned long));
  close(fd);
  

  return val; 

}

int drp_explain(){
  
  unsigned long val = drp_value(6);
  if( val & 0x01 ){
    return 0;
  }else if( val & 0x02 ){
    return 1;
  }else if( val & 0x04 ){
    return 2;
  }else if( val & 0x08 ){
    return 3;
  }else{
    return -1;
  }

}
