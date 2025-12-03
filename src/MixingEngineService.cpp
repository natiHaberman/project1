#include "MixingEngineService.h"
#include <iostream>
#include <memory>
#include <cmath>


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : active_deck(0), auto_sync(false), bpm_tolerance(0)
{
    decks[0] = nullptr;
    decks[1] = nullptr;
    std::cout << "[MixingEngineService] Initialized with 2 empty decks\n";
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    for (auto& deck : decks) {
        delete deck;
        deck = nullptr;
    }
    std::cout << "[MixingEngineService] Cleaning up decks...\n";
}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    //Log message
    std::cout << "\n=== Loading Track to Deck ===\n";


    // Clone the incoming track (mixer owns its own copy)
    PointerWrapper<AudioTrack> clone = track.clone();
    if (!clone) {
        std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone\n";
        return -1;
    }

    // Choose target deck: first empty, else non-active deck
    int target = 0;
    if (!decks[0] && !decks[1]) {
        target = 0;
    } else {
        target = 1 - active_deck;
    }
    std::cout << "[Deck Switch] Target deck: " << target << "\n";

    //Unload target deck
    if (decks[target]) {
        delete decks[target];
        decks[target] = nullptr;
    }


    // Load track and perform beat analysis
    clone->load();
    clone->analyze_beatgrid();

    // If active deck exists and autosync is enabled sync bpm if difference exceeds tolerance
    AudioTrack* previous_track = decks[active_deck];
    if (previous_track && auto_sync) {
        if (!can_mix_tracks(clone)) {
            sync_bpm(clone);
        }
    }

    std::string title = clone->get_title();

    decks[target] = clone.release();
    std::cout << "[Load Complete] '" << title << "' is now loaded on deck " << target << "\n";
    if (previous_track && target != active_deck) {
        std::cout << "[Unload] Unloading previous deck " << active_deck << " (" << previous_track->get_title() << ")\n";
        delete decks[active_deck];
        decks[active_deck] = nullptr;
    }

    active_deck = target;

    std::cout << "[Active Deck] Switched to deck " << target << "\n";

    return target;
}

/**
 * @brief Display current deck status
 */
void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}

/**
 * TODO: Implement can_mix_tracks method
 * 
 * Check if two tracks can be mixed based on BPM difference.
 * 
 * @param track: Track to check for mixing compatibility
 * @return: true if BPM difference <= tolerance, false otherwise
 */
bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {
    AudioTrack* active_track = decks[active_deck];
    if (!active_track || !track) {
        return true;
    }
    int diff = std::abs(active_track->get_bpm() - track->get_bpm());
    return diff <= bpm_tolerance;
}

/**
 * TODO: Implement sync_bpm method
 * @param track: Track to synchronize with active deck
 */
void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {
    AudioTrack* active_track = decks[active_deck];
    if (!auto_sync || !active_track || !track) {
        return;
    }
    int avg_bpm = (active_track->get_bpm() + track->get_bpm()) / 2;
    track -> set_bpm(avg_bpm);
    std::cout << "[Mixing] Auto-sync enabled. Adjusting \"" << track->get_title()
              << "\" BPM to ~" << avg_bpm << " to match \"" << active_track->get_title() << "\"\n";
}
