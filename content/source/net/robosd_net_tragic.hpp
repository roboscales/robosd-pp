	struct ROBO_EXPORT itrafic{
		struct agent{
			robo_size_t prev;
			robo_size_t delta;
			robo_size_t total;
			float rate;
			void inc(robo_size_t _sz);
			void reset(void);
			void tick1sec(void);
		};

		struct ROBO_EXPORT counter{
			agent bytes;
			agent packets;
			float load;
			void inc(robo_size_t _sz);
			void reset(void);
			void tick1sec(void);
		};

		struct ROBO_EXPORT counters{
			counter success;
			counter refuse;
			void reset(void);
			void tick1sec(void);
		};

		counters incom;
		counters outcom;

		void tick1sec(void);
		void reset(void);
		itrafic(void);
	};
