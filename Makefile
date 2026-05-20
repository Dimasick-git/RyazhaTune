export GITHASH 		:= $(shell git -c safe.directory=$(CURDIR) rev-parse --short HEAD 2>/dev/null || echo unknown)
export VERSION := 5.0.0
export API_VERSION 	:= 5
export WANT_FLAC 	:= 1
export WANT_MP3 	:= 1
export WANT_WAV 	:= 1

# Используем наш форк libultrahand: ryazhahand.mk там уже лежит,
# никакой compat-обёртки подкладывать не надо. Pin на 9a7d930 -- это
# та же ревизия, на которой собирается RCU и Ryazha-Status-Monitor под
# GCC 15 в devkitpro-контейнере; новый upstream 9e76f39+ временно
# не годится из-за регрессии tesla.hpp.
LIBRYAZHAHAND_REPO ?= https://github.com/Dimanchikgshehsbshene/libryazhahand.git
LIBRYAZHAHAND_PIN  ?= 9a7d9300541e1d41a95a5bb285ecdc2ce88f3cc4
RYAZHAHAND_DIR     ?= overlay/lib/libryazhahand

all: overlay nxExt module

clean:
	$(MAKE) -C RyazhTune/nxExt clean
	$(MAKE) -C overlay clean
	$(MAKE) -C RyazhTune clean
	-rm -r dist
	-rm RyazhTune-*-*.zip

prepare-overlay-lib:
	@if [ ! -d "$(RYAZHAHAND_DIR)/.git" ]; then \
		echo "Cloning libryazhahand into $(RYAZHAHAND_DIR)..."; \
		rm -rf "$(RYAZHAHAND_DIR)"; \
		mkdir -p "$(dir $(RYAZHAHAND_DIR))"; \
		git clone "$(LIBRYAZHAHAND_REPO)" "$(RYAZHAHAND_DIR)"; \
	fi
	@cd "$(RYAZHAHAND_DIR)" && \
		git fetch --quiet origin "$(LIBRYAZHAHAND_PIN)" 2>/dev/null || true; \
		git checkout --quiet "$(LIBRYAZHAHAND_PIN)"
	@if [ ! -f "$(RYAZHAHAND_DIR)/ryazhahand.mk" ]; then \
		echo "Missing $(RYAZHAHAND_DIR)/ryazhahand.mk -- bad clone?" >&2; \
		exit 1; \
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
