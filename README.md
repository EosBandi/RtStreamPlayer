# RtStreamPlayer
A realtime version of WebEYE StreamPlayer, includes all libraries for recompilling.

If you have to recompile the ffmpeg facade, 
You have to compile a StreamPlayer.dll for Win32 and another for x64. The x64 version should be renamed to StreamPlayer64.dll. Both of them must be added to the resources of the StreamPlayerControl.
