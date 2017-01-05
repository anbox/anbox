/*
 * Copyright © 2012-2016 Canonical, Inc
 *
 * Author: Serge Hallyn <serge.hallyn@ubuntu.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 */
#define _XOPEN_SOURCE 500
#include <errno.h>
#include <ftw.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define min(a,b) (a) < (b) ? (a) : (b)
#define max(a,b) (a) > (b) ? (a) : (b)

static int verbose = 0;
static int convert_uids = 0;
static int convert_gids = 0;
static uid_t srcid;
static uid_t dstid;
static uid_t range;
static uid_t range_uid_max = 0;
static uid_t range_uid_min = ~0;
static gid_t range_gid_max = 0;
static gid_t range_gid_min = ~0;

void usage(void)
{
	extern const char *__progname;
	printf("Usage: %s [OPTIONS] directory [src dst range]\n\n", __progname);
	printf("  -u, --uid                      convert uids in directory\n");
	printf("  -g, --gid                      convert gids in directory\n");
	printf("  -b, --both                     convert uids and gids in directory\n");
	printf("  -r, --range                    find min,max uid/gid used in directory\n");
	printf("  -v, --verbose                  increate verbosity\n\n");
	printf("Note this program always recursively walks all of directory.\n");
	printf("If -u,-g, or -b is given, then [src dst range] are required to convert the \n");
	printf("ids within the range [src..src+range] to [dst..dst+range].\n\n");
	printf("Examples:\n");
	printf("  %s -r /path/to/directory                # show min/max uid/gid\n", __progname);
	printf("  %s -b /path/to/directory 0 100000 500   # map uids and gids up\n", __progname);
	printf("  %s -u /path/to/directory 100000 0 500   # map the uids back down\n", __progname);
}

int ftw_callback(const char *fpath, const struct stat *st,
		 int typeflag, struct FTW *ftw)
{
	uid_t new_uid = -1;
	uid_t new_gid = -1;
	int ret;

	range_uid_max = max(range_uid_max, st->st_uid);
	range_uid_min = min(range_uid_min, st->st_uid);
	range_gid_max = max(range_gid_max, st->st_gid);
	range_gid_min = min(range_gid_min, st->st_gid);

	if (convert_uids && st->st_uid >= srcid && st->st_uid < srcid+range)
		new_uid = (st->st_uid-srcid) + dstid;
	if (convert_gids && st->st_gid >= srcid && st->st_gid < srcid+range)
		new_gid = (st->st_gid-srcid) + dstid;
	if (new_uid != -1 || new_gid != -1) {
		ret = lchown(&fpath[ftw->base], new_uid, new_gid);
		if (ret) {
			fprintf(stderr, "failed to chown %d:%d %s\n",
				new_uid, new_gid, fpath);
			/* well, let's keep going */
		} else {
			if (!S_ISLNK(st->st_mode)) {
				if (verbose > 1)
					fprintf(stderr, "resetting mode to %o on %s\n",
						st->st_mode, fpath);
				ret = chmod(&fpath[ftw->base], st->st_mode);
				if (ret) {
					fprintf(stderr, "failed to reset mode %o on %s\n",
						st->st_mode, fpath);
					/* well, let's keep going */
				}
			}
			if (verbose)
				printf("u:%07d=%07d g:%07d=%07d m:%#07o %s %s\n",
				       st->st_uid, new_uid,
				       st->st_gid, new_gid,
				       st->st_mode, fpath, &fpath[ftw->base]);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	const char *base;
	int show_range = 0;
	int opt,ret;

	static const struct option long_opts[] = {
		{ "help",    no_argument, NULL, 'h' },
		{ "uids",    no_argument, NULL, 'u' },
		{ "gids",    no_argument, NULL, 'g' },
		{ "both",    no_argument, NULL, 'b' },
		{ "range",   no_argument, NULL, 'r' },
		{ "verbose", no_argument, NULL, 'v' },
		{ NULL,      0,           NULL, 0   }
	};

        while ((opt = getopt_long(argc, argv, "hugbrv", long_opts, NULL)) >= 0) {
		switch (opt) {
		case 'h': usage(); exit(EXIT_SUCCESS);
		case 'u': convert_uids = 1; break;
		case 'g': convert_gids = 1; break;
		case 'b': convert_uids = convert_gids = 1; break;
		case 'r': show_range = 1; break;
		case 'v': verbose++; break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		usage();
		exit(EXIT_FAILURE);
	}

	base = argv[0];
	if (convert_uids || convert_gids) {
		if (argc < 4) {
			usage();
			exit(EXIT_FAILURE);
		}
		srcid = atoi(argv[1]);
		dstid = atoi(argv[2]);
		range = atoi(argv[3]);
	}

	ret = nftw(base, ftw_callback, 1000, FTW_PHYS|FTW_CHDIR);
	if (ret < 0) {
		fprintf(stderr, "Failed to walk path %s %s\n", base, strerror(errno));
		usage();
		return EXIT_FAILURE;
	}

	if (show_range) {
		printf("UIDs %d - %d\n"
		       "GIDs %d - %d\n",
		       range_uid_min, range_uid_max,
		       range_gid_min, range_gid_max);
	}

	return EXIT_SUCCESS;
}
