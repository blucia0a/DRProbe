#ifndef _DRPROBE_H_
#define _DRPROBE_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef enum _drp_trigger_type{
  DRP_HOW_RDWR,
  DRP_HOW_WRONLY
} drp_trigger_type;

void drp_init();
void drp_disable(int which);
void drp_enable(int which, drp_trigger_type how);
void drp_set(unsigned long addr, int which);
void drp_watch(unsigned long addr, int which);
void drp_watch_wr(unsigned long addr, int which);
void drp_unwatch(int which);
int drp_status(int which);
unsigned long drp_value(int which);
int drp_explain();

#ifdef __cplusplus
}
#endif

#endif /* _DRPROBE_H_ */
