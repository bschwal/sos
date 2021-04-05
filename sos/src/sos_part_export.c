/*
 * Copyright (c) 2017 Open Grid Computing, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the BSD-type
 * license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *      Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *      Neither the name of Open Grid Computing nor the names of any
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *      Modified source versions must be plainly marked as such, and
 *      must not be misrepresented as being the original software.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \page sos_part_export Export a Partition's Contents to Another Container
 *
 * sos_part_export -C <SRC-PATH> -E <DST-PATH> <PART-NAME>
 *
 * Export the objects from a partition in one container to the
 * primary partition of another container.
 *
 * The source partition must not be in the *primary* state.
 *
 * @param "-C PATH" The *PATH* to the source container.
 * @param "-E PATH" The *PATH* to the destination container.
 * @param -I Add the exported objects to their respective indices
 *           in the destination container.
 * @param part_name The name of the partition in the source container.
 */
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sos/sos.h>

int verbose;

void usage(int argc, char *argv[])
{
	printf("sos_part_export -C <src-path> -E <dst-path> [-I] <part-name>\n");
	printf("    -C <src-path> The path to the source container.\n");
	printf("    -E <dst-path> The path to the destination container.\n");
	printf("    -I            Add exported objects to their schema indices.\n");
	printf("    <part-name>   The name of the partition in the source container to export.\n");
	exit(1);
}

const char *short_options = "C:E:I";

struct option long_options[] = {
	{"help",    no_argument,        0,  '?'},
	{"path",    required_argument,  0,  'C'},
	{"export",  required_argument,  0,  'E'},
	{"index",	no_argument,	    0,  'I'},
	{0,         0,                  0,  0}
};

int main(int argc, char **argv)
{
	sos_t src_sos, dst_sos;
	int opt;
	char *part_name = NULL;
	char *src_path = NULL;
	char *dst_path = NULL;
	int reindex = 0;
	while (0 < (opt = getopt_long(argc, argv, short_options, long_options, NULL))) {
		switch (opt) {
		case 'C':
			src_path = strdup(optarg);
			break;
		case 'E':
			dst_path = strdup(optarg);
			break;
		case 'I':
			reindex = 1;
			break;
		case '?':
		default:
			usage(argc, argv);
		}
	}

	if (!src_path || !dst_path)
		usage(argc, argv);

	if (optind < argc)
		part_name = strdup(argv[optind]);
	else
		usage(argc, argv);

	src_sos = sos_container_open(src_path, SOS_PERM_RO);
	if (!src_sos) {
		printf("Error %d opening the source container %s.\n",
		       errno, src_path);
		exit(1);
	}
	sos_part_t part = sos_part_find(src_sos, part_name);
	if (!part) {
		printf("The partition named '%s' was not found.\n", part_name);
		exit(1);
	}
	if (sos_part_state(part) == SOS_PART_STATE_PRIMARY) {
		printf("You cannot export objects from the primary partition.\n");
		sos_part_put(part);
		exit(2);
	}
	dst_sos = sos_container_open(dst_path, SOS_PERM_RW);
	if (!dst_sos) {
		printf("Error %d opening the destination container %s.\n",
		       errno, dst_path);
		sos_part_put(part);
		exit(1);
	}
	int64_t count = sos_part_export(part, dst_sos, reindex);
	sos_part_put(part);
	sos_container_close(src_sos, SOS_COMMIT_SYNC);
	sos_container_close(dst_sos, SOS_COMMIT_SYNC);
	if (count < 0)
		printf("Error %ld encountered exporting objects.\n", -count);
	else
		printf("%ld objects were exported.\n", count);
	return 0;
}
