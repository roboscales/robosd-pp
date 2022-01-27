#include "net/robosd_net_link.hpp"
namespace robo{
    namespace net{
        link::map & link::map_(void){
            static map  map__;
            return map__;
        }
        link::map & link::frontend_poll_map_(void){
            static map  frontend_poll_map___;
            return frontend_poll_map___;
        }
        link::map & link::backend_poll_map_(void){
            static map  backend_poll_map__;
            return backend_poll_map__;
        }

        link::link(): ref_(*this,0), poll_ref_(*this,0){
        };

        link::~link(void){
            finish();
        }

        bool link::begin(cstr _alias) {
            int h = hash(_alias, 0);
            ref_.set_key(h);
            poll_ref_.set_key(h);
            ref_.attach_to(map_());
            alias_ = _alias;
            return ref_.attached();
        }

        void link::finish(void) {
            ref_.dettach();
        }

        bool link::start_poll(bool _backend) {
            //todo  проверить состояние - только при загрузке!
            if (_backend) {
                return  poll_ref_.attach_to(backend_poll_map_());
            }
            else {
                return  poll_ref_.attach_to(frontend_poll_map_());
            }
        }

        void link::release(void) {
            ref_.attach_to(map_());
        }

        void  link::stop_poll(void) {
            poll_ref_.dettach();
        }

        void link::frontend_poll(void) {
            for (auto it = frontend_poll_map_().first(); it; it = it->next()) {
                it->owner().poll();
            }
        }
        void link::backend_poll(void) {
            for (auto it = backend_poll_map_().first(); it; it = it->next()) {
                it->owner().poll();
            }
        }

        port::port() : reader_(nullptr) {};

        port::~port(void) {}

        bool port::listen(cstr _alias, reader * _reader ){
            if(_reader) {
                port* u = query<port>(_alias);
                ROBO_LBREAKN(u!=nullptr);
                u->reader_ = _reader;
                return true;
            } else{
                ROBO_LBREAK();
            }
        };
        bool port::listen( reader *  _reader ){
            if(_reader) {
                reader_ = _reader;
                return true;
            } else{
                ROBO_LBREAK();
            }
        }


        void port::listen(reader &  _reader ){
            reader_ = &_reader;
        }


        bool port::post(const uint8_t * _data, size_t _len){
            //todo статистику сюда
            return do_post(_data, _len);
        }
        void port::receive(const uint8_t * _data, size_t _len){
            //todo статистику сюда
            if(reader_) (*reader_)(_data, _len);
        }

    }
}
