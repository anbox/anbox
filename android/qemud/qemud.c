#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#include <cutils/sockets.h>
#include <sys/un.h>

/*
 *  the qemud daemon program is only used within Android as a bridge
 *  between the emulator program and the emulated system. it really works as
 *  a simple stream multiplexer that works as follows:
 *
 *    - qemud is started by init following instructions in
 *      /system/etc/init.goldfish.rc (i.e. it is never started on real devices)
 *
 *    - qemud communicates with the emulator program through a single serial
 *      port, whose name is passed through a kernel boot parameter
 *      (e.g. android.qemud=ttyS1)
 *
 *    - qemud binds one unix local stream socket (/dev/socket/qemud, created
 *      by init through /system/etc/init.goldfish.rc).
 *
 *
 *      emulator <==serial==> qemud <---> /dev/socket/qemud <-+--> client1
 *                                                            |
 *                                                            +--> client2
 *
 *   - the special channel index 0 is used by the emulator and qemud only.
 *     other channel numbers correspond to clients. More specifically,
 *     connection are created like this:
 *
 *     * the client connects to /dev/socket/qemud
 *
 *     * the client sends the service name through the socket, as
 *            <service-name>
 *
 *     * qemud creates a "Client" object internally, assigns it an
 *       internal unique channel number > 0, then sends a connection
 *       initiation request to the emulator (i.e. through channel 0):
 *
 *           connect:<id>:<name>
 *
 *       where <name> is the service name, and <id> is a 2-hexchar
 *       number corresponding to the channel number.
 *
 *     * in case of success, the emulator responds through channel 0
 *       with:
 *
 *           ok:connect:<id>
 *
 *       after this, all messages between the client and the emulator
 *       are passed in pass-through mode.
 *
 *     * if the emulator refuses the service connection, it will
 *       send the following through channel 0:
 *
 *           ko:connect:<id>:reason-for-failure
 *
 *     * If the client closes the connection, qemud sends the following
 *       to the emulator:
 *
 *           disconnect:<id>
 *
 *       The same message is the opposite direction if the emulator
 *       chooses to close the connection.
 *
 *     * any command sent through channel 0 to the emulator that is
 *       not properly recognized will be answered by:
 *
 *           ko:unknown command
 *
 *
 *  Internally, the daemon maintains a "Client" object for each client
 *  connection (i.e. accepting socket connection).
 */

/* name of the single control socket used by the daemon */
#define CONTROL_SOCKET_NAME  "qemud"

#define  DEBUG     0
#define  T_ACTIVE  0  /* set to 1 to dump traffic */

#if DEBUG
#  define LOG_TAG  "qemud"
#  include <cutils/log.h>
#  define  D(...)   ALOGD(__VA_ARGS__)
#else
#  define  D(...)  ((void)0)
#  define  T(...)  ((void)0)
#endif

#if T_ACTIVE
#  define  T(...)   D(__VA_ARGS__)
#else
#  define  T(...)   ((void)0)
#endif

/** UTILITIES
 **/

static void
fatal( const char*  fmt, ... )
{
    va_list  args;
    va_start(args, fmt);
    fprintf(stderr, "PANIC: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n" );
    va_end(args);
    exit(1);
}

static void*
xalloc( size_t   sz )
{
    void*  p;

    if (sz == 0)
        return NULL;

    p = malloc(sz);
    if (p == NULL)
        fatal( "not enough memory" );

    return p;
}

#define  xnew(p)   (p) = xalloc(sizeof(*(p)))

static void*
xalloc0( size_t  sz )
{
    void*  p = xalloc(sz);
    memset( p, 0, sz );
    return p;
}

#define  xnew0(p)   (p) = xalloc0(sizeof(*(p)))

#define  xfree(p)    (free((p)), (p) = NULL)

static void*
xrealloc( void*  block, size_t  size )
{
    void*  p = realloc( block, size );

    if (p == NULL && size > 0)
        fatal( "not enough memory" );

    return p;
}

#define  xrenew(p,count)  (p) = xrealloc((p),sizeof(*(p))*(count))

static int
hex2int( const uint8_t*  data, int  len )
{
    int  result = 0;
    while (len > 0) {
        int       c = *data++;
        unsigned  d;

        result <<= 4;
        do {
            d = (unsigned)(c - '0');
            if (d < 10)
                break;

            d = (unsigned)(c - 'a');
            if (d < 6) {
                d += 10;
                break;
            }

            d = (unsigned)(c - 'A');
            if (d < 6) {
                d += 10;
                break;
            }

            return -1;
        }
        while (0);

        result |= d;
        len    -= 1;
    }
    return  result;
}


static void
int2hex( int  value, uint8_t*  to, int  width )
{
    int  nn = 0;
    static const char hexchars[16] = "0123456789abcdef";

    for ( --width; width >= 0; width--, nn++ ) {
        to[nn] = hexchars[(value >> (width*4)) & 15];
    }
}

static int
fd_read(int  fd, void*  to, int  len)
{
    int  ret;

    do {
        ret = read(fd, to, len);
    } while (ret < 0 && errno == EINTR);

    return ret;
}

static int
fd_write(int  fd, const void*  from, int  len)
{
    int  ret;

    do {
        ret = write(fd, from, len);
    } while (ret < 0 && errno == EINTR);

    return ret;
}

