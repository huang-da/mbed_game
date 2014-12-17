#include "sound.h"

Sound::~Sound() {}

SquareWave::SquareWave() : pos(false) {}

bool SquareWave::finished() { return false; }

void SquareWave::data(byte *to, nat size) {
	for (nat i = 0; i < size; i++) {
		to[i] = pos ? 0xFF : 0x00;
		pos = !pos;
	}
}

RawFile::RawFile(const char *name) {
	fp = fopen(name, "r");
}

RawFile::~RawFile() {
	fclose(fp);
}

void RawFile::data(byte *to, nat size) {
	nat pos = fread(to, sizeof(*to), size, fp);
	for (; pos < size; pos++)
		to[pos] = 0;
}

bool RawFile::finished() {
	if (feof(fp))
		return true;
	return false;
}

