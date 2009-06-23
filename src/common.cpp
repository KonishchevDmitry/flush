#include <memory>
#include <string>
#include <limits>

#if M_BOOST_GET_VERSION() >= M_GET_VERSION(1, 35, 0)
	#include <boost/exception.hpp>
#endif

#include <glibmm/convert.h>
#include <glibmm/regex.h>

#include <gtkmm/stock.h>

#include <libtorrent/alert.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/peer_id.hpp>
#include <libtorrent/session_status.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>

#include "daemon_types.hpp"



// Auto_clean_type -->
	Auto_clean_type::Auto_clean_type(void)
	:
		type(NONE)
	{
	}



	Auto_clean_type::Auto_clean_type(Type type)
	:
		type(type)
	{
	}



	Auto_clean_type Auto_clean_type::from_string(const std::string& string) throw(m::Exception)
	{
		if(string == "none")
			return NONE;
		else if(string == "pause")
			return PAUSE;
		else if(string == "remove")
			return REMOVE;
		else if(string == "remove_with_data")
			return REMOVE_WITH_DATA;
		else
			M_THROW(__("invalid auto clean action type '%1'", string));
	}



	void Auto_clean_type::set_if_stricter(const Auto_clean_type& type)
	{
		if(this->type < type.type)
			*this = type;
	}



	Gtk::StockID Auto_clean_type::to_stock_id(void) const
	{
		switch(this->type)
		{
			case NONE:
				return Gtk::Stock::ADD;
				break;

			case PAUSE:
				return Gtk::Stock::MEDIA_PAUSE;
				break;

			case REMOVE:
				return Gtk::Stock::REMOVE;
				break;

			case REMOVE_WITH_DATA:
				return Gtk::Stock::DELETE;
				break;

			default:
				MLIB_LE();
				break;
		}
	}



	std::string Auto_clean_type::to_string(void) const
	{
		switch(this->type)
		{
			case NONE:
				return "none";
				break;

			case PAUSE:
				return "pause";
				break;

			case REMOVE:
				return "remove";
				break;

			case REMOVE_WITH_DATA:
				return "remove_with_data";
				break;

			default:
				MLIB_LE();
				break;
		}
	}



	Auto_clean_type& Auto_clean_type::operator++(void)
	{
		if(this->type == REMOVE_WITH_DATA)
			this->type = NONE;
		else
			this->type = static_cast<Type>( static_cast<int>(this->type) + 1 );

		return *this;
	}



	Auto_clean_type::operator bool(void) const
	{
		return this->type != NONE;
	}
// Auto_clean_type <--



// Daemon_message -->
	Daemon_message::Daemon_message(const lt::alert& alert)
	{
		this->set(alert);
	}



	void Daemon_message::set(const lt::alert& alert)
	{
		// Тип -->
			if(typeid(alert) == typeid(lt::file_error_alert))
				this->type = WARNING;
			else
				this->type = INFO;
		// Тип <--

		this->message = alert.message() + ".";
	}
// Daemon_message <--



// Download_settings_light -->
	Download_settings_light::Download_settings_light(void)
	{
	}



	Download_settings_light::Download_settings_light(bool copy_when_finished, const std::string& copy_when_finished_to)
	:
		copy_when_finished(copy_when_finished),
		copy_when_finished_to(copy_when_finished_to)
	{
	}
// Download_settings_light <--



// Download_settings -->
	Download_settings::Download_settings(void)
	{
	}



	Download_settings::Download_settings(const std::string& copy_when_finished_to)
	:
		Download_settings_light(copy_when_finished_to != "", copy_when_finished_to)
	{
	}



	Download_settings::Download_settings(const Download_settings_light& light_settings, bool sequential_download)
	{
		*static_cast<Download_settings_light*>(this) = light_settings;
		this->sequential_download = sequential_download;
	}
// Download_settings <--



