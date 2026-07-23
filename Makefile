export GITHASH 		:= $(shell git -c safe.directory=$(CURDIR) rev-parse --short HEAD 2>/dev/null || echo unknown)
export VERSION := 5.0.1
export API_VERSION 	:= 5
export WANT_FLAC 	:= 1
export WANT_MP3 	:= 1
export WANT_WAV 	:= 1

# Используем наш форк libultrahand: ryazhahand.mk там уже лежит,
# никакой compat-обёртки подкладывать не надо. URL канонический Dimasick-git
# (старый аккаунт Dimanchikgshehsbshene -- зеркало). Pin на fe699fd = ветка
# compat/gc-legacy-67f4486: снапшот 67f4486 (do/while TSL_R_TRY, GCC 15 ok,
# старый API — на нём этот оверлей собирается) + однострочный GC-фикс
# padConfigureInput|NpadGc (issue #33). Прямой бамп на новый main (856ddbd+)
# ломает сборку — апгрейд API оверлея трекается в issue #36.
# 1b3bcc0 = ПОЛНЫЙ снапшот 67f4486 (включая ultra.hpp, в main переименованный
# в ryz.hpp) + GC-фикс; недоделанное компат-дерево fe699fd роняло make.
LIBRYAZHAHAND_REPO ?= https://github.com/Dimasick-git/libryazhahand.git
LIBRYAZHAHAND_PIN  ?= 1b3bcc0e30a90c878a7a274fd32e5d9bc6a9cec1
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
