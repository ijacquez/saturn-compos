ifeq ($(strip $(YAUL_INSTALL_ROOT)),)
  $(error Undefined YAUL_INSTALL_ROOT (install root directory))
endif

include $(YAUL_INSTALL_ROOT)/share/build.pre.mk

SH_PROGRAM:= port
SH_SRCS:= \
	animation.c \
	blocks.c \
	collision.c \
	enemies.c \
	level.c \
	main.c \
	pcmsys.c \
	player.c \
	print.c \
	scroll.c \
	sound.c \
	sprite.c \
	vdp1_hw.c \
	spritecode/cannon.c \
	spritecode/explosion.c \
	spritecode/float.c \
	spritecode/missile.c \
	spritecode/worm.c \
	graphics/sprs.c

ifeq ($(strip $(PORT_USE_FILECLIENT)),)
SH_SRCS+= \
	cd_iso9660.c
else
$(error No longer supported)
SH_SRCS+= \
	cd_fileclient.c
endif

SH_LIBRARIES:=
SH_CFLAGS+= -DDEBUG -std=c11 -O2 -save-temps -g -Wno-unused -Wno-unused-parameter -Wno-error

IP_VERSION:= V1.000
IP_RELEASE_DATE:= 20160101
IP_AREAS:= JTUBKAEL
IP_PERIPHERALS:= JAMKST
IP_TITLE:= Compo entry 2020
IP_MASTER_STACK_ADDR:= 0x06004000
IP_SLAVE_STACK_ADDR:= 0x06001E00
IP_1ST_READ_ADDR:= 0x06004000
IP_1ST_READ_SIZE:= 0

include $(YAUL_INSTALL_ROOT)/share/build.post.iso-cue.mk
