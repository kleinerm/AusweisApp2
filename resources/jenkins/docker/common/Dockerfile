FROM alpine:swarm
MAINTAINER Governikus KG <support@ausweisapp.de>

ARG UNCRUSTIFY=0.74.0
ENV NAME=Common LABELS=Common

RUN mkdir /build/ && \
    wget -O uncrustify.tar.gz https://github.com/uncrustify/uncrustify/archive/uncrustify-$UNCRUSTIFY.tar.gz && \
    tar xf uncrustify.tar.gz && \
    apk --no-cache --virtual deps add g++ samurai && \
    cmake uncrustify-uncrustify-$UNCRUSTIFY -GNinja -DCMAKE_BUILD_TYPE=Release -B build && \
    cmake --build build && \
    cmake --install build && \
    apk --no-cache del deps && \
    rm -rf /build


USER governikus

ENTRYPOINT ["/sbin/tini", "--"]
CMD /swarm.sh
