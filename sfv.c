/*
 * Copyright (c) 2009 Henrik Gustafsson <henrik.gustafsson@fnord.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <libgen.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "crc.h"
#include "sfvlib.h"

extern char *optarg;
extern int opterr;
extern int optind;
extern int optopt;
extern int optreset;

void usage(void);

int bad = 0;
int missing = 0;
int total = 0;

void verify_status(const char *name, int status) {
	const char *status_string;
	total++;
	switch (status) {
	case 0:
		status_string = "missing.";
		missing++;
		break;
	case SFV_FOUND:
		status_string = "bad crc.";
		bad++;
		break;
	case SFV_FOUND | SFV_CRC_OK:
		status_string = "crc ok.";
		break;
	default:
		errx(-1, "verify_status()");
	}
	printf("%s %s\n", name, status_string);
}

void create_status(const char *name, int status) {
	const char *status_string;
	total++;
	switch (status) {
	case 0:
		status_string = "missing.";
		missing++;
		break;
	case SFV_FOUND | SFV_CRC_OK:
		status_string = "added.";
		break;
	default:
		errx(-1, "create_status(): %s", name);
	}
	printf("%s %s\n", name, status_string);
}

void quiet_status(const char *name, int status) {
	total++;
	switch (status) {
	case 0:
		missing++;
		break;
	case SFV_FOUND:
		bad++;
		break;
	case SFV_FOUND | SFV_CRC_OK:
		break;
	default:
		errx(-1, "quiet_status()");
	}
}

void
usage(void) {
	extern char *__progname;
	fprintf(stderr, "usage: %s [-d directory] -q sfv\n", __progname);
	fprintf(stderr, "       %s -c [-d directory] -q sfv file ...\n", __progname);
	exit(1);
}

int main(int argc, char **argv) {
	int ch, create = 0, quiet = 0, summary = 0;
	char *dir = NULL;
	const char *sfv;
	struct sfv_list * list;
	
	while ((ch = getopt(argc, argv, "cd:qs")) != -1) {
		switch (ch) {
		case 'c':
			create = 1;
			break;
		case 'd':
			dir = optarg;
			break;
		case 'q':
			quiet = 1;
			break;
		case 's':
			summary = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	
	argc -= optind;
	argv += optind;

	if (!create && argc != 1)
		usage();

	if (create && argc < 2)
		usage();
	
	sfv = argv[0];
	
	argc--;
	argv++;
	
	if (dir == NULL) {
		dir = strdup(sfv);
		dir = dirname(dir);
	}
	
	if (!create) {
		list = open_sfv(sfv);
		if (quiet) {
			verify_sfv(list, dir, quiet_status);
		}
		else {
			verify_sfv(list, dir, verify_status);
		}
	}
	else {
		list = create_sfv();
		add_sfv(list, argc, argv);
		if (quiet) {
			update_sfv(list, dir, quiet_status);
		}
		else {
			update_sfv(list, dir, create_status);
		}
			
		save_sfv(list, sfv);
	}
	close_sfv(list);
	
	if (summary) {
		if (create) {
			printf("%s: %d files added, %d missing, %d bad.\n", sfv, total, missing, bad);
		}
		else {
			printf("%s: %d files tested, %d missing, %d bad.\n", sfv, total, missing, bad);
		}
	}
	
	return ((missing > 0)?1:0) | ((bad > 0)?2:0); 
}
