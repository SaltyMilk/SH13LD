FROM debian:buster
MAINTAINER "sel-melc@student.s19.be"

RUN mkdir /woody
VOLUME ./srcs /woody
# Environment setup
RUN apt-get update\
	&& apt-get install git -y\
	&&apt-get install sudo -y\
	&& apt-get install man -y\
	&& apt-get install nasm -y\
	&& apt-get install gcc -y\
	&& apt-get install make -y\
	&& apt-get install vim -y

#Basic settings for vim and gdb
RUN echo "set nu\nsyn on\nset mouse=a\nset tabstop=4">$HOME/.vimrc
RUN echo "set disassembly-flavor intel">$HOME/.gdb

RUN echo "deb mirror://mirrors.ubuntu.com/mirrors.txt precise main restricted universe multiverse">>etc/apt/sourcs.list;\
	echo "deb mirror://mirrors.ubuntu.com/mirrors.txt precise-updates main restricted universe multiverse">>etc/apt/sourcs.list;\
	echo "deb mirror://mirrors.ubuntu.com/mirrors.txt precise-backports main restricted universe multiverse">>etc/apt/sourcs.list;\
	echo "deb mirror://mirrors.ubuntu.com/mirrors.txt precise-security main restricted universe multiverse">>etc/apt/sourcs.list;
RUN echo "foreign-architecture i386" >>/etc/dpkg/dpkg.cfg.d/multiarch &&  rm /etc/dpkg/dpkg.cfg.d/multiarch\
&& apt-get install gcc-multilib -y
# Tools installation
# Feel free to add tools you need here
RUN apt-get install gdb -y

CMD while true; do sleep infinity; done
