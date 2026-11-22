SDK     ?= ../MacKernelSDK
OBJS     = build/GalaxyBookKbd.o build/kmod_info.o
OUT      = build/GalaxyBookKbd.kext
ARCH     = -arch x86_64 -mmacosx-version-min=10.15
CXXFLAGS = $(ARCH) -std=gnu++17 -O2 -Wall -Werror -nostdinc -isystem $(SDK)/Headers -mkernel -fapple-kext \
           -fno-rtti -fno-exceptions -fno-builtin -fno-common -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT
CFLAGS   = $(ARCH) -std=gnu11 -O2 -nostdinc -isystem $(SDK)/Headers -mkernel -fno-builtin -fno-common -DKERNEL
CCKEXT   = $(shell clang -print-resource-dir)/lib/darwin/libclang_rt.cc_kext.a
LDFLAGS  = $(ARCH) -nostdlib -Xlinker -kext -Xlinker -export_dynamic -L$(SDK)/Library/x86_64 -lkmod $(wildcard $(CCKEXT))

all: $(OUT) build/kbdlight

build/GalaxyBookKbd.o: src/GalaxyBookKbd.cpp src/GalaxyBookKbd.hpp
	@mkdir -p build
	clang++ $(CXXFLAGS) -c $< -o $@
build/kmod_info.o: src/kmod_info.c
	@mkdir -p build
	clang $(CFLAGS) -c $< -o $@
$(OUT): $(OBJS) src/Info.plist
	rm -rf $(OUT) && mkdir -p $(OUT)/Contents/MacOS
	clang++ $(LDFLAGS) $(OBJS) -o $(OUT)/Contents/MacOS/GalaxyBookKbd
	cp src/Info.plist $(OUT)/Contents/Info.plist
	codesign -f -s - $(OUT)
build/kbdlight: tools/kbdlight.c
	@mkdir -p build
	clang -O2 -Wall -framework IOKit -framework CoreFoundation $< -o $@
clean:
	rm -rf build
