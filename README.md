# mpdq
MusicPlayerDaemon Query

![mpdq](http://i.imgur.com/QSDHzoO.jpg)
Adding traybar icons to control mpd. To make this work with dwm install the [sytray patch](http://dwm.suckless.org/patches/systray).
Use the buttons to:
* Move to the previous song
* Move to the next song
* Play/Pause mpd
* Scroll up/down to increase/decrease the volume

Usage
```
chmod a+x ./mpdq
./mpdq
```
Requires mpd to be running and a playlist to be present.

Currently the config file allows you to change the following things
* Scaling of the tray icons
* Color of the tray icons
* Wether to display the current song in the dwm title bar (Title of the root window)
* Changing the update delay
* Wether to write the current song to a given file
