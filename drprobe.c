#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void drp_disable(int which){

  unsigned long dr7 = 0;
  unsigned long olddr7 = 0;

  if( which > 3 || which < 0 ){

    fprintf(stderr,"[DRPROBE] Invalid Debug Register %d\n", which);

  }

  if(which == 0) dr7 |= 0x0000000000000001;
  else if(which == 1) dr7 |= 0x0000000000000004;
  else if(which == 2) dr7 |= 0x0000000000000010;
  else if(which == 3) dr7 |= 0x0000000000000040;

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

void drp_enable(int which){

  unsigned long dr7 = 0;
  unsigned long olddr7 = 0;

  if( which > 3 || which < 0 ){

    fprintf(stderr,"[DRPROBE] Invalid Debug Register %d\n", which);

  }

  if(which == 0) dr7 |= 0x000F0401;
  else if(which == 1) dr7 |= 0x000F0404;
  else if(which == 2) dr7 |= 0x000F0410;
  else if(which == 3) dr7 |= 0x000F0440;

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

void drp_watch(unsigned long addr, int which){

  drp_enable(which);
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
  }else if( val & 0x04 ){
    return 1;
  }else if( val & 0x10 ){
    return 2;
  }else if( val & 0x40 ){
    return 3;
  }else{
    return -1;
  }

}
