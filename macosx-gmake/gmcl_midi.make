ifndef config
  config=release_x32
endif

.PHONY: clean prebuild prelink

ifeq ($(config),release_x32)
  ifeq ($(origin CC), default)
    CC = clang
  endif
  ifeq ($(origin CXX), default)
    CXX = clang++
  endif
  ifeq ($(origin AR), default)
    AR = ar
  endif
  TARGETDIR = ../lib/linux/x32
  TARGET = $(TARGETDIR)/libgmcl_midi.dylib
  OBJDIR = obj/x32
  DEFINES += -DNDEBUG -DGMMODULE -D__MACOSX_CORE__
  INCLUDES += -I../src/GarrysMod/Lua -I../src/RtMidi
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m32 -ffast-math -O2 -fPIC -g -msse
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -m32 -ffast-math -O2 -fPIC -g -msse
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += -framework CoreServices -framework CoreAudio -framework CoreMIDI -framework CoreFoundation
  LDDEPS +=
  ALL_LDFLAGS += $(LDFLAGS) -m32 -dynamiclib -Wl,-install_name,@rpath/libgmcl_midi.dylib
  LINKCMD = $(CXX) -o "$@" $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: prebuild prelink $(TARGET)
	@:

endif

ifeq ($(config),release_x64)
  ifeq ($(origin CC), default)
    CC = clang
  endif
  ifeq ($(origin CXX), default)
    CXX = clang++
  endif
  ifeq ($(origin AR), default)
    AR = ar
  endif
  TARGETDIR = ../lib/linux/x64
  TARGET = $(TARGETDIR)/libgmcl_midi.dylib
  OBJDIR = obj/x64
  DEFINES += -DNDEBUG -DGMMODULE -D__MACOSX_CORE__
  INCLUDES += -I../src/GarrysMod/Lua -I../src/RtMidi
  FORCE_INCLUDE +=
  ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
  ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m64 -ffast-math -O2 -fPIC -g -msse
  ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -m64 -ffast-math -O2 -fPIC -g -msse
  ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
  LIBS += -framework CoreServices -framework CoreAudio -framework CoreMIDI -framework CoreFoundation
  LDDEPS +=
  ALL_LDFLAGS += $(LDFLAGS) -m64 -dynamiclib -Wl,-install_name,@rpath/libgmcl_midi.dylib
  LINKCMD = $(CXX) -o "$@" $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
all: prebuild prelink $(TARGET)
	@:

endif

OBJECTS := \
	$(OBJDIR)/RtMidi.o \
	$(OBJDIR)/gmcl_midi.o \

RESOURCES := \

CUSTOMFILES := \

SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
	SHELLTYPE := msdos
endif

$(TARGET): $(GCH) ${CUSTOMFILES} $(OBJECTS) $(LDDEPS) $(RESOURCES) | $(TARGETDIR)
	@echo Linking gmcl_midi
	$(LINKCMD)
	$(POSTBUILDCMDS)

$(CUSTOMFILES): | $(OBJDIR)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	mkdir -p $(TARGETDIR)
else
	mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	mkdir -p $(OBJDIR)
else
	mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning gmcl_midi
ifeq (posix,$(SHELLTYPE))
	rm -f  $(TARGET)
	rm -rf $(OBJDIR)
else
	if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild:
	$(PREBUILDCMDS)

prelink:
	$(PRELINKCMDS)

ifneq (,$(PCH))
$(OBJECTS): $(GCH) $(PCH) | $(OBJDIR)
$(GCH): $(PCH) | $(OBJDIR)
	@echo $(notdir $<)
	$(CXX) -x c++-header $(ALL_CXXFLAGS) -o "$@" -MF "$(@:%.gch=%.d)" -c "$<"
else
$(OBJECTS): | $(OBJDIR)
endif

$(OBJDIR)/RtMidi.o: ../src/RtMidi/RtMidi.cpp
	@echo $(notdir $<)
	$(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/gmcl_midi.o: ../src/gmcl_midi.cpp
	@echo $(notdir $<)
	$(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

-include $(OBJECTS:%.o=%.d)
ifneq (,$(PCH))
  -include $(OBJDIR)/$(notdir $(PCH)).d
endif
