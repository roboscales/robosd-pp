#ifndef __robo_ipa_h
#define __robo_ipa_h
#include "core/robosd_common.hpp"
namespace robo{
#if ROBO_APP_INI_TYPE == ROBO_APP_TYPE_NATIVE
typedef enum { ROBO_IPA_NORMAL = 0, ROBO_IPA_DBL_BUFFER = 1} robo_ipa_mode_t;
bool robo_ipa_init(unsigned  int _flags);
bool robo_ipa_applay(const char *  _data, size_t _size);
void  robo_ipa_deinit(void);
void  robo_ipa_test(void);
size_t robo_ipa_string_get( cstr  _section, cstr _key, cstr _default, char_t * _dst, size_t _size);
//ROBO_EXPORT bool robo_ipa_ini_load_section(const robo_string_t _section, robo_ini_section_load_f _event, void * _instance);
#endif
}
#endif
