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

#include <errno.h>

#include "dcArray.h"
#include "dcError.h"
#include "dcMemory.h"
#include "dcNode.h"
#include "dcSocket.h"
#include "dcString.h"

#define SOCKET_DEBUG(x)

typedef ssize_t (*SocketWriter)(int _socket,
                                const void *_buffer,
                                size_t _length);

static SocketWriter sSocketWriter = write;

void dcSocket_setWriter(SocketWriter _writer)
{
    sSocketWriter = _writer;
}

static const char *sErrorStrings[] =
{
    "SOCKET_NO_STATUS",
    "SOCKET_SUCCESS",
    "SOCKET_SOCKET_FAILURE",
    "SOCKET_HOST_LOOKUP_FAILURE",
    "SOCKET_BIND_FAILURE",
    "SOCKET_LISTEN_FAILURE",
    "SOCKET_ACCEPT_FAILURE",
    "SOCKET_CONNECT_FAILURE",
    "SOCKET_DISCONNECTED_FAILURE",
    "SOCKET_HOSTNAME_FAILURE",
    "SOCKET_PORT_FAILURE",
    "SOCKET_RECEIVE_FAILURE",
    "SOCKET_SEND_FAILURE",
    "SOCKET_INVALID_FAILURE"
};

#define SET_ERROR(error, value)                 \
    if (error)                                  \
    {                                           \
        *error = value;                         \
    }

const char *dcSocket_displayError(dcSocketErrorCode _type)
{
    if (_type < SOCKET_LAST_FAILURE)
    {
        return sErrorStrings[_type];
    }

    return sErrorStrings[SOCKET_LAST_FAILURE];
}

static int getAddrinfoFromHostname(const char *_hostname,
                                   uint16_t _port,
                                   dcSocketType _sockType,
                                   struct sockaddr_storage *_storage,
                                   socklen_t *_storageSize,
                                   dcSocketErrorCode *_errorCode)
{
    struct addrinfo *result = NULL;
    struct addrinfo *i = NULL;
    char portString[20];
    snprintf(portString, sizeof(portString), "%u", _port);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = _sockType;
    memset(_storage, 0, sizeof(struct sockaddr_storage));

    int error = getaddrinfo(_hostname, portString, &hints, &result);
    int sock = -1;

    if (error == 0)
    {
        for (i = result; i != NULL; i = i->ai_next)
        {
            sock = socket(i->ai_family, i->ai_socktype, i->ai_protocol);

            if (sock >= 0)
            {
                // set the port //
                if (i->ai_family == AF_INET)
                {
                    ((struct sockaddr_in*)i)->sin_port = htons(_port);
                }
                else if (i->ai_family == AF_INET6)
                {
                    ((struct sockaddr_in6*)i)->sin6_port = htons(_port);
                }
                else
                {
                    dcError("unable to set port on unknown ai_family");
                }

                memcpy(_storage, i->ai_addr, i->ai_addrlen);
                *_storageSize = i->ai_addrlen;
                break;
            }
        }
    }
    else
    {
        // failure
        SET_ERROR(_errorCode, SOCKET_HOST_LOOKUP_FAILURE);
    }

    if (result != NULL)
    {
        freeaddrinfo(result);
    }

    return sock;
}

dcSocket *dcSocket_create(dcSocketType _type)
{
    dcSocket *sock = (dcSocket *)(dcMemory_allocateAndInitialize
                                  (sizeof(dcSocket)));
    sock->connected = false;
    sock->type = _type;
    return sock;
}

dcNode *dcSocket_createNode(dcSocketType _type)
{
    return dcNode_createWithGuts(NODE_SOCKET, dcSocket_create(_type));
}

dcNode *dcSocket_createShell(dcSocket *_socket)
{
    return dcNode_createWithGuts(NODE_SOCKET, _socket);
}

