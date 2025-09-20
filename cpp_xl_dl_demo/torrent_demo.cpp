#include "torrent_helper.h"
#include "download_helper.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Torrent Download Demo" << std::endl;
    
    // Test magnet link detection
    std::string test_url1 = "https://example.com/file.zip";
    std::string test_url2 = "magnet:?xt=urn:btih:abc123&dn=test";
    
    std::cout << "Testing magnet link detection:" << std::endl;
    std::cout << "URL 1: " << test_url1 << " -> " << (is_magnet_link(test_url1) ? "MAGNET" : "HTTP") << std::endl;
    std::cout << "URL 2: " << test_url2 << " -> " << (is_magnet_link(test_url2) ? "MAGNET" : "HTTP") << std::endl;
    
    // Test torrent downloader initialization
    TorrentDownloader downloader;
    std::string save_path = "./downloads";
    
    std::cout << "\nTesting torrent downloader initialization..." << std::endl;
    if (downloader.Initialize(save_path)) {
        std::cout << "✓ Torrent downloader initialized successfully" << std::endl;
    } else {
        std::cout << "✗ Failed to initialize torrent downloader" << std::endl;
        return 1;
    }
    
    // Test starting a magnet download (with a test magnet link)
    std::string test_magnet = "magnet:?xt=urn:btih:c12fe1c06bba254a9dc9f519b335aa7c1367a88a";
    std::cout << "\nTesting magnet link download start..." << std::endl;
    if (downloader.StartDownload(test_magnet, "test_download")) {
        std::cout << "✓ Magnet download started successfully" << std::endl;
        
        // Monitor progress for a few seconds
        std::cout << "Monitoring progress for 10 seconds..." << std::endl;
        for (int i = 0; i < 10; i++) {
            TorrentProgress progress = downloader.GetProgress();
            std::cout << "Progress: " << (progress.progress * 100.0f) << "%, " 
                      << "Peers: " << progress.num_peers << ", "
                      << "Seeds: " << progress.num_seeds << ", "
                      << "State: " << progress.state_str << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else {
        std::cout << "✗ Failed to start magnet download" << std::endl;
    }
    
    downloader.Shutdown();
    std::cout << "\nDemo completed." << std::endl;
    return 0;
}