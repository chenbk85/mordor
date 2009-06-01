#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__
// Copyright (c) 2009 - Decho Corp.

#include <list>
#include <set>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "connection.h"
#include "common/fiber.h"

class Scheduler;

namespace HTTP
{
    class ClientConnection;
    class ClientRequest : public boost::enable_shared_from_this<ClientRequest>, boost::noncopyable
    {
    private:
        friend class ClientConnection;
    public:
        typedef boost::shared_ptr<ClientRequest> ptr;

    private:
        ClientRequest(ClientConnection *conn, const Request &request);

    public:
        Stream *requestStream();
        EntityHeaders &requestTrailer();
        // Multipart *requestMultipart();

        const Response &response();
        Stream *responseStream();
        // Multipart *responseMultipart();
        const EntityHeaders &responseTrailer() const;

    private:
        void ensureResponse();
        void requestDone();
        void responseDone();

    private:
        ClientConnection *m_conn;
        Scheduler *m_scheduler;
        Fiber::ptr m_fiber;
        Request m_request;
        Response m_response;
        EntityHeaders m_requestTrailer, m_responseTrailer;
        bool m_requestDone, m_hasResponse, m_hasTrailer, m_responseDone;
        Stream *m_requestStream, *m_responseStream;
    };

    class ClientConnection : public Connection
    {
    private:
        friend class ClientRequest;
    public:
        ClientConnection(Stream *stream, bool own = true);
        ~ClientConnection();

        ClientRequest::ptr request(Request requestHeaders);
    private:
        void scheduleNextRequest();
        void scheduleNextResponse();

    private:
        boost::mutex m_mutex;
        std::list<ClientRequest::ptr> m_pendingRequests;
        std::list<ClientRequest::ptr>::iterator m_currentRequest;
        std::set<ClientRequest::ptr> m_waitingResponses;

        void invariant() const;
    };
};

#endif