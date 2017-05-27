# vim: set ft=dockerfile:
FROM alpine

RUN apk add --no-cache make gcc libc-dev linux-headers py2-pip && \
    pip install --upgrade --no-cache-dir pip && \
    pip install --no-cache-dir scapy
