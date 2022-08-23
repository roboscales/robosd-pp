void burst_inp_begin(burst_inp_p _inp, robo::time_us_t _period_us){
	_inp->period_us = _period_us;
	if( _inp->get() ){
		_inp->state =  burst_inp_set;
	} else {
		_inp->state =  burst_inp_reset;
	}
}
void raise(burst_inp_p _inp){
	burst_time_us_t us = burst_time_us();
	if( us - _inp->last_us >= _inp->period_us ){
		_inp->on_raise();
		_inp->last_us = us;
	}
}		
void poll(burst_inp_p _inp){
	if( _inp->get() ){
		if(_inp->state ==  burst_inp_reset){
			_inp->state_ =  burst_inp_set;
			raise(_inp);
			_inp->pushdown_us = burst_time_us();
		}else{
			_inp->pressed_us = burst_time_us() - _inp->pushdown_us;
		}
	} else {
		if(_inp->state ==  burst_inp_set){
			_inp->state =  burst_inp_reset;
			_inp->pressed_us = _inp->pushdown_us = 0;
		}
	}
}

template <typename D> class pinin : public D{
		public:
		typedef robo::delegat::base<void> delegat;
		private:
		delegat * on_raise_ = nullptr;
		enum class state{ set =1,reset = 0} state_ = state::reset;
		robo::time_us_t last_us_ = 0;
		robo::time_us_t period_us_ = 0;
		robo::time_us_t pressed_us_ = 0;
		robo::time_us_t pushdown_us_ = 0;
		public:
		void attach(delegat * _on_raise ){
			on_raise_ = _on_raise;
		}
		void poll(void){
			if( D::get() ){
				if(state_ ==  state::reset){
					state_ =  state::set;
					raise();
					pushdown_us_ = ::robo::system::env::time_us();
				}else{
					pressed_us_=::robo::system::env::time_us()-pushdown_us_;
				}
			} else {
				if(state_ ==  state::set){
					state_ =  state::reset;
					pressed_us_ = pushdown_us_ = 0;
				}
			}
		}
		bool get(void){
			return D::get();
		}
		robo::time_us_t pressed_us(){ return pressed_us_;}
	};