static void
fd_setnonblock(int  fd)
{
    int  ret, flags;

    do {
        flags = fcntl(fd, F_GETFD);
    } while (flags < 0 && errno == EINTR);

    if (flags < 0) {
        fatal( "%s: could not get flags for fd %d: %s",
               __FUNCTION__, fd, strerror(errno) );
    }

    do {
        ret = fcntl(fd, F_SETFD, flags | O_NONBLOCK);
    } while (ret < 0 && errno == EINTR);

    if (ret < 0) {
        fatal( "%s: could not set fd %d to non-blocking: %s",
               __FUNCTION__, fd, strerror(errno) );
    }
}


static int
fd_accept(int  fd)
{
    struct sockaddr  from;
    socklen_t        fromlen = sizeof(from);
    int              ret;

    do {
        ret = accept(fd, &from, &fromlen);
    } while (ret < 0 && errno == EINTR);

    return ret;
}

/** FD EVENT LOOP
 **/

/* A Looper object is used to monitor activity on one or more
 * file descriptors (e.g sockets).
 *
 * - call looper_add() to register a function that will be
 *   called when events happen on the file descriptor.
 *
 * - call looper_enable() or looper_disable() to enable/disable
 *   the set of monitored events for a given file descriptor.
 *
 * - call looper_del() to unregister a file descriptor.
 *   this does *not* close the file descriptor.
 *
 * Note that you can only provide a single function to handle
 * all events related to a given file descriptor.

 * You can call looper_enable/_disable/_del within a function
 * callback.
 */

/* the current implementation uses Linux's epoll facility
 * the event mask we use are simply combinations of EPOLLIN
 * EPOLLOUT, EPOLLHUP and EPOLLERR
 */
#include <sys/epoll.h>

#define  MAX_CHANNELS  16
#define  MAX_EVENTS    (MAX_CHANNELS+1)  /* each channel + the serial fd */

/* the event handler function type, 'user' is a user-specific
 * opaque pointer passed to looper_add().
 */
typedef void (*EventFunc)( void*  user, int  events );

/* bit flags for the LoopHook structure.
 *
 * HOOK_PENDING means that an event happened on the
 * corresponding file descriptor.
 *
 * HOOK_CLOSING is used to delay-close monitored
 * file descriptors.
 */
enum {
    HOOK_PENDING = (1 << 0),
    HOOK_CLOSING = (1 << 1),
};

/* A LoopHook structure is used to monitor a given
 * file descriptor and record its event handler.
 */
typedef struct {
    int        fd;
    int        wanted;  /* events we are monitoring */
    int        events;  /* events that occured */
    int        state;   /* see HOOK_XXX constants */
    void*      ev_user; /* user-provided handler parameter */
    EventFunc  ev_func; /* event handler callback */
} LoopHook;

/* Looper is the main object modeling a looper object
 */
typedef struct {
    int                  epoll_fd;
    int                  num_fds;
    int                  max_fds;
    struct epoll_event*  events;
    LoopHook*            hooks;
} Looper;

/* initialize a looper object */
static void
looper_init( Looper*  l )
{
    l->epoll_fd = epoll_create(4);
    l->num_fds  = 0;
    l->max_fds  = 0;
    l->events   = NULL;
    l->hooks    = NULL;
}

/* finalize a looper object */
static void
looper_done( Looper*  l )
{
    xfree(l->events);
    xfree(l->hooks);
    l->max_fds = 0;
    l->num_fds = 0;

    close(l->epoll_fd);
    l->epoll_fd  = -1;
}

/* return the LoopHook corresponding to a given
 * monitored file descriptor, or NULL if not found
 */
static LoopHook*
looper_find( Looper*  l, int  fd )
{
    LoopHook*  hook = l->hooks;
    LoopHook*  end  = hook + l->num_fds;

    for ( ; hook < end; hook++ ) {
        if (hook->fd == fd)
            return hook;
    }
    return NULL;
}

/* grow the arrays in the looper object */
static void
looper_grow( Looper*  l )
{
    int  old_max = l->max_fds;
    int  new_max = old_max + (old_max >> 1) + 4;
    int  n;

    xrenew( l->events, new_max );
    xrenew( l->hooks,  new_max );
    l->max_fds = new_max;

    /* now change the handles to all events */
    for (n = 0; n < l->num_fds; n++) {
        struct epoll_event ev;
        LoopHook*          hook = l->hooks + n;

        ev.events   = hook->wanted;
        ev.data.ptr = hook;
        epoll_ctl( l->epoll_fd, EPOLL_CTL_MOD, hook->fd, &ev );
    }
}

/* register a file descriptor and its event handler.
 * no event mask will be enabled
 */
static void
looper_add( Looper*  l, int  fd, EventFunc  func, void*  user )
{
    struct epoll_event  ev;
    LoopHook*           hook;

    if (l->num_fds >= l->max_fds)
        looper_grow(l);

    hook = l->hooks + l->num_fds;

    hook->fd      = fd;
    hook->ev_user = user;
    hook->ev_func = func;
    hook->state   = 0;
    hook->wanted  = 0;
    hook->events  = 0;

    fd_setnonblock(fd);

    ev.events   = 0;
    ev.data.ptr = hook;
    epoll_ctl( l->epoll_fd, EPOLL_CTL_ADD, fd, &ev );

    l->num_fds += 1;
}

/* unregister a file descriptor and its event handler
 */
