configuration=
{
  daemon=false,
  pathSeparator="/",

  logAppenders=
  {
    {
      name="console appender",
      type="coloredConsole",
      level=1
    },
    {
      name="file appender",
      type="file",
      level=1,
      fileName="/tmp/crtmpserver",
      fileHistorySize=10,
      fileLength=1024*256,
      singleLine=true
    }
  },

  applications=
  {
    rootDirectory="applications",
    {
      name="appselector",
      description="Application for selecting the rest of the applications",
      protocol="dynamiclinklibrary",
      validateHandshake=false,
      default=true,
      acceptors =
      {
        {
          ip="0.0.0.0",
          port=1935,
          protocol="inboundRtmp"
        },
      }
    },

    {
      description="Factory Test Application",
      name="factorytest",
      protocol="dynamiclinklibrary",
      mediaFolder="/media/Storage",
      aliases=
      {
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
        },

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
                bitrate="2000000"
              },
          mjpg={
                --start=false,
                mode="mjpg",
                resolution="640x480"
              }
        },
        audio={
            start=false,
            device="hw:1,0"
        },
        record={
            storagepath="/home/timh/Sites/",
            pushvideopath="/home/timh/Sites/pushvideo/"
        },
        profile={
          p1={
              name="Both",
              avcstatus="on",
              avcresolution="1280x720",
              bitrate="2000000",
              fps="30",
              mjpgstatus="on",
              mjpgresolution="640x480"
             },
          p2={
              name="AVC Only",
              avcstatus="on",
              avcresolution="1280x720",
              bitrate="2000000",
              fps="25",
              mjpgstatus="off",
              mjpgresolution=""
             },
          p3={
              name="MJPG Only",
              avcstatus="off",
              avcresolution="",
              bitrate="",
              fps="",
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
      clientSideBuffer=12, --in seconds, between 5 and 30
    },
    --#INSERTION_MARKER# DO NOT REMOVE THIS. USED BY appscaffold SCRIPT.
  }
}

