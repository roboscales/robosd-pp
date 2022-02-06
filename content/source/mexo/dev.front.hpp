#ifndef dev_front_hpp
#define dev_front_hpp
namespace mexo {
	namespace front {
		namespace dev {
			struct mode { enum { idle = 0 }; };
			struct action_s {
				bool actual;
				int mode;
			};
			struct feetback_s {
				bool fault;
				int mode;
			};
		}
	}
}
#endif