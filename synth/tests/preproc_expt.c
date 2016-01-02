/* $Id$ */
/**
 *
 * Experiment with 'C' pre-processor on various compilers.
 * Copyright 2002 Mobileer, PROPRIETARY and CONFIDENTIAL
 *
 */
#include <stdio.h>

#define VAR1  (1)
#define VAR2  (0)

int main(void);
int main(void)
{

    printf("Test preprocessor.\n");
#if  (VAR1 || VAR2)

    printf("(VAR1 || VAR2)\n");
#endif

}
