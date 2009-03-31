#ifdef MLIB_ENABLE_LIBCONFIG

	#include <libconfig.h++>

	#include "types.hpp"


	namespace m { namespace libconfig {
		const Setting::Type Size_type = ::libconfig::Setting::TypeInt64;
		const Setting::Type Speed_type = ::libconfig::Setting::TypeInt64;
		const Setting::Type Time_type = ::libconfig::Setting::TypeInt64;
		const Setting::Type Version_type = ::libconfig::Setting::TypeInt64;
	}}

	namespace m
	{
		const Version NO_VERSION = 0;
	}

#endif

