#ifndef _DRPROBE_H_
#define _DRPROBE_H_

void drp_disable(int which);
void drp_enable(int which);
void drp_set(unsigned long addr, int which);
void drp_watch(unsigned long addr, int which);
void drp_unwatch(int which);
int drp_status(int which);
unsigned long drp_value(int which);
int drp_explain();

#endif /* _DRPROBE_H_ */
