#include <string.h>
#include "irc.h"
#include "lwIP/include/lwip/dns.h"
#include "lwIP/include/lwip/tcp.h"

// todo: null termination?
static void process_message(struct irc_conn *conn, const char *message) {
    char channel[32];
    char sender[32];
    char response[32];
    const char *command, *target, *content;
    size_t user_len, nick_len, target_len;

    conn->msg_callback(conn, message);

    if(!conn->identified && memcmp(message, "NOTICE AUTH :*** Checking Ident", 31) == 0) {
        char buffer[256];
        size_t len = sprintf(buffer, "USER %s 0 * :%s\r\nNICK %s\r\n", conn->nick, conn->realname, conn->nick);
        irc_send_raw(conn, buffer, len);
        conn->identified = true;
        return;
    }

    // Extremely lazy message parsing
    // I should get around to reading the spec

    if(message[0] == ':') {
        command = strchr(message, ' ');
        if(!command) {
            mainlog("No command found.\n");
            return;
        }
        nick_len = user_len = command - message;
        command++;
    } else {
        command = message;
        nick_len = user_len = 0;
    }

    if(memcmp(command, "PING", 4) == 0) {
        uint8_t len;
        mainlog("Got a ping command\n");
        len = strlen(command);
        if(len + 2 > sizeof(response)) {
            mainlog("Ping too long\n");
            return;
        }
        memcpy(response, command, len);
        response[1] = 'O';
        response[len] = '\r';
        response[len + 1] = '\n';

        irc_send_raw(conn, response, len + 2);
        return;
    }

    if(memcmp(command, "PRIVMSG", 7) != 0) return;

    mainlog("Got a PRIVMSG\n");

    target = strchr(command, ' ');
    if(!target) {
        mainlog("No target for PRIVMSG.\n");
        return;
    }
    target++;

    content = strchr(target, ' ');
    if(!content) {
        mainlog("No content for PRIVMSG.\n");
        return;
    }
    target_len = content - target;
    content++;
    if(*content == ':') content++;

    if(user_len) {
        const char *nick_end = strchr(message, '!');
        nick_len = nick_end - message - 1;
    }

//    custom_printf("message %p, command %p, target %p, content %p, user len %u, nick len %u, target len %u\n",
//                  message, command, target, content, user_len, nick_len, target_len);

    memcpy(channel, target, target_len);
    channel[target_len] = 0;
    memcpy(sender, &message[1], nick_len);
    sender[nick_len] = 0;

    conn->privmsg_callback(conn, channel, sender, content);
}

static err_t recv_func(void *arg, struct tcp_pcb *tpcb,
                       struct pbuf *p, err_t err) {
    struct irc_conn *conn = arg;
    char *current;

    if(!p) {
        mainlog("IRC connection closed\n");
        return ERR_OK;
    }
    if(err) {
        custom_printf("Error %u on IRC data receive\n", err);
        return ERR_OK;
    }

    for(current = p->payload;;) {
        char *end = strstr(current, "\r\n");

        if(end == NULL) break;

        *end = 0;
        process_message(conn, current);

        current = end + 2;
    }

    mainlog("processed messages\n");

    tcp_recved(tpcb, p->tot_len);

    return ERR_OK;
}

//todo: should this wait for ident or just a connection?
static err_t on_connect(void *arg, struct tcp_pcb *tpcb, err_t err) {
    struct irc_conn *conn = arg;

    tcp_recv(conn->pcb, recv_func);

    conn->conn_callback(conn, err);

    return ERR_OK;
}

static void name_resolved(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    struct irc_conn *conn = callback_arg;

    custom_printf("'%s' resolved to %u.%u.%u.%u\n", name, ip4_addr1(ipaddr), ip4_addr2(ipaddr), ip4_addr3(ipaddr), ip4_addr4(ipaddr));

    tcp_arg(conn->pcb, conn);
    tcp_connect(conn->pcb, ipaddr, conn->port, on_connect);
}

err_t irc_init(struct irc_conn *conn, const char *server_name, uint16_t port, const char *nick, const char *realname,
               irc_connect_callback_t connect_callback, irc_message_callback_t message_callback,
               irc_privmsg_callback_t privmsg_callback) {
    if(!port) port = 6667;
    conn->port = port;

    if(strlen(nick) >= sizeof(conn->nick))
        return ERR_ARG;
    strcpy(conn->nick, nick);

    if(strlen(realname) >= sizeof(conn->realname))
        return ERR_ARG;
    strcpy(conn->realname, realname);

    conn->msg_callback = message_callback;
    conn->privmsg_callback = privmsg_callback;
    conn->conn_callback = connect_callback;

    conn->pcb = tcp_new();
    if(!conn->pcb) return ERR_MEM;

    conn->identified = false;

    return dns_gethostbyname(server_name, &conn->server, name_resolved, conn);
}

err_t irc_send_raw(struct irc_conn *conn, const char *message, size_t len) {
    err_t err;
    custom_printf("Sending message of length %u: %s", len, message);
    err = tcp_write(conn->pcb, message, len, TCP_WRITE_FLAG_COPY);
    if(err) {
        custom_printf("Error %u for tcp_write\n", err);
        return err;
    }
    err = tcp_output(conn->pcb);
    if(err) {
        custom_printf("Error %u for tcp_output\n", err);
        return err;
    }
    return ERR_OK;
}

err_t irc_send(struct irc_conn *conn, const char *channel, const char *message) {
    char buffer[512];

    //todo: should be snprintf
    size_t len = sprintf(buffer, "PRIVMSG %s :%s\r\n", channel, message);

    return irc_send_raw(conn, buffer, len);
}

err_t irc_close(struct irc_conn *conn) {
    mainlog("Closing IRC connection\n");

    tcp_close(conn->pcb);
    conn->pcb = NULL;

    return ERR_OK;
}

err_t irc_away(struct irc_conn *conn, const char *message) {
    char buffer[512];

    size_t len = sprintf(buffer, "AWAY %s\r\n", message);

    return irc_send_raw(conn, buffer, len);
}

err_t irc_join(struct irc_conn *conn, const char *channel) {
    char buffer[64];

    size_t len = sprintf(buffer, "JOIN %s\r\n", channel);

    return irc_send_raw(conn, buffer, len);
}

err_t irc_part(struct irc_conn *conn, const char *channel) {
    char buffer[64];

    size_t len = sprintf(buffer, "PART %s\r\n", channel);

    return irc_send_raw(conn, buffer, len);
}

err_t irc_nick(struct irc_conn *conn, const char *nick) {
    char buffer[64];

    size_t len = sprintf(buffer, "NICK %s\r\n", nick);

    return irc_send_raw(conn, buffer, len);
}
