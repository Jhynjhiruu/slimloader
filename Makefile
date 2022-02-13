TARGET = slimloader

C_SOURCES = src\cstart.c src\main.c src\small_chars_font.c

MEM_MODEL = l

CC88 := cc88
C88 := c88
ALC88 := alc88
LC88 := lc88

MKDIR := mkdir
RM := del
RMDIR = rmdir /S /Q

PY := python3

LDFLAGS += -Ml -d pokemini -tmp
CFLAGS += -Ml

LDFLAGS += -cl
CFLAGS += -I$(PRODDIR)/../include

OBJS += $(C_SOURCES:.c=.obj)

.SUFFIXES:
.SUFFIXES: .min .bin .sa .map .abs .c .obj .out

.PHONY: all, run, assembly, combined

all: $(TARGET).min

combined: all in.min in2.min in3.min in4.min
	$(PY) tools/mod.py combined.min 0 0 0 in.min 0x2100 0x2100 2 $(TARGET).min 0x41300 0x41300 1 $(TARGET).min 0x41400 0x41400 0 $(TARGET).min 0x80000 0 0 in2.min 0x100000 0 0 in3.min 0x180000 0 0 in4.min

$(TARGET).min: obj/$(TARGET).bin

obj/$(TARGET).bin: obj/$(TARGET).sa

obj/$(TARGET).sa $(TARGET).map: obj/$(TARGET).out

.bin.min:
	$(PY) tools/convertpm.py $< $@

.sa.bin:
	$(PY) tools/srec.py $< $@

.out.sa:
	$(ALC88) . $(<F) $(@F:.sa=.inf)

obj/$(TARGET).out: $(OBJS)
	-$(MKDIR) obj
	$(CC88) $(LDFLAGS) -o $@ $!

.c.s:
	$(CC88) $(CFLAGS) -cs -v -o $@ $<

.asm.obj:
	$(CC88) $(CFLAGS) -c -v -o $@ $<

.c.obj:
	$(CC88) $(CFLAGS) -c -v -o $@ $<

clean:
	$(RM) $(OBJS)
	$(RMDIR) obj
	$(RM) $(TARGET).min $(TARGET).map combined.min

