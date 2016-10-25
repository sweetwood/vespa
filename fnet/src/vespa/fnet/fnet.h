// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#pragma once

#include <vespa/fastos/fastos.h>
#include <vespa/vespalib/component/vtag.h>

// FEATURES

#include "features.h"

// DEFINES

#define FNET_NOID ((uint32_t)-1)

// THREAD/MUTEX STUFF

#ifdef FASTOS_NO_THREADS

#define FNET_HAS_THREADS false

class FNET_Mutex
{
public:
    FNET_Mutex(const char *, bool) {}
    void Lock() {}
    void Unlock() {}
};

class FNET_Cond : public FNET_Mutex
{
    bool Illegal(const char *name) {
        fprintf(stderr, "FNET_Cond::%s called (FASTOS_NO_THREADS)\n", name);
        abort();
        return false;
    }
public:
    FNET_Cond(const char *name, bool leaf)
        : FNET_Mutex(name, leaf) {}
    bool TimedWait(int) { return Illegal("TimedWait"); }
    void Wait() { Illegal("Wait"); }
    void Signal() { Illegal("Signal"); }
    void Broadcast() { Illegal("Broadcast"); }
};

#else // FASTOS_NO_THREADS

#define FNET_HAS_THREADS true

typedef FastOS_Mutex FNET_Mutex;
typedef FastOS_Cond  FNET_Cond;

#endif

// DEPRECATED

#define DEPRECATED __attribute__((deprecated))

// FORWARD DECLARATIONS

class FNET_IPacketFactory;
class FNET_IPacketHandler;
class FNET_IPacketStreamer;
class FNET_IServerAdapter;
class FNET_IExecutable;

class FNET_Channel;
class FNET_ChannelLookup;
class FNET_ChannelPool;
class FNET_Config;
class FNET_Connection;
class FNET_Connector;
class FNET_Context;
class FNET_ControlPacket;
class FNET_DataBuffer;
class FNET_DummyPacket;
class FNET_FDSelector;
class FNET_Info;
class FNET_IOComponent;
class FNET_Packet;
class FNET_PacketQueue;
class FNET_Scheduler;
class FNET_SimplePacketStreamer;
class FNET_StatCounters;
class FNET_Stats;
class FNET_Task;
class FNET_Transport;
class FNET_TransportThread;


// CONTEXT CLASS (union of types)
#include "context.h"

// INTERFACES
#include "ipacketfactory.h"
#include "ipackethandler.h"
#include "ipacketstreamer.h"
#include "iserveradapter.h"
#include "iexecutable.h"

// CLASSES
#include "task.h"
#include "scheduler.h"
#include "config.h"
#include "stats.h"
#include "databuffer.h"
#include "packet.h"
#include "dummypacket.h"
#include "controlpacket.h"
#include "packetqueue.h"
#include "channel.h"
#include "channellookup.h"
#include "simplepacketstreamer.h"
#include "transport_thread.h"
#include "iocomponent.h"
#include "transport.h"
#include "connection.h"
#include "connector.h"
#include "fdselector.h"
#include "info.h"
#include "signalshutdown.h"


#define ASSERT_OBJECT(pt)                                               \
    do {                                                                \
        if (pt == NULL || !pt->CheckObject()) {                         \
            fprintf(stderr, "%s:%d: ASSERT_OBJECT FAILED!\n", __FILE__, __LINE__); \
            abort();                                                    \
        }                                                               \
    } while (false)

#define ASSERT_OBJECT_NOLOCK(pt)                                        \
    do {                                                                \
        if (pt == NULL || !pt->CheckObject_NoLock()) {                  \
            fprintf(stderr, "%s:%d: ASSERT_OBJECT FAILED!\n", __FILE__, __LINE__); \
            abort();                                                    \
        }                                                               \
    } while (false)

