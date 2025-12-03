#include "DJControllerService.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>

DJControllerService::DJControllerService(size_t cache_size)
    : cache(cache_size) {}
/**
 * TODO: Implement loadTrackToCache method
 */
int DJControllerService::loadTrackToCache(AudioTrack& track) {
    const std::string title = track.get_title();

    //HIT: refresh status and return 1

    if (cache.contains(title)) {
        cache.get(title);
        return 1;
    }

    // MISS: clone and prepare track before caching. return -1 on eviction and 0 without eviction
    
    PointerWrapper<AudioTrack> cloned = track.clone();
    if (!cloned) {
        std::cerr << "[ERROR] Track: \"" << title << "\" failed to clone\n";
        return 0;
    }

    AudioTrack* raw = cloned.get();
    raw->load();
    raw->analyze_beatgrid();

    bool evicted = cache.put(std::move(cloned));
    return evicted ? -1 : 0;
}

void DJControllerService::set_cache_size(size_t new_size) {
    cache.set_capacity(new_size);
}
//implemented
void DJControllerService::displayCacheStatus() const {
    std::cout << "\n=== Cache Status ===\n";
    cache.displayStatus();
    std::cout << "====================\n";
}

/**
 * TODO: Implement getTrackFromCache method
 */
AudioTrack* DJControllerService::getTrackFromCache(const std::string& track_title) {
    return cache.get(track_title);
}
