/*
 * Copyright (C) 2014 Sergey Senozhatsky.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/lz4.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

#include "zcomp_lz4.h"

static void *zcomp_lz4_create(void)
{
	void *ret;

	/*
	 * This function can be called in swapout/fs write path
	 * so we can't use GFP_FS|IO. And it assumes we already
	 * have at least one stream in zram initialization so we
	 * don't do best effort to allocate more stream in here.
	 * A default stream will work well without further multiple
	 * streams. That's why we use NORETRY | NOWARN.
	 */
	ret = kzalloc(LZ4_MEM_COMPRESS, GFP_NOIO | __GFP_NORETRY |
					__GFP_NOWARN);
	if (!ret)
		ret = __vmalloc(LZ4_MEM_COMPRESS,
				GFP_NOIO | __GFP_NORETRY | __GFP_NOWARN |
				__GFP_ZERO | __GFP_HIGHMEM,
				PAGE_KERNEL);
	return ret;
}

static void zcomp_lz4_destroy(void *private)
{
	kvfree(private);
}

static int zcomp_lz4_compress(const unsigned char *src, unsigned char *dst,
		size_t *dst_len, void *private)
{
	/* return  : Success if return 0 */
	return LZ4_compress_default(src, dst, PAGE_SIZE, *dst_len, private);
}

static int zcomp_lz4_decompress(const unsigned char *src, size_t src_len,
		unsigned char *dst)
{
	/* return  : Success if return 0 */
	return LZ4_decompress_safe(src, dst, src_len, PAGE_SIZE);
}

struct zcomp_backend zcomp_lz4 = {
	.compress = zcomp_lz4_compress,
	.decompress = zcomp_lz4_decompress,
	.create = zcomp_lz4_create,
	.destroy = zcomp_lz4_destroy,
	.name = "lz4",
};