static void
looper_del( Looper*  l, int  fd )
{
    LoopHook*  hook = looper_find( l, fd );

    if (!hook) {
        D( "%s: invalid fd: %d", __FUNCTION__, fd );
        return;
    }
    /* don't remove the hook yet */
    hook->state |= HOOK_CLOSING;

    epoll_ctl( l->epoll_fd, EPOLL_CTL_DEL, fd, NULL );
}

/* enable monitoring of certain events for a file
 * descriptor. This adds 'events' to the current
 * event mask
 */
static void
looper_enable( Looper*  l, int  fd, int  events )
{
    LoopHook*  hook = looper_find( l, fd );

    if (!hook) {
        D("%s: invalid fd: %d", __FUNCTION__, fd );
        return;
    }

    if (events & ~hook->wanted) {
        struct epoll_event  ev;

        hook->wanted |= events;
        ev.events   = hook->wanted;
        ev.data.ptr = hook;

        epoll_ctl( l->epoll_fd, EPOLL_CTL_MOD, fd, &ev );
    }
}

/* disable monitoring of certain events for a file
 * descriptor. This ignores events that are not
 * currently enabled.
 */
static void
looper_disable( Looper*  l, int  fd, int  events )
{
    LoopHook*  hook = looper_find( l, fd );

    if (!hook) {
        D("%s: invalid fd: %d", __FUNCTION__, fd );
        return;
    }

    if (events & hook->wanted) {
        struct epoll_event  ev;

        hook->wanted &= ~events;
        ev.events   = hook->wanted;
        ev.data.ptr = hook;

        epoll_ctl( l->epoll_fd, EPOLL_CTL_MOD, fd, &ev );
    }
}

/* wait until an event occurs on one of the registered file
 * descriptors. Only returns in case of error !!
 */
static void
looper_loop( Looper*  l )
{
    for (;;) {
        int  n, count;

        do {
            count = epoll_wait( l->epoll_fd, l->events, l->num_fds, -1 );
        } while (count < 0 && errno == EINTR);

        if (count < 0) {
            D("%s: error: %s", __FUNCTION__, strerror(errno) );
            return;
        }

        if (count == 0) {
            D("%s: huh ? epoll returned count=0", __FUNCTION__);
            continue;
        }

        /* mark all pending hooks */
        for (n = 0; n < count; n++) {
            LoopHook*  hook = l->events[n].data.ptr;
            hook->state  = HOOK_PENDING;
            hook->events = l->events[n].events;
        }

        /* execute hook callbacks. this may change the 'hooks'
         * and 'events' array, as well as l->num_fds, so be careful */
        for (n = 0; n < l->num_fds; n++) {
            LoopHook*  hook = l->hooks + n;
            if (hook->state & HOOK_PENDING) {
                hook->state &= ~HOOK_PENDING;
                hook->ev_func( hook->ev_user, hook->events );
            }
        }

        /* now remove all the hooks that were closed by
         * the callbacks */
        for (n = 0; n < l->num_fds;) {
            struct epoll_event ev;
            LoopHook*  hook = l->hooks + n;

            if (!(hook->state & HOOK_CLOSING)) {
                n++;
                continue;
            }

            hook[0]     = l->hooks[l->num_fds-1];
            l->num_fds -= 1;
            ev.events   = hook->wanted;
            ev.data.ptr = hook;
            epoll_ctl( l->epoll_fd, EPOLL_CTL_MOD, hook->fd, &ev );
        }
    }
}

#if T_ACTIVE
char*
quote( const void*  data, int  len )
{
    const char*  p   = data;
    const char*  end = p + len;
    int          count = 0;
    int          phase = 0;
    static char*  buff = NULL;

    for (phase = 0; phase < 2; phase++) {
        if (phase != 0) {
            xfree(buff);
            buff = xalloc(count+1);
        }
        count = 0;
        for (p = data; p < end; p++) {
            int  c = *p;

            if (c == '\\') {
                if (phase != 0) {
                    buff[count] = buff[count+1] = '\\';
                }
                count += 2;
                continue;
            }

            if (c >= 32 && c < 127) {
                if (phase != 0)
                    buff[count] = c;
                count += 1;
                continue;
            }


            if (c == '\t') {
                if (phase != 0) {
                    memcpy(buff+count, "<TAB>", 5);
                }
                count += 5;
                continue;
            }
            if (c == '\n') {
                if (phase != 0) {
                    memcpy(buff+count, "<LN>", 4);
                }
                count += 4;
                continue;
            }
            if (c == '\r') {
                if (phase != 0) {
                    memcpy(buff+count, "<CR>", 4);
                }
                count += 4;
                continue;
            }

            if (phase != 0) {
                buff[count+0] = '\\';
                buff[count+1] = 'x';
                buff[count+2] = "0123456789abcdef"[(c >> 4) & 15];
                buff[count+3] = "0123456789abcdef"[     (c) & 15];
            }
            count += 4;
        }
    }
    buff[count] = 0;
    return buff;
}
#endif /* T_ACTIVE */

/** PACKETS
 **
 ** We need a way to buffer data before it can be sent to the
 ** corresponding file descriptor. We use linked list of Packet
 ** objects to do this.
 **/

typedef struct Packet   Packet;

#define  MAX_PAYLOAD  4000

struct Packet {
    Packet*   next;
    int       len;
    int       channel;
    uint8_t   data[ MAX_PAYLOAD ];
};

/* we expect to alloc/free a lot of packets during
 * operations so use a single linked list of free packets
 * to keep things speedy and simple.
 */
static Packet*   _free_packets;

