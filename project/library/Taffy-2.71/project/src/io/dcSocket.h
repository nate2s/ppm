//
// This file is part of Taffy, a mathematical programming language.
// Copyright (C) 2016-2017 Arithmagic, LLC
//
// Taffy is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Taffy is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __DC_SOCKET_H__
#define __DC_SOCKET_H__

#include "dcDefines.h"

#ifdef TAFFY_WINDOWS
struct dcSocket_t
{
};

typedef struct dcSocket_t dcSocket;

// standard functions //
FREE_FUNCTION(dcSocket_freeNode);
COPY_FUNCTION(dcSocket_copyNode);

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include "dcDefines.h"

enum dcSocketType_e
{
    SOCKET_NO_TYPE,
    SOCKET_TCP,
    SOCKET_UDP
};

typedef enum dcSocketType_e dcSocketType;

enum dcSocketErrorCode_e
{
    SOCKET_NO_STATUS,
    SOCKET_SUCCESS,
    SOCKET_SOCKET_FAILURE,
    SOCKET_HOST_LOOKUP_FAILURE,
    SOCKET_BIND_FAILURE,
    SOCKET_LISTEN_FAILURE,
    SOCKET_ACCEPT_FAILURE,
    SOCKET_CONNECT_FAILURE,
    SOCKET_DISCONNECTED_FAILURE,
    SOCKET_HOSTNAME_FAILURE,
    SOCKET_PORT_FAILURE,
    SOCKET_RECEIVE_FAILURE,
    SOCKET_SEND_FAILURE,
    SOCKET_LAST_FAILURE
};

typedef enum dcSocketErrorCode_e dcSocketErrorCode;

//////////////////////
// dcSocketClassAux //
//////////////////////

struct dcSocket_t
{
    int socket;
    uint16_t port;
    dcSocketType type;
    struct sockaddr_storage sockaddr;
    socklen_t sockaddrSize;
    bool connected;
    char *hostname;
};

typedef struct dcSocket_t dcSocket;

// creating //
dcSocket *dcSocket_create(dcSocketType _type);
struct dcNode_t *dcSocket_createNode(dcSocketType _type);
struct dcNode_t *dcSocket_createShell(dcSocket *_socket);

// freeing //
void dcSocket_free(dcSocket **_aux, dcDepth _depth);

// copying /
dcSocket *dcSocket_copy(const dcSocket *_fromAux, dcDepth _depth);

// standard functions //
FREE_FUNCTION(dcSocket_freeNode);
COPY_FUNCTION(dcSocket_copyNode);

// connecting //
bool dcSocket_connect(dcSocket *_socket,
                      const char *_hostname,
                      uint16_t _port,
                      dcSocketErrorCode *_errorCode);

dcSocket *dcSocket_connectTo(const char *_hostname,
                             uint16_t _port,
                             dcSocketErrorCode *_errorCode);

// listening and stuff //
bool dcSocket_bind(dcSocket *_socket,
                   uint16_t _port,
                   dcSocketErrorCode *_errorCode);
dcSocket *dcSocket_accept(dcSocket *_socket, dcSocketErrorCode *_errorCode);

// closing //
bool dcSocket_close(dcSocket *_socket);

// setting //
void dcSocket_setHostname(dcSocket *_socket, const char *_hostname);
bool dcSocket_setData(dcSocket *_socket,
                      const char *_hostname,
                      uint16_t _port,
                      dcSocketErrorCode *_errorCode);

// getting and querying //
bool dcSocket_isConnected(const dcSocket *_socket);
bool dcSocket_isDisconnected(const dcSocket *_socket);

// debugging hooks //
const char *dcSocket_displayError(dcSocketErrorCode _type);

// putting //
bool dcSocket_put(dcSocket *_socket,
                  const struct dcString_t *_what,
                  dcSocketErrorCode *_errorCode);

bool dcSocket_putCharArray(dcSocket *_socket,
                           const char *_what,
                           dcSocketErrorCode *_errorCode);

bool dcSocket_putInt(dcSocket *_socket,
                     int _what,
                     dcSocketErrorCode *_errorCode);

bool dcSocket_putTcp(dcSocket *_socket,
                     const struct dcString_t *_what,
                     dcSocketErrorCode *_errorCode);

bool dcSocket_putUdp(dcSocket *_socket,
                     const struct dcString_t *_what,
                     dcSocketErrorCode *_errorCode);

// getting //
struct dcString_t *dcSocket_get(dcSocket *_socket,
                                dcSocket **_from,
                                dcSocketErrorCode *_errorCode);

struct dcString_t *dcSocket_getUdp(dcSocket *_socket,
                                   dcSocket *_receivedFrom,
                                   dcSocketErrorCode *_errorCode);

#define SOCKET_LENGTH 1024

#endif // TAFFY_WINDOWS

#endif
