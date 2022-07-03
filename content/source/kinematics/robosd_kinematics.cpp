#include "kinematics/robosd_kinematics.hpp"
namespace robo {
	namespace kinematiks {
		namespace point {
			void convert(const absolute::span& _src, hamilton& _dst, op _op) {
				_dst.position.x = _src[0];
				_dst.position.y = _src[1];
				_dst.position.z = _src[2];
				axis::avionic tmp(_src[3],_src[4],_src[5]);
				_dst.orient.from(tmp);
			}

			void convert(const hamilton::span& _src, absolute& _dst, op _op) {
			}
			void convert(const cilinder::span& _src, hamilton& _dst, op _op) {
			}
			void convert(const hamilton::span& _src, cilinder& _dst, op _op) {
			}
			void convert(const sphere::span& _src, hamilton& _dst, op _op) {
			}
			void convert(const hamilton::span& _src, sphere& _dst, op _op){
			}
		}
		namespace joint {
			actuator::actuator(int _index, series& _series) : ref_(*this, _index) {
				ref_.attach_to(_series.actuators_);
				local.orient.w=1;
				supply.orient.w = 1;
				base.orient.w = 1;
			}
		}
	}
}