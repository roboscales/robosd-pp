#ifndef derv8711_h
#define derv8711_h

#include <stdint.h>
#include "burst/burst.h"

typedef enum  {
	DRV8711_CTRL_ENBL_OFF = 0
	, DRV8711_CTRL_ENBL_ON = 1
	, DRV8711_CTRL_ENBL_DEFAULT = DRV8711_CTRL_ENBL_OFF
} DRV8711_CTRL_ENBL;
typedef enum {
	DRV8711_CTRL_RDIR_DIR = 0
	, DRV8711_CTRL_RDIR_INVERCE = 1
	, DRV8711_CTRL_RDIR_DEFAULT = DRV8711_CTRL_RDIR_DIR
} DRV8711_CTRL_RDIR;
typedef enum {
	DRV8711_CTRL_RSTEP_NO = 0
	, DRV8711_CTRL_RSTEP_INDEXER = 1
	, DRV8711_CTRL_RSTEP_DEFAULT=DRV8711_CTRL_RSTEP_NO
} DRV8711_CTRL_RSTEP;
typedef enum {
	DRV8711_CTRL_MODE_FULL = 0
	, DRV8711_CTRL_MODE_HALF = 1
	, DRV8711_CTRL_MODE_ONE_DIV_4 = 2
	, DRV8711_CTRL_MODE_ONE_DIV_8 = 3
	, DRV8711_CTRL_MODE_ONE_DIV_16 = 4
	, DRV8711_CTRL_MODE_ONE_DIV_32 = 5
	, DRV8711_CTRL_MODE_ONE_DIV_64 = 6
	, DRV8711_CTRL_MODE_ONE_DIV_128 = 7
	, DRV8711_CTRL_MODE_ONE_DIV_256 = 8
	, DRV8711_CTRL_MODE_DEFAULT = DRV8711_CTRL_MODE_ONE_DIV_4
} DRV8711_CTRL_MODE;
typedef enum{
	DRV8711_CTRL_EXSTALL_INTERNAL = 0
	, DRV8711_CTRL_EXSTALL_EXTERNAL = 1
	, DRV8711_CTRL_EXSTALL_DEFAULT = DRV8711_CTRL_EXSTALL_INTERNAL
} DRV8711_CTRL_EXSTALL;

typedef enum {
	DRV8711_CTRL_ISGAIN_5 = 0
	, DRV8711_CTRL_ISGAIN_10 = 1
	, DRV8711_CTRL_ISGAIN_20 = 2
	, DRV8711_CTRL_ISGAIN_40 = 3
	, DRV8711_CTRL_ISGAIN_DEFAULT = DRV8711_CTRL_ISGAIN_5
} DRV8711_CTRL_ISGAIN;

typedef enum {
	DRV8711_CTRL_DTIME_400uS = 0
	, DRV8711_CTRL_DTIME_450uS = 1
	, DRV8711_CTRL_DTIME_650uS = 2
	, DRV8711_CTRL_DTIME_850uS = 3
	, DRV8711_CTRL_DTIME_DEFAULT = DRV8711_CTRL_DTIME_850uS
} DRV8711_CTRL_DTIME;

enum { DRV8711_TORQUE_TORQUE_DEFAULT = 0xFF };
typedef enum {
	DRV8711_TORQUE_SMPLTH_50uS = 0
	, DRV8711_TORQUE_SMPLTH_100uS = 1
	, DRV8711_TORQUE_SMPLTH_200uS = 2
	, DRV8711_TORQUE_SMPLTH_300uS = 3
	, DRV8711_TORQUE_SMPLTH_400uS = 4
	, DRV8711_TORQUE_SMPLTH_600uS = 5
	, DRV8711_TORQUE_SMPLTH_800uS = 6
	, DRV8711_TORQUE_SMPLTH_1000uS = 7
	, DRV8711_TORQUE_SMPLTH_DEFAULT = DRV8711_TORQUE_SMPLTH_100uS
} DRV8711_TORQUE_SMPLTH;


enum { DRV8711_OFF_TOFF_DEFAULT = 0x30 };

