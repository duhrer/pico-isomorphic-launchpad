#include <stdint.h>
#include "launchpad.h"
#include "tusb.h"
#include <math.h>

void clear_all_notes(struct board_state *board_state) {
  for (int a = 0; a < 128; a++) {
    if (board_state ->held_note_velocities[a]) {
      board_state->held_note_velocities[a] = 0;

      uint8_t note_off_message[3] = {
          (MIDI_CIN_NOTE_OFF << 4) | board_state -> host.global_midi_channel , a, 0
      };

      tud_midi_stream_write(2, note_off_message, sizeof note_off_message);
    }
  }
}

// Begin version-specific functions.

void initialise_client_launchpads(void) {
  initialise_mk2_client_launchpads();
  initialise_mk3_client_launchpads();
}

void initialise_mk2_client_launchpads(void) {
  // Select "standalone" mode (it's the default, but for users who also use
  // Ableton, this will ensure things are set up properly).
  uint8_t standalone_mode_packet[9] = {
    0xf0, 0x00, 0x20, 0x29, 0x02, 0x10, 0x2C, 0x03, 0xf7
  };

  // Select "programmer" layout ("note" layout is the default)
  uint8_t programmer_layout_packet[9] = {
    0xf0, 0, 0x20, 0x29, 0x02, 0x10, 0x16, 0x3, 0xf7
  };

  // This should use cable 0.
  tud_midi_stream_write(0, standalone_mode_packet, sizeof(standalone_mode_packet));
  tud_midi_stream_write(0, programmer_layout_packet, sizeof programmer_layout_packet);
}

void initialise_mk3_client_launchpads(void) {
  // Select the programmer's layout, we want layout 11h and page 0
  // F0h 00h 20h 29h 02h 0Eh 00h <layout> <page> 00h F7h
  uint8_t select_programmers_layout[] = {
    0xF0, 0x00, 0x20, 0x29, 0x02, 0x0E, 0x00, 0x11, 0x00, 0x00, 0xF7
  };

  // They don't have a "clear all" method, just a sysex to send a value for
  // every pad, so we skip that.

  // This should use cable 1.
  tud_midi_stream_write(1, select_programmers_layout, sizeof select_programmers_layout);
}

void paint_client_launchpads(struct board_state *board_state) {
  paint_mk2_client_launchpads(board_state);
  paint_mk3_client_launchpads(board_state);
}

void paint_mk2_client_launchpads(struct board_state *board_state) {
  // Sysex messages used to paint the Launchpad in this pass....

  // The "paint all" operation doesn't support RGB, so you have to pick a colour
  // from the built-in 128 colour palette, for example, 0 for black and 3 for
  // white, 24 for green.
  //
  // Paint All F0h 00h 20h 29h 02h 10h 0Eh                                                                                                                                  <Colour> F7h
  uint8_t paint_all_sysex[9] = {
      0xf0, 0, 0x20, 0x29, 0x2, 0x10, 0xE, 0, 0xf7
  };

  // The virtual port for the MK2 should be cable 1.
  tud_midi_stream_write(1, paint_all_sysex, sizeof(paint_all_sysex));

  // We could do this all in one, but per row seems less involved.
  for (int row = 0; row < 8; row++) {
    // Paint a row using the canned colour palette.
    // F0h 00h 20h 29h 02h 10h 0Dh <Row> (<Colour> * 10) F7h
    uint8_t paint_row[19] = {
      0xf0, 0, 0x20, 0x29, 0x2, 0x10, 0xD, row + 1, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0xf7
    };

    for (int column = 0; column < 8; column++) {
      int tuned_note = board_state->note_layout.layout_offset + board_state->client.offset_by_cable[1] + (column * board_state->note_layout.column_pitch_offset) + (row * board_state->note_layout.row_pitch_offset);

      if (board_state->held_note_velocities[tuned_note] > 0) {
        paint_row[8 + column] = board_state->colour_scheme.held_colour_velocity;
      }
      else {
        uint8_t note_offset = tuned_note % 12;
        paint_row[8 + column] = board_state->colour_scheme.note_colour_velocities[note_offset];
      }
    }

    // The virtual port for the MK2 should be cable 1.
    tud_midi_stream_write(1, paint_row, sizeof(paint_row));
  }

  // TODO: Paint mode controls...
  // TODO: Paint colour controls...

  // We currently use the "pulse" method for the side light.
  uint8_t paint_side_light[12] = {
    0xf0, 0x00, 0x20, 0x29, 0x2, 0x10, 0x28, 0x63, 3, 0xf7
  };

  // The virtual port for the MK2 should be cable 1.
  tud_midi_stream_write(1, paint_side_light, sizeof(paint_side_light));
}

