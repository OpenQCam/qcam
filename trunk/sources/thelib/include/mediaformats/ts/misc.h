/*
 * Miscellaneous useful functions.
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the MPEG TS, PS and ES tools.
 *
 * The Initial Developer of the Original Code is Amino Communications Ltd.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
*
 * Contributor(s):
 *   Amino Communications Ltd, Swavesey, Cambridge UK
 *
 * ***** END LICENSE BLOCK *****
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// For the command line utilities
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>    // O_... flags

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#else  // _WIN32
// For the socket handling
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // inet_aton
#include <unistd.h>      // open, close
#endif // _WIN32

#include "compat.h"
#include "misc_fns.h"
#include "es_fns.h"
#include "pes_fns.h"
#include "printing_fns.h"

#define DEBUG_SEEK 1

// ============================================================
// CRC calculation
// ============================================================

static uint32_t crc_table[256];

/*
 * Populate the (internal) CRC table. May safely be called more than once.
 */
static void make_crc_table(void)
{
  int i, j;
  int already_done = 0;
  uint32_t crc;

  if (already_done)
    return;
  else
    already_done = 1;

  for (i = 0; i < 256; i++)
  {
    crc = i << 24;
    for (j = 0; j < 8; j++)
    {
      if (crc & 0x80000000L)
        crc = (crc << 1) ^ 0x04c11db7L;
      else
        crc = ( crc << 1 );
    }
    crc_table[i] = crc;
  }

}

/*
 * Compute CRC32 over a block of data, by table method.
 *
 * Returns a working value, suitable for re-input for further blocks
 *
 * Notes: Input value should be 0xffffffff for the first block,
 *        else return value from previous call (not sure if that
 *        needs complementing before being passed back in).
 */
extern uint32_t crc32_block(uint32_t crc, byte *pData, int blk_len)
{
  static int table_made = FALSE;
  int i, j;

  if (!table_made) make_crc_table();
  
  for (j = 0; j < blk_len; j++)
  {
    i = ((crc >> 24) ^ *pData++) & 0xff;
    crc = (crc << 8) ^ crc_table[i];
  }
  return crc;
}



/*
 * Print out (the last `max`) bytes of a byte array.
 *
 * - `name` is identifying text to start the report with.
 * - `data` is the byte data to print. This may be NULL.
 * - `length` is its length
 * - `max` is the maximum number of bytes to print
 *
 * Prints out::
 *
 *    <name> (<length>): ... b1 b2 b3 b4
 *
 * where no more than `max` bytes are to be printed (and "..." is printed
 * if not all bytes were shown).
 */
extern void print_end_of_data(char *name,
                              byte  data[],
                              int   length,
                              int   max)
{
  int ii;
  if (length == 0)
  {
    fprint_msg("%s (0 bytes)\n",name);
    return;
  }

  fprint_msg("%s (%d byte%s):",name,length,(length==1?"":"s"));
  if (data == NULL)
    print_msg(" <null>");  // Shouldn't happen, but let's be careful.
  else
  {
    if (max < length)
      print_msg(" ...");
    for (ii = (length<max?0:length-max); ii < length; ii++)
      fprint_msg(" %02x",data[ii]);
  }
  print_msg("\n");
}

/*
 * Print out the bottom N bits from a byte
 */
