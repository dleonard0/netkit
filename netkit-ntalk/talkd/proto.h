/* quirks for repairs.c */

#define QUIRK_NONE   0
#define QUIRK_OTALK  1

struct sockaddr_in;

extern char ourhostname[];

/* print.c */
void print_request(const char *cp, const CTL_MSG *mp);
void print_response(const char *cp, const CTL_RESPONSE *rp);
void print_broken_packet(const char *pack, size_t len, struct sockaddr_in *);
void debug(const char *fmt, ...);
void set_debug(int logging, int badpackets);

/* table.c */
void insert_table(CTL_MSG *request, CTL_RESPONSE *response);
CTL_MSG *find_request(CTL_MSG *request);
CTL_MSG *find_match(CTL_MSG *request);

/* repairs.c */
u_int32_t byte_swap32(u_int32_t);
int rationalize_packet(char *buf, size_t len, size_t maxlen, 
		       struct sockaddr_in *);
size_t irrationalize_reply(char *buf, size_t maxbuf, int quirk);

/* other */
int announce(CTL_MSG *request, const char *remote_machine);
void process_request(CTL_MSG *mp, CTL_RESPONSE *rp, const char *fromhost);
int new_id(void);
int delete_invite(unsigned id_num);

