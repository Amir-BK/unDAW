[![Updated Asset Editor](https://img.youtube.com/vi/8OztDqxOvXM/0.jpg)](https://www.youtube.com/watch?v=8OztDqxOvXM)

Creating new fusion patches with unDAW from existing SFZ and DSPreset instruments found online - https://www.youtube.com/watch?v=yBUht2z7a2Y


Quick and dirty quick start guide - https://www.youtube.com/watch?v=-Hr0XsmlV8Y

[![VPiano Test](https://img.youtube.com/vi/NkY0bB5pHyE/0.jpg)](https://www.youtube.com/watch?v=NkY0bB5pHyE)


Some videos until I make a proper demo - https://drive.google.com/drive/folders/1-tbrrys2V091Wv94E5xbvGdLrwm_79i_?usp=sharing

Esentially, drag a midi file into unreal, right click it, choose 'Edit in BK Editor', click on the 'refresh' icon to generate the metasound and then hit 'play', it will default to pianos on all channels, press F1 to switch to the mixer and switch Fusion patches and the such. Ctrl/Shift + Mouse Wheel to zoom in and out in the editor. 

Can also drag SFZ and Decent Preset sample files to unreal and it will auto create fusion patches, more documentation coming...


## V0.0.1! First public release! Currently only the Midi Editor Widget is substantially developed, can also use the Decent Sampler importer to quickly make fusion patches. More to come! 

# Community & Support

https://discord.gg/hTKjSfcbEn


# ALPHA WARNING AND VERSIONING
As Unreal 5.4 is not even in early access to use this plugin you must sync to the Epic repo 5.4 branch, it is expected that this plugin will not maintain a stable API until full 5.4 release, expect changes to be breaking.

This plugin and repo will adhere to semantic versioning - https://semver.org/ - until we reach a V1.0 release all changes on main have the potential to break blueprints and existing APIs. 

## Packaging and Platform Status

Currently the plugin has packaging issues preventing it from being used in a packaged game or in mobile builds, this is (likely) due to warnings that need to be resolved in the codebase, by v1.0 and 5.4 release this plugin is meant to be cross-platform with no packaging issues for release. 

# Modules
## Runtime - BK Musical Widgets 
a collection of widgets that can be used both in Editor and in game (not yet!) to visualize and edit musical information, currently consisting of a Piano Roll graph and some basic transport widgets, planned to include full musical engraving capabilities using SMUFL fonts.

### SPianoRoll Graph
A native widget that hosts a fully pannable and zoomable canvas that allows comfortably viewing midi data.

## Runtime - BK Music Core
Interactions between metasounds and the unreal world, the purpose of unreal daw is after all to create a dynamic musical session that can is controlled by the game world.

## Runtime - unDAW Metasounds
Interactions between metasounds and the unreal world, the purpose of unreal daw is after all to create a dynamic musical session that can is controlled by the game world.

### Timestamped Wav Player Node
A combination of the Timestamp to Trigger Node and Wav Player Transport nodes that allows fully syncing a wav file to a MIDI clock while giving it a timestamp start time and keeping it fully seekable. 

## Editor - BK Editor Utilities
Editor widget for editing midifiles and creating Unreal DAW musical scenes in the editor, factory classes for mass importing Decent Sampler and SFZ libraries into Fusion Sampler Patches 

### UMidiEditorBase
An editor only wrapper for SPianoRollGraph that provides editor only functionality and allows wrapping the widget in an Editor Utility Widget 

# Current Features - 




# Roadmap Features

# Depenencies & third parties
All third party resources include their original licensing documents in the resources folder for the plugin, these should be included when used as per the specifications of the original licenses 
- [SMUFL Fonts - Bravura](https://github.com/steinbergmedia/bravura)
- [SIMPLE UI icons] (https://github.com/Semantic-Org/UI-Icon)
- https://github.com/steinbergmedia/petaluma