extern void print_bits(int     num_bits,
                       byte    value)
{
  int   ii;
  byte  masks[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
  for (ii = 8-num_bits; ii < 8; ii++)
  {
    fprint_msg("%d",((value & masks[ii]) >> (8-ii-1)));
  }
}

/*
 * Calculate log2 of `x` - for some reason this is missing from <math.h>
 */
extern double log2(double x)
{
  if (x == 2.0)
    return 1.0;
  else
    return log10(x) / log10(2);
}


// ============================================================
// Command line "helpers"
// ============================================================
/*
 * Read in an unsigned integer value, checking for extraneous characters.
 *
 * - `prefix` is an optional prefix for error messages, typically the
 *   name of the program. It may be NULL.
 * - `cmd` is the command switch we're reading for (typically ``argv[ii]``),
 *   which is used in error messages.
 * - `str` is the string to read (typically ``argv[ii+1]``).
 * - `base` is the base to read to. If it is 0, then the user can use
 *   C-style expressions like "0x68" to specify the base on the command line.
 * - `value` is the value read.
 *
 * Returns 0 if all went well, 1 otherwise (in which case a message
 * explaining will have been written to stderr).
 */
extern int unsigned_value(char      *prefix,
                          char      *cmd,
                          char      *arg,
                          int        base,
                          uint32_t  *value)
{
  char  *ptr;
  long val;
  errno = 0;
  val = strtoul(arg,&ptr,base);
  if (errno)
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    if (errno == ERANGE && val == 0)
      fprint_err("String cannot be converted to (long) unsigned integer in %s %s\n",
                 cmd,arg);
    else if (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
      fprint_err("Number is too big (overflows) in %s %s\n",cmd,arg);
    else
      fprint_err("Cannot read number in %s %s (%s)\n",
                 cmd,arg,strerror(errno));
    return 1;
  }
  if (ptr[0] != '\0')
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    if (ptr-arg == 0)
      fprint_err("Argument to %s should be a number, in %s %s\n",
                 cmd,cmd,arg);
    else
      fprint_err("Unexpected characters ('%s') after the %.*s in %s %s\n",
                 ptr,
                 (int)(ptr-arg),arg,
                 cmd,arg);
    return 1;
  }

  *value = val;
  return 0;
}

/*
 * Read in an integer value, checking for extraneous characters.
 *
 * - `prefix` is an optional prefix for error messages, typically the
 *   name of the program. It may be NULL.
 * - `cmd` is the command switch we're reading for (typically ``argv[ii]``),
 *   which is used in error messages.
 * - `str` is the string to read (typically ``argv[ii+1]``).
 * - if `positive` is true, then the number read must be positive (0 or more).
 * - `base` is the base to read to. If it is 0, then the user can use
 *   C-style expressions like "0x68" to specify the base on the command line.
 * - `value` is the value read.
 *
 * Returns 0 if all went well, 1 otherwise (in which case a message
 * explaining will have been written to stderr).
 */
extern int int_value(char *prefix,
                     char *cmd,
                     char *arg,
                     int   positive,
                     int   base,
                     int  *value)
{
  char  *ptr;
  long   val;
  errno = 0;
  val = strtol(arg,&ptr,base);
  if (errno)
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    if (errno == ERANGE && val == 0)
      fprint_err("String cannot be converted to (long) integer in %s %s\n",
                 cmd,arg);
    else if (errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
      fprint_err("Number is too big (overflows) in %s %s\n",cmd,arg);
    else
      fprint_err("Cannot read number in %s %s (%s)\n",
                 cmd,arg,strerror(errno));
    return 1;
  }
  if (ptr[0] != '\0')
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    if (ptr-arg == 0)
      fprint_err("Argument to %s should be a number, in %s %s\n",
                 cmd,cmd,arg);
    else
      fprint_err("Unexpected characters ('%s') after the %.*s in %s %s\n",
                 ptr,
                 (int)(ptr-arg),arg,
                 cmd,arg);
    return 1;
  }

  if (val > INT_MAX || val < INT_MIN)
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    fprint_err("Value %ld (in %s %s) is too large (to fit into 'int')\n",
               val,cmd,arg);
    return 1;
  }

  if (positive && val < 0)
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    fprint_err("Value %ld (in %s %s) is less than zero\n",
               val,cmd,arg);
    return 1;
  }

  *value = val;
  return 0;
}

/*
 * Read in an integer value, checking for extraneous characters and a range.
 *
 * - `prefix` is an optional prefix for error messages, typically the
 *   name of the program. It may be NULL.
 * - `cmd` is the command switch we're reading for (typically ``argv[ii]``),
 *   which is used in error messages.
 * - `str` is the string to read (typically ``argv[ii+1]``).
 * - `minimum` is the minimum value allowed.
 * - `maximum` is the maximum value allowed.
 * - `base` is the base to read to. If it is 0, then the user can use
 *   C-style expressions like "0x68" to specify the base on the command line.
 * - `value` is the value read.
 *
 * Returns 0 if all went well, 1 otherwise (in which case a message
 * explaining will have been written to stderr).
 */
