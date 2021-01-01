ifeq ($(strip $(YAUL_INSTALL_ROOT)),)
  $(error Undefined YAUL_INSTALL_ROOT (install root directory))
endif

include $(YAUL_INSTALL_ROOT)/share/pre.common.mk

SH_PROGRAM:= port
SH_OBJECTS:= \
	animation.o \
	blocks.o \
	collision.o \
	enemies.o \
	level.o \
	main.o \
	pcmsys.o \
	player.o \
	print.o \
	scroll.o \
	sound.o \
	sprite.o \
	vdp1_hw.o \
	spritecode/cannon.o \
	spritecode/explosion.o \
	spritecode/float.o \
	spritecode/missile.o \
	spritecode/worm.o \
	graphics/sprs.o

ifeq ($(strip $(PORT_USE_FILECLIENT)),)
SH_OBJECTS+= \
	cd_iso9660.o
else
SH_OBJECTS+= \
	cd_fileclient.o
endif

SH_LIBRARIES:=
SH_CFLAGS+= -O2 -save-temps -ggdb3 -Wno-unused -Wno-unused-parameter -Wno-error

IP_VERSION:= V1.000
IP_RELEASE_DATE:= 20160101
IP_AREAS:= JTUBKAEL
IP_PERIPHERALS:= JAMKST
IP_TITLE:= Compo entry 2020
IP_MASTER_STACK_ADDR:= 0x06004000
IP_SLAVE_STACK_ADDR:= 0x06001000
IP_1ST_READ_ADDR:= 0x06004000

M68K_PROGRAM:=
M68K_OBJECTS:=

include $(YAUL_INSTALL_ROOT)/share/post.common.mk
