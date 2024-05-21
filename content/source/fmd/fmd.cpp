#include "core/robosd_log.hpp"
#include "core/robosd_ini.hpp"
#include "fmd/fmd.hpp"
#include "net/platform/serial/win_com.hpp"
#include <iostream>
#include <windows.h>
#include "mcbcom.h"
#include <process.h>
#include "core/robosd_crc.hpp"
#include <thread>
#include <fstream>  
struct tty {
    HANDLE m_Handle;
    tty(void) {
        m_Handle = INVALID_HANDLE_VALUE;
    }
    virtual ~tty() {
        disconnect();
    }

    bool isok(void) const;
    static inline const int TIMEOUT = 1000;
    bool connect(cstr  _port, int _baudrate) {
        disconnect();

        m_Handle =
            CreateFile(
                _port,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

        if (m_Handle == INVALID_HANDLE_VALUE) {
            return false;
        }

        SetCommMask(m_Handle, EV_RXCHAR);
        SetupComm(m_Handle, 1500, 1500);

        COMMTIMEOUTS CommTimeOuts;
        CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
        CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
        CommTimeOuts.ReadTotalTimeoutConstant = TIMEOUT;
        CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
        CommTimeOuts.WriteTotalTimeoutConstant = TIMEOUT;

        if (!SetCommTimeouts(m_Handle, &CommTimeOuts)) {
            CloseHandle(m_Handle);
            m_Handle = INVALID_HANDLE_VALUE;
            return false;
        }

        DCB ComDCM;

        memset(&ComDCM, 0, sizeof(ComDCM));
        ComDCM.DCBlength = sizeof(DCB);
        GetCommState(m_Handle, &ComDCM);
        ComDCM.BaudRate = DWORD(_baudrate);
        ComDCM.ByteSize = 8;
        ComDCM.Parity = NOPARITY;
        ComDCM.StopBits = ONESTOPBIT;
        ComDCM.fAbortOnError = TRUE;
        ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
        ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
        ComDCM.fBinary = TRUE;
        ComDCM.fParity = FALSE;
        ComDCM.fInX = FALSE;
        ComDCM.fOutX = FALSE;
        ComDCM.XonChar = 0;
        ComDCM.XoffChar = (unsigned char)0xFF;
        ComDCM.fErrorChar = FALSE;
        ComDCM.fNull = FALSE;
        ComDCM.fOutxCtsFlow = FALSE;
        ComDCM.fOutxDsrFlow = FALSE;
        ComDCM.XonLim = 128;
        ComDCM.XoffLim = 128;

        if (!SetCommState(m_Handle, &ComDCM)) {
            CloseHandle(m_Handle);
            m_Handle = INVALID_HANDLE_VALUE;
            return false;
        }
        return true;
    }
    void disconnect() {
        if (m_Handle != INVALID_HANDLE_VALUE) {
            CloseHandle(m_Handle);
            m_Handle = INVALID_HANDLE_VALUE;
        }
    }
    char data[255];
    char* buf = nullptr;
    size_t sz = 255;

    virtual bool readln(void) {
        if (m_Handle == INVALID_HANDLE_VALUE) {
            return false;
        }

        DWORD ReadBytes;
        char BufferSerial[2];
        unsigned int CharCount = 0;

        while (CharCount < 254) {
            if (ReadFile(m_Handle, &BufferSerial, 1, &ReadBytes, NULL)) {
                if ((ReadBytes > 0) && (BufferSerial[0] != '\n') && (BufferSerial[0] != '\r')) {
                    data[CharCount] = BufferSerial[0];
                    CharCount++;
                }
                else if (BufferSerial[0] == '\r' || BufferSerial[0] == '\n') {
                    data[CharCount] = '\0';
                    if (CharCount > 0) {
                        return  true;
                    }
                }
            }
        }
        return false;
    }



};


namespace comms {
    tty current;
    tty voltage;
    net::win_com payload;
}

fmd::content_t<MCB_SCOPE> content_;

fmd::var_t<uint32_t, types::uint32> mode_(RT("mode"));
fmd::var_t<uint32_t, types::uint32> actual_(RT("actual"));
fmd::var_t<int16_t, types::int16> pwm_(RT("pwm"));
fmd::var_t<int16_t, types::int16> voltageVx10_(RT("voltageVx10"));
fmd::var_t<int16_t, types::int16> currentMAx10_(RT("currentMAx10"));
fmd::var_t<int32_t, types::int32> powerWtX100_(RT("powerWtX100"));

fmd::var_t<int16_t, types::int16> rowVoltage_(RT("voltage.raw"));
fmd::var_t<int16_t, types::int16> rowCurrent_(RT("current.raw"));



int voltage_min = 50;
int voltage = voltage_min;
int voltage_max = 5000;
int payload_current_min = 50;
int payload_current_max = 1000;
int payload_current_ = 250;

robo::time_us_t pause_us = 0;

float voltage_real = 0.f;
float voltage_real_prev = 0.f;
bool voltage_stable = false;
float current_real = 0.f;
float current_real_prev = 0.f;
bool current_stable = false;

void delay(robo::time_us_t _tm) {
    pause_us = robo::system::time_us();
    do {

    } while ( robo::system::time_us() - pause_us < _tm);
}


void payload_off(void) {
    uint8_t msg[] = { 0x01, 0x06, 0x01, 0x18, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00,0xBB,0xBB };
    uint16_t crc = crc16_modbus_by_table(msg, sizeof(msg) - 2);
    *(uint16_t*)(msg + 11) = crc;
    comms::payload.put(msg, sizeof(msg));
    while (comms::payload.busy()) { Sleep(0); }
    Sleep(20);
}

void payload_on(void) {
    uint8_t msg[] = { 0x01, 0x06, 0x01, 0x18, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x01,0xBB,0xBB };
    uint16_t crc = crc16_modbus_by_table(msg, sizeof(msg) - 2);
    *(uint16_t*)(msg + 11) = crc;
    comms::payload.put(msg, sizeof(msg));
    while (comms::payload.busy()) { Sleep(0); }
    Sleep(20);
}
void payload_value_set(uint8_t _md, int _val) {
    uint8_t regs[] = { 0, 4, 8, 12 };
    uint8_t msg[] = { 0x01, 0x06, 0x01, 0x1C, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0xAA, 0xBB,0xBB };
    msg[10] = _md;
    uint16_t crc = crc16_modbus_by_table(msg, sizeof(msg) - 2);
    *(uint16_t*)(msg + 11) = crc;
    comms::payload.put(msg, sizeof(msg));
    while (comms::payload.busy()) { Sleep(0); }
    Sleep(20);

    uint8_t msg2[] = { 0x01, 0x06, 0x02, 0xAA,  0x00, 0x01, 0x04, 0xAA, 0xAA, 0xAA, 0xAA, 0xBB,0xBB };
    union {
        int value;
        uint8_t bytes[4];
    };
    value = _val;
    msg2[3] = regs[_md];
    msg2[7] = bytes[3];
    msg2[8] = bytes[2];
    msg2[9] = bytes[1];
    msg2[10] = bytes[0];
    crc = crc16_modbus_by_table(msg2, sizeof(msg2)-2);
    *(uint16_t*)(msg2 + 11) = crc;
    comms::payload.put(msg2, sizeof(msg2));
    while (comms::payload.busy()) { Sleep(0); }
    Sleep(20);
}

static std::thread rdv([&] {
    while (true) {
        if (comms::voltage.m_Handle != INVALID_HANDLE_VALUE) {
            if (comms::voltage.readln()) {
                string(comms::voltage.data).to_number(voltage_real);
                auto delta = voltage_real - voltage_real_prev;
                if(delta<0.f) delta = -delta;
                if (!voltage_stable) {
                    voltage_stable = delta < 0.02f;
                }
                voltage_real_prev = voltage_real;
            }
        }
        Sleep(10);
    }
});
static std::thread rdc([&] {
    while (true) {
        if (comms::current.m_Handle != INVALID_HANDLE_VALUE) {
            if (comms::current.readln()) {
                string(comms::current.data).to_number(current_real);
                auto delta = current_real - current_real_prev;
                if (delta < 0.f) delta = -delta;
                if (!current_stable) {
                    current_stable = delta < 1.f;
                }
                current_real_prev = current_real;
            }
        }
        Sleep(10);
    }
                       });
                       
void inverter_run(void) {
    robo::time_us_t now = 0;
    mode_.value = 2;
    voltageVx10_.value = voltage;
    powerWtX100_.value = 100000;
    currentMAx10_.value = 12000;
    pwm_.value = 1560;
    actual_.value = 1;
    while (!voltageVx10_.write());
    while (!currentMAx10_.write());
    while (!powerWtX100_.write());
    while (!pwm_.write());
    while (!mode_.write());
    while (!actual_.write());
    //пишем ток нагрузки
    uint8_t tmp[] = { 1,2,3 };
    uint16_t r = crc16_modbus_by_table(tmp, 3);


    delay(300000);
    voltage_stable = false;
    current_stable = false;
    while (!(voltage_stable && current_stable)) {
        rowVoltage_.read();
        rowCurrent_.read();
        Sleep(0);
    }

    std::ofstream ofs("result.txt", std::ios_base::out | std::ios_base::app);
    robo_infolog(RT("%f %d %f %d"),voltage_real, rowVoltage_.value, current_real, rowCurrent_.value);
    ofs << voltage_real << ";" << rowVoltage_.value << ";" << current_real << ";" << rowCurrent_.value << "\n";

    //читаем ток
    //читаем ацп
    voltage = voltage + 10;
    if (voltage > voltage_max) {
        voltage = voltage_min;
        voltageVx10_.value = voltage;
        while (!voltageVx10_.write());
        while (!actual_.write());
        Sleep(10000);
        payload_current_ = payload_current_ + 50;
        payload_value_set(1, payload_current_);
        Sleep(1000);
        if (payload_current_ > payload_current_max) {
            payload_current_ = payload_current_min;
        }
    }


}

fmd::fmd(void) {
    //   src_comm_.events.connected = [] { robo_infolog(RT("%s connected"), src_comm_.alias().c_str()); };
    //src_comm_.events.disconnected = [] { robo_infolog(RT("%s disconected"), src_comm_.alias().c_str()); };
    //src_comm_.events.reconnect = [] { robo_infolog(RT("%s reconnect"), src_comm_.alias().c_str()); };
}

HMCBCOM hCom;
MCB_RESP_GETINFO info;


net::win_com bridge;


fmd::~fmd(void) {
    content_.scops.free();
}


bool fmd::write_value(const void* _src, uint32_t _addr, uint16_t _sz) {
    ROBO_LRET(SUCCEEDED(McbWriteVar(
        hCom,       // board handle
        _src,        // source buffer
        _addr,         // variable address
        _sz           // variable size
    )));
}

bool fmd::read_value(void* _dst, uint32_t _addr, uint16_t _sz) {
    ROBO_LRET(SUCCEEDED(McbReadVar(
        hCom,       // board handle
        _dst,        // destination buffer
        _addr,         // variable address
        _sz           // variable size
    )));
}

bool fmd::begin(cstr _alias) {
    int portnum;
    int bitrate;

    ROBO_LBREAKN(ini::load(_alias, string(RT("%s.%s"), _alias, RT("com")),RT("portnumber"), portnum));
    ROBO_LBREAKN(ini::load(_alias, string(RT("%s.%s"), _alias, RT("com")), RT("bitrate"), bitrate));
    // ROBO_LRET(src_comm_.begin(alias));
    DWORD hr = 0;
    wchar_t buffer[50];
    buffer[_swprintf(buffer, L"RS232;COM%d;speed=%d", portnum, bitrate)] = 0;
    hr = McbOpenComEx(&hCom, buffer);
    if (!SUCCEEDED(hr)) {
        robo_errlog("com errpr %u %d", hr, GetLastError());
    }

    ROBO_LBREAKN(content_.load("scops"));
    ROBO_LBREAKN(vref::load_all());
    state = states::begin;
    //bridge.begin("COM39");
    // bridge.connect("\\\\.\\COM39");
    
    comms::payload.begin("COM43");
    //comms::voltage.begin("COM44");
    //comms::current.begin("COM41");
    /*comm_2.begin("COM106");
    comm_3.begin("COM100");*/

    comms::payload.connect("\\\\.\\COM43");
    comms::voltage.connect("\\\\.\\COM44",9600);
    comms::current.connect("\\\\.\\COM41", 9600);

    /*comm_2.connect("\\\\.\\COM106");
    comm_3.connect("\\\\.\\COM100");*/
    return true;
}
bool fmd::poll(void) {
    switch (state) {
    case states::none:
    return false;
    break;
    case states::begin:
    //ждем коннекцию, переподключаемся - но это потом
        state = states::startup;
    break;
    case states::startup:
        ROBO_LBREAKN(startup());
        ROBO_LBREAKN(vref::read_all());
        mode_.value = 0;
        while (!mode_.write());
        payload_off();
        payload_on();
        payload_value_set(1, payload_current_);
        state = states::run;
    break;
    case states::run:
    time_us_t now = system::time_us();
    if (now - scope_query_last_us > scope_query_period_us) {
        scope_query_last_us = now;
        ROBO_ALARMN(SUCCEEDED(McbReadScope(hCom)));
    }
    #if 0
    if (now - scope_show_last_us > scope_show_period_us) {
        scope_show_last_us = now;
        auto& scope = content_.scops.first()->value();
        scope . show();
    }
    #endif
    inverter_run();
    break;

    }
    return true;
}

bool fmd::startup(void) {
    //todo govnocod
    /*
    */
    auto& scope = content_.scops.first()->value();
    ROBO_LBREAKN(SUCCEEDED(McbSetupScope(hCom, scope.count, scope.ps)));
    return true;
}

fmd::vref::list& fmd::vref::list_(void) {
    static list list__;
    return list__;
}
bool fmd::vref::read_all(void) {
    for (auto* i = list_().first(); i; i = i->next()) {
        ROBO_LBREAKN(i->owner().read());
    }
    return true;
}
bool fmd::vref::load_all(void) {
    for (auto* i = list_().first(); i; i = i->next()) {
        ROBO_LBREAKN(i->owner().load());
    }
    return true;
}

fmd::vref::vref(void): ref_(*this) {
    ref_.attach_to(list_());
}
fmd::vref::~vref(void) {
    ref_.dettach();
}


#if 0
/*
    uint8_t buf[255];
    size_t av;
    
    av = bridge.get(buf, 255);
    if(av){
        std::cout << "=>>>> ";
        for (size_t i = 0; i < av; ++i) std::cout << (int)buf[i] << " ";
        std::cout << "\n";
    }

    av = comm_2.get(buf, 255);
    if (av) {
        comm_1.put(buf, av);
        std::cout << "<<<<= ";
        for (size_t i = 0; i < av; ++i) std::cout << (int)buf[i]<<" ";
        std::cout << "\n";
    }
    av = comm_3.get(buf, 255);
    if (av) {
        comm_1.put(buf, av);
        std::cout << "<<<<= ";
        for (size_t i = 0; i < av; ++i) std::cout << (int)buf[i] << " ";
        std::cout << "\n";
    }
    

    static time_us_t last = 0;
    time_us_t now = system::time_us();
    if (now - last > 10000) {
        ROBO_ALARMN(SUCCEEDED(McbGetInfo(hCom, &info)));
        last = now;
    }
    
    static int var = 0;
    static time_us_t last = 0;
    time_us_t now = system::time_us();
    if (now - last > 10000) {
        last = now;
        //0x20000254
        switch (var) {
            #if 0
        case 0:
        {
            uint8_t req[] = { 0x04, 0x05, 0x04, 0x54, 0x02, 0x00, 0x20, 0x7D };
            comm_1.put(43);
            comm_1.put(req, sizeof(req));
            comm_1.put(43);
            var++;
        }
        break;
        case 1:
        {                            
            uint8_t req[] = { 0x04, 0x05, 0x02, 0xF8, 0x00, 0x00, 0x20, 0xDD };
            comm_1.put(43);
            comm_1.put(req,sizeof(req));
            comm_1.put(43);           
            var++;
        }
        break;
        #endif
        case 0:
        { /*
            uint8_t req[] = { 0x04, 0x05, 0x02, 0xD0, 0x02, 0x00, 0x20, 0x03 };
            comm_1.put(43);
            comm_1.put(req, sizeof(req));
            comm_1.put(43);
            var ++;
            uint8_t req[] = { "*IDN\r\n" };
            comm_1.put(req, sizeof(req));*/
        }
        break;
        case 1:
        {
            /*
            static uint8_t n = 1;
            //5 7 2 208 2 0 32 255 0 1
            //5 7 2 208 2 0 32 1 0 255
            uint8_t req[] = { 0x05, 0x07, 0x02, 0xD0, 0x02, 0x00, 0x20, n,n, (uint8_t)(256L-n-n) };
            n++;
            comm_1.put(43);
            comm_1.put(req, sizeof(req));
            comm_1.put(43);
            var = 0;*/
        }

            
        }
    }
    }
    #endif


bool fmd::start(void) {
    ROBO_LBREAKN(SUCCEEDED(McbAttachThread(hCom)));
    //src_comm_.connect( src_comm_.alias() );


    //MCBCOM_API DWORD 
    //uint32_t var=-1;
    //McbReadVar(hCom, &var, 0x20001D34, 4);
    return true;
}
void fmd::stop(void) {
    //src_comm_.connect(src_comm_.alias());
    McbDetachThread(hCom);
}
void fmd::finish(void) {
    McbCloseCom(hCom);

}