typedef enum {
	DRV8711_OFF_PWMMODE_INTERNALL = 0
	, DRV8711_OFF_PWMMODE_BYPASS = 1
	, DRV8711_OFF_PWMMODE_DEFAULT = DRV8711_OFF_PWMMODE_INTERNALL
} DRV8711_OFF_PWMMODE;


enum { DRV8711_BLANK_TBLANK_DEFAULT = 0x80 };
typedef enum {
	DRV8711_BLANK_ABT_DISABLE = 0
	, DRV8711_BLANK_ABT_ENABLE = 1
	, DRV8711_BLANK_ABT_DEFAULT = DRV8711_BLANK_ABT_DISABLE
} DRV8711_BLANK_ABT;


enum { DRV8711_DECAY_TDECAY_DEFAULT = 0x10 };
typedef enum {
	DRV8711_DECAY_DECMOD_SLOW_SLOW = 0
	, DRV8711_DECAY_DECMOD_SLOW_MIXED = 1
	, DRV8711_DECAY_DECMOD_FORCE_FORCE = 2
	, DRV8711_DECAY_DECMOD_MIXED_MIXED = 3
	, DRV8711_DECAY_DECMOD_SLOW_AUTO = 4
	, DRV8711_DECAY_DECMOD_AUTO_AUTO = 5
	, DRV8711_DECAY_DECMOD_DEFAULT = DRV8711_DECAY_DECMOD_SLOW_MIXED
} DRV8711_DECAY_DECMOD;

enum { DRV8711_STALL_SDTHR_DEFAULT = 0x40 };
typedef enum {
	DRV8711_STALL_SDCNT_STEP_FIRST = 0
	, DRV8711_STALL_SDCNT_STEP_2 = 1
	, DRV8711_STALL_SDCNT_STEP_4 = 2
	, DRV8711_STALL_SDCNT_STEP_8 = 3
	, DRV8711_STALL_SDCNT_DEFAULT = DRV8711_STALL_SDCNT_STEP_FIRST
} DRV8711_STALL_SDCNT;

typedef enum {
	DRV8711_STALL_VDIV_32 = 0
	, DRV8711_STALL_VDIV_16 = 1
	, DRV8711_STALL_VDIV_8 = 2
	, DRV8711_STALL_VDIV_4 = 3
	, DRV8711_STALL_VDIV_DEFAULT = DRV8711_STALL_VDIV_32
} DRV8711_STALL_VDIV;

typedef enum {
	DRV8711_DRIVE_OCPTH_250mV = 0
	, DRV8711_DRIVE_OCPTH_500mV = 1
	, DRV8711_DRIVE_OCPTH_750mV = 2
	, DRV8711_DRIVE_OCPTH_1000mV = 3
	, DRV8711_DRIVE_OCPTH_DEFAULT = DRV8711_DRIVE_OCPTH_500mV
} DRV8711_DRIVE_OCPTH;


typedef enum {
	DRV8711_DRIVE_OCPDEG_1uS = 0
	, DRV8711_DRIVE_OCPDEG_2uS = 1
	, DRV8711_DRIVE_OCPDEG_4uS = 2
	, DRV8711_DRIVE_OCPDEG_8uS = 3
	, DRV8711_DRIVE_OCPDEG_DEFAULT = DRV8711_DRIVE_OCPDEG_4uS
} DRV8711_DRIVE_OCPDEG;



typedef enum {
	DRV8711_DRIVE_TDRIVEN_250nS = 0
	, DRV8711_DRIVE_TDRIVEN_500nS = 1
	, DRV8711_DRIVE_TDRIVEN_1uS = 2
	, DRV8711_DRIVE_TDRIVEN_2uS = 3
	, DRV8711_DRIVE_TDRIVEN_DEFAULT = DRV8711_DRIVE_TDRIVEN_500nS
} DRV8711_DRIVE_TDRIVEN;


typedef enum {
	DRV8711_DRIVE_TDRIVEP_250nS = 0
	, DRV8711_DRIVE_TDRIVEP_500nS = 1
	, DRV8711_DRIVE_TDRIVEP_1uS = 2
	, DRV8711_DRIVE_TDRIVEP_2uS = 3
	, DRV8711_DRIVE_TDRIVEP_DEFAULT = DRV8711_DRIVE_TDRIVEP_500nS
} DRV8711_DRIVE_TDRIVEP;

