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
	}

	#if ROBO_APP_BURST_VARTREE_ENABLED
	void actor::regvar_present(robo::cstr _name) {
		using namespace burst::var;
		push(_name);
		do_regvar_present();
		pop();
	}
	void actor::regvar_conf(robo::cstr _name) {
		using namespace burst::var;
		push(_name);
		do_regvar_conf();
		pop();
	}
	#endif

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
			#if 0	
			#if ROBO_APP_BURST_PARANOIC_ENABLE
			ROBO_APP_ASSERT( a->owner().attached() );
			#endif
			#endif
			a->owner().begin();
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

	void subsystem::run(void) {
		for (actor::ref* a = actors_.first(); a; a = a->next()) {
			a->owner().run();
		}
		for (subsystem::ref* a = childs_.first(); a; a = a->next()) {
			a->owner().run();
		}
	}

}
