#ifndef service_proto_hpp
#define service_proto_hpp
namespace burst{
	namespace proto{
		namespace flow{
			struct id {
				enum {					
					serial0=255
					, var=254
					, echo=253
				};
			};
			struct suba {
				enum {
					serial0= 0xF
					,var = 0xE
					,echo=0xD
				};
			};
			struct ans_suba {
				enum {
					serial0= 0xF
					, var = 0xE
					, echo =  0xD
				};
			};

			struct route_s { 
				int id; 
				int suba; 
				int ans_suba;
			};

		}
	}
}
#endif