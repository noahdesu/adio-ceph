/*
 * $HEADER$
 */

#ifndef OMPI_BIT_OPS_H
#define OMPI_BIT_OPS_H

/**
 * Calculates the highest bit in an integer
 *
 * @param value The integer value to examine
 * @param start Position to start looking
 *
 * @returns pos Position of highest-set integer or -1 if none are set.
 *
 * Look at the integer "value" starting at position "start", and move
 * to the right.  Return the index of the highest bit that is set to
 * 1.
 *
 * WARNING: *NO* error checking is performed.  This is meant to be a
 * fast inline function.
 */
static inline int ompi_hibit(int value, int start)
{
  unsigned int mask;

  --start;
  mask = 1 << start;

  for (; start >= 0; --start, mask >>= 1) {
    if (value & mask) {
      break;
    }
  }
  
  return start;
}


/**
 * Returns the cube dimension of a given value.
 *
 * @param value The integer value to examine
 *
 * @returns cubedim The smallest cube dimension containing that value
 *
 * Look at the integer "value" and calculate the smallest power of two
 * dimension that contains that value.
 *
 * WARNING: *NO* error checking is performed.  This is meant to be a
 * fast inline function.
 */
static inline int ompi_cube_dim(int value) 
{
    int dim, size;

    for (dim = 0, size = 1; size < value; ++dim, size <<= 1) {
        continue;
    }

    return dim;
}

#endif /* OMPI_BIT_OPS_H */