/* Allocate a packet */
static Packet*
packet_alloc(void)
{
    Packet*  p = _free_packets;
    if (p != NULL) {
        _free_packets = p->next;
    } else {
        xnew(p);
    }
    p->next    = NULL;
    p->len     = 0;
    p->channel = -1;
    return p;
}

/* Release a packet. This takes the address of a packet
 * pointer that will be set to NULL on exit (avoids
 * referencing dangling pointers in case of bugs)
 */
static void
packet_free( Packet*  *ppacket )
{
    Packet*  p = *ppacket;
    if (p) {
        p->next       = _free_packets;
        _free_packets = p;
        *ppacket = NULL;
    }
}

/** PACKET RECEIVER
 **
 ** Simple abstraction for something that can receive a packet
 ** from a FDHandler (see below) or something else.
 **
 ** Send a packet to it with 'receiver_post'
 **
 ** Call 'receiver_close' to indicate that the corresponding
 ** packet source was closed.
 **/

typedef void (*PostFunc) ( void*  user, Packet*  p );
typedef void (*CloseFunc)( void*  user );

typedef struct {
    PostFunc   post;
    CloseFunc  close;
    void*      user;
} Receiver;

/* post a packet to a receiver. Note that this transfers
 * ownership of the packet to the receiver.
 */
static __inline__ void
receiver_post( Receiver*  r, Packet*  p )
{
    if (r->post)
        r->post( r->user, p );
    else
        packet_free(&p);
}

/* tell a receiver the packet source was closed.
 * this will also prevent further posting to the
 * receiver.
 */
static __inline__ void
receiver_close( Receiver*  r )
{
    if (r->close) {
        r->close( r->user );
        r->close = NULL;
    }
    r->post  = NULL;
}


/** FD HANDLERS
 **
 ** these are smart listeners that send incoming packets to a receiver
 ** and can queue one or more outgoing packets and send them when
 ** possible to the FD.
 **
 ** note that we support clean shutdown of file descriptors,
 ** i.e. we try to send all outgoing packets before destroying
 ** the FDHandler.
 **/

typedef struct FDHandler      FDHandler;
typedef struct FDHandlerList  FDHandlerList;

struct FDHandler {
    int             fd;
    FDHandlerList*  list;
    char            closing;
    Receiver        receiver[1];

    /* queue of outgoing packets */
    int             out_pos;
    Packet*         out_first;
    Packet**        out_ptail;

    FDHandler*      next;
    FDHandler**     pref;

};

struct FDHandlerList {
    /* the looper that manages the fds */
    Looper*      looper;

    /* list of active FDHandler objects */
    FDHandler*   active;

    /* list of closing FDHandler objects.
     * these are waiting to push their
     * queued packets to the fd before
     * freeing themselves.
     */
    FDHandler*   closing;

};

/* remove a FDHandler from its current list */
static void
fdhandler_remove( FDHandler*  f )
{
    f->pref[0] = f->next;
    if (f->next)
        f->next->pref = f->pref;
}

/* add a FDHandler to a given list */
static void
fdhandler_prepend( FDHandler*  f, FDHandler**  list )
{
    f->next = list[0];
    f->pref = list;
    list[0] = f;
    if (f->next)
        f->next->pref = &f->next;
}

/* initialize a FDHandler list */
static void
fdhandler_list_init( FDHandlerList*  list, Looper*  looper )
{
    list->looper  = looper;
    list->active  = NULL;
    list->closing = NULL;
}


/* close a FDHandler (and free it). Note that this will not
 * perform a graceful shutdown, i.e. all packets in the
 * outgoing queue will be immediately free.
 *
 * this *will* notify the receiver that the file descriptor
 * was closed.
 *
 * you should call fdhandler_shutdown() if you want to
 * notify the FDHandler that its packet source is closed.
 */
static void
fdhandler_close( FDHandler*  f )
{
    /* notify receiver */
    receiver_close(f->receiver);

    /* remove the handler from its list */
    fdhandler_remove(f);

    /* get rid of outgoing packet queue */
    if (f->out_first != NULL) {
        Packet*  p;
        while ((p = f->out_first) != NULL) {
            f->out_first = p->next;
            packet_free(&p);
        }
    }

    /* get rid of file descriptor */
    if (f->fd >= 0) {
        looper_del( f->list->looper, f->fd );
        close(f->fd);
        f->fd = -1;
    }

    f->list = NULL;
    xfree(f);
}

/* Ask the FDHandler to cleanly shutdown the connection,
 * i.e. send any pending outgoing packets then auto-free
 * itself.
 */
static void
fdhandler_shutdown( FDHandler*  f )
{
    /* prevent later fdhandler_close() to
     * call the receiver's close.
     */
    f->receiver->close = NULL;

    if (f->out_first != NULL && !f->closing)
    {
        /* move the handler to the 'closing' list */
        f->closing = 1;
        fdhandler_remove(f);
        fdhandler_prepend(f, &f->list->closing);
        return;
    }

    fdhandler_close(f);
}

/* Enqueue a new packet that the FDHandler will
 * send through its file descriptor.
 */
static void
fdhandler_enqueue( FDHandler*  f, Packet*  p )
{
    Packet*  first = f->out_first;

    p->next         = NULL;
    f->out_ptail[0] = p;
    f->out_ptail    = &p->next;

    if (first == NULL) {
        f->out_pos = 0;
        looper_enable( f->list->looper, f->fd, EPOLLOUT );
    }
}


