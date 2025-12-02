#include "DJLibraryService.h"
#include "SessionFileParser.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>
#include <filesystem>


DJLibraryService::DJLibraryService(const Playlist& playlist) 
    : playlist(playlist) {}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */
void DJLibraryService::buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks) {
    // Clean up any existing tracks
    for (auto* track : library) {
        delete track;
    }
    library.clear();

    std::cout << "[Library] Building track library with " << library_tracks.size() << " tracks.\n";
    for (const auto& info : library_tracks) {
        AudioTrack* track_ptr = nullptr;
        try {
            if (info.type == "MP3") {
                bool has_tags = info.extra_param2 != 0;
                track_ptr = new MP3Track(info.title, info.artists, info.duration_seconds,
                                         info.bpm, info.extra_param1, has_tags);
            } else if (info.type == "WAV") {
                track_ptr = new WAVTrack(info.title, info.artists, info.duration_seconds,
                                         info.bpm, info.extra_param1, info.extra_param2);
            } else {
                std::cout << "[WARNING] Unknown track type for '" << info.title << "'. Skipping.\n";
                continue;
            }
        } catch (const std::exception& e) {
            std::cout << "[ERROR] Failed to create track '" << info.title << "': " << e.what() << "\n";
            continue;
        }

        if (track_ptr) {
            library.push_back(track_ptr);
        }
    }
}

/**
 * @brief Display the current state of the DJ library playlist
 * 
 */
void DJLibraryService::displayLibrary() const {
    std::cout << "=== DJ Library Playlist: " 
              << playlist.get_name() << " ===" << std::endl;

    if (playlist.is_empty()) {
        std::cout << "[INFO] Playlist is empty.\n";
        return;
    }

    // Let Playlist handle printing all track info
    playlist.display();

    std::cout << "Total duration: " << playlist.get_total_duration() << " seconds" << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 * 
 * @return Playlist& 
 */
Playlist& DJLibraryService::getPlaylist() {
    // Your implementation here
    return playlist;
}

/**
 * TODO: Implement findTrack method
 * 
 * HINT: Leverage Playlist's find_track method
 */
AudioTrack* DJLibraryService::findTrack(const std::string& track_title) {
    for (auto* track : library) {
        if (track && track->get_title() == track_title) {
            return track;
        }
    }
    return nullptr;
}

void DJLibraryService::loadPlaylistFromIndices(const std::string& playlist_name, 
                                               const std::vector<int>& track_indices) {
    playlist = Playlist(playlist_name);

    for (int idx : track_indices) {
        // Indices are 1-based in config
        size_t pos = (idx > 0) ? static_cast<size_t>(idx - 1) : library.size();
        if (pos >= library.size()) {
            std::cout << "[WARNING] Playlist '" << playlist_name
                      << "' references invalid track index: " << idx << "\n";
            continue;
        }

        AudioTrack* lib_track = library[pos];
        if (!lib_track) {
            std::cout << "[WARNING] Null track at index " << idx << " in library.\n";
            continue;
        }

        PointerWrapper<AudioTrack> cloned = lib_track->clone();
        if (!cloned) {
            std::cout << "[ERROR] Failed to clone track '" << lib_track->get_title() << "'. Skipping.\n";
            continue;
        }

        playlist.add_track(cloned.release());
    }
}
/**
 * TODO: Implement getTrackTitles method
 * @return Vector of track titles in the playlist
 */
std::vector<std::string> DJLibraryService::getTrackTitles() const {
    std::vector<std::string> titles;
    for (auto* track : playlist.getTracks()) {
        if (track) {
            titles.push_back(track->get_title());
        }
    }
    return titles;
}
