#ifndef drv8301_regs_h
#define drv8301_regs_h
#include <stdint.h>
#define DRV8301_STATUS_1_ADDRESS 0x0

#ifdef __cplusplus
	extern "C" {
#endif	

typedef union drv8301_registr_s{
	struct{
		uint16_t	data:11;
		uint16_t	address :4;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8301_registr_t;
typedef drv8301_registr_t * drv8301_registr_p;

typedef union drv8301_STATUS_1_s{
	struct{
		uint16_t	FETLC_OC:1;
		uint16_t	FETHC_OC:1;
		uint16_t	FETLB_OC:1;
		uint16_t	FETHB_OC:1;
		uint16_t	FETLA_OC:1;
		uint16_t	FETHA_OC:1;
		uint16_t	OTW:1;
		uint16_t	OTSD:1;
		uint16_t	PVDD_UV:1;
		uint16_t	GVDD_UV:1;
		uint16_t	FAULT:1;
		uint16_t 	address:4;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8301_STATUS_1;

#define DRV8301_STATUS_2_ADDRESS 0x1

typedef union drv8301_STATUS_2_s{
	struct{
		uint16_t	DEVICE_ID:4;
		uint16_t	unused2:3;
		uint16_t	GVDD_OV:1;
		uint16_t	unused1:3;
		uint16_t 	address:4;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8301_STATUS_2;

#define DRV8301_CONTROL_1_ADDRESS 0x2
typedef enum  {
	DRV8301_GATE_CURRENT_1700mA		= 0
	, DRV8301_GATE_CURRENT_0700mA	= 1
	, DRV8301_GATE_CURRENT_0250mA	= 2
	, DRV8301_GATE_CURRENT_RESERVED	= 3
} DRV8301_GATE_CURRENT;

typedef enum  {
	DRV8301_GATE_RESET_NORMAL_MODE	= 0
	,DRV8301_GATE_RESET_LATCH_MODE	= 1
} DRV8301_GATE_RESET;

typedef enum  {
	DRV8301_PWM_MODE_6_INPUTS		= 0
	, DRV8301_PWM_MODE_3_INPUTS		= 1
} DRV8301_PWM_MODE;

typedef enum  {
	DRV8301_OCP_MODE_CURRENT_LIMIT		=0
	, DRV8301_OCP_MODE_OC_LATCH_SHUT_DOWN	= 1
	, DRV8301_OCP_MODE_REPORT_ONLY		= 2
	, DRV8301_OCP_MODE_OC_DISABLED		= 3
} DRV8301_OCP_MODE;

typedef enum  {
	DRV8301_OC_ADJ_SET_0060mV	= 0
	, DRV8301_OC_ADJ_SET_0068mV	= 1
	, DRV8301_OC_ADJ_SET_0076mV	= 2
	, DRV8301_OC_ADJ_SET_0086mV	= 3
	, DRV8301_OC_ADJ_SET_0097mV	= 4
	, DRV8301_OC_ADJ_SET_0109mV	= 5
	, DRV8301_OC_ADJ_SET_0123mV	= 6
	, DRV8301_OC_ADJ_SET_0138mV	= 7
	, DRV8301_OC_ADJ_SET_0155mV	= 8
	, DRV8301_OC_ADJ_SET_0175mV	= 9
	, DRV8301_OC_ADJ_SET_0197mV	= 10
	, DRV8301_OC_ADJ_SET_0222mV	= 11
	, DRV8301_OC_ADJ_SET_0250mV	= 12
	, DRV8301_OC_ADJ_SET_0282mV	= 13
	, DRV8301_OC_ADJ_SET_0317mV	= 14
	, DRV8301_OC_ADJ_SET_0358mV	= 15
	, DRV8301_OC_ADJ_SET_0403mV	= 16
	, DRV8301_OC_ADJ_SET_0454mV	= 17
	, DRV8301_OC_ADJ_SET_0511mV	= 18
	, DRV8301_OC_ADJ_SET_0576mV	= 19
	, DRV8301_OC_ADJ_SET_0648mV	= 20
	, DRV8301_OC_ADJ_SET_0730mV	= 21
	, DRV8301_OC_ADJ_SET_0822mV	= 22
	, DRV8301_OC_ADJ_SET_0926mV	= 23
	, DRV8301_OC_ADJ_SET_1043mV	= 24
	, DRV8301_OC_ADJ_SET_1175mV	= 25
	, DRV8301_OC_ADJ_SET_1324mV	= 26
	, DRV8301_OC_ADJ_SET_1491mV	= 27
	, DRV8301_OC_ADJ_SET_1679mV	= 28
	, DRV8301_OC_ADJ_SET_1892mV	= 29
	, DRV8301_OC_ADJ_SET_2131mV	= 30
	, DRV8301_OC_ADJ_SET_2400mV	= 31
} DRV8301_OC_ADJ_SET;



typedef union drv8301_CONTROL_1_s{
	struct{
		uint16_t	GATE_CURRENT:2;
		uint16_t	GATE_RESET:1;
		uint16_t	PWM_MODE:1;
		uint16_t	OCP_MODE:2;
		uint16_t	OC_ADJ_SET:5;
		uint16_t 	address:4;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8301_CONTROL_1;



#define DRV8301_CONTROL_2_ADDRESS 0x3
typedef enum  {
	DRV8301_OCTW_MODE_BOTH			= 0
	, DRV8301_OCTW_MODE_OT			= 1
	, DRV8301_OCTW_MODE_OC			= 2
	, DRV8301_OCTW_MODE_RESERVED	= 3
} DRV8301_OCTW_MODE;

typedef enum  {
	DRV8301_GAIN_10VdV				= 0
	, DRV8301_GAIN_20VdV			= 1
	, DRV8301_GAIN_40VdV			= 2
	, DRV8301_GAIN_80VdV			= 3
} DRV8301_GAIN;

typedef enum  {
	DRV8301_DC_CAL_CH1_NORMAL			= 0
	, DRV8301_DC_CAL_CH1_RUN			= 1
} DRV8301_DC_CAL_CH1;

typedef enum  {
	DRV8301_DC_CAL_CH2_NORMAL			= 0
	, DRV8301_DC_CAL_CH2_RUN			= 1
} DRV8301_DC_CAL_CH2;

typedef enum  {
	DRV8301_OC_TOFF_CYCLE_BY_CYCLE	= 0
	, DRV8301_OC_TOFF_OFF_TIME			= 1
} DRV8301_OC_TOFF;

typedef union drv8301_CONTROL_2_s{
	struct{
		uint16_t	OCTW_MODE:2;
		uint16_t	GAIN:2;
		uint16_t	DC_CAL_CH1:1;
		uint16_t	DC_CAL_CH2:1;
		uint16_t	OC_TOFF:1;
		uint16_t	reserved:4;
		uint16_t 	address:4;
		uint16_t 	request:1;
	};
	uint16_t value;	
} drv8301_CONTROL_2;

#define DRV8301_REG_COUNT 4

typedef union drv8301_regs_s{
	struct{
		drv8301_STATUS_1 status1;
		drv8301_STATUS_2 status2;
		drv8301_CONTROL_1 control1;
		drv8301_CONTROL_2 control2;
	};
	uint16_t regs[DRV8301_REG_COUNT];
} drv8301_regs_t;


typedef struct {
		struct{
			DRV8301_GATE_CURRENT GATE_CURRENT;
			DRV8301_GATE_RESET GATE_RESET;
			DRV8301_PWM_MODE PWM_MODE;
			DRV8301_OCP_MODE	OCP_MODE;
			DRV8301_OC_ADJ_SET OC_ADJ_SET;
		} CONTROL1;
		struct{
			DRV8301_OCTW_MODE	OCTW_MODE;
			DRV8301_GAIN	GAIN;
			DRV8301_DC_CAL_CH1	DC_CAL_CH1;
			DRV8301_DC_CAL_CH2	DC_CAL_CH2;
			DRV8301_OC_TOFF	OC_TOFF;
		} CONTROL2;

} drv8301_config_t;
typedef drv8301_config_t * drv8301_config_p;

typedef enum { 
	DRV8301_POLL_STATUS_NONE = 0
	, DRV8301_POLL_STATUS_QUERY1 = 1
	, DRV8301_POLL_STATUS_GET1 = 2
	, DRV8301_POLL_STATUS_QUERY2 = 3
	, DRV8301_POLL_STATUS_GET2 = 4
} drv8301_poll_status_t;;


typedef struct drv8301_s{
	drv8301_regs_t actual;
	drv8301_regs_t deseired;
	drv8301_poll_status_t poll_status;
//	void ( *exchange)(uint16_t,uint16_t * _actual);
	void ( *cs_on)(void);
	void ( *cs_off)(void);
	void ( *put)(uint16_t);
	uint16_t ( *get)(void);
	int ( *complete)(void);
	void ( *on_status_receive)(void);
} drv8301_t;

typedef drv8301_t * drv8301_p;

void drv8301_exchange(drv8301_p _drv, uint16_t _deseired, uint16_t * _actual);

void drv8301_begin(drv8301_p _drv, drv8301_config_p _config);
void drv8301_set_default(drv8301_config_p _config);
//void drv8301_status_query(drv8301_p _drv);
void drv8301_status_poll(drv8301_p _drv);

void drv8301_phy_delay_ns(unsigned int _ns);
void drv8301_phy_crash(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif
	
#endif