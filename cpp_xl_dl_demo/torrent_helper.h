#ifndef TORRENT_HELPER_H
#define TORRENT_HELPER_H

#include <string>
#include <cstdint>
#include <memory>
#include <functional>

// Forward declarations
namespace libtorrent {
    struct session;
    struct torrent_handle;
}

// Torrent download progress information
struct TorrentProgress {
    uint64_t total_size = 0;
    uint64_t downloaded_size = 0;
    uint64_t download_rate = 0; // bytes per second
    uint64_t upload_rate = 0;   // bytes per second
    int num_peers = 0;
    int num_seeds = 0;
    float progress = 0.0f; // 0.0 to 1.0
    std::string state_str;
    bool is_finished = false;
    bool has_error = false;
    std::string error_message;
};

// Callback function type for progress updates
using TorrentProgressCallback = std::function<void(const TorrentProgress&)>;

class TorrentDownloader {
public:
    TorrentDownloader();
    ~TorrentDownloader();

    // Initialize the torrent session
    bool Initialize(const std::string& save_path);
    
    // Check if a URL is a magnet link
    static bool IsMagnetLink(const std::string& url);
    
    // Start downloading a magnet link
    bool StartDownload(const std::string& magnet_url, const std::string& save_name = "");
    
    // Resume a previous download by info hash
    bool ResumeDownload(const std::string& info_hash_hex);
    
    // Save torrent resume data for interruption/resumption
    bool SaveResumeData(const std::string& resume_file);
    
    // Load torrent resume data
    bool LoadResumeData(const std::string& resume_file);
    
    // Get current download progress
    TorrentProgress GetProgress() const;
    
    // Set progress callback for real-time updates
    void SetProgressCallback(TorrentProgressCallback callback);
    
    // Stop the current download
    void StopDownload();
    
    // Pause/Resume the download
    void PauseDownload();
    void ResumeDownload();
    
    // Check if download is active
    bool IsDownloadActive() const;
    
    // Get info hash of current torrent (for resumption)
    std::string GetInfoHashHex() const;
    
    // Cleanup and shutdown
    void Shutdown();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // TORRENT_HELPER_H