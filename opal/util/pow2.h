/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */

#ifndef OPAL_POW2_H
#define OPAL_POW2_H
	
/**
 *  This routine takes in an interger, and returns the nearest
 *  power of 2 integer, same or greater than the input.
 *
 *  @param input_integer input value
 *
 *  @returnvalue power of 2 integer same or greater than the input
 *               parameter.
 */
OMPI_DECLSPEC int opal_round_up_to_nearest_pow2(int input_integer);

#endif /* OPAL_POW2_H */
