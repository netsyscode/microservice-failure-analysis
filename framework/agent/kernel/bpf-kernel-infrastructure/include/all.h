/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* Copyright (c) 2020
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 */
#ifndef _ALL_H__
#define _ALL_H__

#define USE_CO_RE
#ifdef USE_CO_RE

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-attributes"

#include "vmlinux.h"

#else
#include "kernel.h"
#endif 

#include "bpf_endian.h"
#include "bpf_helpers.h"
#include "bpf_tracing.h"
#include "bpf_core_read.h"

#pragma clang diagnostic pop

#endif