// Ip_filter_rule -->
	Ip_filter_rule::Ip_filter_rule(void)
	:
		block(false)
	{
	}



	Ip_filter_rule::Ip_filter_rule(const std::string& from, const std::string& to, bool block)
	:
		from(from), to(to), block(block)
	{
	}



	void Ip_filter_rule::check(void) const throw(m::Exception)
	{
		std::string ip;
		lt::address_v4 from_address;
		lt::address_v4 to_address;

		try
		{
			ip = this->from;
			from_address = lt::address_v4::from_string(ip);

			ip = this->to;
			to_address = lt::address_v4::from_string(ip);
		}
	#if M_BOOST_GET_VERSION() >= M_GET_VERSION(1, 35, 0)
		catch(boost::exception&)
	#else
		catch(asio::system_error&)
	#endif
		{
			M_THROW(__("invalid IP address '%1'", ip));
		}

		if(from_address > to_address)
			M_THROW(__("invalid IP range %1-%2", this->from, this->to));
	}



	bool Ip_filter_rule::operator==(const Ip_filter_rule& rule) const
	{
		return
			this->from	== rule.from	&&
			this->to	== rule.to		&&
			this->block	== rule.block;
	}



	bool Ip_filter_rule::operator!=(const Ip_filter_rule& rule) const
	{
		return !(*this == rule);
	}
// Ip_filter_rule <--



// Session_status -->
	Session_status::Session_status(
		const Daemon_statistics& daemon_statistics,
		const lt::session_status& libtorrent_session_status,
		Speed download_rate_limit, Speed upload_rate_limit,
		bool temporary_action_active
	)
	:
		session_start_time(daemon_statistics.session_start_time),
		statistics_start_time(daemon_statistics.statistics_start_time),

		download_rate_limit(download_rate_limit),
		upload_rate_limit(upload_rate_limit),

		download_speed(libtorrent_session_status.download_rate),
		payload_download_speed(libtorrent_session_status.payload_download_rate),

		upload_speed(libtorrent_session_status.upload_rate),
		payload_upload_speed(libtorrent_session_status.payload_upload_rate),

		download(libtorrent_session_status.total_download + daemon_statistics.download),
		payload_download(libtorrent_session_status.total_payload_download + daemon_statistics.payload_download),

		total_download(daemon_statistics.total_download + this->download),
		total_payload_download(daemon_statistics.total_payload_download + this->payload_download),

		upload(libtorrent_session_status.total_upload + daemon_statistics.upload),
		payload_upload(libtorrent_session_status.total_payload_upload + daemon_statistics.payload_upload),

		total_upload(daemon_statistics.total_upload + this->upload),
		total_payload_upload(daemon_statistics.total_payload_upload + this->payload_upload),

		failed(libtorrent_session_status.total_failed_bytes + daemon_statistics.failed),
		total_failed(daemon_statistics.total_failed + this->failed),

		redundant(libtorrent_session_status.total_redundant_bytes + daemon_statistics.redundant),
		total_redundant(daemon_statistics.total_redundant + this->redundant),

		temporary_action_active(temporary_action_active)
	{
	}
// Session_status <--



// Torrent_file_settings -->
	int Torrent_file_settings::convert_to_libtorrent_prioritry(void)
	{
		if(this->download)
		{
			if(this->priority == HIGH)
				return 7;
			else
				return 1;
		}
		else
			return 0;
	}



	std::string Torrent_file_settings::get_priority_localized_name(Priority priority)
	{
		switch(priority)
		{
			case NORMAL:
				return _("normal");
				break;

			case HIGH:
				return _("high");
				break;

			default:
				MLIB_LE();
		}
	}



	std::string Torrent_file_settings::get_priority_name(Priority priority)
	{
		switch(priority)
		{
			case NORMAL:
				return "normal";
				break;

			case HIGH:
				return "high";
				break;

			default:
				MLIB_LE();
		}
	}



	Torrent_file_settings::Priority Torrent_file_settings::get_priority_by_name(const std::string& name) throw(m::Exception)
	{
		if(name == "normal")
			return NORMAL;
		else if(name == "high")
			return HIGH;
		else
			M_THROW(__("Invalid file priority name: '%1'.", name));
	}
// Torrent_file_settings <--



