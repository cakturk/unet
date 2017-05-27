CFLAGS = -g3 -O2 -Wall -Wno-unused -pipe -fno-strict-aliasing
srcdir = ../

# For more information about 'rwildcard' please visit
# http://blog.jgc.org/2011/07/gnu-make-recursive-wildcard-function.html
rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

unwanted_obj := $(addprefix $(srcdir),tuntap_if.o netsniff.o main.o)
allobjects := $(patsubst %.c,%.o,$(call rwildcard,../,*.c))
allobjects := $(filter-out $(unwanted_obj),$(allobjects))

incsources := $(patsubst test-%.o,%.o,$(filter test-%.o,%.o,$(notdir $(allobjects))))
OBJECTS := $(filter-out $(addprefix $(srcdir),$(incsources)),$(allobjects))

PROGRAM = unet-test-runner
LDLIBS =
LDFLAGS += $(EXTRA_LDFLAGS)

ifndef ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	E_CC	= @echo ' CC    '$@;
	E_LD	= @echo ' LD    '$@;
endif
endif

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)

dep_files := $(foreach f, $(OBJECTS),$(dir $f).depend/$(notdir $f).d)
dep_dirs := $(addsuffix .depend,$(sort $(dir $(OBJECTS))))

$(dep_dirs):
	@mkdir -p $@

missing_dep_dirs := $(filter-out $(wildcard $(dep_dirs)),$(dep_dirs))
dep_file = $(dir $@).depend/$(notdir $@).d
dep_args = -MF $(dep_file) -MQ $@ -MMD -MP

dep_files_present := $(wildcard $(dep_files))
ifneq ($(dep_files_present),)
include $(dep_files_present)
endif

%.o: %.c $(missing_dep_dirs)
	$(E_CC)$(CC) $(CFLAGS) $(CPPFLAGS) -c $(dep_args) $< -o $@

$(PROGRAM): $(OBJECTS)
	$(E_LD)$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJECTS) $(LDLIBS)

.PHONY: clean
clean:
	-$(RM) -r $(PROGRAM) $(OBJECTS) *~ core.* $(dep_dirs)

check: $(PROGRAM)
	@./$(PROGRAM)