/* FDHandler file descriptor event callback for read/write ops */
static void
fdhandler_event( FDHandler*  f, int  events )
{
   int  len;

    /* in certain cases, it's possible to have both EPOLLIN and
     * EPOLLHUP at the same time. This indicates that there is incoming
     * data to read, but that the connection was nonetheless closed
     * by the sender. Be sure to read the data before closing
     * the receiver to avoid packet loss.
     */

    if (events & EPOLLIN) {
        Packet*  p = packet_alloc();
        int      len;

        if ((len = fd_read(f->fd, p->data, MAX_PAYLOAD)) < 0) {
            D("%s: can't recv: %s", __FUNCTION__, strerror(errno));
            packet_free(&p);
        } else if (len > 0) {
            p->len     = len;
            p->channel = -101;  /* special debug value, not used */
            receiver_post( f->receiver, p );
        }
    }

    if (events & (EPOLLHUP|EPOLLERR)) {
        /* disconnection */
        D("%s: disconnect on fd %d", __FUNCTION__, f->fd);
        fdhandler_close(f);
        return;
    }

    if (events & EPOLLOUT && f->out_first) {
        Packet*  p = f->out_first;
        int      avail, len;

        avail = p->len - f->out_pos;
        if ((len = fd_write(f->fd, p->data + f->out_pos, avail)) < 0) {
            D("%s: can't send: %s", __FUNCTION__, strerror(errno));
        } else {
            f->out_pos += len;
            if (f->out_pos >= p->len) {
                f->out_pos   = 0;
                f->out_first = p->next;
                packet_free(&p);
                if (f->out_first == NULL) {
                    f->out_ptail = &f->out_first;
                    looper_disable( f->list->looper, f->fd, EPOLLOUT );
                }
            }
        }
    }
}


/* Create a new FDHandler that monitors read/writes */
static FDHandler*
fdhandler_new( int             fd,
               FDHandlerList*  list,
               Receiver*       receiver )
{
    FDHandler*  f = xalloc0(sizeof(*f));

    f->fd          = fd;
    f->list        = list;
    f->receiver[0] = receiver[0];
    f->out_first   = NULL;
    f->out_ptail   = &f->out_first;
    f->out_pos     = 0;

    fdhandler_prepend(f, &list->active);

    looper_add( list->looper, fd, (EventFunc) fdhandler_event, f );
    looper_enable( list->looper, fd, EPOLLIN );

    return f;
}


/* event callback function to monitor accepts() on server sockets.
 * the convention used here is that the receiver will receive a
 * dummy packet with the new client socket in p->channel
 */
static void
fdhandler_accept_event( FDHandler*  f, int  events )
{
    if (events & EPOLLIN) {
        /* this is an accept - send a dummy packet to the receiver */
        Packet*  p = packet_alloc();

        D("%s: accepting on fd %d", __FUNCTION__, f->fd);
        p->data[0] = 1;
        p->len     = 1;
        p->channel = fd_accept(f->fd);
        if (p->channel < 0) {
            D("%s: accept failed ?: %s", __FUNCTION__, strerror(errno));
            packet_free(&p);
            return;
        }
        receiver_post( f->receiver, p );
    }

    if (events & (EPOLLHUP|EPOLLERR)) {
        /* disconnecting !! */
        D("%s: closing accept fd %d", __FUNCTION__, f->fd);
        fdhandler_close(f);
        return;
    }
}


/* Create a new FDHandler used to monitor new connections on a
 * server socket. The receiver must expect the new connection
 * fd in the 'channel' field of a dummy packet.
 */
static FDHandler*
fdhandler_new_accept( int             fd,
                      FDHandlerList*  list,
                      Receiver*       receiver )
{
    FDHandler*  f = xalloc0(sizeof(*f));

    f->fd          = fd;
    f->list        = list;
    f->receiver[0] = receiver[0];

    fdhandler_prepend(f, &list->active);

    looper_add( list->looper, fd, (EventFunc) fdhandler_accept_event, f );
    looper_enable( list->looper, fd, EPOLLIN );
    listen( fd, 5 );

    return f;
}

/** SERIAL CONNECTION STATE
 **
 ** The following is used to handle the framing protocol
 ** used on the serial port connection.
 **/

/* each packet is made of a 6 byte header followed by a payload
 * the header looks like:
 *
 *   offset   size    description
 *       0       2    a 2-byte hex string for the channel number
 *       4       4    a 4-char hex string for the size of the payload
 *       6       n    the payload itself
 */
#define  HEADER_SIZE    6
#define  CHANNEL_OFFSET 0
#define  LENGTH_OFFSET  2
#define  CHANNEL_SIZE   2
#define  LENGTH_SIZE    4

#define  CHANNEL_CONTROL  0

/* The Serial object receives data from the serial port,
 * extracts the payload size and channel index, then sends
 * the resulting messages as a packet to a generic receiver.
 *
 * You can also use serial_send to send a packet through
 * the serial port.
 */
typedef struct Serial {
    FDHandler*  fdhandler;   /* used to monitor serial port fd */
    Receiver    receiver[1]; /* send payload there */
    int         in_len;      /* current bytes in input packet */
    int         in_datalen;  /* payload size, or 0 when reading header */
    int         in_channel;  /* extracted channel number */
    Packet*     in_packet;   /* used to read incoming packets */
} Serial;


/* a callback called when the serial port's fd is closed */
static void
serial_fd_close( Serial*  s )
{
    fatal("unexpected serial port close !!");
}