extern int int_value_in_range(char *prefix,
                              char *cmd,
                              char *arg,
                              int   minimum,
                              int   maximum,
                              int   base,
                              int  *value)
{
  int err, temp;
  err = int_value(prefix,cmd,arg,(minimum >= 0),base,&temp);
  if (err) return err;

  if (temp > maximum || temp < minimum)
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    fprint_err("Value %d (in %s %s) is not in range %d..%d (0x%x..0x%x)\n",
               temp,cmd,arg,minimum,maximum,minimum,maximum);
    return 1;
  }
  *value = temp;
  return 0;
}

/*
 * Read in a double value, checking for extraneous characters.
 *
 * - `prefix` is an optional prefix for error messages, typically the
 *   name of the program. It may be NULL.
 * - `cmd` is the command switch we're reading for (typically ``argv[ii]``),
 *   which is used in error messages.
 * - `str` is the string to read (typically ``argv[ii+1]``).
 * - if `positive` is true, then the number read must be positive (0 or more).
 * - `value` is the value read.
 *
 * Returns 0 if all went well, 1 otherwise (in which case a message
 * explaining will have been written to stderr).
 */
extern int double_value(char   *prefix,
                        char   *cmd,
                        char   *arg,
                        int     positive,
                        double *value)
{
  char    *ptr;
  double   val;
  errno = 0;
  val = strtod(arg,&ptr);
  if (errno)
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    if (errno == ERANGE && val == 0)
      fprint_err("String cannot be converted to (double) float in %s %s\n",
                 cmd,arg);
    else if (errno == ERANGE && (val == HUGE_VAL || val == -HUGE_VAL))
      fprint_err("Number is too big (overflows) in %s %s\n",cmd,arg);
    else
      fprint_err("Cannot read number in %s %s (%s)\n",
                 cmd,arg,strerror(errno));
    return 1;
  }
  if (ptr[0] != '\0')
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    fprint_err("Unexpected characters ('%s') after the %.*s in %s %s\n",
               ptr,
               (int)(ptr-arg),arg,
               cmd,arg);
    return 1;
  }

  if (positive && val < 0)
  {
    print_err("### ");
    if (prefix != NULL)
      fprint_err("%s: ",prefix);
    fprint_err("Value %f (in %s %s) is less than zero\n",
               val,cmd,arg);
    return 1;
  }

  *value = val;
  return 0;
}

/*
 * Read in a hostname and (optional) port
 *
 * - `prefix` is an optional prefix for error messages, typically the
 *   name of the program. It may be NULL.
 * - `cmd` is the command switch we're reading for (typically ``argv[ii]``),
 *   which is used in error messages. It may be NULL if we are reading a
 *   "plain" host name, with no command switch in front of it.
 * - `arg` is the string to read (typically ``argv[ii+1]``).
 * - `hostname` is the host name read
 * - `port` is the port read (note that this is not touched if there is
 *   no port number, so it may be set to a default before calling this
 *   function)
 *
 * Note that this works by pointing `hostname` to the start of the `arg`
 * string, and then if there is a ':' in `arg`, changing that colon to
 * a '\0' delimiter, and interpreting the string thereafter as the port
 * number. If *that* fails, it resets the '\0' as a ':'.
 *
 * Returns 0 if all went well, 1 otherwise (in which case a message
 * explaining will have been written to stderr).
 */
extern int host_value(char  *prefix,
                      char  *cmd,
                      char  *arg,
                      char **hostname,
                      int   *port)
{
  char *p = strchr(arg,':');

  *hostname = arg;

  if (p != NULL)
  {
    char *ptr;
    p[0] = '\0';  // yep, modifying argv[ii+1]
    errno = 0;
    *port = strtol(p+1,&ptr,10);
    if (errno)
    {
      p[0] = ':';
      print_err("### ");
      if (prefix != NULL)
        fprint_err("%s: ",prefix);
      if (cmd)
        fprint_err("Cannot read port number in %s %s (%s)\n",
                   cmd,arg,strerror(errno));
      else
        fprint_err("Cannot read port number in %s (%s)\n",
                   arg,strerror(errno));
      return 1;
    }
    if (ptr[0] != '\0')
    {
      p[0] = ':';
      print_err("### ");
      if (prefix != NULL)
        fprint_err("%s: ",prefix);
      if (cmd)
        fprint_err("Unexpected characters in port number in %s %s\n",
                   cmd,arg);
      else
        fprint_err("Unexpected characters in port number in %s\n",arg);
      return 1;
    }
    if (*port < 0)
    {
      p[0] = ':';
      print_err("### ");
      if (prefix != NULL)
        fprint_err("%s: ",prefix);
      if (cmd)
        fprint_err("Negative port number in %s %s\n",cmd,arg);
      else
        fprint_err("Negative port number in %s\n",arg);
      return 1;
    }
  }
  return 0;
}