typedef enum {
	DRV8711_DRIVE_IDRIVEN_100mA = 0
	, DRV8711_DRIVE_IDRIVEN_200mA = 1
	, DRV8711_DRIVE_IDRIVEN_300mA = 2
	, DRV8711_DRIVE_IDRIVEN_400mA = 3
	, DRV8711_DRIVE_IDRIVEN_DEFAULT = DRV8711_DRIVE_IDRIVEN_300mA
} DRV8711_DRIVE_IDRIVEN;

typedef enum {
	DRV8711_DRIVE_IDRIVEP_50mA = 0
	, DRV8711_DRIVE_IDRIVEP_100mA = 1
	, DRV8711_DRIVE_IDRIVEP_150mA = 2
	, DRV8711_DRIVE_IDRIVEP_200mA = 3
	, DRV8711_DRIVE_IDRIVEP_DEFAULT = DRV8711_DRIVE_IDRIVEP_150mA
} DRV8711_DRIVE_IDRIVEP;




typedef union drv8711_CTRL_s{
	struct{
		uint16_t	ENBL:1;
		uint16_t	RDIR:1;
		uint16_t	RSTEP:1;
		uint16_t	MODE:4;
		uint16_t	EXSTALL:1;
		uint16_t	ISGAIN:1;
		uint16_t	DTIME:2;
		uint16_t 	address:3;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8711_CTRL_t;

typedef union drv8711_TORQUE_s{
	struct{
		uint16_t	TORQUE:8;
		uint16_t	SMPLTH:3;
		uint16_t	Reserved:1;
		uint16_t 	address:3;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8711_TORQUE_t;

typedef union drv8711_OFF_s{
	struct{
		uint16_t	TOFF:8;
		uint16_t	PWMMODE:1;
		uint16_t	Reserved:3;
		uint16_t 	address:3;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8711_OFF_t;

typedef union drv8711_BLANK_s{
	struct{
		uint16_t	TBLANK:8;
		uint16_t	ABT:1;
		uint16_t	Reserved:3;
		uint16_t 	address:3;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8711_BLANK_t;

typedef union drv8711_DECAY_s{
	struct{
		uint16_t	TDECAY:8;
		uint16_t	DECMOD:3;
		uint16_t	Reserved:1;
		uint16_t 	address:3;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8711_DECAY_t;

typedef union drv8711_STALL_s{
	struct{
		uint16_t	SDTHR:8;
		uint16_t	SDCNT:2;
		uint16_t	VDIV:2;
		uint16_t 	address:3;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8711_STALL_t;

typedef union drv8711_DRIVE_s{
	struct{
		uint16_t	OCPTH:2;
		uint16_t	OCPDEG:2;
		uint16_t	TDRIVEN:2;
		uint16_t	TDRIVEP:2;
		uint16_t	IDRIVEN:2;
		uint16_t	IDRIVEP:2;
		uint16_t 	address:3;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8711_DRIVE_t;

typedef union drv8711_STATUS_s{
	struct{
		uint16_t	OVERTEMPERATURE:1;
		uint16_t	OVERCURRENT_A:1;
		uint16_t	OVERCURRENT_B:1;
		uint16_t	FAULT_A:1;
		uint16_t	FAULT_B:1;
		uint16_t	UNDERVOLTAGE:1;
		uint16_t 	STALL:1;
		uint16_t 	STALL_LANCH:1;
		uint16_t 	NotAvailabled:1;
		uint16_t 	Reserved:3;
		uint16_t 	address:3;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8711_STATUS_t;
#define DRV8711_REG_COUNT 8
typedef struct {
		struct{
			DRV8711_CTRL_ENBL ENBL;
			DRV8711_CTRL_RDIR RDIR;
			DRV8711_CTRL_RSTEP RSTEP;
			DRV8711_CTRL_MODE MODE;
			DRV8711_CTRL_EXSTALL EXSTALL;
			DRV8711_CTRL_ISGAIN ISGAIN;
			DRV8711_CTRL_DTIME DTIME;
		} CTRL;
		struct{
			uint8_t TORQUE;
			DRV8711_TORQUE_SMPLTH SMPLTH;
		}	TORQUE;
		struct{
			uint8_t TOFF;
			DRV8711_OFF_PWMMODE PWMMODE;
		}	OFF;
		struct{
			uint8_t TBLANK;
			DRV8711_BLANK_ABT ABT;
		}	BLANK;
		struct{
			uint8_t TDECAY;
			DRV8711_DECAY_DECMOD DECMOD;
		} DECAY;
		struct{
			uint8_t SDTHR;
			DRV8711_STALL_SDCNT SDCNT;
			DRV8711_STALL_VDIV VDIV;
		} STALL;
		struct{
			DRV8711_DRIVE_OCPTH OCPTH;
			DRV8711_DRIVE_OCPDEG OCPDEG;
			DRV8711_DRIVE_TDRIVEN TDRIVEN;
			DRV8711_DRIVE_TDRIVEP TDRIVEP;
			DRV8711_DRIVE_IDRIVEN IDRIVEN;
			DRV8711_DRIVE_IDRIVEP IDRIVEP;
		} DRIVE;

} drv8711_config_t;

typedef  drv8711_config_t * drv8711_config_p;
typedef union {
	struct{
		drv8711_CTRL_t  	CTRL;
		drv8711_TORQUE_t	TORQUE;
		drv8711_OFF_t			OFF;
		drv8711_BLANK_t		BLANK;
		drv8711_DECAY_t		DECAY;
		drv8711_STALL_t		STALL;
		drv8711_DRIVE_t		DRIVE;
		drv8711_STATUS_t	STATUS;
	};
	uint16_t values[DRV8711_REG_COUNT];
} drv8711_regs_t;
	
typedef struct drv8711_s{
	drv8711_regs_t actual;
	drv8711_regs_t deseired;
		
	burst_bool_t ( *complete)(void);
	void ( *put)(uint16_t);
	void ( *get)(uint16_t * _actual );
	void ( *cs_on)(void);
	void ( *cs_off)(void);

	void ( *begin)(drv8711_config_p);
	void ( *status_query)(void);
	burst_bool_t ( *status_check)(void);
		
} drv8711_t;
typedef  drv8711_t * drv8711_p;




void drv8711_set_default(drv8711_config_p _config);

void drv8711_begin_(drv8711_p _drv,drv8711_config_p);
void drv8711_status_query_(drv8711_p _drv);
burst_bool_t drv8711_status_check_(drv8711_p _drv);

#define BURST_DRV8711( S ) BURST_DRV8711_( S )
#define BURST_DRV8711_( S ) \
extern drv8711_t  S;

#define BURST_DRV8711_CREATE( S ) BURST_DRV8711_CREATE_( S )
#define BURST_DRV8711_CREATE_( S ) \
BURST_WEAK burst_bool_t	S##_complete(void){ return burst_false; }\
BURST_WEAK void					S##_put(uint16_t _data){ BURST_UNUSED(_data); }\
BURST_WEAK void			S##_get(uint16_t * _actual ){ *_actual = 0xFFFF;}\
BURST_WEAK void					S##_cs_on(void){}\
BURST_WEAK void					S##_cs_off(void){}\
BURST_WEAK  void S##_begin(drv8711_config_p _config){\
	drv8711_begin_(&S,_config);\
}\
BURST_WEAK  burst_bool_t S##_status_check(void){\
	return 	drv8711_status_check_(&S);\
}\
BURST_WEAK  void S##_status_query(void){\
	drv8711_status_query_(&S);\
}\
drv8711_t  S ={ \
	{}\
	,{}\
	, S##_complete \
	, S##_put \
	, S##_get \
	, S##_cs_on \
	, S##_cs_off \
	, S##_begin \
	, S##_status_query \
	, S##_status_check \
};

/*
*/
void drv8711_phy_delay_us(unsigned _us);
void drv8711_phy_crash(void);

#endif
