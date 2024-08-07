As UE5.5 fixes some issues related to MIDI nodes and the dynamic builder, and as not a single person has ever reached out to me about currently using unDAW I am switching to ue5-main on the dev branch.

[![Status update + walkthrough](https://img.youtube.com/vi/jQYDYK7mDKQ/0.jpg)](https://www.youtube.com/watch?v=jQYDYK7mDKQ)

# Status

unDAW has a whole bunch of features, most of them in a somewhat unfinished state but some are already useful (like that SFZ/DSpreset converter), given the scope of the project I don't know when if ever it will be 'finished' or 'polished', the source code contains some possibly useful examples for interactions with the new harmonix infrastructure and with the metasound builder subsystem, if you can figure it out you can already use unDAW to create some musical gameplay outcomes which are not so easy to achieve without using the builder and settings things up right (although unDAW may be a bit too complicated in its attempt to be generic), you can easily attach a new metasound patch to an existing midi clock and use it to trigger musically timed events inside the metasound or on the gamethread, for instance, which is not that easy to set up with Quartz (or possible to set up with a static metasound without the builder).

I believe that unDAW is already potentially useful but it will probably take some c++ knowledge and work deciphering my messy codebase, if you have any questions you can join the discord: https://discord.gg/hTKjSfcbEn

it will take many efforts to make unreal into a proper DAW but the way things are set up with metasounds already has some advantages over most modern DAWs, the signal flow is extremely modular and when combined with the new midi capabilities it's possible to create interesting effects chains (which can be controlled via audio parameters or builder manipulation).

# Depenencies & third parties
All third party resources include their original licensing documents in the resources folder for the plugin, these should be included when used as per the specifications of the original licenses 
- [SMUFL Fonts - Bravura](https://github.com/steinbergmedia/bravura)
- [SIMPLE UI icons] (https://github.com/Semantic-Org/UI-Icon)
- https://github.com/steinbergmedia/petaluma
- Using metasound effect patches from https://github.com/JanKXSKI/Concord


