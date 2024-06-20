#ifndef robosd_fmd_hpp
#define robosd_fmd_hpp
#include "core/robosd_common.hpp"

using namespace  robo;
//todo копипаста из mexo
union descriptor {
    struct {
        uint16_t len : 13;
        uint16_t bsign : 1;
        uint16_t bconst : 1;
        uint16_t real : 1;
        //uint16_t fault : 5;
    };
    uint16_t memo;
    uint8_t bytes[2];
};
constexpr inline int descriptor_enco(uint8_t _len, bool _bsign, bool _bconst, bool _real) {
    return (int)((_len & 0x1FFF) + (_bsign ? 0x2000 : 0) + (_bconst ? 0x4000 : 0) + (_real ? 0x8000 : 0));
}


typedef enum {
    uint8 = descriptor_enco(1, false, false, false)
    , int8 = descriptor_enco(1, true, false, false)
    , uint16 = descriptor_enco(2, false, false, false)
    , int16 = descriptor_enco(2, true, false, false)
    , uint32 = descriptor_enco(4, false, false, false)
    , int32 = descriptor_enco(4, true, false, false)
    , uint64 = descriptor_enco(8, false, false, false)
    , int64 = descriptor_enco(8, true, false, false)
    , real = descriptor_enco(4, true, false, true)
    , ext = descriptor_enco(8, true, false, true)
    , const_uint8 = descriptor_enco(1, false, true, false)
    , const_int8 = descriptor_enco(1, true, true, false)
    , const_uint16 = descriptor_enco(2, false, true, false)
    , const_int16 = descriptor_enco(2, true, true, false)
    , const_uint32 = descriptor_enco(4, false, true, false)
    , const_int32 = descriptor_enco(4, true, true, false)
    , const_uint64 = descriptor_enco(8, false, true, false)
    , const_int64 = descriptor_enco(8, true, true, false)
    , const_real = descriptor_enco(4, true, true, true)
    , const_ext = descriptor_enco(8, true, true, true)
} types;

static inline robo::cstr type_name(const descriptor& d) {
    int ix = descriptor_enco(d.len, d.bsign, d.bconst, d.real);
    switch (ix) {
    case uint8: return RT("u8");
    case int8: return RT("i8");
    case uint16: return RT("u16");
    case int16: return RT("i16");
    case uint32: return RT("u32");
    case int32: return RT("i32");
    case uint64: return RT("u54");
    case int64: return RT("i64");
    case real: return RT("real");
    case ext: return RT("ext");
    case const_uint8: return RT("const u8");
    case const_int8: return RT("const i8");
    case const_uint16: return RT("const u16");
    case const_int16: return RT("const i16");
    case const_uint32: return RT("const u32");
    case const_int32: return RT("const i32");
    case const_uint64: return RT("const u54");
    case const_int64: return RT("const i64");
    case const_real: return RT("const real");
    case const_ext: return RT("const ext");
    default: return RT("unknown");
    }
}

class ROBO_EXPORT fmd {
public:

    template<typename T >class ROBO_EXPORT record_t {
    public:
        typedef list::unidir::store_t<record_t<T>, T* > list;
        T * raw;
        string name;
        descriptor type;

        record_t(T * _raw) : raw(_raw) {
            *raw = {};
            type.memo = 0;
        }

        bool load(cstr _section, int _index) {
            string key;
            key.format(RT("%s.%d"), _section, _index);
            ROBO_LBREAKN(name.load(key, RT("name")));
            ROBO_LBREAKN(ini::load(key, RT("size"),raw->size));
            ROBO_LBREAKN(ini::load(key, RT("addr"), raw->addr));
            type.len = raw->size;
            uint16_t tmp = 0;
            ini::try_load(key, RT("sign"), tmp);
            type.bsign = tmp>0;
            tmp = 0;
            ini::try_load(key, RT("const"), tmp);
            type.bconst = tmp > 0;

            tmp = 0;
            ini::try_load(key, RT("real"), tmp);
            type.real = tmp > 0;

            raw->destSize = raw->size;
            raw->destPtr = new uint8_t[raw->size];
            return true;
        }
        ~record_t(void) {
            if (raw->destPtr) {
                delete[] raw->destPtr;
            }   
        }
        void show(void) {
            switch (type.memo) {
            case uint8:
            case const_uint8:
            robo_infolog( RT("%s: %hu"), name.c_str(), (unsigned short)*(uint8_t*)raw->destPtr);
            case int8:
            case const_int8:
            return robo_infolog(RT("%s: %hd"), name.c_str(), (short)*(int8_t*)raw->destPtr);
            case uint16:
            case const_uint16:
            return robo_infolog(RT("%s: %hu"), name.c_str(), (unsigned short)*(uint16_t*)raw->destPtr);
            case int16:
            case const_int16:
            return robo_infolog(RT("%s: %hd"), name.c_str(), (short)*(int16_t*)raw->destPtr);
            case uint32:
            case const_uint32:
            return robo_infolog(RT("%s: %lu"), name.c_str(), (unsigned long)*(uint32_t*)raw->destPtr);
            case int32:
            case const_int32:
            return robo_infolog(RT("%s: %ld"), name.c_str(), (long)*(int32_t*)raw->destPtr);
            case uint64:
            case const_uint64:
            return robo_infolog(RT("%s: %llu"), name.c_str(), (unsigned long long) * (uint64_t*)raw->destPtr);
            case int64:
            case const_int64:
            return robo_infolog(RT("%s: %lld"), name.c_str(), (long long)*(int64_t*)raw->destPtr);
            case real:
            case const_real:
            return robo_infolog(RT("%s: %f"), name.c_str(), (float)*(float*)raw->destPtr);
            case ext:
            case const_ext:
            return robo_infolog(RT("%s: %f"), name.c_str(), (double)*(double*)raw->destPtr);
            }
        }
    };

