/*
 *  Copyright (c) 2010,
 *  Gavriloaie Eugen-Andrei (shiretu@gmail.com)
 *
 *  This file is part of crtmpserver.
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _IOHANDLERTYPE_H
#define _IOHANDLERTYPE_H

typedef enum _IOHandlerType {
  IOHT_ACCEPTOR,
  IOHT_UX_ACCEPTOR, // unix domain socket
  IOHT_UX_CARRIER, // unix domain socket
  IOHT_TCP_CONNECTOR,
  IOHT_TCP_CARRIER,
  IOHT_UDP_CARRIER,
  IOHT_INBOUNDNAMEDPIPE_CARRIER,
  IOHT_TIMER,
  IOHT_STDIO,
  IOHT_QIC_VIDEO,
  IOHT_QIC_AUDIO,
  IOHT_QIC_MOTION,
  IOHT_GPIO,
  IOHT_NVRAM,
  IOHT_STORAGE,
  IOHT_NFS_STORAGE,
  IOHT_CPU,
  IOHT_MEM,
} IOHandlerType;

#endif  /* _IOHANDLERTYPE_H */
