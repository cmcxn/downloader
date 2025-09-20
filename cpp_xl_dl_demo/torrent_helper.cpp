#include "torrent_helper.h"
#include <libtorrent/session.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/session_stats.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/hex.hpp>
#include <libtorrent/span.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/bencode.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace libtorrent;

class TorrentDownloader::Impl {
public:
    std::unique_ptr<session> ses;
    torrent_handle handle;
    std::string save_path;
    TorrentProgressCallback progress_callback;
    bool is_active = false;
    std::thread alert_thread;
    bool should_stop = false;

    void AlertLoop() {
        while (!should_stop) {
            std::vector<alert*> alerts;
            ses->pop_alerts(&alerts);
            
            for (auto* a : alerts) {
                // Handle different alert types
                if (auto* st = alert_cast<state_update_alert>(a)) {
                    // Status update - could trigger callback
                    if (progress_callback && is_active) {
                        progress_callback(GetCurrentProgress());
                    }
                }
                else if (auto* ta = alert_cast<add_torrent_alert>(a)) {
                    std::cout << "Torrent added" << std::endl;
                }
                else if (auto* tf = alert_cast<torrent_finished_alert>(a)) {
                    std::cout << "Torrent finished: " << tf->torrent_name() << std::endl;
                    if (progress_callback) {
                        progress_callback(GetCurrentProgress());
                    }
                }
                else if (auto* te = alert_cast<torrent_error_alert>(a)) {
                    std::cout << "Torrent error: " << te->error.message() << std::endl;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    TorrentProgress GetCurrentProgress() const {
        TorrentProgress progress;
        
        if (!handle.is_valid()) {
            return progress;
        }

        try {
            torrent_status status = handle.status();
            
            progress.total_size = status.total_wanted;
            progress.downloaded_size = status.total_wanted_done;
            progress.download_rate = status.download_rate;
            progress.upload_rate = status.upload_rate;
            progress.num_peers = status.num_peers;
            progress.num_seeds = status.num_seeds;
            progress.progress = status.progress;
            progress.is_finished = status.is_finished;
            progress.has_error = status.errc.operator bool();
            
            if (progress.has_error) {
                progress.error_message = status.errc.message();
            }

            // Convert state to string
            switch (status.state) {
                case torrent_status::checking_files:
                    progress.state_str = "Checking files";
                    break;
                case torrent_status::downloading_metadata:
                    progress.state_str = "Downloading metadata";
                    break;
                case torrent_status::downloading:
                    progress.state_str = "Downloading";
                    break;
                case torrent_status::finished:
                    progress.state_str = "Finished";
                    break;
                case torrent_status::seeding:
                    progress.state_str = "Seeding";
                    break;
                case torrent_status::checking_resume_data:
                    progress.state_str = "Checking resume data";
                    break;
                default:
                    progress.state_str = "Unknown";
                    break;
            }
        }
        catch (const std::exception& e) {
            progress.has_error = true;
            progress.error_message = e.what();
        }

        return progress;
    }
};

TorrentDownloader::TorrentDownloader() : pImpl(std::make_unique<Impl>()) {
}

TorrentDownloader::~TorrentDownloader() {
    Shutdown();
}

bool TorrentDownloader::Initialize(const std::string& save_path) {
    try {
        pImpl->save_path = save_path;
        
        // Create session with default settings
        settings_pack settings;
        settings.set_str(settings_pack::listen_interfaces, "0.0.0.0:6881,[::]:6881");
        settings.set_bool(settings_pack::enable_upnp, true);
        settings.set_bool(settings_pack::enable_natpmp, true);
        settings.set_bool(settings_pack::enable_lsd, true);
        settings.set_bool(settings_pack::enable_dht, true);
        settings.set_str(settings_pack::dht_bootstrap_nodes, "router.bittorrent.com:6881,dht.transmissionbt.com:6881");
        
        pImpl->ses = std::make_unique<session>(settings);
        
        // Start alert thread
        pImpl->should_stop = false;
        pImpl->alert_thread = std::thread(&Impl::AlertLoop, pImpl.get());
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to initialize torrent session: " << e.what() << std::endl;
        return false;
    }
}

bool TorrentDownloader::IsMagnetLink(const std::string& url) {
    return url.length() > 8 && url.substr(0, 8) == "magnet:?";
}

bool TorrentDownloader::StartDownload(const std::string& magnet_url, const std::string& save_name) {
    if (!pImpl->ses) {
        return false;
    }

    try {
        add_torrent_params params;
        
        // Parse magnet URI
        error_code ec;
        parse_magnet_uri(magnet_url, params, ec);
        
        if (ec) {
            std::cerr << "Failed to parse magnet URI: " << ec.message() << std::endl;
            return false;
        }

        // Set save path
        params.save_path = pImpl->save_path;
        
        // Set name if provided
        if (!save_name.empty()) {
            params.name = save_name;
        }

        // Add the torrent
        pImpl->handle = pImpl->ses->add_torrent(params, ec);
        
        if (ec) {
            std::cerr << "Failed to add torrent: " << ec.message() << std::endl;
            return false;
        }

        pImpl->is_active = true;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in StartDownload: " << e.what() << std::endl;
        return false;
    }
}

bool TorrentDownloader::ResumeDownload(const std::string& info_hash_hex) {
    // For now, we'll implement this as starting a new download
    // In a production environment, this would load saved resume data
    // and restore the exact torrent state
    return false;
}

bool TorrentDownloader::SaveResumeData(const std::string& resume_file) {
    // Simplified implementation - just return true for now
    // In a production version, this would properly save resume data
    return true;
}

bool TorrentDownloader::LoadResumeData(const std::string& resume_file) {
    // Simplified implementation - just return false for now
    // In a production version, this would load and apply resume data
    return false;
}

TorrentProgress TorrentDownloader::GetProgress() const {
    return pImpl->GetCurrentProgress();
}

void TorrentDownloader::SetProgressCallback(TorrentProgressCallback callback) {
    pImpl->progress_callback = callback;
}

void TorrentDownloader::StopDownload() {
    if (pImpl->handle.is_valid()) {
        pImpl->ses->remove_torrent(pImpl->handle);
        pImpl->handle = torrent_handle();
        pImpl->is_active = false;
    }
}

void TorrentDownloader::PauseDownload() {
    if (pImpl->handle.is_valid()) {
        pImpl->handle.pause();
    }
}

void TorrentDownloader::ResumeDownload() {
    if (pImpl->handle.is_valid()) {
        pImpl->handle.resume();
    }
}

bool TorrentDownloader::IsDownloadActive() const {
    return pImpl->is_active && pImpl->handle.is_valid();
}

std::string TorrentDownloader::GetInfoHashHex() const {
    if (!pImpl->handle.is_valid()) {
        return "";
    }
    
    try {
        sha1_hash info_hash = pImpl->handle.info_hash();
        return aux::to_hex(span<char const>(info_hash.data(), info_hash.size()));
    }
    catch (const std::exception&) {
        return "";
    }
}

void TorrentDownloader::Shutdown() {
    if (pImpl->ses) {
        pImpl->should_stop = true;
        
        if (pImpl->alert_thread.joinable()) {
            pImpl->alert_thread.join();
        }
        
        StopDownload();
        pImpl->ses.reset();
    }
}