/*
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 * Author: Lijun Pan <Lijun.Pan@freescale.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation  and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of any
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include "resman.h"
#include "utils.h"
#include "fsl_dpci.h"
#include "fsl_dprc.h"

/**
 * dpci info command options
 */
enum dpci_info_options {
	INFO_OPT_HELP = 0,
	INFO_OPT_VERBOSE,
};

static struct option dpci_info_options[] = {
	[INFO_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	[INFO_OPT_VERBOSE] = {
		.name = "verbose",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dpci_info_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

/**
 * dpci create command options
 */
enum dpci_create_options {
	CREATE_OPT_HELP = 0,
	CREATE_OPT_NUM_PRIORITIES,
};

static struct option dpci_create_options[] = {
	[CREATE_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	[CREATE_OPT_NUM_PRIORITIES] = {
		.name = "num-priorities",
		.has_arg = 1,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dpci_create_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

/**
 * dpci destroy command options
 */
enum dpci_destroy_options {
	DESTROY_OPT_HELP = 0,
};

static struct option dpci_destroy_options[] = {
	[DESTROY_OPT_HELP] = {
		.name = "help",
		.has_arg = 0,
		.flag = NULL,
		.val = 0,
	},

	{ 0 },
};

C_ASSERT(ARRAY_SIZE(dpci_destroy_options) <= MAX_NUM_CMD_LINE_OPTIONS + 1);

static int cmd_dpci_help(void)
{
	static const char help_msg[] =
		"\n"
		"resman dpci <command> [--help] [ARGS...]\n"
		"Where <command> can be:\n"
		"   info - displays detailed information about a DPCI object.\n"
		"   create - creates a new child DPCI under the root DPRC.\n"
		"   destroy - destroys a child DPCI under the root DPRC.\n"
		"\n"
		"For command-specific help, use the --help option of each command.\n"
		"\n";

	printf(help_msg);
	return 0;
}

static int print_dpci_attr(uint32_t dpci_id)
{
	uint16_t dpci_handle;
	int error;
	struct dpci_attr dpci_attr;
	struct dpci_peer_attr dpci_peer_attr;
	bool dpci_opened = false;
	int link_state;
	int peer;

	error = dpci_open(&resman.mc_io, dpci_id, &dpci_handle);
	if (error < 0) {
		ERROR_PRINTF("dpci_open() failed for dpci.%u with error %d\n",
			     dpci_id, error);
		goto out;
	}
	dpci_opened = true;
	if (0 == dpci_handle) {
		ERROR_PRINTF(
			"dpci_open() returned invalid handle (auth 0) for dpci.%u\n",
			dpci_id);
		error = -ENOENT;
		goto out;
	}

	memset(&dpci_attr, 0, sizeof(dpci_attr));
	error = dpci_get_attributes(&resman.mc_io, dpci_handle, &dpci_attr);
	if (error < 0) {
		ERROR_PRINTF("dpci_get_attributes() failed with error: %d\n",
			     error);
		goto out;
	}
	assert(dpci_id == (uint32_t)dpci_attr.id);

	peer = dpci_get_peer_attributes(&resman.mc_io, dpci_handle,
					 &dpci_peer_attr);

	error = dpci_get_link_state(&resman.mc_io, dpci_handle, &link_state);
	if (error < 0) {
		ERROR_PRINTF("dpci_get_link_state() failed with error: %d\n",
			     error);
		goto out;
	}

	printf("dpci version: %u.%u\n", dpci_attr.version.major,
	       dpci_attr.version.minor);
	printf("dpci id: %d\n", dpci_attr.id);
	printf("num_of_priorities: %u\n",
	       (unsigned int)dpci_attr.num_of_priorities);
	printf("connected peer: ");
	if (0 == peer) {
		printf("dpci.%d\n", dpci_peer_attr.peer_id);
		printf("peer's num_of_priorities: %u\n",
		       (unsigned int)dpci_peer_attr.num_of_priorities);
	} else {
		printf("no peer\n");
	}
	printf("link status: %d - ", link_state);
	link_state == 0 ? printf("down\n") :
	link_state == 1 ? printf("up\n") : printf("error state\n");

	error = 0;

out:
	if (dpci_opened) {
		int error2;

		error2 = dpci_close(&resman.mc_io, dpci_handle);
		if (error2 < 0) {
			ERROR_PRINTF("dpci_close() failed with error %d\n",
				     error2);
			if (error == 0)
				error = error2;
		}
	}

	return error;
}

static int print_dpci_verbose(uint16_t dprc_handle,
			      int nesting_level, uint32_t target_id)
{
	int num_child_devices;
	int error = 0;
	uint32_t irq_mask;
	uint32_t irq_status;

	assert(nesting_level <= MAX_DPRC_NESTING);

	error = dprc_get_obj_count(&resman.mc_io,
				   dprc_handle,
				   &num_child_devices);
	if (error < 0) {
		ERROR_PRINTF("dprc_get_object_count() failed with error %d\n",
			     error);
		goto out;
	}

	for (int i = 0; i < num_child_devices; i++) {
		struct dprc_obj_desc obj_desc;
		uint16_t child_dprc_handle;
		uint16_t dpci_handle;
		int error2;

		error = dprc_get_obj(
				&resman.mc_io,
				dprc_handle,
				i,
				&obj_desc);
		if (error < 0) {
			ERROR_PRINTF(
				"dprc_get_object(%u) failed with error %d\n",
				i, error);
			goto out;
		}

		if (strcmp(obj_desc.type, "dpci") == 0 &&
		    target_id == (uint32_t)obj_desc.id) {
			printf("plugged state: %splugged\n",
			       (obj_desc.state & DPRC_OBJ_STATE_PLUGGED) ?
			       "" : "un");
			printf("number of mappable regions: %u\n",
			       obj_desc.region_count);
			printf("number of interrupts: %u\n",
			       obj_desc.irq_count);

			error = dpci_open(&resman.mc_io, target_id,
					  &dpci_handle);
			if (error < 0) {
				ERROR_PRINTF(
					"dpci_open() failed for dpci.%u with error %d\n",
					target_id, error);
				goto out;
			}

			for (int j = 0; j < obj_desc.irq_count; j++) {
				dpci_get_irq_mask(&resman.mc_io,
					dpci_handle, j, &irq_mask);
				printf(
					"interrupt %d's mask: %#x\n",
					j, irq_mask);
				dpci_get_irq_status(&resman.mc_io,
					dpci_handle, j, &irq_status);
				(irq_status == 0) ?
				printf(
					"interrupt %d's status: %#x - no interrupt pending.\n",
				j, irq_status) :
				(irq_status == 1) ?
				printf(
					"interrupt %d's status: %#x - interrupt pending.\n",
					j, irq_status) :
				printf(
					"interrupt %d's status: %#x - error status.\n",
					j, irq_status);
			}

			error2 = dpci_close(&resman.mc_io, dpci_handle);
			if (error2 < 0) {
				ERROR_PRINTF(
					"dpci_close() failed with error %d\n",
					error2);
				if (error == 0)
					error = error2;
			}
			goto out;
		} else if (strcmp(obj_desc.type, "dprc") == 0) {
			error = open_dprc(obj_desc.id, &child_dprc_handle);
			if (error < 0)
				goto out;

			error = print_dpci_verbose(child_dprc_handle,
					  nesting_level + 1, target_id);

			error2 = dprc_close(&resman.mc_io, child_dprc_handle);
			if (error2 < 0) {
				ERROR_PRINTF(
					"dprc_close() failed with error %d\n",
					error2);
				if (error == 0)
					error = error2;

				goto out;
			}
		} else {
			continue;
		}
	}

out:
	return error;
}

static int print_dpci_info(uint32_t dpci_id)
{
	int error;

	error = print_dpci_attr(dpci_id);
	if (error < 0)
		goto out;

	if (resman.cmd_option_mask & ONE_BIT_MASK(INFO_OPT_VERBOSE)) {
		resman.cmd_option_mask &= ~ONE_BIT_MASK(INFO_OPT_VERBOSE);
		error = print_dpci_verbose(resman.root_dprc_handle, 0, dpci_id);
		goto out;
	}

out:
	return error;
}

static int cmd_dpci_info(void)
{
	static const char usage_msg[] =
	"\n"
	"Usage: resman dpci info <dpci-object> [--verbose]\n"
	"   e.g. resman dpci info dpci.8\n"
	"\n"
	"--verbose\n"
	"   Shows extended/verbose information about the object\n"
	"   e.g. resman dpci info dpci.8 --verbose\n"
	"\n";

	uint32_t obj_id;
	int error;

	if (resman.cmd_option_mask & ONE_BIT_MASK(INFO_OPT_HELP)) {
		printf(usage_msg);
		resman.cmd_option_mask &= ~ONE_BIT_MASK(INFO_OPT_HELP);
		error = 0;
		goto out;
	}

	if (resman.obj_name == NULL) {
		ERROR_PRINTF("<object> argument missing\n");
		printf(usage_msg);
		error = -EINVAL;
		goto out;
	}

	error = parse_object_name(resman.obj_name, "dpci", &obj_id);
	if (error < 0)
		goto out;

	error = print_dpci_info(obj_id);
out:
	return error;
}

static int cmd_dpci_create(void)
{
	static const char usage_msg[] =
		"\n"
		"Usage: resman dpci create [OPTIONS]\n"
		"   e.g. create a DPCI object with all default options:\n"
		"	resman dpci create\n"
		"\n"
		"OPTIONS:\n"
		"if options are not specified, create DPCI by default options\n"
		"--num-priorities=<number>\n"
		"   specifies the number of priorities\n"
		"   valid values are 1-2\n"
		"   Default value is 1\n"
		"   e.g. resman dpci create --num-priorities=8\n"
		"\n";

	int error;
	long val;
	char *endptr;
	char *str;
	struct dpci_cfg dpci_cfg;
	uint16_t dpci_handle;
	struct dpci_attr dpci_attr;

	if (resman.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_HELP)) {
		printf(usage_msg);
		resman.cmd_option_mask &= ~ONE_BIT_MASK(CREATE_OPT_HELP);
		return 0;
	}

	if (resman.obj_name != NULL) {
		ERROR_PRINTF("Unexpected argument: \'%s\'\n\n",
			     resman.obj_name);
		printf(usage_msg);
		return -EINVAL;
	}

	if (resman.cmd_option_mask & ONE_BIT_MASK(CREATE_OPT_NUM_PRIORITIES)) {
		resman.cmd_option_mask &=
			~ONE_BIT_MASK(CREATE_OPT_NUM_PRIORITIES);
		errno = 0;
		str = resman.cmd_option_args[CREATE_OPT_NUM_PRIORITIES];
		val = strtol(str, &endptr, 0);

		if (STRTOL_ERROR(str, endptr, val, errno)/* ||
		    (val < 1 || val > 2)*/) {
			printf(usage_msg);
			return -EINVAL;
		}

		dpci_cfg.num_of_priorities = val;
	} else {
		dpci_cfg.num_of_priorities = 1;
	}

	error = dpci_create(&resman.mc_io, &dpci_cfg, &dpci_handle);
	if (error < 0) {
		ERROR_PRINTF("dpci_create() failed with error %d\n", error);
		return error;
	}

	memset(&dpci_attr, 0, sizeof(struct dpci_attr));
	error = dpci_get_attributes(&resman.mc_io, dpci_handle, &dpci_attr);
	if (error < 0) {
		ERROR_PRINTF("dpci_get_attributes() failed with error: %d\n",
			     error);
		return error;
	}
	printf("dpci.%d is created in dprc.1\n", dpci_attr.id);

	error = dpci_close(&resman.mc_io, dpci_handle);
	if (error < 0) {
		ERROR_PRINTF("dpci_close() failed with error %d\n", error);
		return error;
	}
	return 0;
}

static int cmd_dpci_destroy(void)
{
	static const char usage_msg[] =
		"\n"
		"Usage: resman dpci destroy <dpci-object>\n"
		"   e.g. resman dpci destroy dpci.3\n"
		"\n";

	int error;
	int error2;
	uint32_t dpci_id;
	uint16_t dpci_handle;
	bool dpci_opened = false;

	if (resman.cmd_option_mask & ONE_BIT_MASK(DESTROY_OPT_HELP)) {
		printf(usage_msg);
		resman.cmd_option_mask &= ~ONE_BIT_MASK(DESTROY_OPT_HELP);
		return 0;
	}

	if (resman.obj_name == NULL) {
		ERROR_PRINTF("<object> argument missing\n");
		printf(usage_msg);
		error = -EINVAL;
		goto out;
	}

	error = parse_object_name(resman.obj_name, "dpci", &dpci_id);
	if (error < 0)
		goto out;

	error = dpci_open(&resman.mc_io, dpci_id, &dpci_handle);
	if (error < 0) {
		ERROR_PRINTF("dpci_open() failed for dpci.%u with error %d\n",
			     dpci_id, error);
		goto out;
	}
	dpci_opened = true;
	if (0 == dpci_handle) {
		ERROR_PRINTF(
			"dpci_open() returned invalid handle (auth 0) for dpci.%u\n",
			dpci_id);
		error = -ENOENT;
		goto out;
	}

	error = dpci_destroy(&resman.mc_io, dpci_handle);
	if (error < 0) {
		ERROR_PRINTF("dpci_destroy() failed with error %d\n", error);
		goto out;
	}
	dpci_opened = false;

out:
	if (dpci_opened) {
		error2 = dpci_close(&resman.mc_io, dpci_handle);
		if (error2 < 0) {
			ERROR_PRINTF("dpci_close() failed with error %d\n",
				     error2);
			if (error == 0)
				error = error2;
		}
	}

	return error;
}

struct object_command dpci_commands[] = {
	{ .cmd_name = "help",
	  .options = NULL,
	  .cmd_func = cmd_dpci_help },

	{ .cmd_name = "info",
	  .options = dpci_info_options,
	  .cmd_func = cmd_dpci_info },

	{ .cmd_name = "create",
	  .options = dpci_create_options,
	  .cmd_func = cmd_dpci_create },

	{ .cmd_name = "destroy",
	  .options = dpci_destroy_options,
	  .cmd_func = cmd_dpci_destroy },

	{ .cmd_name = NULL },
};