    template<typename T >class ROBO_EXPORT scope_t {
    public:
        typedef list::unidir::store_t<scope_t<T>> list;
        typename record_t<T>::list records;
        T * ps = nullptr;
        int count = 0;
        string name;

        bool load(cstr _section, int _index) {
            string key;
            key.format(RT("%s.%d"), _section, _index);
            ROBO_LBREAKN(name.load(key, RT("name")));
            ROBO_LBREAKN(ini::load(key, RT("count"), count));
            ps = new T[count];
            T * s = ps;
            for (int i = 0; i < count; ++i,++s) {
                auto & t = records.push( s );
                t.load(key,i);
            }
            return true;
        }

        virtual ~scope_t(void) {
            records.free();
            if (ps)delete[]  ps;
        }

        void show(void) {
            for (auto* it = records.first(); it != nullptr; it = it->next()) {
                auto& rec = it->value();
                rec.show();
            }
        }

    };
    template<typename T >class ROBO_EXPORT content_t {
    public:
        typename scope_t<T>::list scops;
        int count = 0;
        bool load(cstr _section) {
            ROBO_LBREAKN(ini::load(_section, RT("count"), count));
            for (int i = 0; i < count; ++i) {
                auto& t = scops.push();
                t.load(_section, i);
            }
            return true;
        }
        virtual ~content_t(void) {
            scops.free();
        }
    };

    time_us_t scope_query_last_us = 0;
    time_us_t scope_query_period_us = 100000;

    time_us_t scope_show_last_us = 0;
    time_us_t scope_show_period_us = 1000000;

    static bool write_value(const void* _src, uint32_t _addr, uint16_t _sz);
    static bool read_value(void* _dst, uint32_t _addr, uint16_t _sz);

    class vref{
        typedef list::unsorted<vref> list;
        list::ref ref_;
        static list & list_(void);
    public:
        virtual bool load(void) = 0;
        virtual bool read(void) = 0;
        virtual bool write(void) = 0;
        static bool read_all(void);
        static bool load_all(void);
        vref(void);
        virtual ~vref(void);
    };

    template < typename T, uint16_t _type> struct var_t: public vref {
        uint32_t addr;
        descriptor type;
        T value;
        string name;

        var_t(cstr _name) : name(_name), value(T(0)) {
            type.memo = _type;
        }
        virtual bool load(void) {
            ROBO_LRET(ini::load(RT("vars"), name, addr));
        }
        virtual bool read(void) {
            ROBO_LRET(read_value(&value, addr, type.len));
        }
        virtual bool write(void) {
            ROBO_LRET(write_value(&value, addr, type.len));
        }
        virtual bool writen(int _n) {
            for (int i = 0; i < _n; i++) {
                if(write_value(&value, addr, type.len)) return true;
            }
            robo_errlog( RT("error write var %s"),name.c_str() );
            return false;
        }
    };
    fmd(void);
    ~fmd(void);
    bool begin(cstr _alias, int _port = -1);
    bool start(void);
    void stop(void);
    void finish(void);
    bool startup(void);
    bool select_scope(cstr _name);
    enum class states { begin, startup, run, none} state = states::none;
    bool poll(void);
    bool connected(void);
};
 
#endif
