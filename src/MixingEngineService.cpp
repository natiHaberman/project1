#include "MixingEngineService.h"
#include <iostream>
#include <memory>
#include <cmath>


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : active_deck(0), auto_sync(true), bpm_tolerance(10)
{
    decks[0] = nullptr;
    decks[1] = nullptr;
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    for (auto& deck : decks) {
        delete deck;
        deck = nullptr;
    }
}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    // Clone the incoming track (mixer owns its own copy)
    PointerWrapper<AudioTrack> clone = track.clone();
    if (!clone) {
        std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone\n";
        return -1;
    }

    // Choose target deck: first empty, else non-active deck
    size_t target = 0;
    if (!decks[0]) {
        target = 0;
    } else if (!decks[1]) {
        target = 1;
    } else {
        target = (active_deck == 0) ? 1 : 0;
    }

    size_t previous_active = active_deck;
    AudioTrack* previous_track = decks[previous_active];

    if (decks[target]) {
        delete decks[target];
        decks[target] = nullptr;
    }

    decks[target] = clone.release();
    decks[target]->load();
    decks[target]->analyze_beatgrid();

    if (previous_track && auto_sync) {
        PointerWrapper<AudioTrack> temp_holder(decks[target]); 
        if (!can_mix_tracks(temp_holder)) {
            sync_bpm(temp_holder);
        }
        temp_holder.release(); 
    }

    active_deck = target;

    if (previous_track && target != previous_active) {
        delete decks[previous_active];
        decks[previous_active] = nullptr;
    }

    return static_cast<int>(target);
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
