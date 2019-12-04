cmake_minimum_required(VERSION 3.15)
project(nanotube C)

set(CMAKE_C_STANDARD 90)

include_directories($ENV{CEDEV}/include)

add_custom_target(Make ALL $(MAKE) debug WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

#dummy executable to make CLion quit complaining
add_executable(dummy
        src/lwIP/api/api_lib.c
        src/lwIP/api/api_msg.c
        src/lwIP/api/err.c
        src/lwIP/api/if_api.c
        src/lwIP/api/netbuf.c
        src/lwIP/api/netdb.c
        src/lwIP/api/netifapi.c
        src/lwIP/api/sockets.c
        src/lwIP/api/tcpip.c
#        src/lwIP/apps/altcp_tls/altcp_tls_mbedtls.c
#        src/lwIP/apps/altcp_tls/altcp_tls_mbedtls_mem.c
#        src/lwIP/apps/altcp_tls/altcp_tls_mbedtls_mem.h
#        src/lwIP/apps/altcp_tls/altcp_tls_mbedtls_structs.h
        src/lwIP/apps/http/altcp_proxyconnect.c
        src/lwIP/apps/http/fs.c
        src/lwIP/apps/http/fsdata.c
        src/lwIP/apps/http/fsdata.h
        src/lwIP/apps/http/http_client.c
        src/lwIP/apps/http/httpd.c
        src/lwIP/apps/http/httpd_structs.h
#        src/lwIP/apps/lwiperf/lwiperf.c
#        src/lwIP/apps/mdns/mdns.c
#        src/lwIP/apps/mqtt/mqtt.c
#        src/lwIP/apps/netbiosns/netbiosns.c
#        src/lwIP/apps/smtp/smtp.c
#        src/lwIP/apps/snmp/snmp_asn1.c
#        src/lwIP/apps/snmp/snmp_asn1.h
#        src/lwIP/apps/snmp/snmp_core.c
#        src/lwIP/apps/snmp/snmp_core_priv.h
#        src/lwIP/apps/snmp/snmp_mib2.c
#        src/lwIP/apps/snmp/snmp_mib2_icmp.c
#        src/lwIP/apps/snmp/snmp_mib2_interfaces.c
#        src/lwIP/apps/snmp/snmp_mib2_ip.c
#        src/lwIP/apps/snmp/snmp_mib2_snmp.c
#        src/lwIP/apps/snmp/snmp_mib2_system.c
#        src/lwIP/apps/snmp/snmp_mib2_tcp.c
#        src/lwIP/apps/snmp/snmp_mib2_udp.c
#        src/lwIP/apps/snmp/snmp_msg.c
#        src/lwIP/apps/snmp/snmp_msg.h
#        src/lwIP/apps/snmp/snmp_netconn.c
#        src/lwIP/apps/snmp/snmp_pbuf_stream.c
#        src/lwIP/apps/snmp/snmp_pbuf_stream.h
#        src/lwIP/apps/snmp/snmp_raw.c
#        src/lwIP/apps/snmp/snmp_scalar.c
#        src/lwIP/apps/snmp/snmp_snmpv2_framework.c
#        src/lwIP/apps/snmp/snmp_snmpv2_usm.c
#        src/lwIP/apps/snmp/snmp_table.c
#        src/lwIP/apps/snmp/snmp_threadsync.c
#        src/lwIP/apps/snmp/snmp_traps.c
#        src/lwIP/apps/snmp/snmpv3.c
#        src/lwIP/apps/snmp/snmpv3_mbedtls.c
#        src/lwIP/apps/snmp/snmpv3_priv.h
#        src/lwIP/apps/sntp/sntp.c
#        src/lwIP/apps/tftp/tftp_server.c
        src/lwIP/core/ipv4/autoip.c
        src/lwIP/core/ipv4/dhcp.c
        src/lwIP/core/ipv4/etharp.c
        src/lwIP/core/ipv4/icmp.c
        src/lwIP/core/ipv4/igmp.c
        src/lwIP/core/ipv4/ip4.c
        src/lwIP/core/ipv4/ip4_addr.c
        src/lwIP/core/ipv4/ip4_frag.c
        src/lwIP/core/ipv6/dhcp6.c
        src/lwIP/core/ipv6/ethip6.c
        src/lwIP/core/ipv6/icmp6.c
        src/lwIP/core/ipv6/inet6.c
        src/lwIP/core/ipv6/ip6.c
        src/lwIP/core/ipv6/ip6_addr.c
        src/lwIP/core/ipv6/ip6_frag.c
        src/lwIP/core/ipv6/mld6.c
        src/lwIP/core/ipv6/nd6.c
        src/lwIP/core/altcp.c
        src/lwIP/core/altcp_alloc.c
        src/lwIP/core/altcp_tcp.c
        src/lwIP/core/def.c
        src/lwIP/core/dns.c
        src/lwIP/core/inet_chksum.c
        src/lwIP/core/init.c
        src/lwIP/core/ip.c
        src/lwIP/core/mem.c
        src/lwIP/core/memp.c
        src/lwIP/core/netif.c
        src/lwIP/core/pbuf.c
        src/lwIP/core/raw.c
        src/lwIP/core/stats.c
        src/lwIP/core/sys.c
        src/lwIP/core/tcp.c
        src/lwIP/core/tcp_in.c
        src/lwIP/core/tcp_out.c
        src/lwIP/core/timeouts.c
        src/lwIP/core/udp.c
        src/lwIP/include/compat/posix/arpa/inet.h
        src/lwIP/include/compat/posix/net/if.h
        src/lwIP/include/compat/posix/sys/socket.h
        src/lwIP/include/compat/posix/netdb.h
        src/lwIP/include/compat/stdc/errno.h
#        src/lwIP/include/lwip/apps/altcp_proxyconnect.h
#        src/lwIP/include/lwip/apps/altcp_tls_mbedtls_opts.h
#        src/lwIP/include/lwip/apps/fs.h
#        src/lwIP/include/lwip/apps/http_client.h
        src/lwIP/include/lwip/apps/httpd.h
        src/lwIP/include/lwip/apps/httpd_opts.h
#        src/lwIP/include/lwip/apps/lwiperf.h
#        src/lwIP/include/lwip/apps/mdns.h
#        src/lwIP/include/lwip/apps/mdns_opts.h
#        src/lwIP/include/lwip/apps/mdns_priv.h
#        src/lwIP/include/lwip/apps/mqtt.h
#        src/lwIP/include/lwip/apps/mqtt_opts.h
#        src/lwIP/include/lwip/apps/mqtt_priv.h
#        src/lwIP/include/lwip/apps/netbiosns.h
#        src/lwIP/include/lwip/apps/netbiosns_opts.h
#        src/lwIP/include/lwip/apps/smtp.h
#        src/lwIP/include/lwip/apps/smtp_opts.h
#        src/lwIP/include/lwip/apps/snmp.h
#        src/lwIP/include/lwip/apps/snmp_core.h
#        src/lwIP/include/lwip/apps/snmp_mib2.h
#        src/lwIP/include/lwip/apps/snmp_opts.h
#        src/lwIP/include/lwip/apps/snmp_scalar.h
#        src/lwIP/include/lwip/apps/snmp_snmpv2_framework.h
#        src/lwIP/include/lwip/apps/snmp_snmpv2_usm.h
#        src/lwIP/include/lwip/apps/snmp_table.h
#        src/lwIP/include/lwip/apps/snmp_threadsync.h
#        src/lwIP/include/lwip/apps/snmpv3.h
#        src/lwIP/include/lwip/apps/sntp.h
#        src/lwIP/include/lwip/apps/sntp_opts.h
#        src/lwIP/include/lwip/apps/tftp_opts.h
#        src/lwIP/include/lwip/apps/tftp_server.h
        src/lwIP/include/lwip/priv/altcp_priv.h
        src/lwIP/include/lwip/priv/api_msg.h
        src/lwIP/include/lwip/priv/mem_priv.h
        src/lwIP/include/lwip/priv/memp_priv.h
        src/lwIP/include/lwip/priv/memp_std.h
        src/lwIP/include/lwip/priv/nd6_priv.h
        src/lwIP/include/lwip/priv/raw_priv.h
        src/lwIP/include/lwip/priv/sockets_priv.h
        src/lwIP/include/lwip/priv/tcp_priv.h
        src/lwIP/include/lwip/priv/tcpip_priv.h
        src/lwIP/include/lwip/prot/autoip.h
        src/lwIP/include/lwip/prot/dhcp.h
        src/lwIP/include/lwip/prot/dhcp6.h
        src/lwIP/include/lwip/prot/dns.h
        src/lwIP/include/lwip/prot/etharp.h
        src/lwIP/include/lwip/prot/ethernet.h
        src/lwIP/include/lwip/prot/iana.h
        src/lwIP/include/lwip/prot/icmp.h
        src/lwIP/include/lwip/prot/icmp6.h
        src/lwIP/include/lwip/prot/ieee.h
        src/lwIP/include/lwip/prot/igmp.h
        src/lwIP/include/lwip/prot/ip.h
        src/lwIP/include/lwip/prot/ip4.h
        src/lwIP/include/lwip/prot/ip6.h
        src/lwIP/include/lwip/prot/mld6.h
        src/lwIP/include/lwip/prot/nd6.h
        src/lwIP/include/lwip/prot/tcp.h
        src/lwIP/include/lwip/prot/udp.h
        src/lwIP/include/lwip/altcp.h
        src/lwIP/include/lwip/altcp_tcp.h
        src/lwIP/include/lwip/altcp_tls.h
        src/lwIP/include/lwip/api.h
        src/lwIP/include/lwip/arch.h
        src/lwIP/include/lwip/autoip.h
        src/lwIP/include/lwip/debug.h
        src/lwIP/include/lwip/def.h
        src/lwIP/include/lwip/dhcp.h
        src/lwIP/include/lwip/dhcp6.h
        src/lwIP/include/lwip/dns.h
        src/lwIP/include/lwip/err.h
        src/lwIP/include/lwip/errno.h
        src/lwIP/include/lwip/etharp.h
        src/lwIP/include/lwip/ethip6.h
        src/lwIP/include/lwip/icmp.h
        src/lwIP/include/lwip/icmp6.h
        src/lwIP/include/lwip/if_api.h
        src/lwIP/include/lwip/igmp.h
        src/lwIP/include/lwip/inet.h
        src/lwIP/include/lwip/inet_chksum.h
        src/lwIP/include/lwip/init.h
        src/lwIP/include/lwip/ip.h
        src/lwIP/include/lwip/ip4.h
        src/lwIP/include/lwip/ip4_addr.h
        src/lwIP/include/lwip/ip4_frag.h
        src/lwIP/include/lwip/ip6.h
        src/lwIP/include/lwip/ip6_addr.h
        src/lwIP/include/lwip/ip6_frag.h
        src/lwIP/include/lwip/ip6_zone.h
        src/lwIP/include/lwip/ip_addr.h
        src/lwIP/include/lwip/mem.h
        src/lwIP/include/lwip/memp.h
        src/lwIP/include/lwip/mld6.h
        src/lwIP/include/lwip/nd6.h
        src/lwIP/include/lwip/netbuf.h
        src/lwIP/include/lwip/netdb.h
        src/lwIP/include/lwip/netif.h
        src/lwIP/include/lwip/netifapi.h
        src/lwIP/include/lwip/opt.h
        src/lwIP/include/lwip/pbuf.h
        src/lwIP/include/lwip/raw.h
        src/lwIP/include/lwip/sio.h
        src/lwIP/include/lwip/snmp.h
        src/lwIP/include/lwip/sockets.h
        src/lwIP/include/lwip/stats.h
        src/lwIP/include/lwip/sys.h
        src/lwIP/include/lwip/tcp.h
        src/lwIP/include/lwip/tcpbase.h
        src/lwIP/include/lwip/tcpip.h
        src/lwIP/include/lwip/timeouts.h
        src/lwIP/include/lwip/udp.h
        src/lwIP/include/netif/ppp/polarssl/arc4.h
        src/lwIP/include/netif/ppp/polarssl/des.h
        src/lwIP/include/netif/ppp/polarssl/md4.h
        src/lwIP/include/netif/ppp/polarssl/md5.h
        src/lwIP/include/netif/ppp/polarssl/sha1.h
        src/lwIP/include/netif/ppp/ccp.h
        src/lwIP/include/netif/ppp/chap-md5.h
        src/lwIP/include/netif/ppp/chap-new.h
        src/lwIP/include/netif/ppp/chap_ms.h
        src/lwIP/include/netif/ppp/eap.h
        src/lwIP/include/netif/ppp/ecp.h
        src/lwIP/include/netif/ppp/eui64.h
        src/lwIP/include/netif/ppp/fsm.h
        src/lwIP/include/netif/ppp/ipcp.h
        src/lwIP/include/netif/ppp/ipv6cp.h
        src/lwIP/include/netif/ppp/lcp.h
        src/lwIP/include/netif/ppp/magic.h
        src/lwIP/include/netif/ppp/mppe.h
        src/lwIP/include/netif/ppp/ppp.h
        src/lwIP/include/netif/ppp/ppp_impl.h
        src/lwIP/include/netif/ppp/ppp_opts.h
        src/lwIP/include/netif/ppp/pppapi.h
        src/lwIP/include/netif/ppp/pppcrypt.h
        src/lwIP/include/netif/ppp/pppdebug.h
        src/lwIP/include/netif/ppp/pppoe.h
        src/lwIP/include/netif/ppp/pppol2tp.h
        src/lwIP/include/netif/ppp/pppos.h
        src/lwIP/include/netif/ppp/upap.h
        src/lwIP/include/netif/ppp/vj.h
        src/lwIP/include/netif/bridgeif.h
        src/lwIP/include/netif/bridgeif_opts.h
        src/lwIP/include/netif/etharp.h
        src/lwIP/include/netif/ethernet.h
        src/lwIP/include/netif/ieee802154.h
        src/lwIP/include/netif/lowpan6.h
        src/lwIP/include/netif/lowpan6_ble.h
        src/lwIP/include/netif/lowpan6_common.h
        src/lwIP/include/netif/lowpan6_opts.h
        src/lwIP/include/netif/slipif.h
        src/lwIP/include/netif/zepif.h
        src/lwIP/netif/ppp/polarssl/arc4.c
        src/lwIP/netif/ppp/polarssl/des.c
        src/lwIP/netif/ppp/polarssl/md4.c
        src/lwIP/netif/ppp/polarssl/md5.c
        src/lwIP/netif/ppp/polarssl/sha1.c
        src/lwIP/netif/ppp/auth.c
        src/lwIP/netif/ppp/ccp.c
        src/lwIP/netif/ppp/chap-md5.c
        src/lwIP/netif/ppp/chap-new.c
        src/lwIP/netif/ppp/chap_ms.c
        src/lwIP/netif/ppp/demand.c
        src/lwIP/netif/ppp/eap.c
        src/lwIP/netif/ppp/ecp.c
        src/lwIP/netif/ppp/eui64.c
        src/lwIP/netif/ppp/fsm.c
        src/lwIP/netif/ppp/ipcp.c
        src/lwIP/netif/ppp/ipv6cp.c
        src/lwIP/netif/ppp/lcp.c
        src/lwIP/netif/ppp/magic.c
        src/lwIP/netif/ppp/mppe.c
        src/lwIP/netif/ppp/multilink.c
        src/lwIP/netif/ppp/ppp.c
        src/lwIP/netif/ppp/pppapi.c
        src/lwIP/netif/ppp/pppcrypt.c
        src/lwIP/netif/ppp/pppoe.c
        src/lwIP/netif/ppp/pppol2tp.c
        src/lwIP/netif/ppp/pppos.c
        src/lwIP/netif/ppp/upap.c
        src/lwIP/netif/ppp/utils.c
        src/lwIP/netif/ppp/vj.c
        src/lwIP/netif/bridgeif.c
        src/lwIP/netif/bridgeif_fdb.c
        src/lwIP/netif/ethernet.c
        src/lwIP/netif/lowpan6.c
        src/lwIP/netif/lowpan6_ble.c
        src/lwIP/netif/lowpan6_common.c
        src/lwIP/netif/slipif.c
        src/lwIP/netif/zepif.c
        src/main.c
        src/lwIP/include/lwipopts.h
        src/lwIP/include/lwip/arch/cc.h
        src/lwIP/arch/time.c
        src/lwIP/include/lwip/arch/time.h src/queue.c src/queue.h src/nanotube.h)

#add_executable(fs
#        src/lwIP/apps/http/makefsdata/makefsdata.c
#        src/lwIP/apps/http/makefsdata/tinydir.h)

#target_include_directories(fs PUBLIC /usr/include)
