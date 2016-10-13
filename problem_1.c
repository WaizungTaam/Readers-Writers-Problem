/* 2016-10-13 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_LEN 64
#define NUM_WRITERS 4
#define NUM_READERS 16
#define WRITER_SLEEP 0
#define READER_SLEEP 0

char __resource[MAX_LEN + 1];
unsigned long __resource_len = 0;
unsigned long __reader_count = 0;
pthread_mutex_t __write_mutex;
pthread_mutex_t __read_mutex;

unsigned long min(unsigned long a, unsigned long b) {
  return a <= b ? a : b;
}

void write_resource(unsigned long tid) {
  char ch = 'a' + __resource_len % 26;
  __resource[__resource_len] = ch;
  ++__resource_len;
  printf("[ Writer %lu ] Writing..., len = %lu\n", tid, __resource_len);
}
void read_resource(unsigned long tid) {
  printf("[ Reader %lu ] %s\n", tid, __resource);
}

void* writer(void* thread_id) {
  unsigned long tid = (unsigned long)thread_id;
  pthread_mutex_lock(&__write_mutex);
  while (__resource_len < MAX_LEN) {
    write_resource(tid);
    pthread_mutex_unlock(&__write_mutex);
    sleep(WRITER_SLEEP);
    pthread_mutex_lock(&__write_mutex);
  }
  pthread_mutex_unlock(&__write_mutex);
  pthread_exit(NULL);
}

void* reader(void* thread_id) {
  unsigned long tid = (unsigned long)thread_id;
  while (__resource_len < MAX_LEN) {
    pthread_mutex_lock(&__read_mutex);
    ++__reader_count;
    if (__reader_count == 1) {
      pthread_mutex_lock(&__write_mutex);
    }
    pthread_mutex_unlock(&__read_mutex);

    read_resource(tid);

    pthread_mutex_lock(&__read_mutex);
    --__reader_count;
    if (__reader_count == 0) {
      pthread_mutex_unlock(&__write_mutex);
    }
    pthread_mutex_unlock(&__read_mutex);

    sleep(READER_SLEEP);
  }
  pthread_exit(NULL);
}

int main() {
  pthread_mutex_init(&__write_mutex, NULL);
  pthread_mutex_init(&__read_mutex, NULL);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  pthread_t writers[NUM_WRITERS], readers[NUM_READERS];
  unsigned long i = 0;
  for (i = 0; i < min(NUM_WRITERS, NUM_READERS); ++i) {
    pthread_create(&writers[i], &attr, writer, (void*)i);
    pthread_create(&readers[i], &attr, reader, (void*)i);
  }
  for (unsigned long j = i; j < NUM_WRITERS; ++j) {
    pthread_create(&writers[j], &attr, writer, (void*)j);
  }
  for (unsigned long k = i; k < NUM_READERS; ++k) {
    pthread_create(&readers[k], &attr, reader, (void*)k);
  }

  for (unsigned long j = 0; j < NUM_WRITERS; ++j) {
    pthread_join(writers[j], NULL);
  }
  for (unsigned long k = 0; k < NUM_READERS; ++k) {
    pthread_join(readers[k], NULL);
  }

  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&__write_mutex);
  pthread_mutex_destroy(&__read_mutex);
  pthread_exit(NULL);
  return 0;
}