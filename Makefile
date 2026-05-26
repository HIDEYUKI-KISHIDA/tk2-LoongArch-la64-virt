# tk2-la64-virt — LoongArch LA64
#   make              Phase 1-4 bring-up (tk2-la64.elf)
#   make TK2=1        T-Kernel 2.0 link (build/tk2-la64-tk.elf)

LA64_GCC := $(firstword $(shell command -v loongarch64-linux-gnu-gcc-14 2>/dev/null) \
	$(shell command -v loongarch64-linux-gnu-gcc 2>/dev/null))
ifeq ($(LA64_GCC),)
  LA64_GCC := loongarch64-linux-gnu-gcc-14
endif

ARCH    := -march=loongarch64 -mabi=lp64d
BASE_CFLAGS := $(ARCH) -ffreestanding -fno-builtin -fno-pic -fno-stack-protector \
               -Wall -Wextra -O1 -g

TK2 ?= 0

TKERNEL2_ROOT ?= ../../../B-Free-master/archive/tkernel_2-master
ifeq ($(wildcard $(TKERNEL2_ROOT)/include/basic.h),)
  TKERNEL2_ROOT := /mnt/c/Users/h_kis/Desktop/B-Free-master/archive/tkernel_2-master
endif
TK_SRC_DIR := $(TKERNEL2_ROOT)/kernel/tkernel/src
TKERNEL2_STAMP := build/.tkernel2.ok

ifeq ($(TK2),1)
  TARGET   := build/tk2-la64-tk.elf
  LDSCRIPT := kernel/sysdepend/la64/Kernel.ld
  OFFSET_H := kernel/sysdepend/cpu/la64/offset.h

  TKERNEL_C := tkstart.c timer.c wait.c task.c klock.c task_manage.c task_sync.c \
               mutex.c subsystem.c objname.c misc_calls.c version.c
  LIBSYS_C := bitset.c bitclr.c bittest.c bitsearch0.c bitsearch1.c
  TKERNEL_OBJS := $(addprefix build/tk/,$(TKERNEL_C:.c=.o)) \
                  build/lib/libstr_string.o \
                  $(addprefix build/libsys/,$(LIBSYS_C:.c=.o))

  SRCS_S := \
      kernel/sysdepend/la64/reset.S \
      kernel/sysdepend/la64/dmw_init.S \
      kernel/sysdepend/cpu/la64/exception.S \
      kernel/sysdepend/cpu/la64/cpu_support_tk.S \
      kernel/sysdepend/cpu/la64/tlb_refill.S \
      kernel/sysdepend/cpu/la64/machine_error_entry.S

  SRCS_C := \
      kernel/tk/knl_main.c \
      kernel/tk/m5_tk_task.c \
      kernel/sysdepend/cpu/la64/cpu_init.c \
      kernel/sysdepend/cpu/la64/kernel_exception.c \
      kernel/sysdepend/cpu/la64/kernel_syscall.c \
      kernel/sysdepend/cpu/la64/chkplv.c \
      kernel/sysdepend/cpu/la64/cache.c \
      kernel/sysdepend/cpu/la64/cpu_calls_port.c \
      kernel/sysdepend/cpu/la64/cpu_dispatch_hook.c \
      kernel/sysdepend/cpu/la64/m5b_trace.c \
      kernel/sysdepend/cpu/la64/exception_init.c \
      kernel/sysdepend/device/common/uart_virt.c \
      kernel/sysdepend/device/common/timer_la64.c \
      kernel/sysdepend/device/virt_la64/devinit.c \
      kernel/sysdepend/device/virt_la64/irq_handler.c \
      kernel/sysdepend/device/virt_la64/virt_tk_timer.c \
      kernel/sysdepend/device/tef_virt_la64/tkdev_init.c \
      kernel/sysdepend/device/tef_virt_la64/patch.c \
      kernel/sysdepend/device/tef_virt_la64/power.c \
      kernel/tk/port/tk_port_boot.c \
      kernel/tk/port/tk_port_sysconf.c \
      kernel/tk/port/tk_port_mem.c \
      kernel/tk/port/tk_port_tm.c \
      kernel/tk/port/tk_port_scinfo.c \
      kernel/tk/port/tk_port_api.c \
      kernel/tk/port/tk_port_stub.c \
      kernel/tk/port/tk_port_diag.c \
      kernel/tk/port/tk_port_kernel.c \
      kernel/tk/port/offset_check.c

  OBJS := $(patsubst %.S, build/%.o, $(SRCS_S)) \
          $(patsubst %.c, build/%.o, $(SRCS_C)) \
          $(TKERNEL_OBJS)

  INCLUDES := -Iinclude \
              -Ikernel/sysdepend/cpu/la64 \
              -Ikernel/sysdepend/device/virt_la64 \
              -Ikernel/sysdepend/device/tef_virt_la64 \
              -Ikernel/tk/port \
              -I$(TKERNEL2_ROOT)/include \
              -I$(TK_SRC_DIR)

  CFLAGS := $(BASE_CFLAGS) $(INCLUDES) \
            -include kernel/tk/port/config.h \
            -DM5B_TKERNEL -DTK2_TKERNEL -D_TEF_VIRT_LA64_ -DTEF_VIRT_LA64=1

  AFLAGS := $(CFLAGS) -include $(OFFSET_H)
  LDFLAGS := $(ARCH) -nostdlib -T $(LDSCRIPT) -Wl,--gc-sections -static

