FROM alpine:3.17
MAINTAINER Governikus KG <support@ausweisapp.de>

ARG JENKINS_SWARM_VERSION=3.39
ENV EXECUTOR=3 LABELS= NAME= PASSWORD=

RUN adduser governikus -s /bin/sh -D
RUN apk --no-cache add openjdk11-jre-headless cmake samurai make tini mercurial py3-pip py3-hglib wget && \
    pip3 install rbtools
RUN wget -O /swarm-client.jar https://repo.jenkins-ci.org/releases/org/jenkins-ci/plugins/swarm-client/$JENKINS_SWARM_VERSION/swarm-client-$JENKINS_SWARM_VERSION.jar

ADD swarm.sh /
