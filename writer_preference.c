/*
 * Copyright 2016 Waizung Taam
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* - 2016-10-13
 * - Readers-Writers Problem -- readers-preference
 * - Reference: https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem
 */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_LEN 32
#define NUM_WRITERS 4
#define NUM_READERS 8
#define WRITER_SLEEP 0
#define READER_SLEEP 0

char __resource[MAX_LEN + 1];
unsigned long __resource_len = 0;
unsigned long __reader_count = 0;
pthread_mutex_t __write_mutex;
pthread_mutex_t __read_mutex;

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

  for (unsigned long i = 0; i < NUM_WRITERS; ++i) {
    pthread_create(&writers[i], &attr, writer, (void*)i);
  }
  for (unsigned long i = 0; i < NUM_READERS; ++i) {
    pthread_create(&readers[i], &attr, reader, (void*)i);
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