else
  TARGET := tk2-la64.elf
  LDSCRIPT := linker.ld
  ASRC := sysdepend/start.S sysdepend/dmw_init.S sysdepend/exception_entry.S \
          sysdepend/switch.S sysdepend/tlb_refill.S sysdepend/machine_error.S
  CSRC := src/main.c src/uart.c src/exception.c src/exception_c.c src/timer.c \
          src/kernel/knl_main.c src/kernel/knl_test.c src/kernel/knl_sched.c
  OBJS := $(ASRC:.S=.o) $(CSRC:.c=.o)
  CFLAGS := $(BASE_CFLAGS) -Iinclude
  ASFLAGS := $(ARCH)
  LDFLAGS := $(ARCH) -nostdlib -T $(LDSCRIPT) -Wl,--gc-sections -static
endif

QEMU_ACCEL ?= tcg,thread=single
QEMU_COMMON := -accel $(QEMU_ACCEL) -machine virt -cpu la464 -m 2G -smp 1 \
               -nographic -serial mon:stdio

.PHONY: all clean qemu qemu-win tk2-check print-cc check-deps

ifeq ($(TK2),1)
all: check-deps $(OFFSET_H) $(TARGET)
else
all: check-deps $(TARGET)
endif

print-cc:
	@echo LA64_GCC=$(LA64_GCC)

check-deps:
	@command -v $(LA64_GCC) >/dev/null || { echo "install gcc-14-loongarch64-linux-gnu"; exit 127; }
ifeq ($(TK2),1)
	@test -f "$(TKERNEL2_ROOT)/include/basic.h" || { echo "missing TKERNEL2_ROOT=$(TKERNEL2_ROOT)"; exit 1; }
	@mkdir -p build
	@echo "TKERNEL2_ROOT=$(TKERNEL2_ROOT)" > $(TKERNEL2_STAMP)
endif

tk2-check:
	@test -f "$(TKERNEL2_ROOT)/include/basic.h" || exit 1
	@echo OK TKERNEL2_ROOT=$(TKERNEL2_ROOT)

ifeq ($(TK2),1)
build/offset_vals.s: kernel/tk/port/offset_vals.c | build_dirs
	$(LA64_GCC) $(CFLAGS) -std=gnu11 -S -o $@ $<

