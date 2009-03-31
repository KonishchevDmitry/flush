#include <string>
#include <limits>

#include <glibmm/convert.h>

#include <libtorrent/alert.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/peer_id.hpp>
#include <libtorrent/session_status.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>

#include "daemon_types.hpp"



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



// Session_status -->
	Session_status::Session_status(const Daemon_statistics& daemon_statistics, const lt::session_status& libtorrent_session_status)
	{
		this->session_start_time = daemon_statistics.session_start_time;
		this->statistics_start_time = daemon_statistics.statistics_start_time;

		this->upload_speed = libtorrent_session_status.upload_rate;
		this->payload_upload_speed = libtorrent_session_status.payload_upload_rate;

		this->download_speed = libtorrent_session_status.download_rate;
		this->payload_download_speed = libtorrent_session_status.payload_download_rate;

		this->download = libtorrent_session_status.total_download + daemon_statistics.download;
		this->payload_download = libtorrent_session_status.total_payload_download + daemon_statistics.payload_download;

		this->total_download = daemon_statistics.total_download + this->download;
		this->total_payload_download = daemon_statistics.total_payload_download + this->payload_download;

		this->upload = libtorrent_session_status.total_upload + daemon_statistics.upload;
		this->payload_upload = libtorrent_session_status.total_payload_upload + daemon_statistics.payload_upload;

		this->total_upload = daemon_statistics.total_upload + this->upload;
		this->total_payload_upload = daemon_statistics.total_payload_upload + this->payload_upload;

		this->failed = libtorrent_session_status.total_failed_bytes + daemon_statistics.failed;
		this->total_failed = daemon_statistics.total_failed + this->failed;

		this->redundant = libtorrent_session_status.total_redundant_bytes + daemon_statistics.redundant;
		this->total_redundant = daemon_statistics.total_redundant + this->redundant;
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
	{
		try
		{
			const lt::torrent_info torrent_info = torrent.handle.get_torrent_info();
			const lt::torrent_status torrent_status = torrent.handle.status();

			this->id = torrent.id;
			this->name = torrent.name;
			this->paused = torrent_status.paused;
			this->status = this->get_status(torrent_status);

			this->size = torrent_info.total_size();
			this->requested_size = torrent_status.total_wanted;
			this->downloaded_requested_size = torrent_status.total_wanted_done;

			// progress -->
				if(this->paused)
				{
					if(this->status == allocating || this->status == checking_files)
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
		}
		catch(lt::invalid_handle)
		{
			MLIB_LE();
		}
		catch(lt::invalid_torrent_file)
		{
			MLIB_LE();
		}
	}



	Torrent_info::Torrent_status Torrent_info::get_status(const lt::torrent_status& torrent_status) const
	{
		switch(torrent_status.state)
		{
			case lt::torrent_status::queued_for_checking:
				return queued_for_checking;
				break;

			case lt::torrent_status::checking_files:
				return checking_files;
				break;

			case lt::torrent_status::downloading_metadata:
				return downloading_metadata;
				break;

			case lt::torrent_status::downloading:
				return downloading;
				break;

			case lt::torrent_status::finished:
				// Насколько я понял, finished - это когда скачаны
				// все запрошенные файлы торрента, но в торренте есть
				// еще файлы, которые пользователь не пожелал скачивать.
				// В терминологии данного торрент клиента этот статус
				// приравнивается к статусу seeding.
				return seeding;
				break;

			case lt::torrent_status::seeding:
				return seeding;
				break;

			case lt::torrent_status::allocating:
				return allocating;
				break;

			default:
				return unknown;
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



	std::string Torrent_info::get_status_string(void) const
	{
		if(this->paused && this->status != allocating && this->status != checking_files)
			return _("Paused");

		std::string status_string;

		switch(status)
		{
			case queued_for_checking:
				status_string = _("Queued for checking");
				break;

			case checking_files:
				status_string = _("Checking files");
				break;

			case downloading_metadata:
				status_string = _("Downloading metadata");
				break;

			case downloading:
				status_string = _("Downloading");
				break;

			case seeding:
				status_string = _("Seeding");
				break;

			case allocating:
				status_string = _("Allocating");
				break;

			case unknown:
				status_string = _("Unknown");
				break;

			default:
				MLIB_LE();
				break;
		}

		if(this->paused)
			return std::string(_("Paused")) + " (" + status_string + ")";
		else
			return status_string;
	}



	Time Torrent_info::get_time_left(void) const
	{
		if(this->paused || !this->payload_download_speed)
			return 0;
		else
			return (this->requested_size - this->downloaded_requested_size) / this->payload_download_speed;
	}
// Torrent_info <--



// Torrent_details -->
	Torrent_details::Torrent_details(const Torrent& torrent)
	:
		Torrent_info(torrent)
	{
	}
// Torrent_details <--



// New_torrent_settings -->
	New_torrent_settings::New_torrent_settings(
		bool start,
		const std::string& download_path,
		const std::string& copy_on_finished_path,
		const std::vector<Torrent_file_settings> files_settings
	)
	:
		start(start),
		download_path(download_path),
		copy_on_finished_path(copy_on_finished_path),
		files_settings(files_settings)
	{
	}



	New_torrent_settings::New_torrent_settings(
		const std::string& name,
		bool start,
		const std::string& download_path,
		const std::string& copy_on_finished_path,
		const std::vector<Torrent_file_settings> files_settings
	)
	:
		name(name),
		start(start),
		download_path(download_path),
		copy_on_finished_path(copy_on_finished_path),
		files_settings(files_settings)
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



std::string	get_share_ratio_string(Share_ratio ratio)
{
	// Просто ограничиваем в каких-то фиксированных пределах.
	// Имеет смысл хотя бы потому, что числа с плавующей точкой
	// могут принимать Inf и NaN значения.
	if(ratio < 0 || ratio > 1000)
		return "∞";

	return _F(std::fixed, std::setprecision(2), ratio);
}



std::string get_share_ratio_string(Size upload, Size download)
{
	return get_share_ratio_string(get_share_ratio(upload, download));
}

