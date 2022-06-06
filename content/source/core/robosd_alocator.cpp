#include "core/robosd_system.hpp"
void* operator new(size_t size) {
	void* tmp;
	#if ROBO_APP_ALLOC_ENABLED == 1
	tmp = robo::system::mem::alloc(size);
	#else
	tmp = malloc(size);
	#endif
	ROBO_APP_ASSERT(tmp != nullptr)
	return tmp;
}

void operator delete(void* ptr) {
	#if ROBO_APP_ALLOC_ENABLED == 1
	robo::system::mem::free(ptr);
	#else
	free(ptr);
	#endif
}