$(OFFSET_H): build/offset_vals.s
	@set -e; \
	off_dec() { \
	  v=$$(awk -v sym="off_export_$$1" ' \
	    $$0 ~ ("^[[:space:]]*" sym "[[:space:]]*:") { \
	      while (getline > 0) { \
	        if ($$1 == ".8byte" || $$1 == ".dword" || $$1 == ".quad") { print $$2; exit } \
	        if ($$0 ~ /^[^ \t#\.]/) exit \
	      } \
	    }' build/offset_vals.s); \
	  test -n "$$v" || { echo "missing off_export_$$1"; exit 1; }; echo "$$v"; \
	}; \
	tskid=$$(off_dec tskid); tskatr=$$(off_dec tskatr); state=$$(off_dec state); \
	reqdct=$$(off_dec reqdct); tskctxb=$$(off_dec tskctxb); \
	ctx_uatb=$$(off_dec CTXB_uatb); ctx_lsid=$$(off_dec CTXB_lsid); \
	ctx_svc_ssp=$$(off_dec CTXB_svc_ssp); \
	{ echo "/* AUTO */"; echo "#ifndef _OFFSET_"; echo "#define _OFFSET_"; \
	  echo "#define TCB_tskid $$tskid"; echo "#define TCB_tskatr $$tskatr"; \
	  echo "#define TCB_state $$state"; echo "#define TCB_reqdct $$reqdct"; \
	  echo "#define TCB_tskctxb $$tskctxb"; echo "#define CTXB_ssp 0"; \
	  echo "#define CTXB_uatb $$ctx_uatb"; echo "#define CTXB_lsid $$ctx_lsid"; \
	  echo "#define CTXB_svc_ssp $$ctx_svc_ssp"; echo "#define TA_FPU 0"; echo "#endif"; \
	} > $@.tmp && mv $@.tmp $@

build/kernel/sysdepend/cpu/la64/cpu_support_tk.o: kernel/sysdepend/cpu/la64/cpu_support_tk.S $(OFFSET_H) | build_dirs
	$(LA64_GCC) $(AFLAGS) -c -o $@ $<

build/tk/%.o: $(TK_SRC_DIR)/%.c | build_dirs
	$(LA64_GCC) $(CFLAGS) -std=gnu11 -c -o $@ $<
build/lib/libstr_string.o: $(TKERNEL2_ROOT)/lib/libstr/src/string.c | build_dirs
	$(LA64_GCC) $(CFLAGS) -std=gnu11 -c -o $@ $<
build/libsys/%.o: $(TKERNEL2_ROOT)/lib/libsys/src/%.c | build_dirs
	$(LA64_GCC) $(CFLAGS) -std=gnu11 -c -o $@ $<

build/%.o: %.S | build_dirs
	$(LA64_GCC) $(AFLAGS) -c -o $@ $<
build/%.o: %.c | build_dirs
	$(LA64_GCC) $(CFLAGS) -std=gnu11 -c -o $@ $<
endif

$(TARGET): $(OBJS) | build_dirs
	$(LA64_GCC) $(LDFLAGS) -o $@ $(OBJS)
	@command -v loongarch64-linux-gnu-size >/dev/null && loongarch64-linux-gnu-size $@ || true
	@echo Built $@

ifeq ($(TK2),0)
%.o: %.S
	$(LA64_GCC) $(ASFLAGS) -c -o $@ $<
%.o: %.c
	$(LA64_GCC) $(CFLAGS) -c -o $@ $<
endif

build_dirs:
	@mkdir -p $(sort $(dir $(OBJS))) build/tk build/lib build/libsys

qemu: $(TARGET)
	qemu-system-loongarch64 $(QEMU_COMMON) \
	  -device loader,file=$(TARGET),addr=0x40000000,cpu-num=0

qemu-win: $(TARGET)
	@echo qemu-system-loongarch64.exe $(QEMU_COMMON) -device loader,file=$(TARGET),addr=0x40000000,cpu-num=0

clean:
	rm -rf build tk2-la64.elf
	rm -f kernel/sysdepend/cpu/la64/offset.h build/offset_vals.s $(TKERNEL2_STAMP)
	rm -f sysdepend/*.o src/*.o src/kernel/*.o 2>/dev/null || true