void paint_mk3_client_launchpads(struct board_state *board_state) {
  // For now, use notes.
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 8; column++) {
      // Offset the row by one to skip the very lowest row of buttons and paint the square pads.
      int launchpad_note = ((row + 1) * 10) + column;

      int tuned_note = board_state->note_layout.layout_offset + board_state->client.offset_by_cable[2] + (column * board_state->note_layout.column_pitch_offset) + (row * board_state->note_layout.row_pitch_offset);

      uint8_t note_offset = tuned_note % 12;

      uint8_t velocity = 0; // Black / Unlit

      // We have to paint the first column black because there are also
      // controls we use there.
      if (column) {
        if (board_state->held_note_velocities[tuned_note] > 0) {
          velocity = board_state->colour_scheme.held_colour_velocity;
        }
        else {
          velocity = board_state->colour_scheme.note_colour_velocities[note_offset];
        }
      }

      uint8_t note_on_message[3] = {
        MIDI_CIN_NOTE_ON << 4, launchpad_note, velocity
      };

      // The virtual port for the MK3 should be cable 1.
      tud_midi_stream_write(1, note_on_message, sizeof(note_on_message));
    }
  }

  // TODO: Paint mode controls...
  // TODO: Paint colour controls...
}

void paint_host_launchpad(struct board_state *board_state) {
    if (board_state->host.launchpad_version == MK2) {
        paint_mk2_host_launchpad(board_state);
    }
    else if (board_state->host.launchpad_version == MK3) {
        paint_mk3_host_launchpad(board_state);
    }
}

void generate_mode_messages(struct board_state *board_state, const struct LaunchpadControlScheme control_scheme, uint8_t channel, uint8_t *messages) {
    messages[0] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[1] = control_scheme.ModeOne;
    messages[2] = board_state->note_layout_index == 0 ? 127 : 0;
 
    messages[3] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[4] = control_scheme.ModeTwo;
    messages[5] = board_state->note_layout_index == 1 ? 127 : 0;

    messages[6] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[7] = control_scheme.ModeThree;
    messages[8] = board_state->note_layout_index == 2 ? 127 : 0;

    messages[9] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[10] = control_scheme.ModeFour;
    messages[11] = board_state->note_layout_index == 3 ? 127 : 0;

    messages[12] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[13] = control_scheme.ModeFive;
    messages[14] = board_state->note_layout_index == 4 ? 127 : 0;
 
    messages[15] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[16] = control_scheme.ModeSix;
    messages[17] = board_state->note_layout_index == 5 ? 127 : 0;

    messages[18] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[19] = control_scheme.ModeSeven;
    messages[20] = board_state->note_layout_index == 6 ? 127 : 0;

    messages[21] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[22] = control_scheme.ModeEight;
    messages[23] = board_state->note_layout_index == 7 ? 127 : 0;
}

void generate_colour_scheme_messages(struct board_state *board_state, const struct LaunchpadControlScheme control_scheme, uint8_t channel, uint8_t * messages) {
    messages[0] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[1] = control_scheme.ColourSchemeOne;
    messages[2] = board_state->colour_scheme_index == 0 ? 127 : 0;

    messages[3] = (MIDI_CIN_CONTROL_CHANGE << 4) | channel;
    messages[4] = control_scheme.ColourSchemeTwo;
    messages[5] = board_state->colour_scheme_index == 1 ? 127 : 0;
}

