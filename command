1/
gst-launch filesrc location=video.ogv ! oggdemux name=demux demux. ! queue ! theoradec ! autovideosink demux. ! queue ! vorbisdec ! autoaudiosink 

2/
gst-launch filesrc location=video.ogv ! oggdemux name=demux demux. ! queue ! theoradec ! autovideosink filesrc location=cranberries.mp3 ! queue ! mad  ! autoaudiosink

3/
gst-launch subtitleoverlay name=overlay ! autovideosink  filesrc location=video.ogv ! oggdemux name=demux demux. ! queue ! theoradec ! overlay. demux. ! queue ! vorbisdec ! autoaudiosink filesrc location=video.srt ! subparse subtitle-encoding=UTF-8 ! overlay.
4/
gst-launch subtitleoverlay name=overlay ! theoraenc ! oggmux name=muxer ! filesink location=video_soustitres_partiel.ogv  filesrc location=video.ogv ! oggdemux name=demux demux. ! queue ! theoradec ! overlay. filesrc location=cranberries.mp3 ! queue ! mad ! audioconvert ! vorbisenc ! muxer. filesrc location=video.srt ! subparse subtitle-encoding=UTF-8 ! overlay.

3/ with decodebin
gst-launch subtitleoverlay name=overlay ! autovideosink  filesrc location=video.ogv ! decodebin name=demux demux. ! queue ! overlay. demux. ! queue ! autoaudiosink filesrc location=video.srt ! subparse subtitle-encoding=UTF-8 ! overlay.
