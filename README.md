# OpenQCam

OpenQCam is a streaming server based on crtmpserver. It receive H.264 video stream from QIC1832 encoding camera and deliver to internet by streaming protocols such as RTSP and MJPG over HTTP.

# Usage

### Broadcast video stream

	qcam /etc/qcam/streamer.lua

### Receive and playback video stream

Assume device IP is 192.168.1.1:

	vlc rtsp://192.168.1.1/v2/video/avmuxstream
	vlc http://192.168.1.1:8888/v2/video/mjpg/mjpgstream