// TODO: When we figure out sending sysex to the host's client device, we can simplify this.
void paint_mk2_host_launchpad(struct board_state *board_state) {
  // Write note messages for the host side until we figure out sysex there.
  for (int launchpad_note = 10; launchpad_note < 89; launchpad_note++) {
    int column = launchpad_note % 10;
    int row = ((launchpad_note - column)/10) - 1;

    uint8_t velocity = 0;

    // Skip the first column as we need to keep those black for controls,
    // and anything over 9 to keep ourselves on the square pads.
    if (column && (column < 9)) {
      int tuned_note = board_state->note_layout.layout_offset + (board_state->host.offset) + (column * board_state->note_layout.column_pitch_offset) + (row * board_state->note_layout.row_pitch_offset);

      if (tuned_note < 128) {

        if (board_state->held_note_velocities[tuned_note] > 0) {
          velocity = board_state -> colour_scheme.held_colour_velocity;
        }
        else {
          uint8_t note_offset = tuned_note % 12;
          velocity = board_state -> colour_scheme.note_colour_velocities[note_offset];
        }
      }
    }

    uint8_t note_on_message[3] = {
      (MIDI_CIN_NOTE_ON << 4)  | board_state -> host.global_midi_channel, launchpad_note, velocity
    };

    // One copy for performance firmware, which uses one port
    tuh_midi_stream_write(0, 0, note_on_message, sizeof(note_on_message));
    // Another copy for stock firmware
    tuh_midi_stream_write(0, 1, note_on_message, sizeof(note_on_message));
  } 

  // Paint mode controls
  // TODO: This is pretty brittle if we add more modes
  uint8_t mode_messages[24];
  generate_mode_messages(board_state, MarkTwoControlScheme, board_state -> host.global_midi_channel, mode_messages);

  for (int mode_message_index = 0; mode_message_index < 24; mode_message_index += 3) {
    uint8_t control_change_message[3];
    memcpy(control_change_message, mode_messages + mode_message_index, 3);

    tuh_midi_stream_write(0, 0, control_change_message, sizeof(control_change_message));
    tuh_midi_stream_write(0, 1, control_change_message, sizeof(control_change_message));

  }

  // Paint colour controls
  // TODO: This is pretty brittle if we add another colour scheme
  uint8_t colour_scheme_messages[6];
  generate_colour_scheme_messages(board_state, MarkTwoControlScheme, board_state -> host.global_midi_channel, colour_scheme_messages);
  for (int colour_message_index = 0; colour_message_index < 6; colour_message_index += 3) {
    uint8_t control_change_message[3];
    memcpy(control_change_message, colour_scheme_messages + colour_message_index, 3);
    tuh_midi_stream_write(0, 0, colour_scheme_messages, sizeof(colour_scheme_messages));
    tuh_midi_stream_write(0, 1, colour_scheme_messages, sizeof(colour_scheme_messages));
  }
}

// TODO: When we figure out sending sysex to the host's client device, we can
// simplify this by using their sysex strategy (see the client implementation).

void paint_mk3_host_launchpad(struct board_state *board_state) {
  for (int row = 0; row < 8; row++) {
    for (int column = 0; column < 8; column++) {
      // Offset the row by one to skip the very lowest row of buttons and paint the square pads.
      int launchpad_note = ((row + 1) * 10) + column;

      int tuned_note = board_state->note_layout.layout_offset + (board_state->host.offset) + (column * board_state->note_layout.column_pitch_offset) + (row * board_state->note_layout.row_pitch_offset);
      
      uint8_t velocity = 0; // Black / Unlit

      // We have to paint the first column black because there are also
      // controls we use there.
      if (column) {
        if (board_state->held_note_velocities[tuned_note] > 0) {
          velocity = board_state->colour_scheme.held_colour_velocity;        }
        else {
          uint8_t note_offset = tuned_note % 12;
          velocity = board_state->colour_scheme.note_colour_velocities[note_offset];
        }
      }

      uint8_t note_on_message[3] = {
        // TODO: We need to figure out why we have to use the MK3 drawing method
        // for a MK2 in host mode, this should not use the channel info
        // MIDI_CIN_NOTE_ON << 4, launchpad_note, velocity
        (MIDI_CIN_NOTE_ON << 4)  | board_state -> host.global_midi_channel, launchpad_note, velocity
        // (MIDI_CIN_NOTE_ON << 4)  | 2, launchpad_note, velocity
      };

      // The MK3 wants data on the first cable, i.e. "MIDI" and not "DIN" or "DAW"
      tuh_midi_stream_write(board_state->host.client_idx, 0, note_on_message, sizeof(note_on_message));
    }
  }

  // Paint mode controls
  // TODO: This is pretty brittle if we add more modes
  uint8_t mode_messages[24];
  generate_mode_messages(board_state, MarkThreeControlScheme, board_state -> host.global_midi_channel, mode_messages);

  for (int mode_message_index = 0; mode_message_index < 24; mode_message_index += 3) {
    uint8_t control_change_message[3];
    memcpy(control_change_message, mode_messages + mode_message_index, 3);
    tuh_midi_stream_write(0, 1, control_change_message, sizeof(control_change_message));
  }

  // Paint colour controls
  // TODO: This is pretty brittle if we add another colour scheme
  uint8_t colour_scheme_messages[6];
  generate_colour_scheme_messages(board_state, MarkThreeControlScheme, board_state -> host.global_midi_channel, colour_scheme_messages);
  for (int colour_message_index = 0; colour_message_index < 6; colour_message_index += 3) {
    uint8_t control_change_message[3];
    memcpy(control_change_message, colour_scheme_messages + colour_message_index, 3);
    tuh_midi_stream_write(0, 1, colour_scheme_messages, sizeof(colour_scheme_messages));
  }
}

