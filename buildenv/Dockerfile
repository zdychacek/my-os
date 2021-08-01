FROM ubuntu:impish

ARG DEBIAN_FRONTEND=noninteractive

ADD build-toolchain.sh /opt/build-toolchain.sh

RUN /opt/build-toolchain.sh

ENV PATH "/opt/toolchain:$PATH"
ENV MYOS "true"
ENV BUILDROOT "/opt/"
WORKDIR /opt
