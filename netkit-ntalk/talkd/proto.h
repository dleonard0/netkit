/* print.c */
void print_request(const char *cp, const CTL_MSG *mp);
void print_response(const char *cp, const CTL_RESPONSE *rp);

/* table.c */
void insert_table(CTL_MSG *request, CTL_RESPONSE *response);
CTL_MSG *find_request(CTL_MSG *request);
CTL_MSG *find_match(CTL_MSG *request);

/* other */
int announce(CTL_MSG *request, const char *remote_machine);
void process_request(CTL_MSG *mp, CTL_RESPONSE *rp);
int new_id(void);
int delete_invite(unsigned id_num);

