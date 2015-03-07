#ifndef OPT_H
#define OPT_H


/* types */
typedef struct{
	char *prg_name,
		 *vim_cwd;
} opt_t;


/* external variable */
extern opt_t opt;


/* prototypes */
int opt_parse(int argc, char** argv);


#endif // OPT_H