bool dcSocket_setData(dcSocket *_socket,
                      const char *_hostname,
                      uint16_t _port,
                      dcSocketErrorCode *_errorCode)
{
    socklen_t size;
    bool result = false;
    struct sockaddr_storage storage = {0};
    int sock = getAddrinfoFromHostname(_hostname,
                                       _port,
                                       _socket->type,
                                       &storage,
                                       &size,
                                       _errorCode);
    if (sock >= 0)
    {
        result = true;
        _socket->socket = sock;
        dcSocket_setHostname(_socket, _hostname);
        _socket->port = _port;
        _socket->sockaddr = storage;
        _socket->sockaddrSize = size;

        if (_socket->type == SOCKET_UDP)
        {
            _socket->connected = true;
        }
    }

    return result;
}

void dcSocket_free(dcSocket **_socket, dcDepth _depth)
{
    dcSocket *sock = *_socket;
    dcSocket_close(sock);

    if (sock->hostname)
    {
        dcMemory_free(sock->hostname);
    }

    dcMemory_free(*_socket);
    *_socket = NULL;
}

void dcSocket_freeNode(dcNode *_socketNode, dcDepth _depth)
{
    dcSocket_free(&(CAST_SOCKET(_socketNode)), _depth);
}

dcSocket *dcSocket_copy(const dcSocket *_fromSocket, dcDepth _depth)
{
    dcSocket *result = dcSocket_create(_fromSocket->type);
    memcpy(result, _fromSocket, sizeof(dcSocket));
    return result;
}

void dcSocket_copyNode(dcNode *_to, const dcNode *_from, dcDepth _depth)
{
    CAST_SOCKET(_to) = dcSocket_copy(CAST_SOCKET(_from), _depth);
}

void dcSocket_setHostname(dcSocket *_socket, const char *_hostname)
{
    if (_socket->hostname != NULL)
    {
        dcMemory_free(_socket->hostname);
    }

    _socket->hostname = dcMemory_strdup(_hostname);
}

bool dcSocket_close(dcSocket *_socket)
{
    if (_socket->connected)
    {
        close(_socket->socket);
        _socket->connected = false;
    }

    return true;
}

bool dcSocket_connect(dcSocket *_socket,
                      const char *_hostname,
                      uint16_t _port,
                      dcSocketErrorCode *_errorCode)
{
    SOCKET_DEBUG(char *display = dcSocket_display(_socket);
                 printf("dcSocket_connect::socket: %s\n", display);
                 dcMemory_free(display));

    SET_ERROR(_errorCode, SOCKET_SUCCESS);

    // attempt to lookup the host //
    struct sockaddr_storage result = {0};
    socklen_t resultSize;

    int sock = getAddrinfoFromHostname(_hostname,
                                       _port,
                                       SOCKET_TCP,
                                       &result,
                                       &resultSize,
                                       _errorCode);
    if (sock >= 0)
    {
        if (connect(sock, (struct sockaddr*)&result, resultSize) >= 0)
        {
            _socket->connected = true;
            _socket->socket = sock;
            _socket->sockaddrSize = resultSize;
            _socket->port = _port;
        }
        else
        {
            SET_ERROR(_errorCode, SOCKET_CONNECT_FAILURE);
        }
    }
    // else, error is already set

    return _socket->connected;
}

bool dcSocket_isDisconnected(const dcSocket *_socket)
{
    return (_socket->type == SOCKET_TCP ? !_socket->connected : false);
}

bool dcSocket_isConnected(const dcSocket *_socket)
{
    return (_socket->type == SOCKET_TCP ? _socket->connected : true);
}

dcSocket *dcSocket_connectTo(const char *_hostname,
                             uint16_t _port,
                             dcSocketErrorCode *_errorCode)
{
    dcSocket *result = dcSocket_create(SOCKET_TCP);
    SET_ERROR(_errorCode, SOCKET_SUCCESS);

    if (!dcSocket_connect(result, _hostname, _port, _errorCode))
    {
        SET_ERROR(_errorCode, SOCKET_CONNECT_FAILURE);
        dcSocket_free(&result, DC_DEEP);
        result = NULL;
    }

    return result;
}

#define MAXSOCK 20

