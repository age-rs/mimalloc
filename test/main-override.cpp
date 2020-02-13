#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include <mimalloc-new-delete.h>
#include <mimalloc.h>
#include <new>
#include <vector>
#include <mimalloc-override.h>

static void* p = malloc(8);

void free_p() {
  free(p);
  return;
}

class Test {
private:
  int i;
public:
  Test(int x) { i = x; }
  ~Test() { }
};

void dangling_ptr_write();

int main() {
  mi_stats_reset();  // ignore earlier allocations
  atexit(free_p);
  //dangling_ptr_write();
  void* p1 = malloc(78);
  void* p2 = mi_malloc_aligned(16,24);
  free(p1);  
  p1 = malloc(8);
  char* s = _strdup("hello\n");
  /*
  char* s = _strdup("hello\n");
  char* buf = NULL;
  size_t len;
  _dupenv_s(&buf,&len,"MIMALLOC_VERBOSE"); 
  mi_free(buf);
  */
  mi_free(p2);
  p2 = malloc(16);
  p1 = realloc(p1, 32);
  free(p1);
  free(p2);
  mi_free(s);  
  s[0] = 0;
  Test* t = new Test(42);
  delete t;
  // t = new(std::nothrow) Test(42);   // does not work with overriding :-(
  t = new Test(42);
  delete t;  
  mi_stats_print(NULL);
  return 0;
}

static void dangling_ptr_write() {
  for (int i = 0; i < 1000; i++) {
    uint8_t* p;
    if ((i & 1) == 0) {   // do ==0 or ==1 to get either malloc or new allocation
      p = (uint8_t*)malloc(16);
      free(p);
    }
    else {
      p = new uint8_t[16];
      // delete p;   // delete sets the pointer to an invalid value generally
      free(p);  
    }
    p[0] = 0;
  }
}

class Static {
private:
  void* p;
public:
  Static() {
    p = malloc(64);
    return;
  }
  ~Static() {
    free(p);
    return;
  }
};

static Static s = Static();


bool test_stl_allocator1() {
  std::vector<int, mi_stl_allocator<int> > vec;
  vec.push_back(1);
  vec.pop_back();
  return vec.size() == 0;
}

struct some_struct { int i; int j; double z; };

bool test_stl_allocator2() {  
  std::vector<some_struct, mi_stl_allocator<some_struct> > vec;
  vec.push_back(some_struct());
  vec.pop_back();
  return vec.size() == 0;
}