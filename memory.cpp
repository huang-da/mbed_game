#include "memory.h"

struct MemInfo {
	nat totalFree;
	nat maxChunk;
};

const nat resolution = 16;

static void memory(MemInfo &info) {
	nat max = 0;
	for (nat i = resolution; true; i += resolution) {
		void *p = malloc(i);
		if (p == 0) {
			max = i - resolution;
			break;
		}
		free(p);
	}

	if (max > 0) {
		void *p;
		while ((p = malloc(max)) == 0)
			max -= resolution;

		memory(info);
		if (info.maxChunk < max)
			info.maxChunk = max;
		info.totalFree += max;

		free(p);
	}
}

void memory() {
	MemInfo i = { 0, 0 };
	memory(i);
	printf("Total: %u bytes, max chunk: %u bytes (resolution %u bytes)\n", i.totalFree, i.maxChunk, resolution);
}