// Torrent_id -->
	Torrent_id::Torrent_id(const lt::sha1_hash& hash)
	{
		*this = hash;
	}



	Torrent_id::Torrent_id(const lt::torrent_handle& handle)
	{
		*this = handle.info_hash();
	}



	lt::sha1_hash Torrent_id::hash(void) const
	{
		unsigned char byte;
		std::string hash_string;

		for(size_t i = 0; i < this->size(); i += 2)
		{
			sscanf(this->substr(i, 2).c_str(), "%hhx", &byte);
			hash_string.append(1, byte);
		}

		return lt::sha1_hash(hash_string);
	}



	bool Torrent_id::is_invalid(void) const
	{
		return static_cast<std::string>(*this) == "";
	}



	Torrent_id& Torrent_id::operator=(const lt::sha1_hash& hash)
	{
		char buf[3];
		const unsigned char* cur_char = hash.begin();
		const unsigned char* end_char = hash.end();

		while(cur_char != end_char)
		{
			MLIB_A(snprintf(buf, sizeof buf, "%02x", *cur_char++) == 2);
			this->append(buf);
		}

		return *this;
	}



	bool Torrent_id::operator==(const Torrent_id& torrent_id) const
	{
		return static_cast<std::string>(*this) == static_cast<std::string>(torrent_id);
	}



	bool Torrent_id::operator!=(const Torrent_id& torrent_id) const
	{
		return !(*this == torrent_id);
	}



	Torrent_id::operator bool(void) const
	{
		return !this->is_invalid();
	}



	std::wostream& operator<<(std::wostream& stream, const Torrent_id& torrent_id)
	{
		return stream << torrent_id.c_str();
	}
// Torrent_id <--



// Torrent_peer_info -->
	Torrent_peer_info::Torrent_peer_info(const lt::peer_info& peer_info)
	{
		this->ip                     = peer_info.ip.address().to_string();
		this->client                 = peer_info.client;
		this->download_speed         = peer_info.down_speed;
		this->payload_download_speed = peer_info.payload_down_speed;
		this->upload_speed           = peer_info.up_speed;
		this->payload_upload_speed   = peer_info.payload_up_speed;
		this->total_payload_download = peer_info.total_download;
		this->total_payload_upload   = peer_info.total_upload;
		this->hash_fails             = peer_info.num_hashfails;

		if(peer_info.pieces.size())
			this->availability       = peer_info.pieces.count() * 100 / peer_info.pieces.size();
		else
			this->availability       = 100;

		// uTorrent зачастую отдает имя клиента в кодировке cp1251,
		// поэтому приходится каждый раз проверять и преобразовывать.
		// -->
			if(!m::is_valid_utf(this->client))
			{
				try
				{
					this->client = Glib::convert_with_fallback(this->client, "UTF-8", "cp1251");
				}
				catch(Glib::ConvertError)
				{
					this->client = _("[Invalid encoding]");
				}
			}
		// <--
	}
// Torrent_peer_info <--



