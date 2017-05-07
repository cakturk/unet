#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2017 Cihangir Akturk

from scapy.all import IP, UDP, fuzz
import os, sys, struct, socket, random

def rand_ipaddr():
    return socket.inet_ntoa(struct.pack('>I', random.randint(1, 0xffffffff)))

def get_payload(n):
    if n == 0:
        return ''
    n = n if n > 0 else random.randint(0, 576)
    return os.urandom(n)

def gen_udp_pkt(src=None, dst=None, payload_len=-1):
    """Generate random IP/UDP[Payload] packet

    :src: source IP address
    :dst: destination IP address
    :payload_len: size of payload
    :returns: packet as raw bytes

    """
    getipaddr = lambda addr: rand_ipaddr() if addr is None else addr
    sip = getipaddr(src)
    dip = getipaddr(dst)
    payload = get_payload(payload_len)
    pkt = fuzz(IP(src=sip, dst=dip)/UDP())/payload
    # pkt.show2()
    # os.write(2, str(pkt))
    return str(pkt)

def get_arg(n):
    arg = sys.argv[n] if n < len(sys.argv) else None
    n += 1
    return n, arg

def get_ip_arg(n):
    n, arg = get_arg(n)
    arg = None if arg == '' else arg
    return n, arg

def get_len_arg(n):
    n, arg = get_arg(n)
    arg = -1 if arg == '' or arg is None else int(arg)
    return n, arg

if __name__ == '__main__':
    walk = 1
    walk, sip = get_ip_arg(walk)
    walk, dip = get_ip_arg(walk)
    _, payload_len = get_len_arg(walk)

    pkt = gen_udp_pkt(src=sip, dst=dip, payload_len=payload_len)
    os.write(1, pkt)
