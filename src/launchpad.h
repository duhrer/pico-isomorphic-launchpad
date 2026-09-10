#ifndef _LAUNCHPAD_H_
#define _LAUNCHPAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

enum LaunchpadVersion {
  UNkNOWN,
  MK2,
  MK3
};

struct LaunchpadControlScheme {
    uint8_t UpArrow;
    uint8_t DownArrow;
    uint8_t LeftArrow;
    uint8_t RightArrow;

    uint8_t ModeOne;
    uint8_t ModeTwo;
    uint8_t ModeThree;
    uint8_t ModeFour;
    uint8_t ModeFive;
    uint8_t ModeSix;
    uint8_t ModeSeven;
    uint8_t ModeEight;

    uint8_t ColourSchemeOne;
    uint8_t ColourSchemeTwo;
};

static const struct LaunchpadControlScheme MarkTwoControlScheme = {
    // Top Round Pads
    .UpArrow    = 91,
    .DownArrow  = 92,
    .LeftArrow  = 93,
    .RightArrow = 94,

    // Bottom Round Pads
    .ModeOne    = 1,
    .ModeTwo    = 2,
    .ModeThree  = 3,
    .ModeFour   = 4,
    .ModeFive   = 5,
    .ModeSix    = 6,
    .ModeSeven  = 7,
    .ModeEight  = 8,

    // Right Round Pads
    .ColourSchemeOne = 29,
    .ColourSchemeTwo = 19,
};

static const struct LaunchpadControlScheme MarkThreeControlScheme = {
    // Top Round Pads
    .UpArrow    = 80,
    .DownArrow  = 70,
    .LeftArrow  = 91,
    .RightArrow = 92,

    // Bottom Round Pads
    .ModeOne    = 101,
    .ModeTwo    = 102,
    .ModeThree  = 103,
    .ModeFour   = 104,
    .ModeFive   = 105,
    .ModeSix    = 106,
    .ModeSeven  = 107,
    .ModeEight  = 108,

    // Right Round Pads
    .ColourSchemeOne = 29,
    .ColourSchemeTwo = 19,
};


struct NoteLayout {
    uint8_t row_pitch_offset;
    uint8_t column_pitch_offset;
};

// Staggered layout
// TODO: This is still not right visually or to play.
static const struct NoteLayout WickiHaydenStaggered = {
    .row_pitch_offset = 5,
    .column_pitch_offset = 7
};


// "Unstaggered" Layout rotated 45 degrees clockwise, set as the default to match
// existing instruments.
static const struct NoteLayout WickiHaydenUnstaggeredClockwise = {
    .row_pitch_offset = 5,
    .column_pitch_offset = 2
};

// "Unstaggered" Layout skewed 45 degrees counterclockwise
static const struct NoteLayout WickiHaydenUnstaggeredCounterclockwise = {
    .row_pitch_offset = 7,
    .column_pitch_offset = 2
};


// "Staggered" Layout
static const struct NoteLayout TonnetzStaggered = {
    .row_pitch_offset = 3,
    .column_pitch_offset = 8 
};

// "Unstaggered" Layout skewed 45 degrees clockwise
static const struct NoteLayout TonnetzUnstaggeredClockwise = {
    .row_pitch_offset = 8,
    .column_pitch_offset = 7 
};

// "Unstaggered" Layout skewed 45 degrees counterclockwise
static const struct NoteLayout TonnetzUnstaggeredCounterclockwise = {
    .row_pitch_offset = 3,
    .column_pitch_offset = 7 
};


// "Piano" Layout
static const struct NoteLayout Piano = {
    .row_pitch_offset = 8,
    .column_pitch_offset = 1
};

// Colours set with MIDI Note messages (as we use for a "host" device) are
// limited to a particular palette, which is sent as a velocity. For this
// reason, we use the "standard colour" mode for colours sent via sysex as well.

struct ColourScheme {
    uint8_t note_colour_velocities[12];
    uint8_t held_colour_velocity;
};

#define MAX_OFFSET 74

// "Rainbow" mode uses a di8fferent colour for each natural and black for
// sharps/flats. Held notes are white.
static const struct ColourScheme RainbowColourScheme = {
    .note_colour_velocities = {
        60, // C: Red
        0,  // C#
        9,  // D: Orange
        0,  // D#
        13, // E: Yellow
        64, // F: Green
        0,  // F#
        67, // G: Blue
        0,  // G#
        81, // A: Purple
        0,  // A#
        56  // B: Pink
    },
    .held_colour_velocity = 3
};

// In the "Red Cs" colour scheme, Cs are red, other naturals are white, and
// sharps/flats are black. Held notes are cyan.
static const struct ColourScheme RedCsColourScheme = {
    .note_colour_velocities = {
        60, // C
        0,  // C#
        3,  // D
        0,  // D#
        3,  // E
        3,  // F
        0,  // F#
        3,  // G
        0,  // G#
        3,  // A
        0,  // A#
        3  // B
    },
    .held_colour_velocity = 90
};


struct host_state {
    uint8_t offset;

    uint8_t client_idx;

    uint8_t global_midi_channel;

    enum LaunchpadVersion launchpad_version;
};

struct client_state {
    uint8_t offset_by_cable[3];
};

struct board_state {
    // State for the single device connected to our "host" USB port.
    struct host_state host;

    // State for one or more devices connected via external software to our "client" port.
    struct client_state client;

    // Shared state for all devices
    struct ColourScheme colour_scheme;
    int colour_scheme_index;

    struct NoteLayout note_layout;
    int note_layout_index;

    // What notes are held
    uint8_t held_note_velocities[128];

    // What notes are already playing
    uint8_t playing_note_velocities[128];

    // Whether we need to redraw (for example, when the tuning changes or a pad is held/released).
    bool is_dirty;

};

enum HostOrClient {
    HOST,
    CLIENT
};

void initialise_client_launchpads(void);

void initialise_mk1_client_launchpads(void);
void initialise_mk2_client_launchpads(void);
void initialise_mk3_client_launchpads(void);

void paint_client_launchpads(struct board_state*);

void paint_mk1_client_launchpads(struct board_state*);
void paint_mk2_client_launchpads(struct board_state*);
void paint_mk3_client_launchpads(struct board_state*);

void paint_host_launchpad(struct board_state*);

void paint_mk1_host_launchpad(struct board_state*);
void paint_mk2_host_launchpad(struct board_state*);
void paint_mk3_host_launchpad(struct board_state*);

void process_incoming_host_packet(uint8_t*, struct board_state*);

void process_incoming_client_packet(uint8_t *, struct board_state*);

void process_incoming_mk1_packet (uint8_t*, struct board_state*, enum HostOrClient);
void process_incoming_mk2_packet (uint8_t*, struct board_state*, enum HostOrClient);
void process_incoming_mk3_packet (uint8_t*, struct board_state*, enum HostOrClient);
void process_incoming_external_packet(uint8_t*, struct board_state*);

void process_incoming_control_code (uint8_t, struct board_state*, const struct LaunchpadControlScheme, enum HostOrClient hostOrClient);

enum LaunchpadVersion get_launchpad_version (uint16_t, uint16_t);

#ifdef __cplusplus
}
#endif

#endif /* _LAUNCHPAD_H_ */