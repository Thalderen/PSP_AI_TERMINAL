TARGET = PSP_AI_TERMINAL
OBJS = src/main.o src/core/app.o src/core/commands.o src/ui/render.o src/ui/animations.o src/network/http_client.o src/audio/mic.o

INCDIR =
CFLAGS = -O2 -G0 -Wall -Wextra
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lpspgum -lpspgu -lpsprtc -lpspaudio -lpsputility -lpspnet -lpspnet_apctl -lpsphttp -lpspssl -lpspwlan

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP AI Terminal

BUILD_PRX = 0

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