#ifdef _WIN32
// ============================================================
// WINDOWS32 specific socket stuff
// ============================================================

/*
 * Start up WINSOCK so we can use sockets.
 *
 * Note that each successful call of this *must* be matched by a call
 * of winsock_cleanup().
 *
 * Returns 0 if it works, 1 if it fails.
 */
extern int winsock_startup(void)
{
  // The code herein is borrowed from the example in the Windows Sockets
  // Version 2: Platform DSK documentation for WSAStartup.
  WORD    wVersionRequested;
  WSADATA wsaData;
  int     err;
 
  wVersionRequested = MAKEWORD(2,2);
 
  err = WSAStartup(wVersionRequested,&wsaData);
  if (err != 0)
  {
    // We could not find a usable WinSock DLL
    print_err("### Unable to find a usable WinSock DLL\n");
    return 1;
  }
 
  // Confirm that the WinSock DLL supports 2.2.
  // Note that if the DLL supports versions greater than 2.2 in addition to
  // 2.2, it will still return 2.2 in wVersion since that is the version we
  // requested.
   if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 )
  {
    fprint_err("### WinSock DLL was version %d.%d, not 2.2 or more\n",
               LOBYTE(wsaData.wVersion),HIBYTE(wsaData.wVersion));
    WSACleanup();
    return 1;
  }
  return 0;
}

/*
 * Convert a WinSock error number into a string and print it out on stderr
 */
