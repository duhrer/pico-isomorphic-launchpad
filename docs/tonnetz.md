# Tonnetz Note Layout

[Tonnetz layout](https://en.wikipedia.org/wiki/Tonnetz) was developed to help
understand musical relationships (such as forming chords). Here is an
arrangement of notes in Tonnetz layout covering several octaves in a "staggered"
layout:

![Tonnetz octave](/images/tonnetz-staggered-octaves.svg)

In an isomorphic layout, the relationship of each note to its neighbours is
always consistent. In Tonnetz Layout, octaves are always three rows apart on one
of the diagonals (let's call it ten o'clock and four o'clock). 

Each move left or right is a perfect fifth (seven semitones apart). Other key
relationships like single semitones and perfect fourths are not immediately
adjacent to the "root" note.

## Mapping to Grid Layout

To make this usable on a grid controller, we "unstagger" the rows by shifting
each row half a square.  This results in a layout like:

![Default tuning, with triangles](/images/tonnetz-unstaggered-triangles.svg)

In this layout, octaves are always three rows above or below the current note.
As with the original layout, each pad is a perfect fifth (seven semi-tones) away
from its left and right neighbours. This "unstaggered" arrangement arranges
things so that we can move a single semitone by traveling along the
northwest/southeast diagonal.

## Making Chords

To help in describing simple chords, let's start by representing a full grid of
notes in terms of their relative MIDI note numbers:

![MIDI Note Numbers, Tonnetz "Unstaggered" Layout](/images/tonnetz-unstaggered-midi-note-number.svg)

For a major chord, we want a root (zero) note, four semitones higher, and then
seven semitones higher. In this layout the shape is an arrow pointing up and
left, as in:

!["Unstaggered" Tonnetz Major Chord](/images/tonnetz-unstaggered-layout-major-chord.svg)

This shape can be used to play a major chord starting with any root note. For a
minor chord, we want a root (zero) note, three semitones higher, and then seven
semitones higher.  In this layout, that makes an arrow that points down and to
the right, as in:

!["Unstaggered" Tonnetz Minor Chord](/images/tonnetz-unstaggered-layout-minor-chord.svg)

To extend this example a little, if we start with our major chord and extend it
to includes the note eleven semitones higher, we have the pattern for [a major
7th chord](https://en.wikipedia.org/wiki/Major_seventh_chord):

!["Unstaggered" Tonnetz Major7 Chord](/images/tonnetz-unstaggered-layout-major7-chord.svg)

The same pattern will play a major 7th chord anywhere in our range.