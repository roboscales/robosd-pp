

		/*

		template< class P, typename ... Args > class ROBO_EXPORT repeat {
			P performer_;
			timer timer_;
		protected:
			repeat( Args ... args, bool _once = false)
				: performer_(args)
				, timer_(performer_, _once)
			{
			}
			void start(time_us_t _period) { timer_.start(_period); }
			void stop(void) { timer_.stop(); }
		};

		typedef repeat < void(*)(void)> repeat_simple;
		typedef repeat < lambda< void(void) >  > repeat_lambda;

		
		class ROBO_EXPORT repeat_tandem_lambda {
			signal::lambda frontend_lambda_;
			signal::lambda backend_lambda_;
			timer timer_;
		public:
			repeat_tandem_lambda(
				lambda< void(void) >  _frontend_lambda
				, lambda< void(void) >  _backend_lambda
				, bool _once = false
			)
				: frontend_lambda_(_frontend_lambda)
				, backend_lambda_(_backend_lambda)
				, timer_(frontend_lambda_, _once, &backend_lambda_) {}
			void start(time_us_t _period) { timer_.start(_period); }
			void stop(void) { timer_.stop(); }
		};

		template<typename F, typename B>  class ROBO_EXPORT repeat_tandem_members {
			signal::member<F> frontend_member_;
			signal::member<B> backend_member_;
			timer timer_;
		public:
			repeat_tandem_members(F& _fc, void (F::* _ff)(void), B& _bc, void (B::* _bf)(void), bool _once = false)
				: frontend_member_(_fc, _ff)
				, backend_member_(_bc, _bf)
				, timer_(frontend_member_, _once, &backend_member_) {}
			void start(time_us_t _period) { timer_.start(_period); }
			void stop(void) { timer_.stop(); }
		};

		template<typename F, typename B>  class ROBO_EXPORT tandem_members {
			signal::member<F> frontend_member_;
			signal::member<B> backend_member_;
			signal::member<tandem_members> execute_delegat_;
			void execute_(void);
		public:
			repeat_tandem_members(F& _fc, void (F::* _ff)(void), B& _bc, void (B::* _bf)(void), bool _once = false)
				: frontend_member_(_fc, _ff)
				, backend_member_(_bc, _bf)
				, execute_delegat_(this, &tandem_members::execute_)
			{}
			void start(time_us_t _period);
		};
		*/