extern void print_winsock_err(int err)
{
  switch (err)
  {
  case WSABASEERR:
    print_err("(WSABASEERR) No Error");
    break;

  case WSAEINTR:
    print_err("(WSAEINTR) Interrupted system call");
    break;

  case WSAEBADF:
    print_err("(WSAEBADF) Bad file number");
    break;

  case WSAEACCES:
    print_err("(WSAEACCES) Permission denied");
    break;

  case WSAEFAULT:
    print_err("(WSAEFAULT) Bad address");
    break;

  case WSAEINVAL:
    print_err("(WSAEINVAL) Invalid argument");
    break;

  case WSAEMFILE:
    print_err("(WSAEMFILE) Too many open files");
    break;

  case WSAEWOULDBLOCK:
    print_err("(WSAEWOULDBLOCK) Operation would block");
    break;

  case WSAEINPROGRESS:
    print_err("(WSAEINPROGRESS) A transaction is still in progress");
    break;

  case WSAEALREADY:
    print_err("(WSAEALREADY) Operation already in progress");
    break;

  case WSAENOTSOCK:
    print_err("(WSAENOTSOCK) Socket operation on non-socket");
    break;

  case WSAEDESTADDRREQ:
    print_err("(WSAEDESTADDRREQ) Destination address required");
    break;

  case WSAEMSGSIZE:
    print_err("(WSAEMSGSIZE) Message too long");
    break;

  case WSAEPROTOTYPE:
    print_err("(WSAEPROTOTYPE) Protocol wrong type for socket");
    break;

  case WSAENOPROTOOPT:
    print_err("(WSAENOPROTOOPT) Bad protocol option");
    break;

  case WSAEPROTONOSUPPORT:
    print_err("(WSAEPROTONOSUPPORT) Protocol not supported");
    break;

  case WSAESOCKTNOSUPPORT:
    print_err("(WSAESOCKTNOSUPPORT) Socket type not supported");
    break;

  case WSAEOPNOTSUPP:
    print_err("(WSAEOPNOTSUPP) Operation not supported on socket");
    break;

  case WSAEPFNOSUPPORT:
    print_err("(WSAEPFNOSUPPORT) Protocol family not supported");
    break;

  case WSAEAFNOSUPPORT:
    print_err("(WSAEAFNOSUPPORT) Address family not supported by protocol family");
    break;

  case WSAEADDRINUSE:
    print_err("(WSAEADDRINUSE) Address already in use");
    break;

  case WSAEADDRNOTAVAIL:
    print_err("(WSAEADDRNOTAVAIL) Can't assign requested address");
    break;

  case WSAENETDOWN:
    print_err("(WSAENETDOWN) Network is down");
    break;

  case WSAENETUNREACH:
    print_err("(WSAENETUNREACH) Network is unreachable");
    break;

  case WSAENETRESET:
    print_err("(WSAENETRESET) Net dropped connection or reset");
    break;

  case WSAECONNABORTED:
    print_err("(WSAECONNABORTED) Software caused connection abort");
    break;

  case WSAECONNRESET:
    print_err("(WSAECONNRESET) Connection reset by peer");
    break;

  case WSAENOBUFS:
    print_err("(WSAENOBUFS) No buffer space available");
    break;

  case WSAEISCONN:
    print_err("(WSAEISCONN) Socket is already connected");
    break;

  case WSAENOTCONN:
    print_err("(WSAENOTCONN) Socket is not connected");
    break;

  case WSAESHUTDOWN:
    print_err("(WSAESHUTDOWN) Can't send after socket shutdown");
    break;

  case WSAETOOMANYREFS:
    print_err("(WSAETOOMANYREFS) Too many references, can't splice");
    break;

  case WSAETIMEDOUT:
    print_err("(WSAETIMEDOUT) Connection timed out");
    break;

  case WSAECONNREFUSED:
    print_err("(WSAECONNREFUSED) Connection refused");
    break;

  case WSAELOOP:
    print_err("(WSAELOOP) Too many levels of symbolic links");
    break;

  case WSAENAMETOOLONG:
    print_err("(WSAENAMETOOLONG) File name too long");
    break;

  case WSAEHOSTDOWN:
    print_err("(WSAEHOSTDOWN) Host is down");
    break;

  case WSAEHOSTUNREACH:
    print_err("(WSAEHOSTUNREACH) No Route to Host");
    break;

  case WSAENOTEMPTY:
    print_err("(WSAENOTEMPTY) Directory not empty");
    break;

  case WSAEPROCLIM:
    print_err("(WSAEPROCLIM) Too many processes");
    break;

  case WSAEUSERS:
    print_err("(WSAEUSERS) Too many users");
    break;

  case WSAEDQUOT:
    print_err("(WSAEDQUOT) Disc Quota Exceeded");
    break;

  case WSAESTALE:
    print_err("(WSAESTALE) Stale NFS file handle");
    break;

  case WSASYSNOTREADY:
    print_err("(WSASYSNOTREADY) Network SubSystem is unavailable");
    break;

  case WSAVERNOTSUPPORTED:
    print_err("(WSAVERNOTSUPPORTED) WINSOCK DLL Version out of range");
    break;

  case WSANOTINITIALISED:
    print_err("(WSANOTINITIALISED) Successful WSASTARTUP not yet performed");
    break;

  case WSAEREMOTE:
    print_err("(WSAEREMOTE) Too many levels of remote in path");
    break;

  case WSAHOST_NOT_FOUND:
    print_err("(WSAHOST_NOT_FOUND) Host not found");
    break;

  case WSATRY_AGAIN:
    print_err("(WSATRY_AGAIN) Non-Authoritative Host not found");
    break;

  case WSANO_RECOVERY:
    print_err("(WSANO_RECOVERY) Non-Recoverable errors: FORMERR, REFUSED, NOTIMP");
    break;

  case WSANO_DATA:
    print_err("(WSANO_DATA) Valid name, no data record of requested type");
    break;
    
  default:
    fprint_err("winsock error %d",err);
    break;
  }
}

/*
 * Clean up WINSOCK after we've used sockets.
 *
 * Returns 0 if it works, 1 if it fails.
 */
