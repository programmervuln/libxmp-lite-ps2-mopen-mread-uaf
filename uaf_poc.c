#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

typedef ptrdiff_t mpos_t;

typedef struct {
	const uint8_t *start;
	mpos_t pos;
	mpos_t size;
} MFILE;

static inline ptrdiff_t CAN_READ(MFILE *m)
{
	if (m->size >= 0)
		return m->pos >= 0 ? m->size - m->pos : 0;
	return INT_MAX;
}

static size_t mread(void *buf, size_t size, size_t num, MFILE *m)
{
	size_t should_read = size * num;
	ptrdiff_t can_read = CAN_READ(m);
	if (!size || !num || can_read <= 0) return 0;
	if (should_read > can_read) should_read = can_read;
	memcpy(buf, m->start + m->pos, should_read);
	m->pos += should_read;
	return should_read / size;
}

static MFILE *mopen(const void *ptr, long size)
{
	MFILE *m = malloc(sizeof(MFILE));
	if (!m) return NULL;
	m->start = ptr;
	m->pos = 0;
	m->size = size;
	return m;
}

static int mclose(MFILE *m) { free(m); return 0; }

int main(void)
{
	uint8_t *buf = malloc(256);
	if (!buf) return 1;

	MFILE *mf = mopen(buf, 256);


	free(buf);

	
	uint8_t tmp[16];
	mread(tmp, 1, 16, mf);

	mclose(mf);
	return 0;
}