// Torrent_info -->
	Torrent_info::Torrent_info(const Torrent& torrent)
	:
		processed(false)
	{
		try
		{
			const lt::torrent_status torrent_status = torrent.handle.status();
			this->init(torrent, torrent_status);
		}
		catch(lt::invalid_handle)
		{
			MLIB_LE();
		}
	}



	Torrent_info::Torrent_info(void)
	:
		processed(false)
	{
	}



	Torrent_info::Status Torrent_info::get_status(const lt::torrent_status& torrent_status) const
	{
		switch(torrent_status.state)
		{
			case lt::torrent_status::queued_for_checking:
				return QUEUED_FOR_CHECKING;
				break;

			case lt::torrent_status::checking_files:
				return CHECKING_FILES;
				break;

			case lt::torrent_status::downloading_metadata:
			{
				if(torrent_status.download_payload_rate)
					return DOWNLOADING_METADATA;
				else
					return WAITING_FOR_METADATA_DOWNLOAD;
			}
			break;

			case lt::torrent_status::downloading:
			{
				if(torrent_status.download_payload_rate)
					return DOWNLOADING;
				else
					return WAITING_FOR_DOWNLOAD;
			}
			break;

			case lt::torrent_status::finished:
			{
				// Насколько я понял, finished - это когда скачаны
				// все запрошенные файлы торрента, но в торренте есть
				// еще файлы, которые пользователь не пожелал скачивать.
				// В терминологии данного торрент клиента этот статус
				// приравнивается к статусу seeding.

				if(torrent_status.upload_payload_rate)
					return UPLOADING;
				else
					return SEEDING;
			}
			break;

			case lt::torrent_status::seeding:
			{
				if(torrent_status.upload_payload_rate)
					return UPLOADING;
				else
					return SEEDING;
			}
			break;

			case lt::torrent_status::allocating:
				return ALLOCATING;
				break;

			default:
				return UNKNOWN;
				break;
		}
	}



	int Torrent_info::get_complete_percent(void) const
	{
		if(this->requested_size)
			return this->downloaded_requested_size * 100 / this->requested_size;
		else
			return 100;
	}



	Share_ratio Torrent_info::get_share_ratio(void) const
	{
		return ::get_share_ratio(
			this->total_payload_upload, this->total_payload_download, this->size
		);
	}



	Torrent_info::Status_icon_id Torrent_info::get_status_icon_id(void) const
	{
		if(this->paused)
		{
			if(this->tracker_brocken)
				return TORRENT_STATUS_ICON_PAUSED_BROCKEN_TRACKER;
			else
				return TORRENT_STATUS_ICON_PAUSED;
		}
		else
		{
			switch(this->status)
			{
				case Torrent_info::ALLOCATING:
				{
					if(this->tracker_brocken)
						return TORRENT_STATUS_ICON_ALLOCATING_BROCKEN_TRACKER;
					else
						return TORRENT_STATUS_ICON_ALLOCATING;
				}
				break;

				case Torrent_info::QUEUED_FOR_CHECKING:
				case Torrent_info::CHECKING_FILES:
				{
					if(this->tracker_brocken)
						return TORRENT_STATUS_ICON_CHECKING_BROCKEN_TRACKER;
					else
						return TORRENT_STATUS_ICON_CHECKING;
				}
				break;

				case Torrent_info::WAITING_FOR_METADATA_DOWNLOAD:
				case Torrent_info::WAITING_FOR_DOWNLOAD:
				{
					if(this->tracker_brocken)
						return TORRENT_STATUS_ICON_WAITING_FOR_DOWNLOAD_BROCKEN_TRACKER;
					else
						return TORRENT_STATUS_ICON_WAITING_FOR_DOWNLOAD;
				}
				break;

				case Torrent_info::DOWNLOADING_METADATA:
				case Torrent_info::DOWNLOADING:
				{
					if(this->tracker_brocken)
						return TORRENT_STATUS_ICON_DOWNLOADING_BROCKEN_TRACKER;
					else
						return TORRENT_STATUS_ICON_DOWNLOADING;
				}
				break;

				case Torrent_info::SEEDING:
				{
					if(this->tracker_brocken)
						return TORRENT_STATUS_ICON_SEEDING_BROCKEN_TRACKER;
					else
						return TORRENT_STATUS_ICON_SEEDING;
				}
				break;

				case Torrent_info::UPLOADING:
				{
					if(this->tracker_brocken)
						return TORRENT_STATUS_ICON_UPLOADING_BROCKEN_TRACKER;
					else
						return TORRENT_STATUS_ICON_UPLOADING;
				}
				break;

				case Torrent_info::UNKNOWN:
				{
					if(this->tracker_brocken)
						return TORRENT_STATUS_ICON_UNKNOWN_BROCKEN_TRACKER;
					else
						return TORRENT_STATUS_ICON_UNKNOWN;
				}
				break;

				default:
					MLIB_LE();
					break;
			}
		}
	}



	std::string Torrent_info::get_status_string(void) const
	{
		#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
			// Похоже, что, как минимум, начиная с версии 0.14.3, уже
			// не актуально. По крайней мере в 0.14.3 при преостановке
			// торрента он переходит в состояние QUEUED_FOR_CHECKING.
			//
			// В прошлых версиях (точно не помню каких именно) после приостановки
			// торрента он не останавливался, а продолжал проверяться.
			if(this->paused && this->status != ALLOCATING && this->status != CHECKING_FILES)
				return _Q("(short)|Paused");
		#else
			if(this->paused)
				return _Q("(short)|Paused");
		#endif

		std::string status_string;

		switch(status)
		{
			case ALLOCATING:
				status_string = _Q("(short)|Allocating");
				break;

			case QUEUED_FOR_CHECKING:
				status_string = _Q("(short)|Queued for checking");
				break;

			case CHECKING_FILES:
				status_string = _Q("(short)|Checking files");
				break;

			case WAITING_FOR_METADATA_DOWNLOAD:
				status_string = _Q("Waiting for metadata download (short)|Metadata down wait");
				break;

			case DOWNLOADING_METADATA:
				status_string = _Q("Downloading metadata (short)|Metadata down");
				break;

			case WAITING_FOR_DOWNLOAD:
				status_string = _Q("Waiting for download (short)|Download wait");
				break;

			case DOWNLOADING:
				status_string = _Q("(short)|Downloading");
				break;

			case SEEDING:
				status_string = _Q("(short)|Seeding");
				break;

			case UPLOADING:
				status_string = _Q("(short)|Uploading");
				break;

			case UNKNOWN:
				status_string = _Q("(short)|Unknown");
				break;

			default:
				MLIB_LE();
				break;
		}

	#if M_LT_GET_VERSION() < M_GET_VERSION(0, 14, 3)
		if(this->paused)
			return std::string(_Q("(short)|Paused")) + " (" + status_string + ")";
		else
	#endif
			return status_string;
	}



	Time Torrent_info::get_time_left(void) const
	{
		if(this->paused || !this->payload_download_speed)
			return 0;
		else
			return (this->requested_size - this->downloaded_requested_size) / this->payload_download_speed;
	}



	void Torrent_info::init(const Torrent& torrent, const lt::torrent_status& torrent_status)
	{
		this->id = torrent.id;
		this->name = torrent.name;
		this->paused = torrent_status.paused;
		this->status = this->get_status(torrent_status);

		try
		{
			this->size = torrent.handle.get_torrent_info().total_size();
		}
		catch(lt::invalid_handle&)
		{
			MLIB_LE();
		}

		this->requested_size = torrent_status.total_wanted;
		this->downloaded_requested_size = torrent_status.total_wanted_done;

		// progress -->
			if(this->paused)
			{
				// Похоже, что, как минимум, начиная с версии 0.14.3, уже
				// не актуально. По крайней мере в 0.14.3 при преостановке
				// торрента он переходит в состояние QUEUED_FOR_CHECKING.
				// Но все-же лучше пока оставить - лишним оно не будет.
				COMPATIBILITY
				if(this->status == ALLOCATING || this->status == CHECKING_FILES)
					this->progress = static_cast<int>( torrent_status.progress * 100 );
				else
				{
					if(this->requested_size != 0)
						this->progress = this->downloaded_requested_size * 100 / this->requested_size;
					else
						this->progress = 100;
				}
			}
			else
				this->progress = static_cast<int>( torrent_status.progress * 100 );
		// progress <--

		this->download_speed = static_cast<Speed>( torrent_status.download_rate );
		this->payload_download_speed = static_cast<Speed>( torrent_status.download_payload_rate );
		this->upload_speed = static_cast<Speed>( torrent_status.upload_rate );
		this->payload_upload_speed = static_cast<Speed>( torrent_status.upload_payload_rate );

		this->total_download = torrent.total_download + torrent_status.total_download;
		this->total_payload_download = torrent.total_payload_download + torrent_status.total_payload_download;
		this->total_upload = torrent.total_upload + torrent_status.total_upload;
		this->total_payload_upload = torrent.total_payload_upload + torrent_status.total_payload_upload;
		this->total_failed = torrent.total_failed + torrent_status.total_failed_bytes;
		this->total_redundant = torrent.total_redundant + torrent_status.total_redundant_bytes;

		this->peers_num = torrent_status.num_peers;
		this->seeds_num = torrent_status.num_seeds;

		this->time_added = torrent.time_added;
		this->time_seeding = torrent.time_seeding;

		try
		{
			this->trackers_exists = !torrent.handle.trackers().empty();
		}
		catch(lt::invalid_handle&)
		{
			MLIB_LE();
		}

		this->tracker_brocken =
			this->trackers_exists && torrent_status.current_tracker.empty() &&
			!torrent.tracker_error.empty();
		this->current_tracker = torrent_status.current_tracker;
	}



	bool Torrent_info::operator!=(const Torrent_info& torrent_info) const
	{
		const Torrent_info& i = torrent_info;

		if(this == &i)
			return false;

		return
			this->id						!=	i.id						||

			this->download_speed			!=	i.download_speed			||
			this->payload_download_speed	!=	i.payload_download_speed	||
			this->upload_speed				!=	i.upload_speed				||
			this->payload_upload_speed		!=	i.payload_upload_speed		||

			this->total_download			!=	i.total_download			||
			this->total_payload_download	!=	i.total_payload_download	||
			this->total_upload				!=	i.total_upload				||
			this->total_payload_upload		!=	i.total_payload_upload		||

			this->progress					!=	i.progress					||
			this->downloaded_requested_size	!=	i.downloaded_requested_size	||

			this->time_seeding				!=	i.time_seeding				||

			this->peers_num					!=	i.peers_num					||
			this->seeds_num					!=	i.seeds_num					||

			this->total_failed				!=	i.total_failed				||
			this->total_redundant			!=	i.total_redundant			||

			this->paused					!=	i.paused					||
			this->status					!=	i.status					||

			this->requested_size			!=	i.requested_size			||
			this->trackers_exists			!=	i.trackers_exists			||
			this->tracker_brocken			!=	i.tracker_brocken			||
			this->current_tracker			!=	i.current_tracker			||

			this->processed					!=	i.processed					||
			this->name						!=	i.name						||
			this->size						!=	i.size						||
			this->time_added				!=	i.time_added				||
		0;
	}
