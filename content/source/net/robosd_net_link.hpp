#ifndef __robosd_net_link_hpp
#define __robosd_net_link_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_common.hpp"
#include "core/robosd_string.hpp"
namespace robo{
    namespace net{
        class ROBO_EXPORT link{
            typedef ::robo::list::unique<link, int> map;
            typedef map::ref ref;
            static map & map_(void);
            static map & frontend_poll_map_(void);
            static map & backend_poll_map_(void);
            ref ref_;
            ref poll_ref_;
        public:
            typedef ::robo::delegat::base<void, const uint8_t*, size_t > reader;
        private:
            reader* reader_ = nullptr;
            ::robo::string alias_;
        protected:

           void receive(const uint8_t * _data, size_t _len);
           virtual bool  do_post(const uint8_t * _data, size_t _len) = 0;
           bool start_poll(bool _backend = false);
           void stop_poll(void);
           virtual void poll(void){};
        public:
           const string & alias(void) const { return alias_; }
           link(void);
           virtual ~link(void);
           bool  post(const uint8_t * _data, size_t _len);
           void listen(reader & _reader);
           virtual void reset(void) = 0;
           virtual size_t space(void) = 0;
           virtual size_t max_space(void) = 0;
           virtual bool begin(cstr _alias);
           virtual void finish(void);
           static bool listen(cstr _alias, reader *  _reader );
           bool listen(reader *  _reader );
           static link * find(cstr _alias);
           static void frontend_poll(void);
           static void backend_poll(void);
        };
					
    }
}
#endif
