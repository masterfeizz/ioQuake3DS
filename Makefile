#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

TARGET		:=	Quake3DS
BUILD		:=	build
SOURCES		:=	code
DATA		:=	code/ctr/assets
INCLUDES	:=	code/renderercommon code/qcommon code/botlib code/client code/server code/renderergl1 code/ctr code/sys
#ROMFS		:=	romfs

APP_TITLE		:= Quake3DS 
APP_DESCRIPTION := Quake III
APP_AUTHOR  	:= MasterFeizz
ICON 			:= icon.png

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-Wall -O2 -mword-relocations \
			-fomit-frame-pointer -ffunction-sections \
			-fdata-sections -fno-short-enums \
			-Wl,--gc-sections $(ARCH)

CFLAGS	+=	$(INCLUDE) -D__3DS__ -D__FLOAT_WORD_ORDER=1 -D__GNU__ -DRELEASE \
			-DUSE_ICON -DARCH_STRING=\"arm\" -DBOTLIB -DUSE_CODEC_VORBIS \
			-DNO_VM_COMPILED -DDEFAULT_BASEDIR=\"sdmc:/ioq3\" \
			-DPRODUCT_VERSION=\"1.0.1\" -DNDEBUG

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) 

LIBS	:=   -lvorbisidec -logg -ljpeg -lz -lpicaGL -lctru -lm

BOTLIB_SOURCES += \
	botlib/be_aas_bspq3.c \
	botlib/be_aas_cluster.c \
	botlib/be_aas_debug.c \
	botlib/be_aas_entity.c \
	botlib/be_aas_file.c \
	botlib/be_aas_main.c \
	botlib/be_aas_move.c \
	botlib/be_aas_optimize.c \
	botlib/be_aas_reach.c \
	botlib/be_aas_route.c \
	botlib/be_aas_routealt.c \
	botlib/be_aas_sample.c \
	botlib/be_ai_char.c \
	botlib/be_ai_chat.c \
	botlib/be_ai_gen.c \
	botlib/be_ai_goal.c \
	botlib/be_ai_move.c \
	botlib/be_ai_weap.c \
	botlib/be_ai_weight.c \
	botlib/be_ea.c \
	botlib/be_interface.c \
	botlib/l_crc.c \
	botlib/l_libvar.c \
	botlib/l_log.c \
	botlib/l_memory.c \
	botlib/l_precomp.c \
	botlib/l_script.c \
	botlib/l_struct.c

CLIENT_SOURCES += \
	client/cl_avi.c \
	client/cl_cgame.c \
	client/cl_cin.c \
	client/cl_console.c \
	client/cl_input.c \
	client/cl_keys.c \
	client/cl_main.c \
	client/cl_net_chan.c \
	client/cl_parse.c \
	client/cl_scrn.c \
	client/cl_ui.c \
	client/snd_adpcm.c \
	client/snd_codec.c \
	client/snd_codec_ogg.c \
	client/snd_codec_wav.c \
	client/snd_dma.c \
	client/snd_main.c \
	client/snd_mem.c \
	client/snd_mix.c \
	client/snd_openal.c \
	client/snd_wavelet.c

GAME_SOURCES += \
	game/ai_chat.c \
	game/ai_cmd.c \
	game/ai_dmnet.c \
	game/ai_dmq3.c \
	game/ai_main.c \
	game/ai_team.c \
	game/ai_vcmd.c \
	game/bg_misc.c \
	game/bg_pmove.c \
	game/bg_slidemove.c \
	game/g_active.c \
	game/g_arenas.c \
	game/g_bot.c \
	game/g_client.c \
	game/g_cmds.c \
	game/g_combat.c \
	game/g_items.c \
	game/g_main.c \
	game/g_mem.c \
	game/g_misc.c \
	game/g_missile.c \
	game/g_mover.c \
	game/g_session.c \
	game/g_spawn.c \
	game/g_svcmds.c \
	game/g_syscalls.c \
	game/g_target.c \
	game/g_team.c \
	game/g_trigger.c \
	game/g_utils.c \
	game/g_weapon.c \
	game/q_math.c \
	game/q_shared.c

COMMON_SOURCES += \
	qcommon/cm_load.c \
	qcommon/cm_patch.c \
	qcommon/cm_polylib.c \
	qcommon/cm_test.c \
	qcommon/cm_trace.c \
	qcommon/cmd.c \
	qcommon/common.c \
	qcommon/cvar.c \
	qcommon/files.c \
	qcommon/huffman.c \
	qcommon/ioapi.c \
	qcommon/md4.c \
	qcommon/md5.c \
	qcommon/msg.c \
	qcommon/net_chan.c \
	qcommon/puff.c \
	qcommon/q_math.c \
	qcommon/q_shared.c \
	qcommon/unzip.c \
	qcommon/vm.c \
	qcommon/vm_interpreted.c \

