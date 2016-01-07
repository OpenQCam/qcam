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
          port=1935,
          protocol="inboundRtmp"
        },
      }
    },

    {
      description="QIC Streamer Application",
      name="qicstreamer",
      protocol="dynamiclinklibrary",
      mediaFolder="/media/Storage",
      threads=
      {
        numOfRecordThreads=1,
        numOfCommandThreads=2,
        numOfIVAThreads=3,
      },
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
          port=12345,
          socketPath="/tmp/sock.record",
          protocol='udsRecord',
        },
        {
          ip="0.0.0.0",
          port=12346,
          socketPath="/tmp/sock.command",
          protocol='udsCommand',
        },
        {
          ip="0.0.0.0",
          port=12347,
          socketPath="/tmp/sock.iva",
          protocol='udsIVA',
        },


        --[[
        {
          ip="0.0.0.0",
          port=12345,
          socketPath="/tmp/sock.system",
          protocol='unixSocket'
        },
        {
          ip="0.0.0.0",
          port=12346,
          socketPath="/tmp/sock.cvr",
          protocol='unixSocket'
        },
        {
          ip="0.0.0.0",
          port=12347,
          socketPath="/tmp/sock.info",
          protocol='unixSocket'
        },
        {
          ip="0.0.0.0",
          port=12348,
          socketPath="/tmp/sock.event",
          protocol='unixSocket'
        },
        --]]
        {
          ip="0.0.0.0",
          port=12349,
          socketPath="/tmp/sock.ipc.event",
          protocol='unixSocket'
        },
        {
          ip="0.0.0.0",
          port=444,
          protocol="inboundRestCli",
        },
        {
          ip="0.0.0.0",
          port=445,
          protocol="inboundSSLRestCli",
          sslKey="./applications/qicstreamer/server.key",
          sslCert="./applications/qicstreamer/server.crt",
        },
        {
          ip="0.0.0.0",
          port=446,
          protocol="inboundRestCli",
          --allowed api for this port
          api = "/v2/device/system/timestamp",
        },
        {
          ip="0.0.0.0",
          port=9003,
          protocol="inboundWSHttpConnect",
        },
--        {
--          ip="0.0.0.0",
--          port=9001,
--          protocol="inboundWSSHttpConnect",
--          sslKey="./applications/qicstreamer/server.key",
--          sslCert="./applications/qicstreamer/server.crt",
--        },
        {
          ip="0.0.0.0",
          port=8888,
          protocol="inboundAVHttpStream",
          crossDomainFile="/tmp/crossdomain.xml"
        },

      },

      authentication=
      {
        --rtsp={
        --  usersFile="users.lua",
        --},
        --rtmp={
        --  type="adobe",
        --  encoderAgents={
        --    "FMLE/3.0 (compatible; FMSc/1.0)",
        --  },
        --  usersFile="users.lua",
        --},
      },

      defaultConfig=
      {
        cloud = {
          {
            name = "camReg",
            isreg = true,
          },
          {
            name = "service",
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
      motion={
        deviceNode="/dev/input/event11"
      },
    },

    audio={
      microphone={
        deviceNode="/dev/pcm0",
      },
      speaker={
        deviceNode="/dev/i2s0",
      },
    },
    network={
      externalInterface="ra0"
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