void process_incoming_host_packet(uint8_t *incoming_packet, struct board_state *board_state) {
    uint8_t type = (incoming_packet[1] >> 4) & 0xf;

    // Only update the channel if the message includes it.
    if (type == MIDI_CIN_NOTE_ON || type == MIDI_CIN_NOTE_OFF || type == MIDI_CIN_CONTROL_CHANGE || type == MIDI_CIN_POLY_KEYPRESS) {
      uint8_t global_midi_channel = (incoming_packet[1] & 0xf);
      // uint8_t global_midi_channel = 0;
      if (global_midi_channel != board_state -> host.global_midi_channel) {
        board_state -> host.global_midi_channel = global_midi_channel;
      }
    }

    if (board_state->host.launchpad_version == MK2) {
        process_incoming_mk2_packet(incoming_packet, board_state, HOST);
    }
    else if (board_state->host.launchpad_version == MK3) {
        process_incoming_mk3_packet(incoming_packet, board_state, HOST);
    }
}

void process_incoming_client_packet(uint8_t *incoming_packet, struct board_state *board_state) {
    uint8_t cable = (incoming_packet[0] >> 4) & 0xf;

    // MK2
    if (cable == 0) {
      process_incoming_mk2_packet(incoming_packet, board_state, CLIENT);
    }
    // MK3
    else if (cable == 1) {
      process_incoming_mk3_packet(incoming_packet, board_state, CLIENT);
    }
    // Passthrough "notes" channel
    else if (cable == 2) {
      process_incoming_external_packet(incoming_packet, board_state);
    }
}

void increment_offset(struct board_state *board_state, enum HostOrClient hostOrClient, int cable, int increment) {
  if (hostOrClient == HOST) {
    board_state -> host.offset += increment;
  }
  else {
    board_state -> client.offset_by_cable[cable] += increment;
  }
}

// Respond to MK2 controls

void process_incoming_mk2_packet (uint8_t *incoming_packet, struct board_state *board_state, enum HostOrClient hostOrClient) {
  int user_offset = hostOrClient == HOST ? board_state -> host.offset : board_state->client.offset_by_cable[1]; 

  // Start with the message type
  int type = incoming_packet[1] >> 4;

  // Handle square pads (notes) and relevant round pads (outside columns)
  if (type == MIDI_CIN_NOTE_ON || type == MIDI_CIN_NOTE_OFF || type == MIDI_CIN_POLY_KEYPRESS) {
    uint8_t launchpad_note = incoming_packet[2];

    if (launchpad_note >=10 && launchpad_note <= 89) {
      int column = launchpad_note % 10;
      int row = ((launchpad_note - column)/10) - 1;

      // Calculate the note from the row and ofset
      int tuned_note = board_state->note_layout.layout_offset + user_offset + (column * board_state->note_layout.column_pitch_offset) + (row * board_state->note_layout.row_pitch_offset);

      if (tuned_note >= 0 && tuned_note < 128) {
        uint8_t transformed_packet[3];
        memcpy(transformed_packet, incoming_packet + 1, 3);

        transformed_packet[1] = tuned_note;

        // Pass along the modified message.
        tud_midi_stream_write(2, transformed_packet, sizeof transformed_packet);

        // Store our velocity in board_state->held_note_velocities
        board_state->held_note_velocities[tuned_note] = incoming_packet[3];

        board_state->is_dirty = true;
      }
    }    
  }

  // Only react when a control is changed to a non-zero value, i.e. when it's pressed, and not when it's released.
  else if (type == MIDI_CIN_CONTROL_CHANGE && incoming_packet[3]) {
    process_incoming_control_code((uint8_t) incoming_packet[2], board_state, MarkTwoControlScheme, hostOrClient);
  }
}

