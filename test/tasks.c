#include "tasks.h"

size_t st_assert_count = 0;
struct timespec* st_bench_time_start = 0;
struct timespec* st_bench_time_end = 0;

// move functions here

char* bench_get_current_date() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  char* out = (char*)malloc(sizeof(char) * 50);

  sprintf(out, "%d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1,
          tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  return out;
}

void bench_start() {
  st_bench_time_start = (struct timespec*)malloc(sizeof(struct timespec));
  // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, st_bench_time_start);
  clock_gettime(CLOCK_MONOTONIC, st_bench_time_start);
}

double bench_end() {
  st_bench_time_end = (struct timespec*)malloc(
      sizeof(struct timespec)); // clock_gettime(CLOCK_PROCESS_CPUTIME_ID,
                                // st_bench_time_end);
  clock_gettime(CLOCK_MONOTONIC, st_bench_time_end);

  double result =
      (st_bench_time_end->tv_sec - st_bench_time_start->tv_sec) * 1E9 +
      (st_bench_time_end->tv_nsec - st_bench_time_start->tv_nsec);

  free(st_bench_time_start);
  st_bench_time_start = 0;
  free(st_bench_time_end);
  st_bench_time_end = 0;
  return result;
}

void trace(FILE* t) {
  void* array[10];
  size_t size;
  char** strings;
  size_t i;

  size = backtrace(array, 10);
  strings = backtrace_symbols(array, size);

  fprintf(t, "Obtained %zd stack frames.\n", size);

  for (i = 0; i < size; i++) {
    fprintf(t, "%s\n", strings[i]);
  }

  free(strings);
}
