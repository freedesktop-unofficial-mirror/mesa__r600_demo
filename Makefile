VERSION_FLAGS=-DPACKAGE=\"r600_demo\" -DVERSION=\"`git rev-parse --short HEAD`\"

INCLUDES=-I/usr/include/drm
LIBS=-ldrm
CFLAGS:=-Wall -O0 -g $(INCLUDES) $(VERSION_FLAGS) $(CFLAGS)
CC=gcc
CFILES=r600_demo.c r600_lib.c r600_basic.c r600_init.c r600_triangles.c r600_texture.c r600_pm4.c r600_exa.c r600_perf.c

all:  r600_demo convert_shader

convert_shader: convert_shader.c
	$(CC) convert_shader.c -o convert_shader

r600_demo: $(CFILES:.c=.o) $(LIBS)
	$(CC) $(CFILES:.c=.o) $(LIBS) -l m -o r600_demo

r600_reg.h: r600_reg_auto_r6xx.h

clean:
	rm -f *.o *~ *.bak

depend:
	makedepend -Y *.[ch]

tags:
	etags --lang=c --regex='/[ \t]*\([a-z0-9_]+\)[\t ]*=.*/\1/i' *.[ch]

dump:
	./r600_demo r 0-159c 15b4-15ec 2100-3f28 3f34-3ffc 8000-c14c  >register.dump
	./r600_demo "" 28000-28e7c 30000-31ffc 38000-3effc           >>register.dump
	sed -i -e '/ := /p;d' register.dump

fulldump:
	# Just spare UCODE upload, ranges known to lock something up, and
	# large ranges known to be unused.
	# Microcode: 3f2c-3f30, c154-c160
	# Locking up bus: f840-fffc (full range not tested)
	# Unused: f800-f840 10000-27ffc
	./r600_demo r 0-3f28 3f34-c150 c164-f83c 28000-3fffc  >register.dump
	sed -i -e '/ := /p;d' register.dump

screenshot:
	import -window root -crop 260x260+0+0 png:screenshot.png

tarball:
	tar -cvzf ../r600_demo_git-`git-rev-parse --short HEAD`_`date +%Y-%m-%d`.tgz -C .. `git-ls-tree -r --name-only HEAD | sed 's|^|r600_demo/|'`

# DO NOT DELETE

r600_basic.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h r600_reg_r7xx.h
r600_basic.o: r600_emit.h r600_hwapi.h r600_lib.h
r600_broken.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h
r600_broken.o: r600_reg_r7xx.h r600_emit.h r600_hwapi.h r600_lib.h
r600_broken.o: r600_shader.h
r600_demo.o: radeon_drm.h r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h
r600_demo.o: r600_reg_r7xx.h r600_lib.h r600_hwapi.h
r600_emit.o: r600_hwapi.h
r600_exa.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h r600_reg_r7xx.h
r600_exa.o: r600_emit.h r600_hwapi.h r600_lib.h r600_state.h r600_init.h
r600_exa.o: r600_shader.h
r600_init.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h r600_reg_r7xx.h
r600_init.o: r600_emit.h r600_hwapi.h r600_lib.h r600_state.h
r600_lib.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h r600_reg_r7xx.h
r600_lib.o: r600_emit.h r600_hwapi.h r600_lib.h r600_shader.h radeon_drm.h
r600_perf.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h r600_reg_r7xx.h
r600_perf.o: r600_emit.h r600_hwapi.h r600_lib.h r600_state.h r600_init.h
r600_perf.o: r600_shader.h
r600_pm4.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h r600_reg_r7xx.h
r600_pm4.o: r600_emit.h r600_hwapi.h r600_lib.h r600_state.h r600_init.h
r600_pm4.o: r600_shader.h
r600_reg.o: r600_reg_auto_r6xx.h r600_reg_r6xx.h r600_reg_r7xx.h
r600_texture.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h
r600_texture.o: r600_reg_r7xx.h r600_emit.h r600_hwapi.h r600_lib.h
r600_texture.o: r600_state.h r600_init.h r600_shader.h
r600_triangles.o: r600_reg.h r600_reg_auto_r6xx.h r600_reg_r6xx.h
r600_triangles.o: r600_reg_r7xx.h r600_emit.h r600_hwapi.h r600_lib.h
r600_triangles.o: r600_state.h r600_init.h r600_shader.h
