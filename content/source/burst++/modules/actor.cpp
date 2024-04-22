#include "burst++\modules\actor.hpp"
namespace burst{
	
	actor::actor(const config_s& _config, present_s& _present)
		: ref_(*this)
		, config_(_config)
		, present_(_present) {
	}
	actor::actor(const config_s& _config, present_s& _present, subsystem& _subsystem) 
		: ref_(*this)
		, config_(_config)
		, present_(_present) {
		add(_subsystem);
	}
	void actor::add(subsystem& _subsystem) {
		ref_.attach_to(_subsystem.actors_);
	}
	void actor::remove(void) {
		ref_.dettach();
		delegat::dettach();
	}

	subsystem::subsystem(void): ref_(*this) {
	}
	subsystem::subsystem(subsystem& _subsystem)
		: ref_(*this) {
		add(_subsystem);
	}
	void subsystem::add(subsystem& _subsystem) {
		ref_.attach_to(_subsystem.childs_);
	}
	
	void  subsystem::remove(void) {
		finish();
		ref_.dettach();
	}

	void subsystem::begin(void) {
		for (actor::ref* a = actors_.first(); a; a = a->next()) {
			#if ROBO_APP_BURST_PARANOIC_ENABLE
			ROBO_APP_ASSERT( a->owner().attached() );
			#endif
			a->owner().reset();
		}
		for (subsystem::ref* a = childs_.first(); a; a = a->next()) {
			a->owner().begin();
		}
	}

	void subsystem::finish(void) {
		for (actor::ref* a = actors_.first(); a; a = a->next()) {
			a->owner().finish();
		}
		for (subsystem::ref* a = childs_.first(); a; a = a->next()) {
			a->owner().finish();
		}
	}

}
