FROM ubuntu:22.04

ENV USER=pingme

# install dependencie
RUN apt-get -y update
RUN apt-get -y upgrade
RUN apt-get -y install build-essential git curl unzip software-properties-common
RUN add-apt-repository ppa:xmake-io/xmake
RUN apt-get -y update
RUN apt-get -y install xmake

# create pingme user
RUN groupadd -r ${USER} && \
	useradd --create-home --home /home/pingme -r -g ${USER} ${USER}

USER ${USER}

# building
WORKDIR /home/pingme/src
# copy all files
COPY --chown=${USER}:${USER} . .
# compile
RUN xmake package -y
# move build file to execution location
RUN mkdir /home/pingme/bot
RUN mv /home/pingme/src/build/packages/p/pingme/linux/x86_64/release/bin/pingme /home/pingme/bot/

# running
WORKDIR /home/pingme/bot
CMD ["/home/pingme/bot/pingme"]
