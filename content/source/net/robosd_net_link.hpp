#ifndef __robosd_net_link_hpp
#define __robosd_net_link_hpp
#include "core/robosd_list.hpp"
#include "core/robosd_common.hpp"
#include "core/robosd_string.hpp"
namespace robo{
    namespace net{
        class ROBO_EXPORT link{
				public:
            typedef ::robo::list::unique<link, int> map;
            typedef map::ref ref;
				private:
            static map & map_(void);
            static map & frontend_poll_map_(void);
            static map & backend_poll_map_(void);
            ref ref_;
            ref poll_ref_;
        private:
            ::robo::string alias_;
        protected:

           bool start_poll(bool _backend = false);
           void stop_poll(void);
           virtual void poll(void){};
        public:
           const string & alias(void) const { return alias_; }
           bool busy(void) { return (space_max() > space()); };
           link(void);
           virtual ~link(void);
           virtual void reset(void) = 0;
           virtual size_t space(void) = 0;
           virtual size_t space_max(void) = 0;
           virtual bool begin(cstr _alias);
           virtual void finish(void);
           template<typename T> static T* query(cstr _caption) {
               T * s = dynamic_cast<T*>( map_().find(hash(_caption, 0)) );
               if (s) {
                   ((link *)s)->ref_.dettach();
                   return s;
               }
               else {
                   return 0;
               }
           }
           template<typename T> static T& query_ref(cstr _caption) {
               T* s = dynamic_cast<T*>( map_().find(hash(_caption, 0)) );
               if (s) {
                   s->ref_.dettach();
                   return *s;
               }
               else {
                   static typename T::dummy dummy_;
                   robo_errlog("serial '%s' is't found !", _caption);
                   return  dummy_;
               }
           }
           template<typename T> static void forall(lambda<void(T&)>& _operator) {
               for (ref* r = map_().first(); r; r = r->next()) {
                   link& l = r->owner();
                   if ( dynamic_cast<T *>(&l) != nullptr )
                       _operator( (T&)l );
               }
           }               
           void  release(void);

           static void frontend_poll(void);
           static void backend_poll(void);
        };

        class ROBO_EXPORT port: public link {
        public:
            typedef ::robo::delegat::base<void, const uint8_t*, size_t > reader;
        private:
            reader* reader_ = nullptr;
            ::robo::string alias_;
        protected:

            void receive(const uint8_t* _data, size_t _len);
            virtual bool  do_post(const uint8_t* _data, size_t _len) = 0;
        public:
            port(void);
            virtual ~port(void);
            bool  post(const uint8_t* _data, size_t _len);
            void listen(reader& _reader);
            bool listen(reader* _reader);
            static bool listen(cstr _alias, reader* _reader);
        };
    }
}
#endif
