/*-
 * Copyright (c) 2011 NetApp, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETAPP, INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NETAPP, INC OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/errno.h>
#include <sys/mman.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <libutil.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#include <machine/vmm.h>
#include <vmmapi.h>

#include "intel/vmcs.h"

#define	MB	(1UL << 20)
#define	GB	(1UL << 30)

#define	REQ_ARG		required_argument
#define	NO_ARG		no_argument
#define	OPT_ARG		optional_argument

static const char *progname;

static void
usage(void)
{

	(void)fprintf(stderr,
	"Usage: %s --vm=<name>\n"
	"       [--cpu=<vcpu_number>]\n"
	"       [--create]\n"
	"       [--destroy]\n"
	"       [--get-stats]\n"
	"       [--set-desc-ds]\n"
	"       [--get-desc-ds]\n"
	"       [--set-desc-es]\n"
	"       [--get-desc-es]\n"
	"       [--set-desc-gs]\n"
	"       [--get-desc-gs]\n"
	"       [--set-desc-fs]\n"
	"       [--get-desc-fs]\n"
	"       [--set-desc-cs]\n"
	"       [--get-desc-cs]\n"
	"       [--set-desc-ss]\n"
	"       [--get-desc-ss]\n"
	"       [--set-desc-tr]\n"
	"       [--get-desc-tr]\n"
	"       [--set-desc-ldtr]\n"
	"       [--get-desc-ldtr]\n"
	"       [--set-desc-gdtr]\n"
	"       [--get-desc-gdtr]\n"
	"       [--set-desc-idtr]\n"
	"       [--get-desc-idtr]\n"
	"       [--run]\n"
	"       [--capname=<capname>]\n"
	"       [--getcap]\n"
	"       [--setcap=<0|1>]\n"
	"       [--desc-base=<BASE>]\n"
	"       [--desc-limit=<LIMIT>]\n"
	"       [--desc-access=<ACCESS>]\n"
	"       [--set-cr0=<CR0>]\n"
	"       [--get-cr0]\n"
	"       [--set-cr3=<CR3>]\n"
	"       [--get-cr3]\n"
	"       [--set-cr4=<CR4>]\n"
	"       [--get-cr4]\n"
	"       [--set-dr7=<DR7>]\n"
	"       [--get-dr7]\n"
	"       [--set-rsp=<RSP>]\n"
	"       [--get-rsp]\n"
	"       [--set-rip=<RIP>]\n"
	"       [--get-rip]\n"
	"       [--get-rax]\n"
	"       [--set-rax=<RAX>]\n"
	"       [--get-rbx]\n"
	"       [--get-rcx]\n"
	"       [--get-rdx]\n"
	"       [--get-rsi]\n"
	"       [--get-rdi]\n"
	"       [--get-rbp]\n"
	"       [--get-r8]\n"
	"       [--get-r9]\n"
	"       [--get-r10]\n"
	"       [--get-r11]\n"
	"       [--get-r12]\n"
	"       [--get-r13]\n"
	"       [--get-r14]\n"
	"       [--get-r15]\n"
	"       [--set-rflags=<RFLAGS>]\n"
	"       [--get-rflags]\n"
	"       [--set-cs]\n"
	"       [--get-cs]\n"
	"       [--set-ds]\n"
	"       [--get-ds]\n"
	"       [--set-es]\n"
	"       [--get-es]\n"
	"       [--set-fs]\n"
	"       [--get-fs]\n"
	"       [--set-gs]\n"
	"       [--get-gs]\n"
	"       [--set-ss]\n"
	"       [--get-ss]\n"
	"       [--get-tr]\n"
	"       [--get-ldtr]\n"
	"       [--get-vmcs-pinbased-ctls]\n"
	"       [--get-vmcs-procbased-ctls]\n"
	"       [--get-vmcs-procbased-ctls2]\n"
	"       [--get-vmcs-entry-interruption-info]\n"
	"       [--set-vmcs-entry-interruption-info=<info>]\n"
	"       [--get-vmcs-eptp]\n"
	"       [--get-vmcs-guest-physical-address\n"
	"       [--get-vmcs-guest-linear-address\n"
	"       [--set-vmcs-exception-bitmap]\n"
	"       [--get-vmcs-exception-bitmap]\n"
	"       [--get-vmcs-io-bitmap-address]\n"
	"       [--get-vmcs-tsc-offset]\n"
	"       [--get-vmcs-guest-pat]\n"
	"       [--get-vmcs-host-pat]\n"
	"       [--get-vmcs-host-cr0]\n"
	"       [--get-vmcs-host-cr3]\n"
	"       [--get-vmcs-host-cr4]\n"
	"       [--get-vmcs-host-rip]\n"
	"       [--get-vmcs-host-rsp]\n"
	"       [--get-vmcs-cr0-mask]\n"
	"       [--get-vmcs-cr0-shadow]\n"
	"       [--get-vmcs-cr4-mask]\n"
	"       [--get-vmcs-cr4-shadow]\n"
	"       [--get-vmcs-cr3-targets]\n"
	"       [--get-vmcs-apic-access-address]\n"
	"       [--get-vmcs-virtual-apic-address]\n"
	"       [--get-vmcs-tpr-threshold]\n"
	"       [--get-vmcs-msr-bitmap]\n"
	"       [--get-vmcs-msr-bitmap-address]\n"
	"       [--get-vmcs-vpid]\n"
	"       [--get-vmcs-ple-gap]\n"
	"       [--get-vmcs-ple-window]\n"
	"       [--get-vmcs-instruction-error]\n"
	"       [--get-vmcs-exit-ctls]\n"
	"       [--get-vmcs-entry-ctls]\n"
	"       [--get-vmcs-guest-sysenter]\n"
	"       [--get-vmcs-link]\n"
	"       [--get-vmcs-exit-reason]\n"
	"       [--get-vmcs-exit-qualification]\n"
	"       [--get-vmcs-exit-interruption-info]\n"
	"       [--get-vmcs-exit-interruption-error]\n"
	"       [--get-vmcs-interruptibility]\n"
	"       [--set-pinning=<host_cpuid>]\n"
	"       [--get-pinning]\n"
	"       [--set-lowmem=<memory below 4GB in units of MB>]\n"
	"       [--get-lowmem]\n"
	"       [--set-highmem=<memory above 4GB in units of MB>]\n"
	"       [--get-highmem]\n",
	progname);
	exit(1);
}

static int get_stats, getcap, setcap, capval;
static const char *capname;
static int create, destroy, get_lowmem, get_highmem;
static uint64_t lowmem, highmem;
static int set_cr0, get_cr0, set_cr3, get_cr3, set_cr4, get_cr4;
static int set_efer, get_efer;
static int set_dr7, get_dr7;
static int set_rsp, get_rsp, set_rip, get_rip, set_rflags, get_rflags;
static int set_rax, get_rax;
static int get_rbx, get_rcx, get_rdx, get_rsi, get_rdi, get_rbp;
static int get_r8, get_r9, get_r10, get_r11, get_r12, get_r13, get_r14, get_r15;
static int set_desc_ds, get_desc_ds;
static int set_desc_es, get_desc_es;
static int set_desc_fs, get_desc_fs;
static int set_desc_gs, get_desc_gs;
static int set_desc_cs, get_desc_cs;
static int set_desc_ss, get_desc_ss;
static int set_desc_gdtr, get_desc_gdtr;
static int set_desc_idtr, get_desc_idtr;
static int set_desc_tr, get_desc_tr;
static int set_desc_ldtr, get_desc_ldtr;
static int set_cs, set_ds, set_es, set_fs, set_gs, set_ss, set_tr, set_ldtr;
static int get_cs, get_ds, get_es, get_fs, get_gs, get_ss, get_tr, get_ldtr;
static int set_pinning, get_pinning, pincpu;
static int run;

/*
 * VMCS-specific fields
 */
