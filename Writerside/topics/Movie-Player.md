# Movie Player
A simple application that uses [QMediaPlayer](https://doc.qt.io/qt-6/qmediaplayer.html)
along with *QMediaPlaylist* and
displays in a [QVideoWidget](https://doc.qt.io/qt-6/qvideowidget.html).
I use this primarily to play all media files in a directory tree. Users open a root directory 
of choice, and the application loads all media files in the directory tree. The loaded 
files are played in sequence in a loop until the user closes the application. Unlike 
VLC, MPlayer etc. which spawn new windows to play the media file, our application keeps 
it in the same application (in a separate tab). This allows users to keep the application 
running (usually on a secondary monitor) without grabbing focus and stepping into 
the users regular work.

The application presents two tab, the first “Files” tab shows the list of media files 
that have been loaded. User may double-click any file to switch to playing that file.
Playback occurs in the “Player” tab. User may switch between tabs at any time without 
disrupting the playback.

<img src="video-player-files.png" alt="Files tab" thumbnail="true"/>

<img src="video-player-video.png" alt="Video tab" thumbnail="true"/>

## Controls
Common controls allow the user to quickly switch between files loaded.
* **First** - Use to start playback from the first file in the list
* **Previous** - Jump to the previous file in the list and start playing.
* **Play/Pause** - Use to play/pause the current file. On start, it will play the first file in the list.
* **Next** - Jump to the next file in the list and start playing.
* **Last** - Jump to the last file in the list and start playing.
* **Location Scroll** - Use to scroll to a specific location in the current media file.

Audio controls for audio output are also provided. On exit, the index of the current file 
being played is saved. If the same directory is re-opened later playback will attempt 
to pick up at the same index (note that the actual file being played back may differ 
if the parent directory has been modified via file addition/removal).

## Playback Mode

Two modes are supported:
* **Repeat** - The files in the generated playlist are played back sequentially in a loop.
* **Random** - The files in the generated playlist are played back in random order. Each file is played back only once per loop.
* 
At the end of each loop the playlist is refreshed to account for any filesystem level changes
(files/directories added/removed) for both the modes.

## Filters

It is possible to specify regular expression based filters which are used when
the playlist is generated. Files or patterns representing files you do not wish 
to play may be specified using the *Playback->Ignore...* menu. Ignore 
rules/patterns are global and are not specific to any particular directory tree 
that is loaded in the application.

## Limitations

Currently the applicaion supports only adding one nested directory hierarchy at one time.
You can add additional directories, however at the end of the playlist, only the 
last added directory will be reloaded and then replayed. The application reloads
the playlist at the end of each loop to pick up any newly added files (as well 
as remove any deleted files, pick up renamed files etc). In a future release I
may properly track multiple playlists per loaded directory to avoid this limitation.

I developed this app in a few hours using the easy to use Qt Multimedia module.