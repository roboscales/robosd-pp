#ifndef burst_vartree_hpp
#define burst_vartree_hpp
#include "burst/burst_common.h"

#ifndef BURST_VAR_ENABLED
#define BURST_VAR_ENABLED 1
#endif

#if BURST_VAR_ENABLED == 1

typedef union burst_var_descriptor_u {
	struct {
		uint16_t len : 13;
		uint16_t bsign : 1;
		uint16_t bconst : 1;
		uint16_t real : 1;
		//uint16_t fault : 5;
	};
	uint16_t value;
	uint8_t bytes[2];
} burst_var_descriptor;
#define descriptor_enco(_len, _bsign, _bconst,  _real)\
	 (int)((_len & 0x1FFF) + (_bsign ? 0x2000 : 0) + (_bconst ? 0x4000 : 0) + (_real ? 0x8000 : 0))

typedef enum {
	burst_var_uint8 = descriptor_enco(1, 0, 0, 0)
	, burst_var_int8 = descriptor_enco(1, 1, 0, 0)
	, burst_var_uint16 = descriptor_enco(2, 0, 0, 0)
	, burst_var_int16 = descriptor_enco(2, 1, 0, 0)
	, burst_var_uint32 = descriptor_enco(4, 0, 0, 0)
	, burst_var_int32 = descriptor_enco(4, 1, 0, 0)
	, burst_var_uint64 = descriptor_enco(8, 0, 0, 0)
	, burst_var_int64 = descriptor_enco(8, 1, 0, 0)
	, burst_var_real = descriptor_enco(4, 1, 0, 1)
	, burst_var_ext = descriptor_enco(8, 1, 0, 1)
	, burst_var_const_uint8 = descriptor_enco(1, 0, 1, 0)
	, burst_var_const_int8 = descriptor_enco(1, 1, 1, 0)
	, burst_var_const_uint16 = descriptor_enco(2, 0, 1, 0)
	, burst_var_const_int16 = descriptor_enco(2, 1, 1, 0)
	, burst_var_const_uint32 = descriptor_enco(4, 0, 1, 0)
	, burst_var_const_int32 = descriptor_enco(4, 1, 1, 0)
	, burst_var_burst_var_const_uint64 = descriptor_enco(8, 0, 1, 0)
	, burst_var_const_int64 = descriptor_enco(8, 1, 1, 0)
	, burst_var_const_real = descriptor_enco(4, 1, 1, 1)
	, burst_var_const_ext = descriptor_enco(8, 1, 1, 1)
} burst_var_types;

const char * burst_var_type_name(const uint16_t _type);

#include "burst/burst_common.h"


#define BURST_VAR_TYPE_VAR 0
#define BURST_VAR_TYPE_PUSH 1
#define BURST_VAR_TYPE_POP  2
#define BURST_VAR_MAX_LEN  0x6

#define BURST_VAR_QUERY_INDEX 0
#define BURST_VAR_QUERY_GET 1
#define BURST_VAR_QUERY_PUT 2

#define  BURST_VAR_QUERY_INVALID_KEY   0x10
#define  BURST_VAR_QUERY_INVALID_INDEX	0x20
#define  BURST_VAR_QUERY_INVALID_OFFSET	0x30
#define  BURST_VAR_QUERY_INVALID_LENGTH	0x40

#ifndef BURST_VAR_ENABLED
#define BURST_VAR_ENABLED 1
#endif

#if BURST_VAR_ENABLED == 1

#ifndef BURST_VAR_PROG_ADDR_T
#define BURST_VAR_PROG_ADDR_T uint8_t *
#endif

#ifndef BURST_VAR_ADDR_T
#define BURST_VAR_ADDR_T uint8_t *
#endif

#ifndef BURST_VAR_PROGRAMM_STR_T
#define BURST_VAR_PROGRAMM_STR_T const char *
#endif


#define burst_varreg_t struct burst_varreg_s
#define burst_varreg_p burst_varreg_t *
struct burst_varreg_s{
	uint8_t type;
};

#define burst_varnode_t struct burst_varnode_s
#define burst_varnode_p burst_varnode_t *
struct burst_varnode_s{
	burst_varreg_t req;
	uint8_t name_len;
	BURST_VAR_PROG_ADDR_T name;
};

#define burst_var_t struct burst_var_s
#define burst_var_p burst_var_t *
struct burst_var_s{
	burst_varnode_t ref;
	burst_var_descriptor desc;
	BURST_VAR_ADDR_T address;
	int32_t full_path_hash;
};


#define BURST_VAR_REG(vartable, var, n, t) _BURST_VAR_REG(vartable, var,n, t)
#define _BURST_VAR_REG( vartable, var, n,t)\
{\
	static BURST_VAR_PROGRAMM_STR_T _c = n; \
	static  burst_var_t _varreg = { \
		{\
			{\
				BURST_VAR_TYPE_VAR\
			}\
			, 0\
			, (BURST_VAR_PROG_ADDR_T)0\
		}\
		, {}\
		, (BURST_VAR_ADDR_T)0\
		, 0\
	}; \
	_varreg.ref.name = (BURST_VAR_PROG_ADDR_T)_c; \
	_varreg.ref.name_len = burst_var_name_len(_c); \
	_varreg.address = (BURST_VAR_ADDR_T)(&var);\
	_varreg.desc.value = burst_var_##t;\
	burst_alarm(_varreg.desc.len == sizeof(var));\
	burst_vartable_reg(vartable, (burst_varreg_p)&_varreg); \
}

#define BURST_VAR_PUSH(dev, n) _BURST_VAR_PUSH(dev, n)
#define _BURST_VAR_PUSH( dev, n)\
{\
	static BURST_VAR_PROGRAMM_STR_T _c = n; \
	static  burst_varnode_t _varreg = { \
		{\
			BURST_VAR_TYPE_PUSH\
		}\
				, 0\
				, (BURST_VAR_PROG_ADDR_T)0\
        };\
	_varreg.name = (BURST_VAR_PROG_ADDR_T)_c; \
	_varreg.name_len = burst_var_name_len(_c); \
	burst_vartable_reg(dev, (burst_varreg_p)&_varreg); \
}



#define BURST_VAR_POP(dev) _BURST_VAR_POP(dev)
#define _BURST_VAR_POP( dev)\
{\
	static  burst_varreg_t _varreg; \
	_varreg.type = BURST_VAR_TYPE_POP; \
	burst_vartable_reg(dev, &_varreg); \
}

typedef struct burst_vartable_s{
	uint8_t count;
	uint8_t tableSize;
	burst_varreg_p * table;
} burst_vartable_t;
typedef burst_vartable_t * burst_vartable_p;

uint8_t burst_var_name_len(const char * s);

int burst_vartable_perform(burst_vartable_p _vartable, const uint8_t * buf_in, uint8_t * buf_out);

void burst_vartable_init(burst_vartable_p _vartable, burst_varreg_p * _vars ,  uint8_t _size);

void burst_vartable_deinit(burst_vartable_p _vartable);
void burst_vartable_reg(burst_vartable_p _vartable, burst_varreg_p _var);
uint8_t burst_vartable_hash_to_id(burst_vartable_p _vartable, int32_t _hash);
void burst_vartable_create_index(burst_vartable_p _vartable);
#endif

#endif

#endif
