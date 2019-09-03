#
#		Makefile�Υץ����å���¸����RISCV/32�ѡ�
#

#
#  GNU��ȯ�Ķ��Υ������åȥ������ƥ���������
#
GCC_TARGET = riscv32-hifive1-elf

#
#  �ץ����å���¸���ǥ��쥯�ȥ�̾�����
#
PRCDIR = $(SRCDIR)/arch/$(PRC)_$(TOOL)

#
#  ����ѥ��륪�ץ����
#
INCLUDES := $(INCLUDES) -I$(PRCDIR)
COPTS := $(COPTS) -march=rv32imac -mabi=ilp32 -mcmodel=medany -nostdlib
LDFLAGS := $(LDFLAGS) -nostdlib -nostartfiles

#
#  �����ͥ�˴ؤ������
#
KERNEL_DIR := $(KERNEL_DIR) $(PRCDIR)
KERNEL_ASMOBJS := $(KERNEL_ASMOBJS) prc_support.o
KERNEL_COBJS := $(KERNEL_COBJS) prc_config.o prc_timer.o

#
#  ����ե�����졼���ط����ѿ������
#
CFG_TABS := $(CFG_TABS) --cfg1-def-table $(PRCDIR)/prc_def.csv

#
#  ���ե��åȥե����������Τ�������
#
OFFSET_TF = $(PRCDIR)/prc_offset.tf