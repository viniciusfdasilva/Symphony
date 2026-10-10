#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

// Descriptor of memref 1D
struct MemRef1D {
  float *allocated;
  float *aligned;
  int64_t offset;
  int64_t sizes[1];
  int64_t strides[1];
};

extern "C" void _mlir_ciface_fx(MemRef1D *result);

static void put16(FILE *f, uint16_t v) { fwrite(&v, 2, 1, f); }
static void put32(FILE *f, uint32_t v) { fwrite(&v, 4, 1, f); }

int main(int argc, char **argv) {
  const char *path = argc > 1 ? argv[1] : "tone.wav";
  const uint32_t sampleRate = 48000;

  MemRef1D buf{};
  _mlir_ciface_fx(&buf);
  const uint32_t n = (uint32_t)buf.sizes[0];
  const float *data = buf.aligned + buf.offset;

  FILE *f = fopen(path, "wb");
  if (!f) { perror("fopen"); return 1; }

  const uint32_t dataBytes = n * 2; // 16 bits, mono
  fwrite("RIFF", 1, 4, f); put32(f, 36 + dataBytes);
  fwrite("WAVE", 1, 4, f);
  fwrite("fmt ", 1, 4, f); put32(f, 16);
  put16(f, 1);                 // PCM
  put16(f, 1);                 // mono
  put32(f, sampleRate);
  put32(f, sampleRate * 2);    // byte rate
  put16(f, 2);                 // block align
  put16(f, 16);                // bits per sample
  fwrite("data", 1, 4, f); put32(f, dataBytes);

  for (uint32_t i = 0; i < n; ++i) {
    float x = std::clamp(data[i], -1.0f, 1.0f);
    put16(f, (uint16_t)(int16_t)(x * 32767.0f));
  }
  fclose(f);

  printf("Recording %s: %u samples\n", path, n);
  free(buf.allocated);
  return 0;
}
