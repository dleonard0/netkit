/****** rpc_clntout.c ******/

void write_stubs(void);
void printarglist(proc_list *proc, char *addargname, char *addargtype);

/****** rpc_cout.c ******/

void emit(definition *def);
void emit_inline(declaration *decl, int flag);
void emit_single_in_line(declaration *decl, int flag, relation rel);

/****** rpc_hout.c ******/

void print_datadef(definition *def);
void print_funcdef(definition *def);
void pxdrfuncdecl(char *name, int pointerp);
void pprocdef(proc_list *proc, version_list *vp, 
	      char *addargtype, int server_p, int mode);
void pdeclaration(char *name, declaration *dec, int tab, char *separator);

/****** rpc_main.c ******/
	/* nil */

/****** rpc_parse.c ******/
definition *get_definition(void);

/****** rpc_sample.c ******/
void write_sample_svc(definition *def);
int write_sample_clnt(definition *def);
void add_sample_msg(void);
void write_sample_clnt_main(void);

/****** rpc_scan.c ******/
   /* see rpc_scan.h */

/****** rpc_svcout.c ******/
void write_most(char *infile , int netflag, int nomain);
void write_programs(char *storage);
int nullproc(proc_list *proc);
void write_svc_aux(int nomain);
void write_msg_out(void);
void write_inetd_register(char *transp);

/****** rpc_tblout.c ******/
void write_tables(void);

/****** rpc_util.c ******/
void reinitialize(void);
int streq(const char *a, const char *b);
definition *findval(list *lst, char *val, int (*cmp)());
void storeval(list **lstp, definition *val);
char *fixtype(char *type);
char *stringfix(char *type);
void ptype(char *prefix, char *type, int follow);
int isvectordef(char *type, relation rel);
char *locase(char *str);
void pvname_svc(char *pname, char *vnum);
void pvname(char *pname, char *vnum);
void error(const char *msg);
void crash(void);
void record_open(char *file);
void tabify(FILE *f, int tab);
char *make_argname(char *pname, char *vname);
void add_type(int len, char *type);
bas_type *find_type(char *type);
