/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _RUSERS_H_RPCGEN
#define _RUSERS_H_RPCGEN

#include <rpc/rpc.h>

/*
 * Find out about remote users
 */
#define RUSERS_MAXUSERLEN 32
#define RUSERS_MAXLINELEN 32
#define RUSERS_MAXHOSTLEN 257

struct rusers_utmp {
	char *ut_user;
	char *ut_line;
	char *ut_host;
	int ut_type;
	int ut_time;
	u_int ut_idle;
};
typedef struct rusers_utmp rusers_utmp;
#ifdef __cplusplus 
extern "C" bool_t xdr_rusers_utmp(XDR *, rusers_utmp*);
#elif __STDC__ 
extern  bool_t xdr_rusers_utmp(XDR *, rusers_utmp*);
#else /* Old Style C */ 
bool_t xdr_rusers_utmp();
#endif /* Old Style C */ 


typedef struct {
	u_int utmp_array_len;
	rusers_utmp *utmp_array_val;
} utmp_array;
#ifdef __cplusplus 
extern "C" bool_t xdr_utmp_array(XDR *, utmp_array*);
#elif __STDC__ 
extern  bool_t xdr_utmp_array(XDR *, utmp_array*);
#else /* Old Style C */ 
bool_t xdr_utmp_array();
#endif /* Old Style C */ 


/*
 * Values for ut_type field above.
 */
#define RUSERS_EMPTY 0
#define RUSERS_RUN_LVL 1
#define RUSERS_BOOT_TIME 2
#define RUSERS_OLD_TIME 3
#define RUSERS_NEW_TIME 4
#define RUSERS_INIT_PROCESS 5
#define RUSERS_LOGIN_PROCESS 6
#define RUSERS_USER_PROCESS 7
#define RUSERS_DEAD_PROCESS 8
#define RUSERS_ACCOUNTING 9

#define RUSERSPROG ((u_long)100002)
#define RUSERSVERS_3 ((u_long)3)

#ifdef __cplusplus
#define RUSERSPROC_NUM ((u_long)1)
extern "C" int * rusersproc_num_3(void *, CLIENT *);
extern "C" int * rusersproc_num_3_svc(void *, struct svc_req *);
#define RUSERSPROC_NAMES ((u_long)2)
extern "C" utmp_array * rusersproc_names_3(void *, CLIENT *);
extern "C" utmp_array * rusersproc_names_3_svc(void *, struct svc_req *);
#define RUSERSPROC_ALLNAMES ((u_long)3)
extern "C" utmp_array * rusersproc_allnames_3(void *, CLIENT *);
extern "C" utmp_array * rusersproc_allnames_3_svc(void *, struct svc_req *);

#elif __STDC__
#define RUSERSPROC_NUM ((u_long)1)
extern  int * rusersproc_num_3(void *, CLIENT *);
extern  int * rusersproc_num_3_svc(void *, struct svc_req *);
#define RUSERSPROC_NAMES ((u_long)2)
extern  utmp_array * rusersproc_names_3(void *, CLIENT *);
extern  utmp_array * rusersproc_names_3_svc(void *, struct svc_req *);
#define RUSERSPROC_ALLNAMES ((u_long)3)
extern  utmp_array * rusersproc_allnames_3(void *, CLIENT *);
extern  utmp_array * rusersproc_allnames_3_svc(void *, struct svc_req *);

#else /* Old Style C */ 
#define RUSERSPROC_NUM ((u_long)1)
extern  int * rusersproc_num_3();
extern  int * rusersproc_num_3_svc();
#define RUSERSPROC_NAMES ((u_long)2)
extern  utmp_array * rusersproc_names_3();
extern  utmp_array * rusersproc_names_3_svc();
#define RUSERSPROC_ALLNAMES ((u_long)3)
extern  utmp_array * rusersproc_allnames_3();
extern  utmp_array * rusersproc_allnames_3_svc();
#endif /* Old Style C */ 

#endif /* !_RUSERS_H_RPCGEN */
