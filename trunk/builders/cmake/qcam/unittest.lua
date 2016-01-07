-- Start of the configuration. This is the only node in the config file. 
-- The rest of them are sub-nodes
configuration=
{
	-- if true, the server will run as a daemon.
	-- NOTE: all console appenders will be ignored if this is a daemon
	daemon=false,
	-- the OS's path separator. Used in composing paths
	pathSeparator="/",

	-- this is the place where all the logging facilities are setted up
	-- you can add/remove any number of locations

	logAppenders=
	{
		{
			-- name of the appender. Not too important, but is mandatory
			name="console appender",
			-- type of the appender. We can have the following values:
			-- console, coloredConsole and file
			-- NOTE: console appenders will be ignored if we run the server
			-- as a daemon
			type="coloredConsole",
			-- the level of logging. 6 is the FINEST message, 0 is FATAL message.
			-- The appender will "catch" all the messages below or equal to this level
			-- bigger the level, more messages are recorded
			level=6
		},
		{
			name="file appender",
			type="file",
			level=6,
			-- the file where the log messages are going to land
			fileName="/tmp/crtmpserver",
			--newLineCharacters="\r\n",
			fileHistorySize=10,
			fileLength=1024*256,
			singleLine=true
		}
	},
	
	-- this node holds all the RTMP applications
	applications=
	{
		-- this is the root directory of all applications
		-- usually this is relative to the binary execuable
		rootDirectory="applications",
		
		
		--this is where the applications array starts
		{
			-- The name of the application. It is mandatory and must be unique 
			name="appselector",
			-- Short description of the application. Optional
			description="Application for selecting the rest of the applications",
			
			-- The type of the application. Possible values are:
			-- dynamiclinklibrary - the application is a shared library
			protocol="dynamiclinklibrary",
			-- the complete path to the library. This is optional. If not provided, 
			-- the server will try to load the library from here
			-- <rootDirectory>/<name>/lib<name>.{so|dll|dylib}
			-- library="/some/path/to/some/shared/library.so"
			
			-- Tells the server to validate the clien's handshake before going further. 
			-- It is optional, defaulted to true
			validateHandshake=false,
			-- this is the folder from where the current application gets it's content.
			-- It is optional. If not specified, it will be defaulted to:
			-- <rootDirectory>/<name>/mediaFolder
			-- mediaFolder="/tmp"
			-- the application will also be known by that names. It is optional
			--aliases=/home/vita/streamer403/trunk/builders/cmake
			--{
			--	"simpleLive",
			--	"vod",
			--	"live",
			--},
			-- This flag designates the default application. The default application
			-- is responsable of analyzing the "connect" request and distribute 
			-- the future connection to the correct application.
			default=true,
			acceptors = 
			{
				{
					ip="0.0.0.0",
					port=1935,
					protocol="inboundRtmp"
				},
        --[[
        {
          ip="0.0.0.0",
          port=8081,
          protocol="inboundRtmps",
          sslKey="server.key",
          sslCert="server.crt"
        },
        {
          ip="0.0.0.0",
          port=8080,
          protocol="inboundRtmpt"
                },
      name="qicstreamer",
        --]]
      }
    },


		{
			description="QCAM Unit Test Application",
			name="qcamunittest",
			protocol="dynamiclinklibrary",
			mediaFolder="/media/Storage",
			aliases=
			{
				"test",
        "live"
      },
      acceptors =
      {
        {
          ip="0.0.0.0",
          port=554,
          protocol="inboundRtsp"
        },
        {
          ip="0.0.0.0",
          port=444,
          protocol="inboundHttpQICCli",
          sslKey="server.key",
          sslCert="server.crt",
        },
        {
          ip="0.0.0.0",
          port=8888,
          protocol="inboundAVHttpStream",
          crossDomainFile="/tmp/crossdomain.xml"
        }
      },

      authentication=
      {
        rtsp={
          usersFile="users.lua"
        }
      }, 

      defaultConfig=
      {
        device={
          pir=true,
          ntpserver="0.tw.pool.ntp.org",
          ptz={
                pen="",
                tilt="",
                zoom=""
              }
        },
        video={
          avc={
                --start=true,
                mode="avc",
                resolution="1280x720",
                fps="30",
                bitrate="2000000",
                gop="30"
              },
          mjpg={
                --start=false,
                mode="mjpg",
                resolution="1280x720"
              }
        },
        audio={
            start=false,
            device="hw:1,0"
        },
        record={
            storagepath="/media/sda1/video",
            pushvideopath="/media/sda1/pushvideo"
        },
        profile={
          p1={
              name="Both",
              avcstatus="on",
              avcresolution="1280x720",
              bitrate="2000000",
              fps="30",
              gop="30",
              mjpgstatus="on",
              mjpgresolution="640x480"
             },
          p2={
              name="AVC Only",
              avcstatus="on",
              avcresolution="1280x720",
              bitrate="2000000",
              fps="25",
              gop="30",
              mjpgstatus="off",
              mjpgresolution=""
             },
          p3={
              name="MJPG Only",
              avcstatus="off",
              avcresolution="",
              bitrate="",
              fps="",
              gop="",
              mjpgstatus="on",
              mjpgresolution="640x480"
             }
        }

        
      },


      externalStreams =
      {
      },
      validateHandshake=false,
      keyframeSeek=true,
      seekGranularity=5, --in seconds, between 0.1 and 600
      clientSideBuffer=12 --in seconds, between 5 and 30
    },

    --[[
    {
      description="QIC Streamer Unit Test",
      name="qamunittest",
      protocol="dynamiclinklibrary",
      mediaFolder="/media/sdb1",
      aliases=
      {
        "unitest"
      },
      acceptors =
      {
        {
          ip="0.0.0.0",
          port=999,
          protocol="inboundRtsp"
        },
        {
          ip="0.0.0.0",
          port=333,
          protocol="inboundHttpQICCli",
          sslKey="server.key",
          sslCert="server.crt",
        },
        {
          ip="0.0.0.0",
          port=7777,
          protocol="inboundAVHttpStream",
          crossDomainFile="/tmp/crossdomain.xml"
        }
      },

      authentication=
      {
        rtsp={
          usersFile="users.lua"
        }
      },
      validateHandshake=false,
      keyframeSeek=true,
      seekGranularity=5, --in seconds, between 0.1 and 600
      clientSideBuffer=12 --in seconds, between 5 and 30
    }
    --]]
    --#INSERTION_MARKER# DO NOT REMOVE THIS. USED BY appscaffold SCRIPT.
  }
}

