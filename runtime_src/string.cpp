#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" {

  struct String {
    char* data;
    int64_t length;
    int64_t capacity;
  };

  void str_concat(String* out, const String* a, const String* b) {
    out->length = a->length + b->length;
    out->capacity = out->length + 1;
    out->data = (char*)malloc(out->capacity);

    memcpy(out->data, a->data, a->length);
    memcpy(out->data + a->length, b->data, b->length);
    out->data[out->length] = '\0';

  }

  void str_repeat(String* out, const String* a, int64_t b) {
    if (b <= 0) {
      out->length = 0;
      out->capacity = 1;
      out->data = (char*)malloc(1);
      out->data[0] = '\0';
      return ;
    }

    out->length = a->length * b;
    out->capacity = out->length + 1;
    out->data = (char*)malloc(out->capacity);

    char* current = out->data;
    for (int64_t i = 0; i < b; ++i) {
      memcpy(current, a->data, a->length);
      current += a->length;
    }
    *current = '\0';

  }
}