bool dcSocket_bind(dcSocket *_socket,
                   uint16_t _port,
                   dcSocketErrorCode *_errorCode)
{
    struct sockaddr_storage storage = {0};
    socklen_t storageSize;
    bool success = false;
    SET_ERROR(_errorCode, SOCKET_BIND_FAILURE);

    int sock = getAddrinfoFromHostname(NULL,
                                       _port,
                                       _socket->type,
                                       &storage,
                                       &storageSize,
                                       _errorCode);
    if (sock >= 0)
    {
        if (bind(sock, (struct sockaddr*)&storage, storageSize) >= 0)
        {
            listen(sock, 5);
            _socket->sockaddr = storage;
            _socket->socket = sock;
            _socket->port = _port;
            SET_ERROR(_errorCode, SOCKET_SUCCESS);
            success = true;
        }
    }

    return success;
}

dcSocket *dcSocket_accept(dcSocket *_socket, dcSocketErrorCode *_errorCode)
{
    SOCKET_DEBUG(printf("dcSocket_listen::socket: %s\n", dcSocket_display(_socket)));
    SOCKET_DEBUG(printf("dcSocket_listen::accept socket: %s\n",
                 dcSocket_display(_socket)));

    dcSocket *result = dcSocket_create(SOCKET_TCP);
    int sock = accept(_socket->socket,
                      (struct sockaddr*)&(result->sockaddr),
                      &_socket->sockaddrSize);
    SET_ERROR(_errorCode, SOCKET_SUCCESS);

    if (sock >= 0)
    {
        result->socket = sock;
        result->connected = true;
        result->type = SOCKET_TCP;
    }
    else
    {
        dcSocket_free(&result, DC_DEEP);
        SET_ERROR(_errorCode, SOCKET_ACCEPT_FAILURE);
    }

    return result;
}

bool dcSocket_put(dcSocket *_socket,
                  const dcString *_what,
                  dcSocketErrorCode *_errorCode)
{
    bool result = false;
    SET_ERROR(_errorCode, SOCKET_SUCCESS);

    if (_socket->type == SOCKET_TCP)
    {
        result = dcSocket_putTcp(_socket, _what, _errorCode);
    }
    else if (_socket->type == SOCKET_UDP)
    {
        result = dcSocket_putUdp(_socket, _what, _errorCode);
    }
    else
    {
        dcError_internal("invalid socket type: %d", _socket->type);
    }

    return result;
}

bool dcSocket_putInt(dcSocket *_socket,
                     int _what,
                     dcSocketErrorCode *_errorCode)
{
    dcError_assert(false);
    dcString *message = dcString_create();
    dcString_appendCharacter(message, _what);
    bool result = dcSocket_putTcp(_socket, message, _errorCode);
    dcString_free(&message, DC_DEEP);
    return result;
}

bool dcSocket_putTcp(dcSocket *_socket,
                     const dcString *_what,
                     dcSocketErrorCode *_errorCode)
{
    bool result = false;

    if (dcSocket_isConnected(_socket))
    {
        if (sSocketWriter(_socket->socket, _what->string, _what->length) < 0)
        {
            // failure, disconnect socket //
            SET_ERROR(_errorCode, SOCKET_SEND_FAILURE);
        }
        else
        {
            result = true;
            SET_ERROR(_errorCode, SOCKET_SUCCESS);
        }
    }
    else
    {
        SET_ERROR(_errorCode, SOCKET_SEND_FAILURE);
    }

    return result;
}

bool dcSocket_putUdp(dcSocket *_socket,
                     const dcString *_what,
                     dcSocketErrorCode *_errorCode)
{
    bool result = true;
    SET_ERROR(_errorCode, SOCKET_SUCCESS);

    if (_socket->socket >= 0)
    {
        ssize_t sendtoLength = sendto(_socket->socket,
                                      _what->string,
                                      _what->length,
                                      0,
                                      (struct sockaddr*)&_socket->sockaddr,
                                      _socket->sockaddrSize);

        if (sendtoLength != (int)_what->length)
        {
            result = false;
            SET_ERROR(_errorCode, SOCKET_SEND_FAILURE);
        }
    }
    // else error is already set

    SOCKET_DEBUG(printf("done sending '%s' on socket: %s\n",
                 _what,
                 dcSocket_display(_socket)));

    return result;
}

