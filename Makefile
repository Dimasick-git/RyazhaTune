export GITHASH 		:= $(shell git rev-parse --short HEAD)
export VERSION 		:= 5.0.0
export API_VERSION 	:= 4
export WANT_FLAC 	:= 1
export WANT_MP3 	:= 1
export WANT_WAV 	:= 1

all: overlay nxExt module

clean:
	$(MAKE) -C RyazhaTune/nxExt clean
	$(MAKE) -C overlay clean
	$(MAKE) -C RyazhaTune clean
	-rm -r dist
	-rm RyazhaTune-*-*.zip

overlay:
	$(MAKE) -C overlay

nxExt:
	$(MAKE) -C RyazhaTune/nxExt

module:
	$(MAKE) -C RyazhaTune

dist: all
	mkdir -p dist/switch/.overlays
		mkdir -p dist/atmosphere/contents/420000000000000E/flags
		touch dist/atmosphere/contents/420000000000000E/flags/boot2.flag
		cp RyazhaTune/RyazhaTune.nsp dist/atmosphere/contents/420000000000000E/exefs.nsp
		cp overlay/RyazhaTune.ovl dist/switch/.overlays/
		cp RyazhaTune/toolbox.json dist/atmosphere/contents/420000000000000E/
	cd dist; zip -r RyazhaTune-$(VERSION)-$(GITHASH).zip ./**/; cd ../;
	-hactool -t nso RyazhaTune/RyazhaTune.nso

.PHONY: all overlay module
