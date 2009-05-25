#ifndef HEADER_MLIB_TYPES
	#define HEADER_MLIB_TYPES

	#include <stdint.h>


	/// Этим символом будет помечаться весь код, который написан
	/// для совместимости с предыдущими версиями библиотек - просто, чтобы его
	/// было легко найти.
	#define M_LIBRARY_COMPATIBILITY

	// libconfig -->
		#ifdef MLIB_ENABLE_LIBCONFIG
			#include <libconfig.h++>

			namespace libconfig
			{
				class Setting;
			}

			namespace m
			{
				namespace libconfig
				{
					using namespace ::libconfig;

					// Т. к. libconfig писался без учета x86-64 систем -->
						extern const Setting::Type Size_type;
						extern const Setting::Type Speed_type;
						extern const Setting::Type Time_type;
						extern const Setting::Type Version_type;

						typedef long long	Size;
						typedef long long	Speed;
						typedef long long	Time;
						typedef long long	Version;
					// Т. к. libconfig писался без учета x86-64 систем <--
				}
			}
		#endif
	// libconfig <--

	// libtorrent -->
		#ifdef MLIB_ENABLE_LIBTORRENT
			namespace libtorrent
			{
				class alert;
				class big_number;
				class entry;
				class peer_info;
				class session_status;
				class torrent_alert;
				class torrent_handle;
				class torrent_info;
				class torrent_status;

				typedef big_number sha1_hash;
			}

			namespace m
			{
				namespace libtorrent
				{
					using namespace ::libtorrent;

					class Torrent_file;
				}

				namespace lt = ::m::libtorrent;
			}
		#endif
	// libtorrent <--

	// boost -->
		namespace boost
		{
			namespace filesystem {}
			namespace fs = filesystem;
		}

		namespace m
		{
			namespace boost
			{
				using namespace ::boost;
			}
		}
	// boost <--

	// glibmm -->
		namespace Glib
		{
			class Dispatcher;
		}
	// glibmm <--

	namespace m
	{
		typedef long long	Size;
		typedef long double	Size_float;
		typedef int32_t		Speed;

		/// К Time предъявляется следующее требование.
		/// Размер Time должен быть не меньше Size, чтобы
		/// вместить время в секундах, необходимое для того,
		/// чтобы скачать какой-либо файл. Т. е. для наихудшего
		/// случая, когда размер файла - максимальное значение
		/// Size, а скорость - 1 байт/сек.
		typedef long long	Time;

		/// Время в милисекундах.
		typedef long long	Time_ms;

		// Числовое представление версии приложения/библиотеки.
		// К примеру версия 1.12.4 должна записываться следующим
		// образом: 1012004.
		// Для извлечения из этого числа отдельных версий
		// (минорной, мажорной и т. п.) предназначены специальные
		// функции.
		typedef int32_t			Version;
		extern const Version	NO_VERSION;

		class Exception;

		namespace fs
		{
			class Path;
		}
	}

	#ifdef MLIB_ENABLE_ALIASES
		#ifdef MLIB_ENABLE_LIBTORRENT
			namespace lt = libtorrent;
		#endif

		namespace fs = boost::filesystem;

		using m::Size;
		using m::Size_float;
		using m::Speed;
		using m::Time;
		using m::Time_ms;
		using m::Version;
	#endif

#endif

