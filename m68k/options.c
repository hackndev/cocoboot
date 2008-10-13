/*
 * This program is free software ; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <PalmOS.h>
#include <VFSMgr.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "cocoboot.h"
#include "options.h"
#include "imgloader.h"
#include "cocoboot_r.h"

struct option {
	char key[1024];
	char value[1024];
} options[] = {
	{
		.key = "cmdline",
		.value = " ",
	},
	{
		.key = "kernel",
		.value = "/zImage",
	},
	{
		.key = "initrd",
		.value = "/initrd.gz",
	},
	{
		.key = "noprompt",
		.value = "0",
	},
	{
		.key = "openserial",
		.value = "0",
	},
	{
		.key = "", /* sentinal */
	},
};

//int search_file(char *name, UInt16 *vol_ref, UInt32 *size);

/**
 * Read a single line from file f into the given buffer, stripping linefeeds.
 */
Err read_line(FileRef f, char *buf, int buflen)
{
	Err err;
	char c;
	int i;

	for (i=0; i<buflen; i++) {
		err = VFSFileRead(f, 1, &c, NULL);
		if (err != errNone) {
			return err;
		}

		if (c == '\n') {
			buf[i] = 0;
			break;
		}

		if (c == '\r') {
			continue;
		}

		buf[i] = c;
	}
	return errNone;
}

char *parse_config_line(char *line, char **out_key, char **out_value)
{
	char *p = line;
	char *key, *value;

	*out_key = *out_value = NULL;

	/* skip whitespace at the start of the key */
	key = line;
	while (*key && isspace(*key))
		key++;
	if (!*key || *key == '#') return NULL; /* error: empty line or comment */

	/* divide key and value into seperate strings */
	while (*p && *p != '=') p++;
	if (!*p) return "no equals sign found";
	*p = 0;
	value = p + 1;
	
	/* work backwards to find the end of the key, again skipping spaces */
	p--;
	while (p >= line && *p && isspace(*p))
		p--;
	if (p < line) return "empty key";
	p++;
	*p = 0;

	/* strip whitespace at the start of the value */
	while (*value && isspace(*value))
		value++;
	if (!*value) return "empty value";
	p = value;

	/* find end of value */
	while (*p) p++;
	p--;

	/* work backwards to find end of value, again skipping spaces */
	while (p >= value && *p && isspace(*p))
		p--;
	p++;
	*p = 0;

	*out_key = key;
	*out_value = value;
	return NULL;
}

void show_parse_error(char *errmsg, int line_no)
{
	char s_line_no[128];
	sprintf(s_line_no, "\nLine %d of " CONFIG_FILE, line_no);
	FrmCustomAlert(ErrorAlert, "Parse Error: ", errmsg, s_line_no);
}

int set_option(char *key, char *value)
{
	struct option *opt = &options[0];

	while (opt->key[0]) {
		if (!strcmp(key, opt->key)) {
			strncpy(opt->value, value, sizeof(opt->value)-1);
			return 0;
		}
		opt++;
	}
	return -1;
}

char *get_option(char *key)
{
	struct option *opt = &options[0];

	while (opt->key[0]) {
		if (!strcmp(key, opt->key)) {
			return opt->value;
		}
		opt++;
	}
	return NULL;
}

int read_config(void)
{
	Err err;
	UInt16 vol;
	UInt32 size;
	FileRef f;
	int line_no;
	char *errmsg, *key, *value;
	char buf[2048];

	if (!search_file(CONFIG_FILE, &vol, &size)) {
		return -1;
	}

	err = VFSFileOpen(vol, CONFIG_FILE, vfsModeRead, &f);
	if (err != errNone) {
		return err;
	}

	for (line_no = 1; 1; line_no++) {
		err = read_line(f, buf, sizeof(buf)-1);
		if (err != errNone) break;


		if ((errmsg=parse_config_line(buf, &key, &value))) {
			show_parse_error(errmsg, line_no);
			continue;
		}

		if (!key) continue; /* blank line or comment */

		if (set_option(key, value) < 0) {
			show_parse_error("unrecognised option", line_no);
			continue;
		}
	}
	return 0;
}