// Torrent_info <--



// Torrent_details -->
	Torrent_details::Torrent_details(const Torrent& torrent)
	:
		Torrent_info(),
		publisher_url(torrent.publisher_url)
	{
		try
		{
			const lt::torrent_status torrent_status = torrent.handle.status();

			Torrent_info::init(torrent, torrent_status);

			this->next_announce = torrent_status.next_announce.total_seconds();
			this->announce_interval = ( !this->paused ) * torrent_status.announce_interval.total_seconds();

			if(this->trackers_exists)
			{
				if(this->current_tracker.empty())
				{
					if(this->tracker_brocken)
						this->tracker_status = torrent.tracker_error;
					else
						this->tracker_status = _Q("Tracker status is not given yet|Not given yet");
				}
				else
					this->tracker_status = __("OK (%1)", this->current_tracker);
			}
			else
				this->tracker_status = _("There are no trackers has been setted for this torrent");
		}
		catch(lt::invalid_handle&)
		{
			MLIB_LE();
		}
	}
// Torrent_details <--



// New_torrent_settings -->
	New_torrent_settings::New_torrent_settings(
		bool start,
		const std::string& download_path,
		const std::string& copy_on_finished_path,
		const std::string& encoding,
		const std::vector<Torrent_file_settings> files_settings,
		bool duplicate_is_error
	)
	:
		start(start),
		download_path(download_path),
		copy_on_finished_path(copy_on_finished_path),
		encoding(encoding),
		files_settings(files_settings),
		duplicate_is_error(duplicate_is_error)
	{
	}



	New_torrent_settings::New_torrent_settings(
		const std::string& name,
		bool start,
		const std::string& download_path,
		const std::string& copy_on_finished_path,
		const std::string& encoding,
		const std::vector<Torrent_file_settings> files_settings,
		bool duplicate_is_error
	)
	:
		name(name),
		start(start),
		download_path(download_path),
		copy_on_finished_path(copy_on_finished_path),
		encoding(encoding),
		files_settings(files_settings),
		duplicate_is_error(duplicate_is_error)
	{
	}