// TODO: Standardise controls using some kind of map to a common set of
// definitions.  Also clean up the constant dirty/clear logic.

// Respond to MK3 controls
void process_incoming_mk3_packet (uint8_t *incoming_packet, struct board_state *board_state, enum HostOrClient hostOrClient) {
  int user_offset = hostOrClient == HOST ? board_state -> host.offset : board_state->client.offset_by_cable[2]; 

  // Start with the message type
  int type = incoming_packet[1] >> 4;

  // Handle square pads (notes) and relevant round pads (outside columns)
  if (type == MIDI_CIN_NOTE_ON || type == MIDI_CIN_NOTE_OFF || type == MIDI_CIN_POLY_KEYPRESS) {
    if (incoming_packet[2] >=11 && incoming_packet[2] <= 89) {
      int column = incoming_packet[2] % 10;

      // Skip the first column, which we have to use as controls.
      if (column) {
        // Offset the row by one to skip the very lowest row of buttons and paint the square pads.
        int row = ((incoming_packet[2] - column)/10) - 1;


        // Calculate the note from the row and ofset
        int tuned_note = board_state->note_layout.layout_offset + user_offset + (column * board_state->note_layout.column_pitch_offset) + (row * board_state->note_layout.row_pitch_offset);

        if (tuned_note >= 0 && tuned_note < 128) {
          uint8_t transformed_packet[3];
          memcpy(transformed_packet, incoming_packet + 1, 3);

          transformed_packet[1] = tuned_note;

          // Pass along the modified message.
          tud_midi_stream_write(2, transformed_packet, sizeof transformed_packet);

          // Store our velocity in board_state -> held_note_velocities
          board_state->held_note_velocities[tuned_note] = incoming_packet[3];

          board_state->is_dirty = true;
        }
      }
    }    
  }

  // Only react when a control is changed to a non-zero value, i.e. when it's pressed, and not when it's released.
  else if (type == MIDI_CIN_CONTROL_CHANGE && incoming_packet[3]) {
    process_incoming_control_code((uint8_t) incoming_packet[2], board_state, MarkThreeControlScheme, hostOrClient);
  }
}

void process_incoming_external_packet(uint8_t *incoming_packet, struct board_state *board_state) {
  uint8_t data[3];
  memcpy(data, incoming_packet + 1, 3);

  // Start with the message type
  int type = data[0] >> 4;

  if (type == MIDI_CIN_NOTE_ON || type == MIDI_CIN_POLY_KEYPRESS) {
    // Store our velocity in board_state -> held_note_velocities
    board_state->held_note_velocities[data[1]] = data[2];
    board_state->is_dirty = true;
  } 
  else if (type == MIDI_CIN_NOTE_OFF) {
    // Store our velocity in board_state -> held_note_velocities
    board_state->held_note_velocities[data[1]] = 0;
    board_state->is_dirty = true;
  } 
}