static int winsock_cleanup(void)
{
  int     err = WSACleanup();
  if (err != 0)
  {
    err = WSAGetLastError();
    print_err("### Error cleaning up WinSock: ");
    print_winsock_err(err);
    print_err("\n");
    return 1;
  }
  return 0;
}
#endif

// ============================================================
// Socket support
// ============================================================
/*
 * Connect to a socket, to allow us to write to it, using TCP/IP.
 *
 * - `hostname` is the name of the host to connect to
 * - `port` is the port to use
 * - if `use_tcpip`, then a TCP/IP connection will be made, otherwise UDP.
 *   For UDP, multicast TTL will be enabled.
 * - If the destination address (`hostname`) is multicast and `multicast_ifaddr`
 *   is supplied, it is used to select (by IP address) the network interface
 *   on which to send the multicasts.  It may be NULL to use the default,
 *   or for non-multicast cases.
 *
 * A socket connected to via this function must be disconnected from with
 * disconnect_socket().
 *
 *   (This is actually only crucial on Windows, where WinSock must be
 *   neatly shut down, but should also be done on Unix in case future
 *   termination code is added.)
 *
 * Returns a positive integer (the file descriptor for the socket) if it
 * succeeds, or -1 if it fails, in which case it will have complained on
 * stderr.
 */
extern int connect_socket(char *hostname,
                          int   port,
                          int   use_tcpip,
                          char *multicast_ifaddr)
{
#ifdef _WIN32
  SOCKET output;
#else  // _WIN32
  int output;
#endif // _WIN32
  int result;
  struct hostent *hp;
  struct sockaddr_in ipaddr;

#ifdef _WIN32
  int err = winsock_startup();
  if (err) return 1;
#endif  
  
  output = socket(AF_INET, (use_tcpip?SOCK_STREAM:SOCK_DGRAM), 0);
#ifdef _WIN32
  if (output == INVALID_SOCKET)
  {
      err = WSAGetLastError();
      print_err("### Unable to create socket: ");
      print_winsock_err(err);
      print_err("\n");
    return -1;
  }
#else  // _WIN32      
  if (output == -1)
  {
    fprint_err("### Unable to create socket: %s\n",strerror(errno));
    return -1;
  }
#endif // _WIN32

#if _WIN32
  // On Windows, apparently, gethostbyname will not work for numeric IP addresses.
  // The clever solution would be to move to using getaddrinfo for all forms of
  // host address, but the simpler solution is just to do:
  {
    unsigned long addr = inet_addr(hostname);
    if (addr != INADDR_NONE) // i.e., success
    {
      ipaddr.sin_addr.s_addr = addr;
      ipaddr.sin_family = AF_INET;
    }
    else
    {
      hp = gethostbyname(hostname);
      if (hp == NULL)
      {
        err = WSAGetLastError();
        fprint_err("### Unable to resolve host %s: ",hostname);
        print_winsock_err(err);
        print_err("\n");
        return -1;
      }
      memcpy(&ipaddr.sin_addr.s_addr, hp->h_addr, hp->h_length);
      ipaddr.sin_family = hp->h_addrtype;
    }
  }
  ipaddr.sin_port = htons(port);
#else  // _WIN32
  hp = gethostbyname(hostname);
  if (hp == NULL)
  {
    fprint_err("### Unable to resolve host %s: %s\n",
            hostname,hstrerror(h_errno));
    return -1;
  }
  memcpy(&ipaddr.sin_addr.s_addr, hp->h_addr, hp->h_length);
  ipaddr.sin_family = hp->h_addrtype;
#if !defined(__linux__)
  // On BSD, the length is defined in the datastructure
  ipaddr.sin_len = sizeof(struct sockaddr_in);
#endif // __linux__
  ipaddr.sin_port = htons(port);
#endif // _WIN32

  if (IN_CLASSD(ntohl(ipaddr.sin_addr.s_addr)))
  {
    // Needed if we're doing multicast
    byte ttl = 16;
    result = setsockopt(output, IPPROTO_IP, IP_MULTICAST_TTL,
                        (char *)&ttl, sizeof(ttl));
#ifdef _WIN32
    if (result == SOCKET_ERROR)
    {
      err = WSAGetLastError();
      print_err("### Error setting socket for IP_MULTICAST_TTL: ");
      print_winsock_err(err);
      print_err("\n");
      return -1;
    }
#else // _WIN32
    if (result < 0)
    {    
      fprint_err("### Error setting socket for IP_MULTICAST_TTL: %s\n",
                 strerror(errno));
      return -1;
    }
#endif // _WIN32

    if (multicast_ifaddr)
    {
#ifdef _WIN32
      unsigned long addr;
      print_err("!!! Specifying the multicast interface is not supported on "
                "some versions of Windows\n");
      // Also, choosing an invalid address is not (may not be) detected on Windows
      addr = inet_addr(multicast_ifaddr);
      if (addr == INADDR_NONE)
      {
        err = WSAGetLastError();
        fprint_err("### Error translating '%s' as a dotted IP address: ",
                   multicast_ifaddr);
        print_winsock_err(err);
        print_err("\n");
        return -1;
      }
#else  // _WIN32
      struct in_addr addr;
      inet_aton(multicast_ifaddr, &addr);
#endif // _WIN32
      result = setsockopt(output,IPPROTO_IP,IP_MULTICAST_IF,
		         (char *)&addr,sizeof(addr));
#ifdef _WIN32
      if (result == SOCKET_ERROR)
      {
        err = WSAGetLastError();
        fprint_err("### Unable to set multicast interface %s: ");
        print_winsock_err(err);
        print_err("\n");
        return -1;
      }
#else // _WIN32      
      if (result < 0)
      {
        fprint_err("### Unable to set multicast interface %s: %s\n",
                   multicast_ifaddr,strerror(errno));
        return -1;
      }
#endif // _WIN32
    }
  }

  result = connect(output,(struct sockaddr*)&ipaddr,sizeof(ipaddr));
#ifdef _WIN32
  if (result == SOCKET_ERROR)
  {
      err = WSAGetLastError();
      fprint_err("### Unable to connect to host %s: ",hostname);
      print_winsock_err(err);
      print_err("\n");
    return -1;
  }
#else  // _WIN32      
  if (result < 0)
  {
    fprint_err("### Unable to connect to host %s: %s\n",
               hostname,strerror(errno));
    return -1;
  }
#endif // _WIN32
  return output;
}

