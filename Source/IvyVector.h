#ifndef IVY_VECTOR_H
#define IVY_VECTOR_H

#include "IvyDeclarations.h"
#include "IvyMemoryAllocator.h"

typedef uint32_t IvyU32;

#define IVY_VECTOR_TYPENAME(nameForVector) Ivy##nameForVector##Vector

#define IVY_DECLARE_VECTOR_FOR_TYPE(nameForVector, type)                      \
  typedef struct IVY_VECTOR_TYPENAME(nameForVector) {                         \
    type *items;                                                              \
    uint64_t itemCount;                                                       \
    uint64_t itemCapacity;                                                    \
  } IVY_VECTOR_TYPENAME(nameForVector)

#define IVY_DECLARE_VECTOR_CREATE_FUNCTION(nameForVector, type)               \
  IvyCode ivyCreate##nameForVector##Vector(IvyAnyMemoryAllocator allocator,   \
      uint64_t itemCount, IVY_VECTOR_TYPENAME(nameForVector) * vector)

#define IVY_DECLARE_VECTOR_DESTROY_FUNCTION(nameForVector, type)              \
  IvyCode ivyDestroy##nameForVector##Vector(IvyAnyMemoryAllocator allocator,  \
      IVY_VECTOR_TYPENAME(nameForVector) * vector)

#define IVY_DECLARE_VECTOR_APPEND_FUNCTION(nameForVector, type)               \
  IvyCode ivyAppendTo##nameForVectortype##Vector(                             \
      IvyAnyMemoryAllocator allocator,                                        \
      IVY_VECTOR_TYPENAME(nameForVector) * vector, type const *value)

#define IVY_DECLARE_VECTOR(nameForVector, type)                               \
  IVY_DECLARE_VECTOR_FOR_TYPE(nameForVector, type);                           \
  IVY_DECLARE_VECTOR_CREATE_FUNCTION(nameForVector, type);                    \
  IVY_DECLARE_VECTOR_DESTROY_FUNCTION(nameForVector, type);                   \
  IVY_DECLARE_VECTOR_APPEND_FUNCTION(nameForVector, type);

#define IVY_IMPLEMENT_VECTOR_CREATE_FUNCTION(nameForVector, type)             \
  IVY_DECLARE_VECTOR_CREATE_FUNCTION(nameForVector, type) {                   \
    vector->itemCount = itemCount;                                            \
    vector->itemCapacity = itemCount * 2;                                     \
    vector->items = ivyAllocateMemory(allocator,                              \
        vector->itemCapacity * sizeof(*vector->items));                       \
    if (!vector->items)                                                       \
      return IVY_NO_MEMORY;                                                   \
    return IVY_OK;                                                            \
  }

#define IVY_IMPLEMENT_VECTOR(nameForVector, type)                             \
  IVY_IMPLEMENT_VECTOR_CREATE_FUNCTION(nameForVector, type)

IVY_DECLARE_VECTOR(U32, uint32_t)

IVY_IMPLEMENT_VECTOR(U32, uint32_t)

#endif