void process_incoming_control_code (uint8_t controlCodeNumber, struct board_state *board_state, struct LaunchpadControlScheme control_scheme, enum HostOrClient hostOrClient) {
  bool state_changed = false;

  int offset = hostOrClient == HOST ? board_state -> host.offset : board_state->client.offset_by_cable[1]; 

  // Upward arrow
  if (controlCodeNumber == control_scheme.UpArrow) {
      if (offset <= (MAX_OFFSET - board_state->note_layout.row_pitch_offset)) {
        increment_offset(board_state, hostOrClient, 1, board_state->note_layout.row_pitch_offset);
      }
      state_changed = true;
  } 
  // Downward arrow
  else if (controlCodeNumber == control_scheme.DownArrow) {
      if (offset >= board_state->note_layout.row_pitch_offset) {
        increment_offset(board_state, hostOrClient, 1, -1 * board_state->note_layout.row_pitch_offset);
        state_changed = true;
      }
  }
  // Left Arrow
  else if (controlCodeNumber == control_scheme.LeftArrow) {
      if (offset >= board_state->note_layout.column_pitch_offset) {
        increment_offset(board_state, hostOrClient, 1, -1 * board_state->note_layout.column_pitch_offset);
        state_changed = true;
      }
  }

  // Right Arrow
  else if (controlCodeNumber == control_scheme.RightArrow) {
      if (offset <= (MAX_OFFSET - board_state->note_layout.column_pitch_offset)) {
        increment_offset(board_state, hostOrClient, 1, board_state->note_layout.column_pitch_offset);
        state_changed = true;
      }
  }

  // Mode controls.
  else if (controlCodeNumber == control_scheme.ModeOne) {
      board_state->note_layout = WickiHaydenUnstaggeredClockwise;
      board_state->note_layout_index = 0;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ModeTwo) {
      board_state->note_layout = WickiHaydenUnstaggeredCounterclockwise;
      board_state->note_layout_index = 1;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ModeThree) {
      board_state->note_layout = TonnetzUnstaggeredClockwise;
      board_state->note_layout_index = 2;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ModeFour) {
      board_state->note_layout = TonnetzUnstaggeredCounterclockwise;
      board_state->note_layout_index = 3;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ModeFive) {
      board_state->note_layout = TonnetzStaggered;
      board_state->note_layout_index = 4;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ModeSix) {
      board_state->note_layout = JankoUnstaggeredClockwise;
      board_state->note_layout_index = 5;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ModeSeven) {
      board_state->note_layout = JankoUnstaggeredCounterclockwise;
      board_state->note_layout_index = 6;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ModeEight) {
    board_state->note_layout = PianoMandolin;
    board_state->note_layout_index = 7;
    state_changed = true;
  }

  // Colour Scheme Controls
  else if (controlCodeNumber == control_scheme.ColourSchemeOne) {
      board_state->colour_scheme = RainbowColourScheme;
      board_state->colour_scheme_index = 0;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ColourSchemeTwo) {
      board_state->colour_scheme = RedCsColourScheme;
      board_state->colour_scheme_index = 1;
      state_changed = true;
  }
  else if (controlCodeNumber == control_scheme.ColourSchemeThree) {
    board_state->colour_scheme = StrongSubtleBlues;
    board_state->colour_scheme_index = 1;
    state_changed = true;
  }

  if (state_changed) {
    clear_all_notes(board_state);
    board_state->is_dirty = true;
  }
}

/*

  For stock Launchpads, we can just use the vendor and product numbers.

  Stock Pro MK2, MIDI channel 0:
  Bus 007 Device 119: ID 1235:0051 Focusrite-Novation Launchpad Pro

  Stock Pro MK2, MIDI channel 1:
  Bus 007 Device 121: ID 1235:0052 Focusrite-Novation Launchpad Pro 2

  The "performance" custom firmeware
  (https://github.com/mat1jaczyyy/lpp-performance-cfw) reuses these, so we also
  need to look at the strings or use another strategy.

  Pro 2 CFW, MIDI Channel 0:
  Bus 007 Device 125: ID 1235:0051 Focusrite-Novation Launchpad Open

  Pro 2 CFW, MIDI Channel 1:
  Bus 007 Device 127: ID 1235:0052 Focusrite-Novation Launchpad Open 2

  There's also this firmware, which I haven't been able to build or install yet:
  The CoreFW (https://github.com/anthonyhfm/launchpad-core-firmware)

  CoreFW
  Bus 007 Device 092: ID 1235:0051 Focusrite-Novation Launchpad Pro
  
*/

bool contains_string(char *haystack, int haystack_length, char *needle, int needle_length) {
  int match_length = 0;
  for (int idx = 0; idx < haystack_length; idx++) {
    if(haystack[idx] == needle[match_length]) {
      match_length++;
    }
    // Loop back to catch things like neneneedle that have adjoining partial matches
    else if (haystack[idx] == needle[0]) {
      match_length = 1;
    }
    else {
      match_length = 0;
    }

    if (match_length == (needle_length - 1)) {
      return true;
    }
  }
  return false;
}

enum LaunchpadVersion get_launchpad_version (uint16_t idVendor, uint16_t idProduct) {
  enum LaunchpadVersion launchpad_version;

  launchpad_version = MK2;

  if (idVendor == 0x1235) {
    if (
      idProduct >= 0x0051 && idProduct <= 0x0060
    ) {
      launchpad_version = MK2;
    }
    else if (idProduct >= 0x0123 && idProduct <= 0x0132) {
      launchpad_version = MK3;
    }
  }

  return launchpad_version;
}