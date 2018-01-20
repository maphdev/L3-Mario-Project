
LIB_MODE=1

PROGRAM	:= game

# Must be the first rule
.PHONY: default
default: $(PROGRAM)

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:src/%.c=obj/%.o)
DEPENDS := $(SOURCES:src/%.c=deps/%.d)

MAKEFILES := Makefile

CUSTOM_OBJ := obj/mapio.o obj/tempo.o obj/helplib.o
LIB	:= lib/libgame.a

#CC=gcc
CFLAGS := -O3 -g -std=c99 -Wall -Wno-unused-function
CFLAGS += -DPADAWAN
CFLAGS += -I./include
CFLAGS += $(shell pkg-config SDL2_image SDL2_mixer --cflags)
LDLIBS := $(shell pkg-config SDL2_image SDL2_mixer --libs)
LDLIBS += -lpthread

$(OBJECTS): $(MAKEFILES)

ifndef LIB_MODE
$(LIB): $(filter-out $(CUSTOM_OBJ),$(OBJECTS))
	ar -crs $(LIB) $^
endif

$(PROGRAM): $(CUSTOM_OBJ) $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(OBJECTS): obj/%.o: src/%.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: depend
depend: $(DEPENDS)

$(DEPENDS): $(MAKEFILES)

$(DEPENDS): deps/%.d: src/%.c
	$(CC) $(CFLAGS) -MM $< | \
		sed -e 's|\(.*\)\.o:|deps/\1.d obj/\1.o:|g' > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

missing_lib:
	sudo apt-get install libsdl2-2.0
	sudo apt-get install libsdl2-dev
	sudo apt-get install libsdl2-mixer-dev
	sudo apt-get install libsdl2-image-dev

.PHONY: clean
clean: 
	rm -f game obj/*.o deps/*.d