static int get_pinbased_ctls, get_procbased_ctls, get_procbased_ctls2;
static int get_eptp, get_io_bitmap, get_tsc_offset;
static int get_vmcs_entry_interruption_info, set_vmcs_entry_interruption_info;
static int get_vmcs_interruptibility;
uint32_t vmcs_entry_interruption_info;
static int get_vmcs_gpa, get_vmcs_gla;
static int get_exception_bitmap, set_exception_bitmap, exception_bitmap;
static int get_cr0_mask, get_cr0_shadow;
static int get_cr4_mask, get_cr4_shadow;
static int get_cr3_targets;
static int get_apic_access_addr, get_virtual_apic_addr, get_tpr_threshold;
static int get_msr_bitmap, get_msr_bitmap_address;
static int get_vpid, get_ple_gap, get_ple_window;
static int get_inst_err, get_exit_ctls, get_entry_ctls;
static int get_host_cr0, get_host_cr3, get_host_cr4;
static int get_host_rip, get_host_rsp;
static int get_guest_pat, get_host_pat;
static int get_guest_sysenter, get_vmcs_link;
static int get_vmcs_exit_reason, get_vmcs_exit_qualification;
static int get_vmcs_exit_interruption_info, get_vmcs_exit_interruption_error;

static uint64_t desc_base;
static uint32_t desc_limit, desc_access;

static void
dump_vm_run_exitcode(struct vm_exit *vmexit, int vcpu)
{
	printf("vm exit[%d]\n", vcpu);
	printf("\trip\t\t0x%016lx\n", vmexit->rip);
	printf("\tinst_length\t%d\n", vmexit->inst_length);
	switch (vmexit->exitcode) {
	case VM_EXITCODE_INOUT:
		printf("\treason\t\tINOUT\n");
		printf("\tdirection\t%s\n", vmexit->u.inout.in ? "IN" : "OUT");
		printf("\tbytes\t\t%d\n", vmexit->u.inout.bytes);
		printf("\tflags\t\t%s%s\n",
			vmexit->u.inout.string ? "STRING " : "",
			vmexit->u.inout.rep ? "REP " : "");
		printf("\tport\t\t0x%04x\n", vmexit->u.inout.port);
		printf("\teax\t\t0x%08x\n", vmexit->u.inout.eax);
		break;
	case VM_EXITCODE_VMX:
		printf("\treason\t\tVMX\n");
		printf("\terror\t\t%d\n", vmexit->u.vmx.error);
		printf("\texit_reason\t0x%08x (%u)\n",
		    vmexit->u.vmx.exit_reason, vmexit->u.vmx.exit_reason);
		printf("\tqualification\t0x%016lx\n",
			vmexit->u.vmx.exit_qualification);
		break;
	default:
		printf("*** unknown vm run exitcode %d\n", vmexit->exitcode);
		break;
	}
}

static int
dump_vmcs_msr_bitmap(int vcpu, u_long addr)
{
	int error, fd, byte, bit, readable, writeable;
	u_int msr;
	const char *bitmap;

	error = -1;
	bitmap = MAP_FAILED;

	fd = open("/dev/mem", O_RDONLY, 0);
	if (fd < 0)
		goto done;

	bitmap = mmap(NULL, PAGE_SIZE, PROT_READ, 0, fd, addr);
	if (bitmap == MAP_FAILED)
		goto done;

	for (msr = 0; msr < 0x2000; msr++) {
		byte = msr / 8;
		bit = msr & 0x7;

		/* Look at MSRs in the range 0x00000000 to 0x00001FFF */
		readable = (bitmap[byte] & (1 << bit)) ? 0 : 1;
		writeable = (bitmap[2048 + byte] & (1 << bit)) ? 0 : 1;
		if (readable || writeable) {
			printf("msr 0x%08x[%d]\t\t%c%c\n", msr, vcpu,
				readable ? 'R' : '-',
				writeable ? 'W' : '-');
		}

		/* Look at MSRs in the range 0xC0000000 to 0xC0001FFF */
		byte += 1024;
		readable = (bitmap[byte] & (1 << bit)) ? 0 : 1;
		writeable = (bitmap[2048 + byte] & (1 << bit)) ? 0 : 1;
		if (readable || writeable) {
			printf("msr 0x%08x[%d]\t\t%c%c\n",
				0xc0000000 + msr, vcpu,
				readable ? 'R' : '-',
				writeable ? 'W' : '-');
		}
	}

	error = 0;
done:
	if (bitmap != MAP_FAILED)
		munmap((void *)bitmap, PAGE_SIZE);
	if (fd >= 0)
		close(fd);
	return (error);
}

static int
vm_get_vmcs_field(struct vmctx *ctx, int vcpu, int field, uint64_t *ret_val)
{

	return (vm_get_register(ctx, vcpu, VMCS_IDENT(field), ret_val));
}

static int
vm_set_vmcs_field(struct vmctx *ctx, int vcpu, int field, uint64_t val)
{

	return (vm_set_register(ctx, vcpu, VMCS_IDENT(field), val));
}

