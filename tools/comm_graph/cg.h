#define NUM_WATCHPOINTS 4
#define CTSIZE 1024

typedef struct _comm_tab_entry{

  unsigned long src;
  unsigned long snk;

} comm_tab_entry;

typedef struct _comm_tab{

  int num_entries;
  int cur_entry;
  comm_tab_entry *ct;

} comm_tab;

comm_tab *drp_tool_cg_ct;

typedef struct _last_writer_record{
  
  void *pc;
  pthread_t thd;

} last_writer_record;

last_writer_record drp_tool_cg_lwt[NUM_WATCHPOINTS];


