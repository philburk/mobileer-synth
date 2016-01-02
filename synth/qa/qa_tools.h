/* $Id: qa_tools.h,v 1.2 2006/05/23 01:36:27 philjmsl Exp $ */
#ifndef  _QA_TOOLS_H
#define  _QA_TOOLS_H
/**
 * @file qa_tools.h
 * @brief <b>Quality Assurance Tools</b>
 * @author Phil Burk, Copyright 1996 Phil Burk and Mobileer, PROPRIETARY and CONFIDENTIAL
 */

/**
 * Initialize QA system.
 * @param testName name of the test that will be printed in PASS/FAIL report
 */
void QA_Init( const char *testName );

/**
 * Increment internal success counter.
 */
void QA_CountSuccess( void );

/**
 * Increment internal success counter.
 */
void QA_CountError( void );

/**
 * Assert that a statement is true. Increment success of error flag as appropriate.
 * Report SUCCESS or ERROR.
 */
void QA_Assert( int ok, const char *message );

/**
 * Determine return code for program: 0 if OK,  1 if not the right number of successes,
 * or 2 if error detected.
 * It is important to pass the expected number of success in case part of the test got skipped
 * because of a silent error.
 * @param expectedSuccesses the expected value for the internal success counter
 */
int  QA_Term( int expectedSuccesses );

#endif
