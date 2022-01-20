#ifndef mexo_drive_1B_config_hpp
#define  mexo_drive_1B_config_hpp

#define mexo_drive_types_t ::mexo::fixed_point<::mexo::int15>

#define MEXO_DRIVE_ACTUATOR_NAME joint
#define MEXO_DRIVE_PS_SUB_NAME pss

#define  joint_pss_VOLTAGE_REGULATOR_ENABLED 1
#define  joint_pss_CURRENT_MEASSURY_ENABLED 1
#define  joint_pss_CURRENT_DIFF_ENABLED 0
#define  joint_pss_CURRENT_FILTER_ENABLED 0
#define  joint_pss_CURRENT_FAST_FILTER_ENABLED 1

#define joint_MOTOR_POSTITION_MEASSURY_ENABLED 1
#define joint_MOTOR_SPEED_FILTER_ENABLED 1

#define joint_pss_CURRENT_REGULATOR_ENABLED 0
#define joint_SPEED_OV_CURRENT_MODE_ENABLED 0
#define joint_POSITION_OV_CURRENT_MODE_ENABLED 0

#define joint_pss_CURRENT_LIMMITER_ENABLED 1
#define joint_SPEED_OV_VOLTAGE_CL_MODE_ENABLED 1
#define joint_POSITION_OV_VOLTAGE_CL_MODE_ENABLED 1

#define can0_PATH RT("can0")

#define echo_PATH RT("echo")
#define echo_KIND backend

#define echo_can0_SUBA 0x1
#define echo_can0_SUBA_ANSW 0x1

#define exchange_PATH RT("exchange")
#define exchange_KIND backend
#define exchange_can0_SUBA 0x2
#define exchange_can0_SUBA_ANSW 0x2

#define serial0_PATH RT("serial0")
#define serial0_KIND frontend
#define serial0_can0_SUBA 0xF
#define serial0_can0_SUBA_ANSW 0xF

#define var_PATH RT("var")
#define var_KIND frontend
#define var_can0_SUBA 0xB
#define var_can0_SUBA_ANSW 0xB


#endif