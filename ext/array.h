// based on: http://www.happybearsoftware.com/implementing-a-dynamic-array.html
// vector.h

#define VECTOR_INITIAL_CAPACITY 100
#define ARRAY_T void*
// Define a vector type
typedef struct {
  int size;
  int capacity;
  void **data;     // array of pointers
} array;

void array_new(array *arr);

void array_append(array *arr, ARRAY_T value);

ARRAY_T array_get(array *arr, int index);

void array_set(array *arr, int index, ARRAY_T value);

void array_double_capacity_if_full(array *arr);

void array_delete(array *arr);