static void
serial_dump( Packet*  p, const char*  funcname )
{
    T("%s: %03d bytes: '%s'",
      funcname, p->len, quote(p->data, p->len));
}

/* a callback called when a packet arrives from the serial port's FDHandler.
 *
 * This will essentially parse the header, extract the channel number and
 * the payload size and store them in 'in_datalen' and 'in_channel'.
 *
 * After that, the payload is sent to the receiver once completed.
 */
static void
serial_fd_receive( Serial*  s, Packet*  p )
{
    int      rpos  = 0, rcount = p->len;
    Packet*  inp   = s->in_packet;
    int      inpos = s->in_len;

    serial_dump( p, __FUNCTION__ );

    while (rpos < rcount)
    {
        int  avail = rcount - rpos;

        /* first, try to read the header */
        if (s->in_datalen == 0) {
            int  wanted = HEADER_SIZE - inpos;
            if (avail > wanted)
                avail = wanted;

            memcpy( inp->data + inpos, p->data + rpos, avail );
            inpos += avail;
            rpos  += avail;

            if (inpos == HEADER_SIZE) {
                s->in_datalen = hex2int( inp->data + LENGTH_OFFSET,  LENGTH_SIZE );
                s->in_channel = hex2int( inp->data + CHANNEL_OFFSET, CHANNEL_SIZE );

                if (s->in_datalen <= 0) {
                    D("ignoring %s packet from serial port",
                      s->in_datalen ? "empty" : "malformed");
                    s->in_datalen = 0;
                }

                //D("received %d bytes packet for channel %d", s->in_datalen, s->in_channel);
                inpos = 0;
            }
        }
        else /* then, populate the packet itself */
        {
            int   wanted = s->in_datalen - inpos;

            if (avail > wanted)
                avail = wanted;

            memcpy( inp->data + inpos, p->data + rpos, avail );
            inpos += avail;
            rpos  += avail;

            if (inpos == s->in_datalen) {
                if (s->in_channel < 0) {
                    D("ignoring %d bytes addressed to channel %d",
                       inpos, s->in_channel);
                } else {
                    inp->len     = inpos;
                    inp->channel = s->in_channel;
                    receiver_post( s->receiver, inp );
                    s->in_packet  = inp = packet_alloc();
                }
                s->in_datalen = 0;
                inpos         = 0;
            }
        }
    }
    s->in_len = inpos;
    packet_free(&p);
}


/* send a packet to the serial port.
 * this assumes that p->len and p->channel contain the payload's
 * size and channel and will add the appropriate header.
 */
static void
serial_send( Serial*  s, Packet*  p )
{
    Packet*  h = packet_alloc();

    //D("sending to serial %d bytes from channel %d: '%.*s'", p->len, p->channel, p->len, p->data);

    /* insert a small header before this packet */
    h->len = HEADER_SIZE;
    int2hex( p->len,     h->data + LENGTH_OFFSET,  LENGTH_SIZE );
    int2hex( p->channel, h->data + CHANNEL_OFFSET, CHANNEL_SIZE );

    serial_dump( h, __FUNCTION__ );
    serial_dump( p, __FUNCTION__ );

    fdhandler_enqueue( s->fdhandler, h );
    fdhandler_enqueue( s->fdhandler, p );
}


/* initialize serial reader */
static void
serial_init( Serial*         s,
             int             fd,
             FDHandlerList*  list,
             Receiver*       receiver )
{
    Receiver  recv;

    recv.user  = s;
    recv.post  = (PostFunc)  serial_fd_receive;
    recv.close = (CloseFunc) serial_fd_close;

    s->receiver[0] = receiver[0];

    s->fdhandler = fdhandler_new( fd, list, &recv );
    s->in_len     = 0;
    s->in_datalen = 0;
    s->in_channel = 0;
    s->in_packet  = packet_alloc();
}


/** CLIENTS
 **/

typedef struct Client       Client;
typedef struct Multiplexer  Multiplexer;

/* A Client object models a single qemud client socket
 * connection in the emulated system.
 *
 * the client first sends the name of the system service
 * it wants to contact (no framing), then waits for a 2
 * byte answer from qemud.
 *
 * the answer is either "OK" or "KO" to indicate
 * success or failure.
 *
 * In case of success, the client can send messages
 * to the service.
 *
 * In case of failure, it can disconnect or try sending
 * the name of another service.
 */
struct Client {
    Client*       next;
    Client**      pref;
    int           channel;
    char          registered;
    FDHandler*    fdhandler;
    Multiplexer*  multiplexer;
};

struct Multiplexer {
    Client*        clients;
    int            last_channel;
    Serial         serial[1];
    Looper         looper[1];
    FDHandlerList  fdhandlers[1];
};


static int   multiplexer_open_channel( Multiplexer*  mult, Packet*  p );
static void  multiplexer_close_channel( Multiplexer*  mult, int  channel );
static void  multiplexer_serial_send( Multiplexer* mult, int  channel, Packet*  p );

static void
client_dump( Client*  c, Packet*  p, const char*  funcname )
{
    T("%s: client %p (%d): %3d bytes: '%s'",
      funcname, c, c->fdhandler->fd,
      p->len, quote(p->data, p->len));
}

