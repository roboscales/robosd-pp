#ifndef __robo_ini_hpp
#define  __robo_ini_hpp
#include "core/robosd_log.hpp"
#include "core/robosd_string.hpp"
#include  <limits>


namespace robo {
	namespace ini {
		template <typename T>	bool try_load(cstr _sect, cstr _key, T& _value) {
			string value;
			ROBO_LBREAKN(value.load(_sect, _key));
			ROBO_LBREAKN(value.to_number(_value));
			return true;
		}

		template <typename T>	bool load(cstr _sect, cstr _key, T& _value) {
			ROBO_LRET_F(try_load(_sect, _key, _value), "error load number param %s/%s", _sect, _key);
		}
	}
}

/*
#define ROBO_BREAKEN_LOAD_NUMBER(s,p,v) ROBO_BREAKN_F ( (robo_ini_load_number( RS(s) , RS(p) , v)==ROBO_ANSW_YES) , "error load number param %s/%s", (const  char*)(s),(const  char*)(p) )


#define ROBO_BREAKEN_LOAD_INT_LIST(s,p,x,v, pc) ROBO_BREAKN_F (robo_ini_load_int_list( RS(s) , RS(p) , x, v, pc)>=ROBO_SUCCESS, "error load int array %s/%s",(const char *)(s),(const char *)(p) )

#define ROBO_BREAKEN_LOAD_INT(s,p,v) ROBO_BREAKN_F ( (robo_ini_load_int( RS(s) , RS(p) , *v, v)==ROBO_ANSW_YES) , "error load int param %s/%s", (const  char*)(s),(const  char*)(p) )
#define ROBO_BREAKEN_LOAD_FLOAT(s,p,v) ROBO_BREAKN_F ( (robo_ini_load_float( RS(s) , RS(p) , *v, v)==ROBO_ANSW_YES) , "error load float param %s/%s",(const  char*)(s),(const  char*)(p) )
#define ROBO_BREAKEN_LOAD_STR(s,p,v, m) ROBO_BREAKN_F ( (robo_ini_load_str( RS(s) , RS(p) , RS(""), v, m)==ROBO_ANSW_YES) , "error load string param %s/%s",(const  char*)(s),(const  char*)(p) )

#define  ROBO_BREAKEN_LOAD_FLOAT_ARR(s,p,v, m)  ROBO_BREAKN_F ( robo_ini_load_float_arr( RS(s) , RS(p) , v, m)==ROBO_ANSW_YES, "error load float array %s/%s",(const char *)(s),(const char *)(p) )
#define  ROBO_BREAKEN_LOAD_INT_ARR(s,p,v, m)  ROBO_BREAKN_F ( robo_ini_load_int_arr( RS(s) , RS(p) , v, m)==ROBO_ANSW_YES, "error load float array %s/%s",(const char *)(s),(const char *)(p) )
*/
#endif
