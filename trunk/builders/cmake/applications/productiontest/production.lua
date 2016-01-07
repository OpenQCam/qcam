configuration=
{
  daemon=false,
  pathSeparator="/",
  logAppenders=
  {
    {
      name="console appender",
      type="coloredConsole",
      level=6
    },
    {
      name="file appender",
      type="file",
      level=6,
      fileName="/var/log/icamera",
      fileHistorySize=7,
      fileLength=1024*256,
      singleLine=true
    }
  },
  applications=
  {
    rootDirectory="applications",
    --this is where the applications array starts
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
          port=1936,
          protocol="inboundRtmp"
        },
      }
    },

    {
      description="iCamera Production Test Application",
      name="productiontest",
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
          port=555,
          protocol="inboundRtsp"
        },
        {
          ip="0.0.0.0",
          port=445,
          protocol="inboundRestCli",
        },
        {
          ip="0.0.0.0",
          port=9001,
          protocol="inboundWSHttpConnect",
        },
        {
          ip="0.0.0.0",
          port=8889,
          protocol="inboundAVHttpStream",
          crossDomainFile="/tmp/crossdomain.xml"
        },
        --[[
        {
          ip="0.0.0.0",
          port=12345,
          socketPath="/tmp/sock.system",
          protocol='unixSocket'
        },
        --]]

      },

      --[[
      authentication=
      {
        rtsp={
          usersFile="users.lua"
        }
      },
      --]]

      defaultConfig=
      {
        cloud = {
          {
            name = "camerareg_ws",
            isreg = true,
          },
          {
            name = "service_ws",
          },
          {
            name = "systeminfo",
          },
        },

        wsOutConfig= {
          -- msg=0, video=1, audio=2,
          {
            name = "service_ws",
            apptype = 0,
            outstream = "OMCLI",
            outstreamName="syscmdstream",
          },
          {
            name = "systeminfo",
            apptype = 0,
            instream = "IMSYS",
            instreamName = "sysinfostream"
          },
          {
            name = "pushmjpg",
            apptype = 1,
            instream = "ICVJ",
            instreamName = "mjpgstream"
          },
          {
            name = "pushwave",
            apptype = 2,
            instream = "ICAP",
            instreamName = "wavestream"
          },
        },


        wsInConfig= {
          -- msg=0, video=1, audio=2,
          {
            name = "systeminfo",
            apptype = 0,
            instream = "IMSYS",
            instreamName = "sysinfostream"
          },

          {
            name = "clicmd",
            apptype = 0,
            outstream = "OMCLI",
            outstreamName = "syscmdstream"
          },
          {
            name = "pushwave",
            apptype = 2,
            instream = "ICAP",
            instreamName = "wavestream"
          },
          {
            name = "pushmjpg",
            apptype = 1,
            instream = "ICVJ",
            instreamName = "mjpgstream"
          },
        },
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
  }, --applications

  -- control the hardware manager behavior
  -- [enable] enumerate the hardware device or not
  hardware=
  {

    video={
      avc={
        deviceNode="/dev/video1",
        enable=true,
        start=true,
        mode="avc",
        resolution="1280x720",
        fps="30",
        bitrate="2000000",
        gop="30"
      },
      mjpg={
        deviceNode="/dev/video0",
        enable=true,
        start=true,
        mode="mjpg",
        resolution="1280x720"
      },
    },

    audio={
      microphone={
        deviceNode="/dev/pcm0",
      },
      --speaker={
      --  deviceNode="/dev/i2s0",
      --},
    },
    network={
      externalInterface="br0"
    },
    io={
      pir={
        enable=true
      },
      als={
        enable=false
      },
      dhcp={
        enable=true
      },
    },
    nvram={
      -- nvram simulation for X86 platform
      path="./nvram_test.xml",
      -- nvram section for cl1830
      section="2860"
    },
    system={
      -- for X86 platform
      scriptPath="scripts"
      -- for cl1830 platform
      --scriptPath="/sbin/scripts"
    },
    storage= {
      mountPath= "/"
    };
  }, --hardware
} --configuration

