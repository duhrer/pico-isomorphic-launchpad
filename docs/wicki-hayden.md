# Wicki-Hayden Note Layout

[Wicki-Hayden note
layout](https://en.wikipedia.org/wiki/Wicki%E2%80%93Hayden_note_layout) is an
isomorphic arrangement of keys used for various accordion-like instruments.  

Here is the "staggered" layout typically used in other instruments (I've
represented the C notes in red, naturals in white, and sharps/flats in black):

![Several Octaves in the Wicki-Hayden Layout](/images/wicki-hayden-staggered-layout.svg)

In an isomorphic layout, the relationship of each note to its neighbours is
always consistent. In Wicki-Hayden layout, octaves are always two rows above or
below the current note:

!["Staggered" Octaves](/images/wicki-hayden-staggered-layout-octaves.svg)

Each move left or right is a whole step:

!["Staggered" Whole Steps](/images/wicki-hayden-staggered-layout-whole-steps.svg)

Perfect fourths (five semitones apart) are arranged on one of the diagonals:

!["Staggered" Perfect Fourths](/images/wicki-hayden-staggered-layout-fourths.svg)

Perfect fifths (seven semitones apart) are arranged on the other diagonal:

!["Staggered" Perfect Fifths](/images/wicki-hayden-staggered-layout-fifths.svg)

## Mapping to Grid Layout

To make this usable on a grid controller, we "unstagger" the rows by shifting
each row half a square.  This results in a layout like:

!["Unstaggered" Layout](/images/wicki-hayden-unstaggered-layout.svg)

This "unstaggered" layout preserves the rough grouping of naturals and
sharps/flats, and the relationships are very similar:

!["Unstaggered" Octaves](/images/wicki-hayden-unstaggered-layout-octaves.svg)

Each move left or right is still a whole step:

!["Unstaggered" Whole Steps](/images/wicki-hayden-unstaggered-layout-whole-steps.svg)

!["Unstaggered" Perfect Fourths](/images/wicki-hayden-unstaggered-layout-fourths.svg)

!["Unstaggered" Perfect Fifths](/images/wicki-hayden-unstaggered-layout-fifths.svg)

## Making Chords

To help in describing simple chords, let's start by representing a full grid of
notes in terms of their relative MIDI note numbers:

![MIDI Note Numbers, "Unstaggered" Wicki-Hayden
Layout](/images/wicki-hayden-unstaggered-numbers.svg)

For a major chord, we want a root (zero) note, four semitones higher, and then
seven semitones higher. In this layout the shape is an arrow pointing upwards,
as in:

!["Unstaggered" Wicki-Hayden Major Chord](/images/wicki-hayden-unstaggered-layout-major-chord.svg)

This shape can be used to play a major chord starting with any root note. For a
minor chord, we want a root (zero) note, three semitones higher, and then seven
semitones higher.  In this layout, that makes an arrow that points down, as in:

!["Unstaggered" Wicki-Hayden Minor Chord](/images/wicki-hayden-unstaggered-layout-minor-chord.svg)

To extend this example a little, if we start with our major chord and extend it
to includes the note eleven semitones higher, we have the pattern for [a major
7th chord](https://en.wikipedia.org/wiki/Major_seventh_chord):

!["Unstaggered" Wicki-Hayden Major7 Chord](/images/wicki-hayden-unstaggered-layout-major7-chord.svg)

The same pattern will play a major 7th chord anywhere in our range.