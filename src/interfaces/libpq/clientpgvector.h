// The MIT License (MIT)
// Copyright (c) 2016 Peter Goldsborough
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VECTOR_H
#define VECTOR_H

#include <stdbool.h>
#include <stddef.h>

/***** DEFINITIONS *****/

#define VECTOR_MINIMUM_CAPACITY 2
#define VECTOR_GROWTH_FACTOR 2
#define VECTOR_SHRINK_THRESHOLD (1 / 4)

#define VECTOR_ERROR -1
#define VECTOR_SUCCESS 0

#define VECTOR_UNINITIALIZED NULL
#define VECTOR_INITIALIZER \
    { 0, 0, 0, VECTOR_UNINITIALIZED }

/***** STRUCTURES *****/

typedef struct pgvector {
    size_t size;
    size_t capacity;
    size_t element_size;

    void* data;
} pgvector;

typedef struct Iterator {
    void* pointer;
    size_t element_size;
} Iterator;

/***** METHODS *****/

/* Constructor */
int vector_setup(pgvector* vector, size_t capacity, size_t element_size);

/* Copy Constructor */
int vector_copy(pgvector* destination, pgvector* source);

/* Copy Assignment */
int vector_copy_assign(pgvector* destination, pgvector* source);

/* Move Constructor */
int vector_move(pgvector* destination, pgvector* source);

/* Move Assignment */
int vector_move_assign(pgvector* destination, pgvector* source);

int vector_swap(pgvector* destination, pgvector* source);

/* Destructor */
int vector_destroy(pgvector* vector);

/* Insertion */
int vector_push_back(pgvector* vector, void* element);
int vector_push_front(pgvector* vector, void* element);
int vector_insert(pgvector* vector, size_t index, void* element);
int vector_assign(pgvector* vector, size_t index, void* element);

/* Deletion */
int vector_pop_back(pgvector* vector);
int vector_pop_front(pgvector* vector);
int vector_erase(pgvector* vector, size_t index);
int vector_clear(pgvector* vector);

/* Lookup */
void* vector_get(pgvector* vector, size_t index);
const void* vector_const_get(const pgvector* vector, size_t index);
void* vector_front(pgvector* vector);
void* vector_back(pgvector* vector);
#define VECTOR_GET_AS(type, vector_pointer, index) \
    *((type*)vector_get((vector_pointer), (index)))

/* Information */
bool vector_is_initialized(const pgvector* vector);
size_t vector_byte_size(const pgvector* vector);
size_t vector_free_space(const pgvector* vector);
bool vector_is_empty(const pgvector* vector);

/* Memory management */
int vector_resize(pgvector* vector, size_t new_size);
int vector_reserve(pgvector* vector, size_t minimum_capacity);
int vector_shrink_to_fit(pgvector* vector);

/* Iterators */
Iterator vector_begin(pgvector* vector);
Iterator vector_end(pgvector* vector);
Iterator vector_iterator(pgvector* vector, size_t index);

void* iterator_get(Iterator* iterator);
#define ITERATOR_GET_AS(type, iterator) *((type*)iterator_get((iterator)))

int iterator_erase(pgvector* vector, Iterator* iterator);

void iterator_increment(Iterator* iterator);
void iterator_decrement(Iterator* iterator);

void* iterator_next(Iterator* iterator);
void* iterator_previous(Iterator* iterator);

bool iterator_equals(Iterator* first, Iterator* second);
bool iterator_is_before(Iterator* first, Iterator* second);
bool iterator_is_after(Iterator* first, Iterator* second);

size_t iterator_index(pgvector* vector, Iterator* iterator);

#define VECTOR_FOR_EACH(vector_pointer, iterator_name)           \
    for (Iterator(iterator_name) = vector_begin((vector_pointer)), \
            end = vector_end((vector_pointer));                        \
             !iterator_equals(&(iterator_name), &end);                 \
             iterator_increment(&(iterator_name)))

/***** PRIVATE *****/

#define MAX(a, b) ((a) > (b) ? (a) : (b))

bool _vector_should_grow(pgvector* vector);
bool _vector_should_shrink(pgvector* vector);

size_t _vector_free_bytes(const pgvector* vector);
void* _vector_offset(pgvector* vector, size_t index);
const void* _vector_const_offset(const pgvector* vector, size_t index);

void _vector_assign(pgvector* vector, size_t index, void* element);

int _vector_move_right(pgvector* vector, size_t index);
void _vector_move_left(pgvector* vector, size_t index);

int _vector_adjust_capacity(pgvector* vector);
int _vector_reallocate(pgvector* vector, size_t new_capacity);

void _vector_swap(size_t* first, size_t* second);

#endif /* VECTOR_H */
