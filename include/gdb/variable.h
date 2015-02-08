#ifndef VARIABLEID_H
#define VARIABLEID_H


typedef enum{
	V_ID = 1,
	V_MSG,
	V_BREAKPT,
	V_NUMBER,
	V_TYPE,
	V_CATCH_TYPE,
	V_DISPOSITION,
	V_ENABLED,
	V_ADDR,
	V_FUNCTION,
	V_FILE,
	V_FULLNAME,
	V_LINE,
	V_AT,
	V_PENDING,
	V_EVAL_BY,
	V_THREAD,
	V_TASK,
	V_CONDITION,
	V_IGNORE,
	V_ENABLE,
	V_TRACEFRAME,
	V_TRACEMARKER,
	V_MASK,
	V_PASS,
	V_ORIG_LOCATION,
	V_TIMES,
	V_INSTALLED,
	V_WHAT,
	V_THREAD_GROUPS,
	V_PARAM,
	V_VALUE,

} variable_id_t;

struct variable_t{
	const char* name;
	variable_id_t id;
};

typedef variable_t variable_t;


#endif // VARIABLEID_H