/*
 * Disconnect from a socket (close it).
 *
 * Returns 0 if all goes well, 1 otherwise.
 */
#ifdef _WIN32
extern int disconnect_socket(SOCKET  socket)
{
  int err = closesocket(socket);
  if (err != 0)
  {
    err = WSAGetLastError();
    print_err("### Error closing output: ");
    print_winsock_err(err);
    print_err("\n");
    return 1;
  }

  err = winsock_cleanup();
  if (err) return 1;
  return 0;
}
#else  // _WIN32
extern int disconnect_socket(int  socket)
{
  int err = close(socket);
  if (err == EOF)
  {
    fprint_err("### Error closing output: %s\n",strerror(errno));
    return 1;
  }
  return 0;
}
#endif // _WIN32

const char *ipv4_addr_to_string(const uint32_t addr)
{
  static char buf[64];

  snprintf(buf, sizeof(buf), "%d.%d.%d.%d",
	  (addr >> 24)&0xff,
	  (addr >> 16)&0xff,
	  (addr >> 8)&0xff,
	  (addr & 0xff));
  return buf;
}

int ipv4_string_to_addr(uint32_t *dest, const char *string)
{
  char *str_cpy = strdup(string);
  int rv  =0;
  char *p, *p2;
  int val;
  int nr;
  uint32_t out = 0;
  
  for (nr = 0,p = str_cpy; nr < 4 && *p; p = p2+1, ++nr)
    {
      char *px = NULL;
      p2 = strchr(p, '.');
      if (p2)
	{
          *p2 = '\0';
	}
      val = strtoul(p, &px, 0);
      if (px && *px)
        {
          return -1;
        }
      out |= (val << ((3-nr)<<3));
    }

  (*dest) = out;
  free(str_cpy);
  return rv;

}


// Local Variables:
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 2
// End:
// vim: set tabstop=8 shiftwidth=2 expandtab:
