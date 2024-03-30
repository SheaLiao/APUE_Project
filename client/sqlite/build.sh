#!/bin/bash

# library name and version
# Official: https://www.sqlite.org/index.html
LIB_NAME=sqlite-autoconf-3430000
PACK_SUFIX=tar.gz

# LingYun source code FTP server
LY_FTP=http://main.iot-yun.club:2211/src/

# library download URL address
LIB_URL=$LY_FTP

# Cross compiler for cross compile on Linux server
CROSS_COMPILE=arm-linux-gnueabihf-

# compile jobs
JOBS=`cat /proc/cpuinfo |grep "processor"|wc -l`

# this project absolute path
PRJ_PATH=$(cd $(dirname "${BASH_SOURCE[0]}") && pwd)

# top project absolute path
TOP_PATH=$(realpath $PRJ_PATH/..)

# binaries install path
PREFIX_PATH=$PRJ_PATH/install
BIN_PATH=$PREFIX_PATH/bin
LIB_PATH=$PREFIX_PATH/lib
INC_PATH=$PREFIX_PATH/include

# check installed or not file
INST_FILE=$PREFIX_PATH/bin/sqlite3

# shell script will exit once get command error
set -e

#+-------------------------+
#| Shell script functions  |
#+-------------------------+

function pr_error() {
	echo -e "\033[40;31m $1 \033[0m"
}

function pr_warn() {
	echo -e "\033[40;33m $1 \033[0m"
}

function pr_info() {
	echo -e "\033[40;32m $1 \033[0m"
}

function check_result()
{
	if [ $? != 0 ] ; then
		pr_error $1
	fi
}
# decompress a packet to destination path
function do_unpack()
{
	tarball=$1
	dstpath=`pwd`

	if [[ $# == 2 ]] ; then
		dstpath=$2
	fi

	pr_info "decompress $tarball => $dstpath"

	mkdir -p $dstpath
	case $tarball in
		*.tar.gz)
			tar -xzf $tarball -C $dstpath
			;;

		*.tar.bz2)
			tar -xjf $tarball -C $dstpath
			;;

		*.tar.xz)
			tar -xJf $tarball -C $dstpath
			;;

		*.tar.zst)
			tar -I zstd -xf $tarball -C $dstpath
			;;

		*.tar)
			tar -xf $tarball -C $dstpath
			;;

		*.zip)
			unzip -qo $tarball -d $dstpath
			;;

		*)
			pr_error "decompress Unsupport packet: $tarball"
			return 1;
			;;
	esac
}

function do_export()
{
	BUILD_ARCH=$(uname -m)
	if [[ $BUILD_ARCH =~ "arm" ]] ; then
		pr_warn "local($BUILD_ARCH) compile $LIB_NAME"
		return ;
	fi

	pr_warn "cross(${CROSS_COMPILE}) compile $LIB_NAME"

	# export cross toolchain
	export CC=${CROSS_COMPILE}gcc
	export CXX=${CROSS_COMPILE}g++
	export AS=${CROSS_COMPILE}as
	export AR=${CROSS_COMPILE}ar
	export LD=${CROSS_COMPILE}ld
	export NM=${CROSS_COMPILE}nm
	export RANLIB=${CROSS_COMPILE}ranlib
	export OBJDUMP=${CROSS_COMPILE}objdump
	export STRIP=${CROSS_COMPILE}strip

	# export cross configure
	export CONFIG_CROSS=" --build=i686-pc-linux --host=arm-linux "

	# Clear LDFLAGS and CFLAGS
	export LDFLAGS=
	export CFLAGS=
}

function do_fetch()
{
	if [ -e ${INST_FILE} ] ; then
		pr_warn "$LIB_NAME compile and installed alredy"
		exit ;
	fi

	if [ -d $LIB_NAME ] ; then
		pr_warn "$LIB_NAME fetch already"
		return ;
	fi

	if [ ! -f ${LIB_NAME}.${PACK_SUFIX} ] ; then
		wget ${LIB_URL}/${LIB_NAME}.${PACK_SUFIX}
		check_result "ERROR: download ${LIB_NAME} failure"
	fi

	do_unpack ${LIB_NAME}.${PACK_SUFIX}
}

function do_build()
{
	cd $LIB_NAME

	do_export

	./configure --prefix=${PREFIX_PATH} ${CONFIG_CROSS} --enable-static --enable-static-shell
	check_result "ERROR: configure ${LIB_NAME} failure"

	make && make install
	check_result "ERROR: compile ${LIB_NAME} failure"
}

function do_clean()
{
	rm -rf *${LIB_NAME}*
}

if [[ $# == 1 && $1 == -c ]] ;then
	pr_warn "start clean ${LIB_NAME}"
	do_clean
	exit;
fi

do_fetch

do_build