// New_torrent_settings <--



Share_ratio get_share_ratio(Size upload, Size download)
{
	// TODO: при больших числах можно очень сильно потерять в точности.

	if(download)
		return Size_float(upload) / Size_float(download);
	else
		return std::numeric_limits<Share_ratio>::max();
}



Share_ratio get_share_ratio(Size upload, Size download, Size size)
{
	if(download > 0)
		return get_share_ratio(upload, download);
	else
		return get_share_ratio(upload, size);
}



std::string get_share_ratio_string(Share_ratio ratio, bool show_zero_values)
{
	// Просто ограничиваем в каких-то фиксированных пределах.
	// Имеет смысл хотя бы потому, что числа с плавующей точкой
	// могут принимать Inf и NaN значения.
	if(ratio < 0 || ratio > 1000)
		return "∞";

	if(!ratio && !show_zero_values)
		return "";

	return _F(std::fixed, std::setprecision(2), ratio);
}



std::string get_share_ratio_string(Size upload, Size download, Size size, bool show_zero_values)
{
	return get_share_ratio_string(get_share_ratio(upload, download, size), show_zero_values);
}



std::string get_tracker_name_by_url(const std::string& tracker_url)
{
	const size_t prefix_len = sizeof "http://" - 1;

	if(tracker_url.size() <= prefix_len || tracker_url.compare(0, prefix_len, "http://"))
		return tracker_url;

	size_t slash_pos = tracker_url.find('/', prefix_len);

	std::string tracker_name = tracker_url.substr(prefix_len,
		slash_pos == std::string::npos ? std::string::npos : slash_pos - prefix_len
	);

	String_vector domains = m::split(tracker_name, '.');

	switch(domains.size())
	{
		case 0:
			MLIB_LE();
			break;

		case 1:
		case 2:
			return tracker_name;
			break;

		case 3:
		{
			std::string& second = domains[1];

			if(second == "co" || second == "com" || second == "net" || second == "org")
				return tracker_name;
			else
				return tracker_name.substr(domains[0].size() + 1);
		}
		break;

		default:
			return tracker_name.substr(domains[0].size() + 1);
			break;
	}

	MLIB_LE();
}

