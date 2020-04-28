#ifndef NANOTUBE_IRC_H
#define NANOTUBE_IRC_H

#include <stdint.h>
#include "lwIP/include/lwip/err.h"
#include "lwIP/include/lwip/ip4_addr.h"
#include "lwIP/include/lwip/tcp.h"

struct irc_conn;

typedef err_t (*irc_connect_callback_t)(struct irc_conn *conn, err_t err);
typedef err_t (*irc_message_callback_t)(struct irc_conn *conn, const char *message);
typedef err_t (*irc_privmsg_callback_t)(struct irc_conn *conn, const char *channel, const char *sender, const char *message);

struct irc_conn {
    ip4_addr_t server;
    uint16_t port;
    char nick[10];
    char realname[32];
    irc_message_callback_t msg_callback;
    irc_privmsg_callback_t privmsg_callback;
    irc_connect_callback_t conn_callback;

    struct tcp_pcb *pcb;
    bool identified;
};

err_t irc_init(struct irc_conn *conn, const char *server_name, uint16_t port, const char *nick, const char *realname,
               irc_connect_callback_t connect_callback, irc_message_callback_t message_callback,
               irc_privmsg_callback_t privmsg_callback);

err_t irc_send_raw(struct irc_conn *conn, const char *message, size_t len);

err_t irc_send(struct irc_conn *conn, const char *channel, const char *message);

err_t irc_close(struct irc_conn *conn);

err_t irc_away(struct irc_conn *conn, const char *message);

err_t irc_join(struct irc_conn *conn, const char *channel);

err_t irc_part(struct irc_conn *conn, const char *channel);

err_t irc_nick(struct irc_conn *conn, const char *nick);

#endif //NANOTUBE_IRC_H
