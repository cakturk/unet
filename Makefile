CFLAGS = -g3 -O2 -Wall -pipe -fno-strict-aliasing
OBJECTS = program_options.o eth_print.o ip_print.o \
	  netif.o etherif.o arp.o udp_print.o tcp_print.o \
	  mbuf.o ip.o udp.o checksum.o ip_cmd.o hwaddr_cmd.o \
	  route_cmd.o nc_cmd.o shell.o main.o
PROGRAM = unet
LDLIBS =
CPPFLAGS += $(EXTRA_CPPFLAGS)
LDFLAGS += $(EXTRA_LDFLAGS)

build_image := cakturkunet
docker = docker run --rm -it -u $(shell id -u):$(shell id -g) \
    -v $(shell pwd):/build:Z -w /build ${build_image}

ifndef ($(findstring $(MAKEFLAGS),s),s)
ifndef V
	E_CC	= @echo ' CC    '$@;
	E_LD	= @echo ' LD    '$@;
endif
endif

ifndef BUILD_IN_DOCKER
all: $(PROGRAM)
else
all: build_in_docker
endif

${build_image}.created:
	docker build -f Dockerfile.build -t ${build_image} .
	@touch ${build_image}.created

build_in_docker: ${build_image}.created
	@echo "Building in Docker"
	@${docker} make $(PROGRAM) EXTRA_LDFLAGS=-static

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

.PHONY: clean check build_in_docker
clean:
	-$(RM) -r $(PROGRAM) $(OBJECTS) *~ core.* $(dep_dirs)

check:
ifndef BUILD_IN_DOCKER
	@$(MAKE) -C t/ check
else
	@${docker} make check EXTRA_LDFLAGS=-static
endif
