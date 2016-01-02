/* $Id: sdls_parser_internal.h,v 1.1 2005/12/09 22:01:47 marsanyi Exp $ */
#ifndef _SDLS_INTERNAL_H
#define _SDLS_INTERNAL_H
/**
 
   sdls_parser_internal.h
   Internal SDLS parser info.  SDLS is a custom streamed DLS file format.

   Author: rnm
   (C) Robert Marsanyi, Phil Burk, Mobileer, PROPRIETARY and CONFIDENTIAL
*/

#define SDLS_MAJOR_VERSION 0
#define SDLS_MINOR_VERSION 1
#define FOURCC_MXMF MakeFourCC('M','X','M','F')
#define FOURCC_WAVG MakeFourCC('W','A','V','G')
#define FOURCC_INSG MakeFourCC('I','N','S','G')
#define FOURCC_INST MakeFourCC('I','N','S','T')
#define FOURCC_REGN MakeFourCC('R','E','G','N')
#define FOURCC_ARTG MakeFourCC('A','R','T','G')
#define FOURCC_ARTD MakeFourCC('A','R','T','D')
#define FOURCC_ARTI MakeFourCC('A','R','T','I')
#define FOURCC_ARTR MakeFourCC('A','R','T','R')
#define FOURCC_EOMX MakeFourCC('E','O','M','X')

#endif /* _SDLS_INTERNAL_H */