/* destroy a client */
static void
client_free( Client*  c )
{
    /* remove from list */
    c->pref[0] = c->next;
    if (c->next)
        c->next->pref = c->pref;

    c->channel    = -1;
    c->registered = 0;

    /* gently ask the FDHandler to shutdown to
     * avoid losing queued outgoing packets */
    if (c->fdhandler != NULL) {
        fdhandler_shutdown(c->fdhandler);
        c->fdhandler = NULL;
    }

    xfree(c);
}


/* a function called when a client socket receives data */
static void
client_fd_receive( Client*  c, Packet*  p )
{
    client_dump(c, p, __FUNCTION__);

    if (c->registered) {
        /* the client is registered, just send the
         * data through the serial port
         */
        multiplexer_serial_send(c->multiplexer, c->channel, p);
        return;
    }

    if (c->channel > 0) {
        /* the client is waiting registration results.
         * this should not happen because the client
         * should wait for our 'ok' or 'ko'.
         * close the connection.
         */
         D("%s: bad client sending data before end of registration",
           __FUNCTION__);
     BAD_CLIENT:
         packet_free(&p);
         client_free(c);
         return;
    }

    /* the client hasn't registered a service yet,
     * so this must be the name of a service, call
     * the multiplexer to start registration for
     * it.
     */
    D("%s: attempting registration for service '%.*s'",
      __FUNCTION__, p->len, p->data);
    c->channel = multiplexer_open_channel(c->multiplexer, p);
    if (c->channel < 0) {
        D("%s: service name too long", __FUNCTION__);
        goto BAD_CLIENT;
    }
    D("%s:    -> received channel id %d", __FUNCTION__, c->channel);
    packet_free(&p);
}


/* a function called when the client socket is closed. */
static void
client_fd_close( Client*  c )
{
    T("%s: client %p (%d)", __FUNCTION__, c, c->fdhandler->fd);

    /* no need to shutdown the FDHandler */
    c->fdhandler = NULL;

    /* tell the emulator we're out */
    if (c->channel > 0)
        multiplexer_close_channel(c->multiplexer, c->channel);

    /* free the client */
    client_free(c);
}

/* a function called when the multiplexer received a registration
 * response from the emulator for a given client.
 */
static void
client_registration( Client*  c, int  registered )
{
    Packet*  p = packet_alloc();

    /* sends registration status to client */
    if (!registered) {
        D("%s: registration failed for client %d", __FUNCTION__, c->channel);
        memcpy( p->data, "KO", 2 );
        p->len = 2;
    } else {
        D("%s: registration succeeded for client %d", __FUNCTION__, c->channel);
        memcpy( p->data, "OK", 2 );
        p->len = 2;
    }
    client_dump(c, p, __FUNCTION__);
    fdhandler_enqueue(c->fdhandler, p);

    /* now save registration state
     */
    c->registered = registered;
    if (!registered) {
        /* allow the client to try registering another service */
        c->channel = -1;
    }
}

/* send data to a client */
static void
client_send( Client*  c, Packet*  p )
{
    client_dump(c, p, __FUNCTION__);
    fdhandler_enqueue(c->fdhandler, p);
}


/* Create new client socket handler */
static Client*
client_new( Multiplexer*    mult,
            int             fd,
            FDHandlerList*  pfdhandlers,
            Client**        pclients )
{
    Client*   c;
    Receiver  recv;

    xnew(c);

    c->multiplexer = mult;
    c->next        = NULL;
    c->pref        = &c->next;
    c->channel     = -1;
    c->registered  = 0;

    recv.user  = c;
    recv.post  = (PostFunc)  client_fd_receive;
    recv.close = (CloseFunc) client_fd_close;

    c->fdhandler = fdhandler_new( fd, pfdhandlers, &recv );

    /* add to client list */
    c->next   = *pclients;
    c->pref   = pclients;
    *pclients = c;
    if (c->next)
        c->next->pref = &c->next;

    return c;
}

/**  GLOBAL MULTIPLEXER
 **/

/* find a client by its channel */
static Client*
multiplexer_find_client( Multiplexer*  mult, int  channel )
{
    Client* c = mult->clients;

    for ( ; c != NULL; c = c->next ) {
        if (c->channel == channel)
            return c;
    }
    return NULL;
}

/* handle control messages coming from the serial port
 * on CONTROL_CHANNEL.
 */
static void
multiplexer_handle_control( Multiplexer*  mult, Packet*  p )
{
    /* connection registration success */
    if (p->len == 13 && !memcmp(p->data, "ok:connect:", 11)) {
        int      channel = hex2int(p->data+11, 2);
        Client*  client  = multiplexer_find_client(mult, channel);

        /* note that 'client' can be NULL if the corresponding
         * socket was closed before the emulator response arrived.
         */
        if (client != NULL) {
            client_registration(client, 1);
        } else {
            D("%s: NULL client: '%.*s'", __FUNCTION__, p->len, p->data+11);
        }
        goto EXIT;
    }

    /* connection registration failure */
    if (p->len == 13 && !memcmp(p->data, "ko:connect:",11)) {
        int     channel = hex2int(p->data+11, 2);
        Client* client  = multiplexer_find_client(mult, channel);

        if (client != NULL)
            client_registration(client, 0);

        goto EXIT;
    }

    /* emulator-induced client disconnection */
    if (p->len == 13 && !memcmp(p->data, "disconnect:",11)) {
        int      channel = hex2int(p->data+11, 2);
        Client*  client  = multiplexer_find_client(mult, channel);

        if (client != NULL)
            client_free(client);

        goto EXIT;
    }

    /* A message that begins with "X00" is a probe sent by
     * the emulator used to detect which version of qemud it runs
     * against (in order to detect 1.0/1.1 system images. Just
     * silently ignore it there instead of printing an error
     * message.
     */
    if (p->len >= 3 && !memcmp(p->data,"X00",3)) {
        goto EXIT;
    }

    D("%s: unknown control message (%d bytes): '%.*s'",
      __FUNCTION__, p->len, p->len, p->data);

EXIT:
    packet_free(&p);
}