bool dcSocket_putCharArray(dcSocket *_socket,
                           const char *_what,
                           dcSocketErrorCode *_errorCode)
{
    dcString toSend;
    toSend.string = (char*)_what;
    toSend.index = 0;
    toSend.length = strlen(_what);
    return dcSocket_putTcp(_socket, &toSend, _errorCode);
}

static dcString *dcSocket_getTcp(dcSocket *_socket,
                                 dcSocketErrorCode *_errorCode)
{
    char *buffer = (char *)dcMemory_allocateAndInitialize(SOCKET_LENGTH);
    dcString *result = NULL;

    if (_socket->connected)
    {
        SOCKET_DEBUG(char *display = dcSocket_display(_socket);
              printf("dcSocket_getTcp::reading from socket: %s\n", display);
              dcMemory_free(display));

        ssize_t readAmount = read(_socket->socket, buffer, SOCKET_LENGTH);

        if (readAmount <= 0)
        {
            // the socket is disconnected //
            _socket->connected = false;
            dcMemory_free(buffer);
            SET_ERROR(_errorCode, SOCKET_DISCONNECTED_FAILURE);
        }
        else
        {
            SET_ERROR(_errorCode, SOCKET_SUCCESS);
            buffer = (char *)realloc(buffer, readAmount + 1);
            buffer[readAmount] = 0;
            result = dcString_createWithString(buffer, false);

            SOCKET_DEBUG(display = dcString_display(result);
                         printf("dcSocket_getTcp::got: %s\n", display);
                         dcMemory_free(display));
        }
    }
    else
    {
        SET_ERROR(_errorCode, SOCKET_RECEIVE_FAILURE);
    }

    return result;
}

dcString *dcSocket_getUdp(dcSocket *_socket,
                          dcSocket *_from,
                          dcSocketErrorCode *_errorCode)
{
    char *buffer = (char *)dcMemory_allocateAndInitialize(SOCKET_LENGTH);
    _from->sockaddrSize = sizeof(_from->sockaddr);
    dcString *result = NULL;
    ssize_t got = recvfrom(_socket->socket,
                           buffer,
                           SOCKET_LENGTH,
                           0,
                           (struct sockaddr*)&(_from->sockaddr),
                           &_from->sockaddrSize);
    if (got < 0)
    {
        dcMemory_free(buffer);
        buffer = NULL;
        SET_ERROR(_errorCode, SOCKET_RECEIVE_FAILURE);
    }
    else
    {
        result = dcString_createWithString(buffer, got);
        _from->socket = _socket->socket;

        if (_from->sockaddr.ss_family == AF_INET)
        {
            _from->port =
                ntohs((((struct sockaddr_in*)&_from->sockaddr)->sin_port));
        }
        else if (_from->sockaddr.ss_family == AF_INET6)
        {
            _from->port =
                ntohs((((struct sockaddr_in6*)&_from->sockaddr)->sin6_port));
        }
        else
        {
            dcError("Invalid sockaddr type");
        }

        SET_ERROR(_errorCode, SOCKET_SUCCESS);
    }

    return result;
}

dcString *dcSocket_get(dcSocket *_socket,
                       dcSocket **_from,
                       dcSocketErrorCode *_errorCode)
{
    dcString *result = NULL;

    if (_socket->type == SOCKET_TCP)
    {
        result = dcSocket_getTcp(_socket, _errorCode);
    }
    else if (_socket->type == SOCKET_UDP)
    {
        *_from = dcSocket_create(SOCKET_UDP);
        result = dcSocket_getUdp(_socket, *_from, _errorCode);
    }
    else
    {
        dcError_internal("invalid socket type: %d", _socket->type);
    }

    return result;
}
