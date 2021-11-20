ifndef config
  config=release_x32
endif

ifeq ($(config),release_x32)
  gmcl_midi_config = release_x32
endif
ifeq ($(config),release_x64)
  gmcl_midi_config = release_x64
endif

PROJECTS := gmcl_midi

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

gmcl_midi:
ifneq (,$(gmcl_midi_config))
	@echo "==== Building gmcl_midi ($(gmcl_midi_config)) ===="
	@${MAKE} --no-print-directory -C . -f gmcl_midi.make config=$(gmcl_midi_config)
endif

clean:
	@${MAKE} --no-print-directory -C . -f gmcl_midi.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  release_x32"
	@echo "  release_x64"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   gmcl_midi"