RENDERER_SOURCES += \
	renderercommon/tr_font.c \
	renderercommon/tr_image_bmp.c \
	renderercommon/tr_image_jpg.c \
	renderercommon/tr_image_pcx.c \
	renderercommon/tr_image_png.c \
	renderercommon/tr_image_tga.c \
	renderercommon/tr_noise.c \
	renderergl1/tr_animation.c  \
	renderergl1/tr_backend.c  \
	renderergl1/tr_bsp.c  \
	renderergl1/tr_cmds.c  \
	renderergl1/tr_curve.c  \
	renderergl1/tr_flares.c  \
	renderergl1/tr_image.c  \
	renderergl1/tr_init.c  \
	renderergl1/tr_light.c  \
	renderergl1/tr_main.c  \
	renderergl1/tr_marks.c  \
	renderergl1/tr_mesh.c  \
	renderergl1/tr_model.c  \
	renderergl1/tr_model_iqm.c  \
	renderergl1/tr_scene.c  \
	renderergl1/tr_shade.c  \
	renderergl1/tr_shade_calc.c  \
	renderergl1/tr_shader.c  \
	renderergl1/tr_shadows.c  \
	renderergl1/tr_sky.c  \
	renderergl1/tr_surface.c  \
	renderergl1/tr_world.c

SERVER_SOURCES += \
	server/sv_init.c \
	server/sv_bot.c \
	server/sv_ccmds.c \
	server/sv_client.c \
	server/sv_game.c \
	server/sv_main.c \
	server/sv_net_chan.c \
	server/sv_snapshot.c \
	server/sv_world.c

PLATFORM_SOURCES += \
	ctr/ctr_con.c \
	ctr/ctr_gamma.c \
	ctr/ctr_glimp.c \
	ctr/ctr_input.c \
	ctr/ctr_snd.c \
	ctr/ctr_net.c \
	ctr/ctr_main.c \
	ctr/ctr_heap.c \
	sys/sys_ctr.c

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CTRULIB) $(DEVKITPRO)/portlibs/3ds $(DEVKITPRO)/picaGL

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)



CFILES		:=	$(SRC_FILES) $(CLIENT_SOURCES) $(SERVER_SOURCES) $(COMMON_SOURCES) $(RENDERER_SOURCES) $(PLATFORM_SOURCES) $(BOTLIB_SOURCES)
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_SOURCES 	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES)) \
			$(PICAFILES:.v.pica=.shbin.o) $(SHLISTFILES:.shlist=.shbin.o)

export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)

export HFILES	:=	$(PICAFILES:.v.pica=_shbin.h) $(SHLISTFILES:.shlist=_shbin.h) $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.png)
	ifneq (,$(findstring $(TARGET).png,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).png
	else
		ifneq (,$(findstring icon.png,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.png
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_SMDH)),)
	export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh
endif

ifneq ($(ROMFS),)
	export _3DSXFLAGS += --romfs=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p build/renderercommon build/game build/qcommon build/botlib build/client build/server build/renderergl1 build/ctr build/sys
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf


#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
ifeq ($(strip $(NO_SMDH)),)
$(OUTPUT).3dsx	:	$(OUTPUT).elf $(OUTPUT).smdh
else
$(OUTPUT).3dsx	:	$(OUTPUT).elf
endif

$(OFILES_SOURCES) : $(HFILES)

$(OUTPUT).elf	:	$(OFILES)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
# rules for assembling GPU shaders
#---------------------------------------------------------------------------------
define shader-as
	$(eval CURBIN := $*.shbin)
	$(eval DEPSFILE := $(DEPSDIR)/$*.shbin.d)
	echo "$(CURBIN).o: $< $1" > $(DEPSFILE)
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u8" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(CURBIN) | tr . _)`.h
	echo "extern const u32" `(echo $(CURBIN) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(CURBIN) | tr . _)`.h
	picasso -o $(CURBIN) $1
	bin2s $(CURBIN) | $(AS) -o $*.shbin.o
endef

%.shbin.o %_shbin.h : %.v.pica %.g.pica
	@echo $(notdir $^)
	@$(call shader-as,$^)

%.shbin.o %_shbin.h : %.v.pica
	@echo $(notdir $<)
	@$(call shader-as,$<)

%.shbin.o %_shbin.h : %.shlist
	@echo $(notdir $<)
	@$(call shader-as,$(foreach file,$(shell cat $<),$(dir $<)$(file)))


-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------