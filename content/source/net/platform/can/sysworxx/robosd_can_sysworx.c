#include "net/can/robosd_can.h"
#include "core/robosd_log.h"
#include "core/robosd_string.h"
#include "CanItf.h"
#include "robosd_target_api.h"

robo_can_p g_robo_cans[ROBO_CAN_CHAN_COUNT] = { PLATFORM_STATIC_ZEROS_STRUCT };

void ROBO_DECL robo_can_close(robo_can_p _can){
	if (_can->channel < ROBO_CAN_CHAN_COUNT) {
		if (g_robo_cans[_can->channel]){
			if (g_robo_cans[_can->channel] == _can){
				UCanShutDown(_can->channel);
				g_robo_cans[_can->channel] = 0;
				return;
			}
		}
		else{
			return;
		}
	}
	robo_errlog("error disconnec from  can channel %d", _can->channel);
}


tCanMsgStruct g_can_out_msgs[ROBO_CAN_CHAN_COUNT] =
{//  ID     , FrameFormat      , DLC, Data                    ,  TimeStamp
	{ 0x00000101, USBCAN_MSG_FF_STD, 8, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0 }
	//, { 0x00000101, USBCAN_MSG_FF_STD, 8, { 0, 0, 0, 0, 0, 0, 0, 0 }, 0 }
};

char g_buf[50];
void _show_msg(uint16_t msg, uint8_t * buf, int len){
	char * sptr = g_buf;
	int c = 49;
	int n = robo_sprintf(sptr, c, "%3x\t%2u\t", msg, len); sptr += n; c -= n;
	while (len--){
		n = robo_sprintf(sptr, c, "%2x ", (unsigned int)*buf++);  sptr += n; c -= n;
	}
}

void _show_write(uint16_t msg, uint8_t * buf, int len){
	if (len > 1) {
		_show_msg(msg, buf, len);
		printf("\n >> %s\n", g_buf);
	}
}
void _show_read(uint16_t msg, uint8_t * buf, int len){
	if (len > 0) {
		_show_msg(msg, buf, len);
		printf("\n << %s\n", g_buf);
	}
}

robo_result_t ROBO_DECL robo_can_send(robo_can_p _can, robo_can_msg_id_t msg, robo_byte_p buf, robo_size_t len){
	if (robo_can_ready(_can) == ROBO_ANSW_YES){
		tCanMsgStruct * can_out_msg = g_can_out_msgs + _can->channel;
		can_out_msg->m_dwID = msg;
		can_out_msg->m_bDLC = len;
		_show_write(msg, buf, len);
		ROBO_STD_MEM_COPY_TO(buf, can_out_msg->m_bData, len);
		ROBO_RETEX(UCanWriteMsg(_can->channel, can_out_msg, 1) == TRUE ? ROBO_SUCCESS : ROBO_ERROR);
	}
	else{
		ROBO_RETERR("channel %d is busy", (int)(_can->channel));
	}
}

void _robo_can_on_read3(BYTE bChannel_p, tCanMsgStruct *pCanMsg_p){
	robo_can_p can = g_robo_cans[bChannel_p];
	if (can){
		can->on_receive(can, pCanMsg_p->m_dwID, pCanMsg_p->m_bData, (DWORD)pCanMsg_p->m_bDLC);
		_show_read(pCanMsg_p->m_dwID, pCanMsg_p->m_bData, (uint16_t)pCanMsg_p->m_bDLC);
	}
}

robo_result_t ROBO_DECL  robo_can_pool(robo_can_p _can){
	UcanPoll(_can->channel);
	return ROBO_SUCCESS;
}



robo_result_t ROBO_DECL robo_can_open(robo_can_p _can){
	if (_can->channel < ROBO_CAN_CHAN_COUNT && g_robo_cans[_can->channel] == 0 ){
		ROBO_CHECKRET(UCanInit(_robo_can_on_read3, USBCAN_CHANNEL_CH0, USBCAN_ANY_MODULE, _can->bitrate, _can->channel, 0x7FF) == TRUE ? ROBO_SUCCESS : ROBO_ERROR);
		g_robo_cans[_can->channel] = _can;
		return ROBO_SUCCESS;
	}
	else{
		ROBO_RETERR("error connect to can channel %d with bitrate %d", _can->channel, _can->bitrate);
	}
}


robo_result_t ROBO_DECL  robo_can_ready(robo_can_p _can){
	return  UCanReady(_can->channel) == TRUE ? ROBO_ANSW_YES : ROBO_ANSW_NO;
}



void ROBO_DECL  robo_can_reset(robo_can_p _can){
	UCanReset(_can->channel, 0);
}