/* Copyright 2013-2016 Freescale Semiconductor Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of the above-listed copyright holders nor the
 * names of any contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _FSL_DPAIOP_CMD_V10_H
#define _FSL_DPAIOP_CMD_V10_H

/* DPAIOP Version */
#define DPAIOP_VER_MAJOR				2
#define DPAIOP_VER_MINOR				2

/* Command IDs */
#define DPAIOP_CMDID_CREATE                     (0x90a << 4) | (0x1)
#define DPAIOP_CMDID_DESTROY                    (0x98a << 4) | (0x1)
#define DPAIOP_CMDID_GET_VERSION                (0xa0a << 4) | (0x1)
#define DPAIOP_CMDID_GET_ATTR                   (0x004 << 4) | (0x1)

/*                cmd, param, offset, width, type, arg_name */
#define DPAIOP_CMD_CREATE(cmd, cfg) \
do { \
	MC_CMD_OP(cmd, 0, 0,  32, int,	    cfg->aiop_id);\
	MC_CMD_OP(cmd, 0, 32, 32, int,	    cfg->aiop_container_id);\
} while (0)

/*                cmd, param, offset, width, type,	arg_name */
#define DPAIOP_RSP_GET_ATTRIBUTES(cmd, attr) \
do { \
	MC_RSP_OP(cmd, 0, 0,  32, int,	    attr->id);\
} while (0)

/*                cmd, param, offset, width, type,      arg_name */
#define DPAIOP_RSP_GET_VERSION(cmd, major, minor) \
do { \
        MC_RSP_OP(cmd, 0, 0,  16, uint16_t, major);\
        MC_RSP_OP(cmd, 0, 16, 16, uint16_t, minor);\
} while (0)

#endif /* _FSL_DPAIOP_CMD_V10_H */
