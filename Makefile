export GITHASH 		:= $(shell git -c safe.directory=$(CURDIR) rev-parse --short HEAD 2>/dev/null || echo unknown)
export VERSION := 5.6.0
export API_VERSION 	:= 8
export WANT_FLAC 	:= 1
export WANT_MP3 	:= 1
export WANT_WAV 	:= 1

# Используем закреплённую ревизию открытого libryazhahand владельца.
# Этот pin содержит Switch 2 style renderer (ult::useSwitch2Style), который
# используется оверлеем как штатная библиотечная возможность. Изменять pin
# можно только вместе с полной devkitA64-проверкой CI.
LIBRYAZHAHAND_REPO ?= https://github.com/Dimasick-git/libryazhahand.git
LIBRYAZHAHAND_PIN  ?= fd11fe0e31a3c73a293504710a8336a5c74d4254
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
			mkdir -p dist/config/RyazhTune/lang
			touch dist/atmosphere/contents/420000000000000E/flags/boot2.flag
			cp RyazhTune/RyazhTune.nsp dist/atmosphere/contents/420000000000000E/exefs.nsp
			cp overlay/RyazhTune-Overlay.ovl dist/switch/.overlays/
			cp overlay/lang/*.json dist/config/RyazhTune/lang/
		cp RyazhTune/toolbox.json dist/atmosphere/contents/420000000000000E/
	cd dist; zip -r RyazhTune-$(VERSION)-$(GITHASH).zip ./**/; cd ../;
	-hactool -t nso RyazhTune/RyazhTune.nso

.PHONY: all clean overlay nxExt module dist prepare-overlay-lib
