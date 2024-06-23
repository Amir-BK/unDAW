Current Status + unscripted tutorial:

[![Status update + walkthrough](https://img.youtube.com/vi/03D83qDnJc8/0.jpg)](https://www.youtube.com/watch?v=03D83qDnJc8)

Midi creation and editing in unreal editor: 

[![Create Midi from scratch](https://img.youtube.com/vi/6XvwXRE4sAw/0.jpg)](https://www.youtube.com/watch?v=6XvwXRE4sAw)

Creating new fusion patches with unDAW from existing SFZ and DSPreset instruments found online - https://www.youtube.com/watch?v=yBUht2z7a2Y

[![VPiano Test](https://img.youtube.com/vi/NkY0bB5pHyE/0.jpg)](https://www.youtube.com/watch?v=NkY0bB5pHyE)


Some videos until I make a proper demo - https://drive.google.com/drive/folders/1-tbrrys2V091Wv94E5xbvGdLrwm_79i_?usp=sharing

# Features & Current Status 

**UDAWSequencerData** - This is a UObject class that is managed as an asset in the editor or in game, this class contains an editable copy of the midi data and of the "M2Sound Graph" which is basically a graph like description of a metasound that can be instantiated using the metasound builder, this allows creating 'live metasounds' that render the MIDI data via metasound instrument patches (and the new Harmonix Fusion Sampler metasound node that allows rendering polyphonic sounds with a single node inside a metasound). **Right blicking a midi file and generating a DAWSequencerData asset from it allows playing back multi-track midi files with 2-3 clicks**.

**DAW Data Asset Editor** - The daw sequencer data asset can be edited with a custom asset editor class, just double click it, this will automatically create a live copy of the metasound builder that can me controlled via the asset editor's transport, you'll be shown two tabs one being a graph editor for the 'M2Sound' graph and one a piano roll. 

**SPianoRollGrpah** - a native slate widget that uses the parsed midi data found in the UDAWSequencer data class and can paint it, it is rather robust for up until 100k MIDI notes or so beyond that it gets choppy, if all are displayed on the widget at the same time (the widget automatically culls notes outside its view area, so with reasonable zoom settings it's probably possible to play massive midi notes as long as they're not too dense). The piano roll graph supports several control/input modes that are currently pretty glitchy to use but allow adding/deleting notes and tracks and to change the quantization value of the grid. Fully synced to the playback state of the DAWSequencerData asset it is linked to.

**UPianoRollGraph** - a user widget wrapping spianorollgraph, intended to allow full midi editing in game.

**M2SoundGraph** - M-square-sound or Meta-Meta-Sound Graph, basically it's a live metasound with some abstractions that hide 'system nodes' from the user, using the builder to filter the MIDI stream into its constituent tracks and channels which are presented to the user as 'Midi Track Inputs', which can then be connected to 'Patch Instruments' and to audio outputs, the graph automatically exposes the controls of the instrument/insert patches as graph controls. The roadmap is to also to create in game representation for the graph, think MIDI driven 'Patchwork' accessible in unreal editor.

**DSPreset/SFZ to Fusion Patch Importer** - just grab DSPreset and SFZ instrument libraries into the unreal content editor and they will get auto-imported as Fusion Patches, all of the non-standard fusion patches used in the demo videos are free SFZ/DS instruments from around the web. There is also WIP support for importing DSLibrary instruments which are more common but it's unfinished at the moment. 

**[Early WIP] Glyph Viewer Widget** - some initial tests using SMUFL music fonts to render MIDI info as actual music notes, this was an early attempt and is unfinished, it's in the repo largely as reference.

**Timestamped Wav Player Transport Interface Node** - an extension of the new "Transport Wav Player" node added by harmonix that also gives it awareness of an FMusicTimestamp that can be used as an arbitrary start point (if you want to sync a wav file to a midi file non on clock start, basically

**[Early WIP] Midi Arpeggiator Metasound Node** - A node that takes in midi notes and transposes them to a different random scale degree, far from complete. 

# Community & Support

https://discord.gg/hTKjSfcbEn

# Samples and Midi Files
I don't own the rights to any of the samples or midi files I use in the demo so no midi files or extra instruments are shared in the repo, the default fusion instruments can be found in the harmonix plugin (which must be enabled for unDAW to work). 

# Roadmap Features

# Depenencies & third parties
All third party resources include their original licensing documents in the resources folder for the plugin, these should be included when used as per the specifications of the original licenses 
- [SMUFL Fonts - Bravura](https://github.com/steinbergmedia/bravura)
- [SIMPLE UI icons] (https://github.com/Semantic-Org/UI-Icon)
- https://github.com/steinbergmedia/petaluma


