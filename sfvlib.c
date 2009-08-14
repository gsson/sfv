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

#include <ctype.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <unistd.h>

#include "sfvlib.h"
#include "crc.h"     

const char *pattern = "^([^;#].+)[[:space:]]+(0x)?([0-9A-Fa-f]{1,8})[[:space:]]*$";

#define iscomment(a) ((a) == ';' || (a) == '#' || isspace(a))

char *
matchdup(const char *src, const regmatch_t *match) {
	char *dst, *c;
	size_t length = match->rm_eo - match->rm_so;
	src += match->rm_so;
	
	if ((dst = malloc(length + 1)) == NULL)
		err(-1, "malloc()");
	
	for (c = dst; c < dst + length; c++, src++)
		*c = *src;
	*c = '\0';
	
	return dst;
}

struct sfv_list *
open_sfv(const char *path) {
	char line[SFV_LINE_MAX];
	struct sfv_list *list;
	struct sfv_entry *e = NULL;
	struct sfv_entry *e_last = NULL;
	FILE *f;

	regex_t regex;
	regmatch_t matches[4];
	
	if (regcomp(&regex, pattern, REG_NEWLINE | REG_EXTENDED) != 0)
		errx(-1, "regcomp()");
	
	if ((list = malloc(sizeof(struct sfv_list))) == NULL)
			err(-1, "malloc()");

	LIST_INIT(list);

	if ((f = fopen(path, "rt")) == NULL)
		err(-1, "fopen()");
	
	if ((e = malloc(sizeof(struct sfv_entry))) == NULL)
			err(-1, "malloc()");

	while (fgets(line, SFV_LINE_MAX, f) != NULL) {
		if (regexec(&regex, line, 4, matches, 0) == 0) {
			if (matches[1].rm_so != -1 && matches[3].rm_so != -1) {
				errno = 0;
				uint32_t crc = strtoul(line + matches[3].rm_so, NULL, 16);
				if (errno) continue;

				e->path = matchdup(line, &matches[1]);
				e->crc = crc;
				e->status = 0;
				e->realpath = NULL;

				if (e_last == NULL) {
					LIST_INSERT_HEAD(list, e, entries);
				}
				else {
					LIST_INSERT_AFTER(e_last, e, entries);
				}
				e_last = e;

				if ((e = malloc(sizeof(struct sfv_entry))) == NULL)
						err(-1, "malloc()");
			}
		}
	}
	
	free(e);
	fclose(f);
	regfree(&regex);
	return list;
}

struct sfv_list *
create_sfv() {
	struct sfv_list *list;
	
	if ((list = malloc(sizeof(struct sfv_list))) == NULL)
			err(-1, "malloc()");

	LIST_INIT(list);

	return list;
}

struct sfv_entry *
match_name(const char *path, struct sfv_list *list) {
	struct sfv_entry *e;
	LIST_FOREACH(e, list, entries) {
		if (e->status == 0 && strcasecmp(path, e->path) == 0)
			return e;
	}
	return NULL;
}

void
match_sfv(const char *dirname, struct sfv_list *list) {
	struct dirent *d;
	struct sfv_entry *e;
	DIR *dir = opendir(dirname);
	
	while((d = readdir(dir)) != NULL) {
		if ((e = match_name(d->d_name, list)) != NULL) {
			e->realpath = strdup(d->d_name);
			e->status |= SFV_FOUND;
		}
	}
	closedir(dir);
}

void
close_sfv(struct sfv_list *list) {
	struct sfv_entry *e;
	while (list->lh_first != NULL) {
		e = list->lh_first;
		LIST_REMOVE(list->lh_first, entries);
		
		if (e->path != NULL) {
			free(e->path);
		}
		
		if (e->realpath != NULL) {
			free(e->realpath);
		}
		free(e);
	}
}

void
verify_sfv(struct sfv_list *list, const char *dirname, void (*fn)(const char *, int)) {
	struct sfv_entry *e;
	int curdir_fd;

	match_sfv(dirname, list);
	
	if ((curdir_fd = open(".", O_RDONLY, 0)) == -1)
		err(-1, "open()");

	if (chdir(dirname))
		err(-1, "chdir()");
	
	LIST_FOREACH(e, list, entries) {
		if (e->realpath != NULL && e->crc == crc32_file(e->realpath)) {
			e->status |= SFV_CRC_OK;
		}
		fn(e->path, e->status);
	}
	
	fchdir(curdir_fd);
	close(curdir_fd);
}

void
update_sfv(struct sfv_list *list, const char *dirname, void (*fn)(const char *, int)) {
	struct sfv_entry *e;
	int curdir_fd;

	match_sfv(dirname, list);
	
	if ((curdir_fd = open(".", O_RDONLY, 0)) == -1)
		err(-1, "open()");

	if (chdir(dirname))
		err(-1, "chdir()");
	
	LIST_FOREACH(e, list, entries) {
		if (e->realpath != NULL) {
			e->crc = crc32_file(e->realpath);
			fn(e->path, SFV_FOUND | SFV_CRC_OK);
		}
		else {
			fn(e->path, 0);
		}
	}
	
	fchdir(curdir_fd);
	close(curdir_fd);
}

void
add_sfv(struct sfv_list *list, int c, char **paths) {
	struct sfv_entry *e;
	struct sfv_entry *e_last = NULL;

	if (c > 0) {
		if ((e = malloc(sizeof(struct sfv_entry))) == NULL)
				err(-1, "malloc()");
		
		for (int i = 0; i < c; i++) {
			e->path = strdup(paths[i]);
			e->realpath = NULL;
			e->crc = 0;
			e->status = 0;
			
			if (e_last == NULL) {
				LIST_INSERT_HEAD(list, e, entries);
			}
			else {
				LIST_INSERT_AFTER(e_last, e, entries);
			}
			e_last = e;
	
			if ((e = malloc(sizeof(struct sfv_entry))) == NULL)
					err(-1, "malloc()");
		}
	}
}

void
save_sfv(struct sfv_list *list, const char *path) {
	struct sfv_entry *e;
	FILE *f;

	if ((f = fopen(path, "w+t")) == NULL)
		err(-1, "fopen()");

	LIST_FOREACH(e, list, entries) {
		if (e->realpath != NULL) {
			fprintf(f, "%s %08x\n", e->path, e->crc);
		}
		else {
			fprintf(f, "; %s failed.\n", e->path);
		}
	}
	fclose(f);
}
