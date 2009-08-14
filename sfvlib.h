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

#ifndef SFVLIB_H_
#define SFVLIB_H_

#include <sys/queue.h>

#define SFV_LINE_MAX (1024)

#define SFV_FOUND (1 << 0)
#define SFV_CRC_OK (1 << 1)

LIST_HEAD(sfv_list, sfv_entry);

struct sfv_entry {
	LIST_ENTRY(sfv_entry) entries;

	char *path;
	char *realpath;
	uint32_t crc;
	int status;
};

struct sfv_list *open_sfv(const char *path);
struct sfv_list *create_sfv();
void verify_sfv(struct sfv_list *list, const char *dirname, void (*fn)(const char *, int));
void add_sfv(struct sfv_list *list, int c, char **paths);
void update_sfv(struct sfv_list *list, const char *dirname, void (*fn)(const char *, int));
void close_sfv(struct sfv_list *list);
void save_sfv(struct sfv_list *list, const char *path);



#endif /*SFVREAD_H_*/
