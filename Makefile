export GITHASH 		:= $(shell git -c safe.directory=$(CURDIR) rev-parse --short HEAD 2>/dev/null || echo unknown)
export VERSION := 5.0.0
export API_VERSION 	:= 4
export WANT_FLAC 	:= 1
export WANT_MP3 	:= 1
export WANT_WAV 	:= 1

LIBULTRAHAND_REPO ?= https://github.com/ppkantorski/libultrahand.git
RYAZHAHAND_DIR   ?= overlay/lib/libryazhahand
LEGACY_HAND_MK  := $(RYAZHAHAND_DIR)/$(subst ryazha,ultra,ryazhahand).mk

all: overlay nxExt module

clean:
	$(MAKE) -C RyazhTune/nxExt clean
	$(MAKE) -C overlay clean
	$(MAKE) -C RyazhTune clean
	-rm -r dist
	-rm RyazhTune-*-*.zip

prepare-overlay-lib:
	@if [ ! -d "$(RYAZHAHAND_DIR)/.git" ]; then \
		echo "Cloning libultrahand into $(RYAZHAHAND_DIR)..."; \
		rm -rf "$(RYAZHAHAND_DIR)"; \
		mkdir -p "$(dir $(RYAZHAHAND_DIR))"; \
		git clone --depth 1 "$(LIBULTRAHAND_REPO)" "$(RYAZHAHAND_DIR)"; \
	fi
	@if [ ! -f "$(RYAZHAHAND_DIR)/ryazhahand.mk" ]; then \
		if [ -f "$(LEGACY_HAND_MK)" ]; then \
			echo "Installing ryazhahand.mk compatibility makefile..."; \
			cp "$(LEGACY_HAND_MK)" "$(RYAZHAHAND_DIR)/ryazhahand.mk"; \
		else \
			echo "Missing $(RYAZHAHAND_DIR)/ryazhahand.mk" >&2; \
			exit 1; \
		fi; \
	fi

overlay: prepare-overlay-lib
	$(MAKE) -C overlay

nxExt:
	$(MAKE) -C RyazhTune/nxExt

module: nxExt
	$(MAKE) -C RyazhTune

dist: all
	rm -rf dist
	mkdir -p dist/switch/.overlays
		mkdir -p dist/atmosphere/contents/420000000000000E/flags
		mkdir -p dist/config/ryazhahand/lang
		touch dist/atmosphere/contents/420000000000000E/flags/boot2.flag
		cp RyazhTune/RyazhTune.nsp dist/atmosphere/contents/420000000000000E/exefs.nsp
		cp overlay/RyazhTune-Overlay.ovl dist/switch/.overlays/
		cp overlay/lang/*.json dist/config/ryazhahand/lang/
		cp RyazhTune/toolbox.json dist/atmosphere/contents/420000000000000E/
	cd dist; zip -r RyazhTune-$(VERSION)-$(GITHASH).zip ./**/; cd ../;
	-hactool -t nso RyazhTune/RyazhTune.nso

.PHONY: all clean overlay nxExt module dist prepare-overlay-lib