enum {
	VMNAME = 1000,	/* avoid collision with return values from getopt */
	VCPU,
	SET_LOWMEM,
	SET_HIGHMEM,
	SET_EFER,
	SET_CR0,
	SET_CR3,
	SET_CR4,
	SET_DR7,
	SET_RSP,
	SET_RIP,
	SET_RAX,
	SET_RFLAGS,
	DESC_BASE,
	DESC_LIMIT,
	DESC_ACCESS,
	SET_CS,
	SET_DS,
	SET_ES,
	SET_FS,
	SET_GS,
	SET_SS,
	SET_TR,
	SET_LDTR,
	SET_PINNING,
	SET_VMCS_EXCEPTION_BITMAP,
	SET_VMCS_ENTRY_INTERRUPTION_INFO,
	SET_CAP,
	CAPNAME,
};

int
main(int argc, char *argv[])
{
	char *vmname;
	int error, ch, vcpu;
	vm_paddr_t hpa;
	size_t len;
	struct vm_exit vmexit;
	uint64_t ctl, eptp, bm, addr, u64;
	struct vmctx *ctx;

	uint64_t cr0, cr3, cr4, dr7, rsp, rip, rflags, efer, pat;
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t cs, ds, es, fs, gs, ss, tr, ldtr;

	struct option opts[] = {
		{ "vm",		REQ_ARG,	0,	VMNAME },
		{ "cpu",	REQ_ARG,	0,	VCPU },
		{ "set-lowmem",	REQ_ARG,	0,	SET_LOWMEM },
		{ "set-highmem",REQ_ARG,	0,	SET_HIGHMEM },
		{ "set-efer",	REQ_ARG,	0,	SET_EFER },
		{ "set-cr0",	REQ_ARG,	0,	SET_CR0 },
		{ "set-cr3",	REQ_ARG,	0,	SET_CR3 },
		{ "set-cr4",	REQ_ARG,	0,	SET_CR4 },
		{ "set-dr7",	REQ_ARG,	0,	SET_DR7 },
		{ "set-rsp",	REQ_ARG,	0,	SET_RSP },
		{ "set-rip",	REQ_ARG,	0,	SET_RIP },
		{ "set-rax",	REQ_ARG,	0,	SET_RAX },
		{ "set-rflags",	REQ_ARG,	0,	SET_RFLAGS },
		{ "desc-base",	REQ_ARG,	0,	DESC_BASE },
		{ "desc-limit",	REQ_ARG,	0,	DESC_LIMIT },
		{ "desc-access",REQ_ARG,	0,	DESC_ACCESS },
		{ "set-cs",	REQ_ARG,	0,	SET_CS },
		{ "set-ds",	REQ_ARG,	0,	SET_DS },
		{ "set-es",	REQ_ARG,	0,	SET_ES },
		{ "set-fs",	REQ_ARG,	0,	SET_FS },
		{ "set-gs",	REQ_ARG,	0,	SET_GS },
		{ "set-ss",	REQ_ARG,	0,	SET_SS },
		{ "set-tr",	REQ_ARG,	0,	SET_TR },
		{ "set-ldtr",	REQ_ARG,	0,	SET_LDTR },
		{ "set-pinning",REQ_ARG,	0,	SET_PINNING },
		{ "set-vmcs-exception-bitmap",
				REQ_ARG,	0, SET_VMCS_EXCEPTION_BITMAP },
		{ "set-vmcs-entry-interruption-info",
				REQ_ARG, 0, SET_VMCS_ENTRY_INTERRUPTION_INFO },
		{ "capname",	REQ_ARG,	0,	CAPNAME },
		{ "setcap",	REQ_ARG,	0,	SET_CAP },
		{ "getcap",	NO_ARG,		&getcap,	1 },
		{ "get-stats",	NO_ARG,		&get_stats,	1 },
		{ "get-desc-ds",NO_ARG,		&get_desc_ds,	1 },
		{ "set-desc-ds",NO_ARG,		&set_desc_ds,	1 },
		{ "get-desc-es",NO_ARG,		&get_desc_es,	1 },
		{ "set-desc-es",NO_ARG,		&set_desc_es,	1 },
		{ "get-desc-ss",NO_ARG,		&get_desc_ss,	1 },
		{ "set-desc-ss",NO_ARG,		&set_desc_ss,	1 },
		{ "get-desc-cs",NO_ARG,		&get_desc_cs,	1 },
		{ "set-desc-cs",NO_ARG,		&set_desc_cs,	1 },
		{ "get-desc-fs",NO_ARG,		&get_desc_fs,	1 },
		{ "set-desc-fs",NO_ARG,		&set_desc_fs,	1 },
		{ "get-desc-gs",NO_ARG,		&get_desc_gs,	1 },
		{ "set-desc-gs",NO_ARG,		&set_desc_gs,	1 },
		{ "get-desc-tr",NO_ARG,		&get_desc_tr,	1 },
		{ "set-desc-tr",NO_ARG,		&set_desc_tr,	1 },
		{ "set-desc-ldtr", NO_ARG,	&set_desc_ldtr,	1 },
		{ "get-desc-ldtr", NO_ARG,	&get_desc_ldtr,	1 },
		{ "set-desc-gdtr", NO_ARG,	&set_desc_gdtr, 1 },
		{ "get-desc-gdtr", NO_ARG,	&get_desc_gdtr, 1 },
		{ "set-desc-idtr", NO_ARG,	&set_desc_idtr, 1 },
		{ "get-desc-idtr", NO_ARG,	&get_desc_idtr, 1 },
		{ "get-lowmem", NO_ARG,		&get_lowmem,	1 },
		{ "get-highmem",NO_ARG,		&get_highmem,	1 },
		{ "get-efer",	NO_ARG,		&get_efer,	1 },
		{ "get-cr0",	NO_ARG,		&get_cr0,	1 },
		{ "get-cr3",	NO_ARG,		&get_cr3,	1 },
		{ "get-cr4",	NO_ARG,		&get_cr4,	1 },
		{ "get-dr7",	NO_ARG,		&get_dr7,	1 },
		{ "get-rsp",	NO_ARG,		&get_rsp,	1 },
		{ "get-rip",	NO_ARG,		&get_rip,	1 },
		{ "get-rax",	NO_ARG,		&get_rax,	1 },
		{ "get-rbx",	NO_ARG,		&get_rbx,	1 },
		{ "get-rcx",	NO_ARG,		&get_rcx,	1 },
		{ "get-rdx",	NO_ARG,		&get_rdx,	1 },
		{ "get-rsi",	NO_ARG,		&get_rsi,	1 },
		{ "get-rdi",	NO_ARG,		&get_rdi,	1 },
		{ "get-rbp",	NO_ARG,		&get_rbp,	1 },
		{ "get-r8",	NO_ARG,		&get_r8,	1 },
		{ "get-r9",	NO_ARG,		&get_r9,	1 },
		{ "get-r10",	NO_ARG,		&get_r10,	1 },
		{ "get-r11",	NO_ARG,		&get_r11,	1 },
		{ "get-r12",	NO_ARG,		&get_r12,	1 },
		{ "get-r13",	NO_ARG,		&get_r13,	1 },
		{ "get-r14",	NO_ARG,		&get_r14,	1 },
		{ "get-r15",	NO_ARG,		&get_r15,	1 },
		{ "get-rflags",	NO_ARG,		&get_rflags,	1 },
		{ "get-cs",	NO_ARG,		&get_cs,	1 },
		{ "get-ds",	NO_ARG,		&get_ds,	1 },
		{ "get-es",	NO_ARG,		&get_es,	1 },
		{ "get-fs",	NO_ARG,		&get_fs,	1 },
		{ "get-gs",	NO_ARG,		&get_gs,	1 },
		{ "get-ss",	NO_ARG,		&get_ss,	1 },
		{ "get-tr",	NO_ARG,		&get_tr,	1 },
		{ "get-ldtr",	NO_ARG,		&get_ldtr,	1 },
		{ "get-vmcs-pinbased-ctls",
				NO_ARG,		&get_pinbased_ctls, 1 },
		{ "get-vmcs-procbased-ctls",
				NO_ARG,		&get_procbased_ctls, 1 },
		{ "get-vmcs-procbased-ctls2",
				NO_ARG,		&get_procbased_ctls2, 1 },
		{ "get-vmcs-guest-linear-address",
				NO_ARG,		&get_vmcs_gla,	1 },
		{ "get-vmcs-guest-physical-address",
				NO_ARG,		&get_vmcs_gpa,	1 },
		{ "get-vmcs-entry-interruption-info",
				NO_ARG, &get_vmcs_entry_interruption_info, 1},
		{ "get-vmcs-eptp", NO_ARG,	&get_eptp,	1 },
		{ "get-vmcs-exception-bitmap",
				NO_ARG,		&get_exception_bitmap, 1 },
		{ "get-vmcs-io-bitmap-address",
				NO_ARG,		&get_io_bitmap,	1 },
		{ "get-vmcs-tsc-offset", NO_ARG,&get_tsc_offset, 1 },
		{ "get-vmcs-cr0-mask", NO_ARG,	&get_cr0_mask,	1 },
		{ "get-vmcs-cr0-shadow", NO_ARG,&get_cr0_shadow, 1 },
		{ "get-vmcs-cr4-mask", NO_ARG,	&get_cr4_mask,	1 },
		{ "get-vmcs-cr4-shadow", NO_ARG,&get_cr4_shadow, 1 },
		{ "get-vmcs-cr3-targets", NO_ARG, &get_cr3_targets, 1},
		{ "get-vmcs-apic-access-address",
				NO_ARG,		&get_apic_access_addr, 1},
		{ "get-vmcs-virtual-apic-address",
				NO_ARG,		&get_virtual_apic_addr, 1},
		{ "get-vmcs-tpr-threshold",
				NO_ARG,		&get_tpr_threshold, 1 },
		{ "get-vmcs-msr-bitmap",
				NO_ARG,		&get_msr_bitmap, 1 },
		{ "get-vmcs-msr-bitmap-address",
				NO_ARG,		&get_msr_bitmap_address, 1 },
		{ "get-vmcs-vpid", NO_ARG,	&get_vpid,	1 },
		{ "get-vmcs-ple-gap", NO_ARG,	&get_ple_gap,	1 },
		{ "get-vmcs-ple-window", NO_ARG,&get_ple_window,1 },
		{ "get-vmcs-instruction-error",
				NO_ARG,		&get_inst_err,	1 },
		{ "get-vmcs-exit-ctls", NO_ARG,	&get_exit_ctls,	1 },
		{ "get-vmcs-entry-ctls",
					NO_ARG,	&get_entry_ctls, 1 },
		{ "get-vmcs-guest-pat",	NO_ARG,	&get_guest_pat,	1 },
		{ "get-vmcs-host-pat",	NO_ARG,	&get_host_pat,	1 },
		{ "get-vmcs-host-cr0",
				NO_ARG,		&get_host_cr0,	1 },
		{ "get-vmcs-host-cr3",
				NO_ARG,		&get_host_cr3,	1 },
		{ "get-vmcs-host-cr4",
				NO_ARG,		&get_host_cr4,	1 },
		{ "get-vmcs-host-rip",
				NO_ARG,		&get_host_rip,	1 },
		{ "get-vmcs-host-rsp",
				NO_ARG,		&get_host_rsp,	1 },
		{ "get-vmcs-guest-sysenter",
				NO_ARG,		&get_guest_sysenter, 1 },
		{ "get-vmcs-link", NO_ARG,	&get_vmcs_link, 1 },
		{ "get-vmcs-exit-reason",
				NO_ARG,		&get_vmcs_exit_reason, 1 },
		{ "get-vmcs-exit-qualification",
			NO_ARG,		&get_vmcs_exit_qualification, 1 },
		{ "get-vmcs-exit-interruption-info",
				NO_ARG,	&get_vmcs_exit_interruption_info, 1},
		{ "get-vmcs-exit-interruption-error",
				NO_ARG,	&get_vmcs_exit_interruption_error, 1},
		{ "get-vmcs-interruptibility",
				NO_ARG, &get_vmcs_interruptibility, 1 },
		{ "get-pinning",NO_ARG,		&get_pinning,	1 },
		{ "run",	NO_ARG,		&run,		1 },
		{ "create",	NO_ARG,		&create,	1 },
		{ "destroy",	NO_ARG,		&destroy,	1 },
		{ NULL,		0,		NULL,		0 }
	};

	vcpu = 0;
	progname = basename(argv[0]);

	while ((ch = getopt_long(argc, argv, "", opts, NULL)) != -1) {
		switch (ch) {
		case 0:
			break;
		case VMNAME:
			vmname = optarg;
			break;
		case VCPU:
			vcpu = atoi(optarg);
			break;
		case SET_LOWMEM:
			lowmem = atoi(optarg) * MB;
			lowmem = roundup(lowmem, 2 * MB);
			break;
		case SET_HIGHMEM:
			highmem = atoi(optarg) * MB;
			highmem = roundup(highmem, 2 * MB);
			break;
		case SET_EFER:
			efer = strtoul(optarg, NULL, 0);
			set_efer = 1;
			break;
		case SET_CR0:
			cr0 = strtoul(optarg, NULL, 0);
			set_cr0 = 1;
			break;
		case SET_CR3:
			cr3 = strtoul(optarg, NULL, 0);
			set_cr3 = 1;
			break;
		case SET_CR4:
			cr4 = strtoul(optarg, NULL, 0);
			set_cr4 = 1;
			break;
		case SET_DR7:
			dr7 = strtoul(optarg, NULL, 0);
			set_dr7 = 1;
			break;
		case SET_RSP:
			rsp = strtoul(optarg, NULL, 0);
			set_rsp = 1;
			break;
		case SET_RIP:
			rip = strtoul(optarg, NULL, 0);
			set_rip = 1;
			break;
		case SET_RAX:
			rax = strtoul(optarg, NULL, 0);
			set_rax = 1;
			break;
		case SET_RFLAGS:
			rflags = strtoul(optarg, NULL, 0);
			set_rflags = 1;
			break;
		case DESC_BASE:
			desc_base = strtoul(optarg, NULL, 0);
			break;
		case DESC_LIMIT:
			desc_limit = strtoul(optarg, NULL, 0);
			break;
		case DESC_ACCESS:
			desc_access = strtoul(optarg, NULL, 0);
			break;
		case SET_CS:
			cs = strtoul(optarg, NULL, 0);
			set_cs = 1;
			break;
		case SET_DS:
			ds = strtoul(optarg, NULL, 0);
			set_ds = 1;
			break;
		case SET_ES:
			es = strtoul(optarg, NULL, 0);
			set_es = 1;
			break;
		case SET_FS:
			fs = strtoul(optarg, NULL, 0);
			set_fs = 1;
			break;
		case SET_GS:
			gs = strtoul(optarg, NULL, 0);
			set_gs = 1;
			break;
		case SET_SS:
			ss = strtoul(optarg, NULL, 0);
			set_ss = 1;
			break;
		case SET_TR:
			tr = strtoul(optarg, NULL, 0);
			set_tr = 1;
			break;
		case SET_LDTR:
			ldtr = strtoul(optarg, NULL, 0);
			set_ldtr = 1;
			break;
		case SET_PINNING:
			pincpu = strtol(optarg, NULL, 0);
			set_pinning = 1;
			break;
		case SET_VMCS_EXCEPTION_BITMAP:
			exception_bitmap = strtoul(optarg, NULL, 0);
			set_exception_bitmap = 1;
			break;
		case SET_VMCS_ENTRY_INTERRUPTION_INFO:
			vmcs_entry_interruption_info = strtoul(optarg, NULL, 0);
			set_vmcs_entry_interruption_info = 1;
			break;
		case SET_CAP:
			capval = strtoul(optarg, NULL, 0);
			setcap = 1;
			break;
		case CAPNAME:
			capname = optarg;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (vmname == NULL)
		usage();

	error = 0;

	if (!error && create)
		error = vm_create(vmname);

	if (!error) {
		ctx = vm_open(vmname);
		if (ctx == NULL)
			error = -1;
	}

	if (!error && lowmem)
		error = vm_setup_memory(ctx, 0, lowmem, NULL);

	if (!error && highmem)
		error = vm_setup_memory(ctx, 4 * GB, highmem, NULL);

	if (!error && set_efer)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_EFER, efer);

	if (!error && set_cr0)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_CR0, cr0);

	if (!error && set_cr3)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_CR3, cr3);

	if (!error && set_cr4)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_CR4, cr4);

	if (!error && set_dr7)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_DR7, dr7);

	if (!error && set_rsp)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_RSP, rsp);

	if (!error && set_rip)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_RIP, rip);

	if (!error && set_rax)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_RAX, rax);

	if (!error && set_rflags) {
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_RFLAGS,
					rflags);
	}

	if (!error && set_desc_ds) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_DS,
				    desc_base, desc_limit, desc_access);
	}

	if (!error && set_desc_es) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_ES,
				    desc_base, desc_limit, desc_access);
	}

	if (!error && set_desc_ss) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_SS,
				    desc_base, desc_limit, desc_access);
	}

	if (!error && set_desc_cs) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_CS,
				    desc_base, desc_limit, desc_access);
	}

	if (!error && set_desc_fs) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_FS,
				    desc_base, desc_limit, desc_access);
	}

	if (!error && set_desc_gs) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_GS,
				    desc_base, desc_limit, desc_access);
	}

	if (!error && set_desc_tr) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_TR,
				    desc_base, desc_limit, desc_access);
	}

	if (!error && set_desc_ldtr) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_LDTR,
				    desc_base, desc_limit, desc_access);
	}

	if (!error && set_desc_gdtr) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_GDTR,
				    desc_base, desc_limit, 0);
	}

	if (!error && set_desc_idtr) {
		error = vm_set_desc(ctx, vcpu, VM_REG_GUEST_IDTR,
				    desc_base, desc_limit, 0);
	}

	if (!error && set_cs)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_CS, cs);

	if (!error && set_ds)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_DS, ds);

	if (!error && set_es)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_ES, es);

	if (!error && set_fs)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_FS, fs);

	if (!error && set_gs)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_GS, gs);

	if (!error && set_ss)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_SS, ss);

	if (!error && set_tr)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_TR, tr);

	if (!error && set_ldtr)
		error = vm_set_register(ctx, vcpu, VM_REG_GUEST_LDTR, ldtr);

	if (!error && set_pinning)
		error = vm_set_pinning(ctx, vcpu, pincpu);

	if (!error && set_exception_bitmap) {
		error = vm_set_vmcs_field(ctx, vcpu, VMCS_EXCEPTION_BITMAP,
					  exception_bitmap);
	}

	if (!error && set_vmcs_entry_interruption_info) {
		error = vm_set_vmcs_field(ctx, vcpu, VMCS_ENTRY_INTR_INFO,
					  vmcs_entry_interruption_info);
	}

	if (!error && get_lowmem) {
		error = vm_get_memory_seg(ctx, 0, &hpa, &len);
		if (error == 0)
			printf("lowmem\t\t0x%016lx/%ld\n", hpa, len);
	}

	if (!error && get_highmem) {
		error = vm_get_memory_seg(ctx, 4 * GB, &hpa, &len);
		if (error == 0)
			printf("highmem\t\t0x%016lx/%ld\n", hpa, len);
	}

	if (!error && get_efer) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_EFER, &efer);
		if (error == 0)
			printf("efer[%d]\t\t0x%016lx\n", vcpu, efer);
	}

	if (!error && get_cr0) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_CR0, &cr0);
		if (error == 0)
			printf("cr0[%d]\t\t0x%016lx\n", vcpu, cr0);
	}

	if (!error && get_cr3) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_CR3, &cr3);
		if (error == 0)
			printf("cr3[%d]\t\t0x%016lx\n", vcpu, cr3);
	}

	if (!error && get_cr4) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_CR4, &cr4);
		if (error == 0)
			printf("cr4[%d]\t\t0x%016lx\n", vcpu, cr4);
	}

	if (!error && get_dr7) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_DR7, &dr7);
		if (error == 0)
			printf("dr7[%d]\t\t0x%016lx\n", vcpu, dr7);
	}

	if (!error && get_rsp) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RSP, &rsp);
		if (error == 0)
			printf("rsp[%d]\t\t0x%016lx\n", vcpu, rsp);
	}

	if (!error && get_rip) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RIP, &rip);
		if (error == 0)
			printf("rip[%d]\t\t0x%016lx\n", vcpu, rip);
	}

	if (!error && get_rax) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RAX, &rax);
		if (error == 0)
			printf("rax[%d]\t\t0x%016lx\n", vcpu, rax);
	}

	if (!error && get_rbx) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RBX, &rbx);
		if (error == 0)
			printf("rbx[%d]\t\t0x%016lx\n", vcpu, rbx);
	}

	if (!error && get_rcx) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RCX, &rcx);
		if (error == 0)
			printf("rcx[%d]\t\t0x%016lx\n", vcpu, rcx);
	}

	if (!error && get_rdx) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RDX, &rdx);
		if (error == 0)
			printf("rdx[%d]\t\t0x%016lx\n", vcpu, rdx);
	}

	if (!error && get_rsi) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RSI, &rsi);
		if (error == 0)
			printf("rsi[%d]\t\t0x%016lx\n", vcpu, rsi);
	}

	if (!error && get_rdi) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RDI, &rdi);
		if (error == 0)
			printf("rdi[%d]\t\t0x%016lx\n", vcpu, rdi);
	}

	if (!error && get_rbp) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RBP, &rbp);
		if (error == 0)
			printf("rbp[%d]\t\t0x%016lx\n", vcpu, rbp);
	}

	if (!error && get_r8) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_R8, &r8);
		if (error == 0)
			printf("r8[%d]\t\t0x%016lx\n", vcpu, r8);
	}

	if (!error && get_r9) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_R9, &r9);
		if (error == 0)
			printf("r9[%d]\t\t0x%016lx\n", vcpu, r9);
	}

	if (!error && get_r10) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_R10, &r10);
		if (error == 0)
			printf("r10[%d]\t\t0x%016lx\n", vcpu, r10);
	}

	if (!error && get_r11) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_R11, &r11);
		if (error == 0)
			printf("r11[%d]\t\t0x%016lx\n", vcpu, r11);
	}

	if (!error && get_r12) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_R12, &r12);
		if (error == 0)
			printf("r12[%d]\t\t0x%016lx\n", vcpu, r12);
	}

	if (!error && get_r13) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_R13, &r13);
		if (error == 0)
			printf("r13[%d]\t\t0x%016lx\n", vcpu, r13);
	}

	if (!error && get_r14) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_R14, &r14);
		if (error == 0)
			printf("r14[%d]\t\t0x%016lx\n", vcpu, r14);
	}

	if (!error && get_r15) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_R15, &r15);
		if (error == 0)
			printf("r15[%d]\t\t0x%016lx\n", vcpu, r15);
	}

	if (!error && get_rflags) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RFLAGS,
					&rflags);
		if (error == 0)
			printf("rflags[%d]\t0x%016lx\n", vcpu, rflags);
	}

	if (!error && get_stats) {
		int i, num_stats;
		uint64_t *stats;
		struct timeval tv;
		const char *desc;

		stats = vm_get_stats(ctx, vcpu, &tv, &num_stats);
		if (stats != NULL) {
			printf("vcpu%d\n", vcpu);
			for (i = 0; i < num_stats; i++) {
				desc = vm_get_stat_desc(ctx, i);
				printf("%-32s\t%ld\n", desc, stats[i]);
			}
		}
	}

	if (!error && get_desc_ds) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_DS,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("ds desc[%d]\t0x%016lx/0x%08x/0x%08x\n",
			       vcpu, desc_base, desc_limit, desc_access);	
		}
	}

	if (!error && get_desc_es) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_ES,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("es desc[%d]\t0x%016lx/0x%08x/0x%08x\n",
			       vcpu, desc_base, desc_limit, desc_access);	
		}
	}

	if (!error && get_desc_fs) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_FS,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("fs desc[%d]\t0x%016lx/0x%08x/0x%08x\n",
			       vcpu, desc_base, desc_limit, desc_access);	
		}
	}

	if (!error && get_desc_gs) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_GS,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("gs desc[%d]\t0x%016lx/0x%08x/0x%08x\n",
			       vcpu, desc_base, desc_limit, desc_access);	
		}
	}

	if (!error && get_desc_ss) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_SS,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("ss desc[%d]\t0x%016lx/0x%08x/0x%08x\n",
			       vcpu, desc_base, desc_limit, desc_access);	
		}
	}

	if (!error && get_desc_cs) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_CS,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("cs desc[%d]\t0x%016lx/0x%08x/0x%08x\n",
			       vcpu, desc_base, desc_limit, desc_access);	
		}
	}

	if (!error && get_desc_tr) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_TR,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("tr desc[%d]\t0x%016lx/0x%08x/0x%08x\n",
			       vcpu, desc_base, desc_limit, desc_access);	
		}
	}

	if (!error && get_desc_ldtr) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_LDTR,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("ldtr desc[%d]\t0x%016lx/0x%08x/0x%08x\n",
			       vcpu, desc_base, desc_limit, desc_access);	
		}
	}

	if (!error && get_desc_gdtr) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_GDTR,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("gdtr[%d]\t\t0x%016lx/0x%08x\n",
			       vcpu, desc_base, desc_limit);	
		}
	}

	if (!error && get_desc_idtr) {
		error = vm_get_desc(ctx, vcpu, VM_REG_GUEST_IDTR,
				    &desc_base, &desc_limit, &desc_access);
		if (error == 0) {
			printf("idtr[%d]\t\t0x%016lx/0x%08x\n",
			       vcpu, desc_base, desc_limit);	
		}
	}

	if (!error && get_cs) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_CS, &cs);
		if (error == 0)
			printf("cs[%d]\t\t0x%04lx\n", vcpu, cs);
	}

	if (!error && get_ds) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_DS, &ds);
		if (error == 0)
			printf("ds[%d]\t\t0x%04lx\n", vcpu, ds);
	}

	if (!error && get_es) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_ES, &es);
		if (error == 0)
			printf("es[%d]\t\t0x%04lx\n", vcpu, es);
	}

	if (!error && get_fs) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_FS, &fs);
		if (error == 0)
			printf("fs[%d]\t\t0x%04lx\n", vcpu, fs);
	}

	if (!error && get_gs) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_GS, &gs);
		if (error == 0)
			printf("gs[%d]\t\t0x%04lx\n", vcpu, gs);
	}

	if (!error && get_ss) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_SS, &ss);
		if (error == 0)
			printf("ss[%d]\t\t0x%04lx\n", vcpu, ss);
	}

	if (!error && get_tr) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_TR, &tr);
		if (error == 0)
			printf("tr[%d]\t\t0x%04lx\n", vcpu, tr);
	}

	if (!error && get_ldtr) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_LDTR, &ldtr);
		if (error == 0)
			printf("ldtr[%d]\t\t0x%04lx\n", vcpu, ldtr);
	}

	if (!error && get_pinning) {
		error = vm_get_pinning(ctx, vcpu, &pincpu);
		if (error == 0) {
			if (pincpu < 0)
				printf("pincpu[%d]\tunpinned\n", vcpu);
			else
				printf("pincpu[%d]\t%d\n", vcpu, pincpu);
		}
	}

	if (!error && get_pinbased_ctls) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_PIN_BASED_CTLS, &ctl);
		if (error == 0)
			printf("pinbased_ctls[%d]\t0x%08lx\n", vcpu, ctl);
	}

	if (!error && get_procbased_ctls) {
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_PRI_PROC_BASED_CTLS, &ctl);
		if (error == 0)
			printf("procbased_ctls[%d]\t0x%08lx\n", vcpu, ctl);
	}

	if (!error && get_procbased_ctls2) {
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_SEC_PROC_BASED_CTLS, &ctl);
		if (error == 0)
			printf("procbased_ctls2[%d]\t0x%08lx\n", vcpu, ctl);
	}

	if (!error && get_vmcs_gla) {
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_GUEST_LINEAR_ADDRESS, &u64);
		if (error == 0)
			printf("gla[%d]\t\t0x%016lx\n", vcpu, u64);
	}

	if (!error && get_vmcs_gpa) {
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_GUEST_PHYSICAL_ADDRESS, &u64);
		if (error == 0)
			printf("gpa[%d]\t\t0x%016lx\n", vcpu, u64);
	}

	if (!error && get_vmcs_entry_interruption_info) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_ENTRY_INTR_INFO,&u64);
		if (error == 0) {
			printf("entry_interruption_info[%d]\t0x%08lx\n",
				vcpu, u64);
		}
	}

	if (!error && get_eptp) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_EPTP, &eptp);
		if (error == 0)
			printf("eptp[%d]\t\t0x%016lx\n", vcpu, eptp);
	}

	if (!error && get_exception_bitmap) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_EXCEPTION_BITMAP,
					  &bm);
		if (error == 0)
			printf("exception_bitmap[%d]\t0x%08lx\n", vcpu, bm);
	}

	if (!error && get_io_bitmap) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_IO_BITMAP_A, &bm);
		if (error == 0)
			printf("io_bitmap_a[%d]\t0x%08lx\n", vcpu, bm);
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_IO_BITMAP_B, &bm);
		if (error == 0)
			printf("io_bitmap_b[%d]\t0x%08lx\n", vcpu, bm);
	}

	if (!error && get_tsc_offset) {
		uint64_t tscoff;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_TSC_OFFSET, &tscoff);
		if (error == 0)
			printf("tsc_offset[%d]\t0x%016lx\n", vcpu, tscoff);
	}

	if (!error && get_cr0_mask) {
		uint64_t cr0mask;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR0_MASK, &cr0mask);
		if (error == 0)
			printf("cr0_mask[%d]\t\t0x%016lx\n", vcpu, cr0mask);
	}

	if (!error && get_cr0_shadow) {
		uint64_t cr0shadow;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR0_SHADOW,
					  &cr0shadow);
		if (error == 0)
			printf("cr0_shadow[%d]\t\t0x%016lx\n", vcpu, cr0shadow);
	}

	if (!error && get_cr4_mask) {
		uint64_t cr4mask;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR4_MASK, &cr4mask);
		if (error == 0)
			printf("cr4_mask[%d]\t\t0x%016lx\n", vcpu, cr4mask);
	}

	if (!error && get_cr4_shadow) {
		uint64_t cr4shadow;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR4_SHADOW,
					  &cr4shadow);
		if (error == 0)
			printf("cr4_shadow[%d]\t\t0x%016lx\n", vcpu, cr4shadow);
	}
	
	if (!error && get_cr3_targets) {
		uint64_t target_count, target_addr;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR3_TARGET_COUNT,
					  &target_count);
		if (error == 0) {
			printf("cr3_target_count[%d]\t0x%08lx\n",
				vcpu, target_count);
		}

		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR3_TARGET0,
					  &target_addr);
		if (error == 0) {
			printf("cr3_target0[%d]\t\t0x%016lx\n",
				vcpu, target_addr);
		}

		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR3_TARGET1,
					  &target_addr);
		if (error == 0) {
			printf("cr3_target1[%d]\t\t0x%016lx\n",
				vcpu, target_addr);
		}

		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR3_TARGET2,
					  &target_addr);
		if (error == 0) {
			printf("cr3_target2[%d]\t\t0x%016lx\n",
				vcpu, target_addr);
		}

		error = vm_get_vmcs_field(ctx, vcpu, VMCS_CR3_TARGET3,
					  &target_addr);
		if (error == 0) {
			printf("cr3_target3[%d]\t\t0x%016lx\n",
				vcpu, target_addr);
		}
	}

	if (!error && get_apic_access_addr) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_APIC_ACCESS, &addr);
		if (error == 0)
			printf("apic_access_addr[%d]\t0x%016lx\n", vcpu, addr);
	}

	if (!error && get_virtual_apic_addr) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_VIRTUAL_APIC, &addr);
		if (error == 0)
			printf("virtual_apic_addr[%d]\t0x%016lx\n", vcpu, addr);
	}

	if (!error && get_tpr_threshold) {
		uint64_t threshold;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_TPR_THRESHOLD,
					  &threshold);
		if (error == 0)
			printf("tpr_threshold[%d]\t0x%08lx\n", vcpu, threshold);
	}

	if (!error && get_msr_bitmap_address) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_MSR_BITMAP, &addr);
		if (error == 0)
			printf("msr_bitmap[%d]\t\t0x%016lx\n", vcpu, addr);
	}

	if (!error && get_msr_bitmap) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_MSR_BITMAP, &addr);
		if (error == 0)
			error = dump_vmcs_msr_bitmap(vcpu, addr);
	}

	if (!error && get_vpid) {
		uint64_t vpid;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_VPID, &vpid);
		if (error == 0)
			printf("vpid[%d]\t\t0x%04lx\n", vcpu, vpid);
	}
	
	if (!error && get_ple_window) {
		uint64_t window;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_PLE_WINDOW, &window);
		if (error == 0)
			printf("ple_window[%d]\t\t0x%08lx\n", vcpu, window);
	}

	if (!error && get_ple_gap) {
		uint64_t gap;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_PLE_GAP, &gap);
		if (error == 0)
			printf("ple_gap[%d]\t\t0x%08lx\n", vcpu, gap);
	}

	if (!error && get_inst_err) {
		uint64_t insterr;
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_INSTRUCTION_ERROR,
					  &insterr);
		if (error == 0) {
			printf("instruction_error[%d]\t0x%08lx\n",
				vcpu, insterr);
		}
	}

	if (!error && get_exit_ctls) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_EXIT_CTLS, &ctl);
		if (error == 0)
			printf("exit_ctls[%d]\t\t0x%08lx\n", vcpu, ctl);
	}

	if (!error && get_entry_ctls) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_ENTRY_CTLS, &ctl);
		if (error == 0)
			printf("entry_ctls[%d]\t\t0x%08lx\n", vcpu, ctl);
	}

	if (!error && get_host_pat) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_HOST_IA32_PAT, &pat);
		if (error == 0)
			printf("host_pat[%d]\t\t0x%016lx\n", vcpu, pat);
	}

	if (!error && get_guest_pat) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_GUEST_IA32_PAT, &pat);
		if (error == 0)
			printf("guest_pat[%d]\t\t0x%016lx\n", vcpu, pat);
	}

	if (!error && get_host_cr0) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_HOST_CR0, &cr0);
		if (error == 0)
			printf("host_cr0[%d]\t\t0x%016lx\n", vcpu, cr0);
	}

	if (!error && get_host_cr3) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_HOST_CR3, &cr3);
		if (error == 0)
			printf("host_cr3[%d]\t\t0x%016lx\n", vcpu, cr3);
	}

	if (!error && get_host_cr4) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_HOST_CR4, &cr4);
		if (error == 0)
			printf("host_cr4[%d]\t\t0x%016lx\n", vcpu, cr4);
	}

	if (!error && get_host_rip) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_HOST_RIP, &rip);
		if (error == 0)
			printf("host_rip[%d]\t\t0x%016lx\n", vcpu, rip);
	}

	if (!error && get_host_rsp) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_HOST_RSP, &rsp);
		if (error == 0)
			printf("host_rip[%d]\t\t0x%016lx\n", vcpu, rsp);
	}

	if (!error && get_guest_sysenter) {
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_GUEST_IA32_SYSENTER_CS, &cs);
		if (error == 0)
			printf("guest_sysenter_cs[%d]\t0x%08lx\n", vcpu, cs);

		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_GUEST_IA32_SYSENTER_ESP, &rsp);
		if (error == 0)
			printf("guest_sysenter_sp[%d]\t0x%016lx\n", vcpu, rsp);
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_GUEST_IA32_SYSENTER_EIP, &rip);
		if (error == 0)
			printf("guest_sysenter_ip[%d]\t0x%016lx\n", vcpu, rip);
	}

	if (!error && get_vmcs_link) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_LINK_POINTER, &addr);
		if (error == 0)
			printf("vmcs_pointer[%d]\t0x%016lx\n", vcpu, addr);
	}

	if (!error && get_vmcs_exit_reason) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_EXIT_REASON, &u64);
		if (error == 0)
			printf("vmcs_exit_reason[%d]\t0x%016lx\n", vcpu, u64);
	}

	if (!error && get_vmcs_exit_qualification) {
		error = vm_get_vmcs_field(ctx, vcpu, VMCS_EXIT_QUALIFICATION,
					  &u64);
		if (error == 0)
			printf("vmcs_exit_qualification[%d]\t0x%016lx\n",
				vcpu, u64);
	}

	if (!error && get_vmcs_exit_interruption_info) {
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_EXIT_INTERRUPTION_INFO, &u64);
		if (error == 0) {
			printf("vmcs_exit_interruption_info[%d]\t0x%08lx\n",
				vcpu, u64);
		}
	}

	if (!error && get_vmcs_exit_interruption_error) {
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_EXIT_INTERRUPTION_ERROR, &u64);
		if (error == 0) {
			printf("vmcs_exit_interruption_error[%d]\t0x%08lx\n",
				vcpu, u64);
		}
	}

	if (!error && get_vmcs_interruptibility) {
		error = vm_get_vmcs_field(ctx, vcpu,
					  VMCS_GUEST_INTERRUPTIBILITY, &u64);
		if (error == 0) {
			printf("vmcs_guest_interruptibility[%d]\t0x%08lx\n",
				vcpu, u64);
		}
	}

	if (!error && setcap) {
		int captype;
		captype = vm_capability_name2type(capname);
		error = vm_set_capability(ctx, vcpu, captype, capval);
		if (error != 0 && errno == ENOENT)
			printf("Capability \"%s\" is not available\n", capname);
	}

	if (!error && getcap) {
		int captype, val;
		captype = vm_capability_name2type(capname);
		error = vm_get_capability(ctx, vcpu, captype, &val);
		if (error == 0) {
			printf("Capability \"%s\" is %s on vcpu %d\n", capname,
				val ? "set" : "not set", vcpu);
		} else if (errno == ENOENT) {
			printf("Capability \"%s\" is not available\n", capname);
		}
	}

	if (!error && run) {
		error = vm_get_register(ctx, vcpu, VM_REG_GUEST_RIP, &rip);
		assert(error == 0);

		error = vm_run(ctx, vcpu, rip, &vmexit);
		if (error == 0)
			dump_vm_run_exitcode(&vmexit, vcpu);
		else
			printf("vm_run error %d\n", error);
	}

	if (error)
		printf("errno = %d\n", errno);

	if (!error && destroy)
		vm_destroy(ctx);

	exit(error);
}