/* a function called when an incoming packet comes from the serial port */
static void
multiplexer_serial_receive( Multiplexer*  mult, Packet*  p )
{
    Client*  client;

    T("%s: channel=%d '%.*s'", __FUNCTION__, p->channel, p->len, p->data);

    if (p->channel == CHANNEL_CONTROL) {
        multiplexer_handle_control(mult, p);
        return;
    }

    client = multiplexer_find_client(mult, p->channel);
    if (client != NULL) {
        client_send(client, p);
        return;
    }

    D("%s: discarding packet for unknown channel %d", __FUNCTION__, p->channel);
    packet_free(&p);
}

/* a function called when the serial reader closes */
static void
multiplexer_serial_close( Multiplexer*  mult )
{
    fatal("unexpected close of serial reader");
}

/* a function called to send a packet to the serial port */
static void
multiplexer_serial_send( Multiplexer*  mult, int  channel, Packet*  p )
{
    p->channel = channel;
    serial_send( mult->serial, p );
}



/* a function used by a client to allocate a new channel id and
 * ask the emulator to open it. 'service' must be a packet containing
 * the name of the service in its payload.
 *
 * returns -1 if the service name is too long.
 *
 * notice that client_registration() will be called later when
 * the answer arrives.
 */
static int
multiplexer_open_channel( Multiplexer*  mult, Packet*  service )
{
    Packet*   p = packet_alloc();
    int       len, channel;

    /* find a free channel number, assume we don't have many
     * clients here. */
    {
        Client*  c;
    TRY_AGAIN:
        channel = (++mult->last_channel) & 0xff;

        for (c = mult->clients; c != NULL; c = c->next)
            if (c->channel == channel)
                goto TRY_AGAIN;
    }

    len = snprintf((char*)p->data, sizeof p->data, "connect:%.*s:%02x", service->len, service->data, channel);
    if (len >= (int)sizeof(p->data)) {
        D("%s: weird, service name too long (%d > %d)", __FUNCTION__, len, sizeof(p->data));
        packet_free(&p);
        return -1;
    }
    p->channel = CHANNEL_CONTROL;
    p->len     = len;

    serial_send(mult->serial, p);
    return channel;
}

/* used to tell the emulator a channel was closed by a client */
static void
multiplexer_close_channel( Multiplexer*  mult, int  channel )
{
    Packet*  p   = packet_alloc();
    int      len = snprintf((char*)p->data, sizeof(p->data), "disconnect:%02x", channel);

    if (len > (int)sizeof(p->data)) {
        /* should not happen */
        return;
    }

    p->channel = CHANNEL_CONTROL;
    p->len     = len;

    serial_send(mult->serial, p);
}

/* this function is used when a new connection happens on the control
 * socket.
 */
static void
multiplexer_control_accept( Multiplexer*  m, Packet*  p )
{
    /* the file descriptor for the new socket connection is
     * in p->channel. See fdhandler_accept_event() */
    int      fd     = p->channel;
    Client*  client = client_new( m, fd, m->fdhandlers, &m->clients );

    D("created client %p listening on fd %d", client, fd);

    /* free dummy packet */
    packet_free(&p);
}

static void
multiplexer_control_close( Multiplexer*  m )
{
    fatal("unexpected multiplexer control close");
}

static void
multiplexer_init( Multiplexer*  m, const char*  serial_dev )
{
    int       fd, control_fd;
    Receiver  recv;

    /* initialize looper and fdhandlers list */
    looper_init( m->looper );
    fdhandler_list_init( m->fdhandlers, m->looper );

    /* open the serial port */
    do {
        fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    } while (fd < 0 && errno == EINTR);

    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, serial_dev, sizeof(addr.sun_path));

    if (connect(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        close(fd);
        fd = -1;
    }

    if (fd < 0) {
        fatal( "%s: could not open '%s': %s", __FUNCTION__, serial_dev,
               strerror(errno) );
    }

    /* initialize the serial reader/writer */
    recv.user  = m;
    recv.post  = (PostFunc)  multiplexer_serial_receive;
    recv.close = (CloseFunc) multiplexer_serial_close;

    serial_init( m->serial, fd, m->fdhandlers, &recv );

    /* open the qemud control socket */
    recv.user  = m;
    recv.post  = (PostFunc)  multiplexer_control_accept;
    recv.close = (CloseFunc) multiplexer_control_close;

    fd = android_get_control_socket(CONTROL_SOCKET_NAME);
    if (fd < 0) {
        fatal("couldn't get fd for control socket '%s'", CONTROL_SOCKET_NAME);
    }

    fdhandler_new_accept( fd, m->fdhandlers, &recv );

    /* initialize clients list */
    m->clients = NULL;
}

/** MAIN LOOP
 **/

static Multiplexer  _multiplexer[1];

int  main( void )
{
    Multiplexer*  m = _multiplexer;

    multiplexer_init(m, "/dev/qemud");

    D( "entering main loop");
    looper_loop( m->looper );
    D( "unexpected termination !!" );
    return 0;
}
