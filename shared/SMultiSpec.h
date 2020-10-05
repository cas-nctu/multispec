//	 									MultiSpec
//
//					Laboratory for Applications of Remote Sensing
//									Purdue University
//								West Lafayette, IN 47907
//								 Copyright (1988-2018)
//							(c) Purdue Research Foundation
//									All rights reserved.
//
// File:						SMultiSpec.h
//
//	Authors:					Larry L. Biehl
//
//	Revision number:		3.0
//
//	Language:				C
//
//	System:					Linux, Macintosh, and Windows Operating Systems
//
//	Brief description:	Main header file for MultiSpec which contains calls to other 
//								include files for constants, resource constants, definitions, 
//								and function prototypes.
//
//	Written By:				Larry L. Biehl			Date: 03/29/1988
//	Revised By:				Abdur Maud				Date: 06/24/2013
//	Revised By:				Larry L. Biehl			Date: 10/19/2018
//
//------------------------------------------------------------------------------------

#ifndef __SMulSpec__
#define __SMulSpec__

#if defined __MWERKS__ 
	#pragma once
#endif	// defined __MWERKS__ 

//#if !defined __MWERKS__
//	#define _X86_ 
//#endif	// !defined __MWERKS__

#if defined multispec_mac || defined multispec_mac_swift
	#include "wchar.h"
#endif

#if defined multispec_win
	#include "stdafx.h"
	//#ifndef WINVER
	//	#define  WINVER  0x0501
	//#endif	// !WINVER

	//	#ifndef _WIN32_WINNT
	//#define _WIN32_WINNT	0x0501 // _WIN32_WINNT_NT4
	//	#endif	// !_WIN32_WINNT

	//	#ifndef _WIN32_WINDOWS
	//#define _WIN32_WINDOWS	0x0501 // _WIN32_WINNT_NT4
	//	#endif	// !_WIN32_WINDOWS

	//	#ifndef _WIN32_IE
	//#define _WIN32_IE		0x0501 // _WIN32_WINNT_NT4
	//	#endif	// !_WIN32_IE

	//#ifndef DOUBLE
	//	typedef double DOUBLE;
	//#endif // !DOUBLE_ 

	#define include_hdf_capability 0
	#define include_gdal_capability 0
	#define include_hdf5_capability 0

	//	#include "stdafx.h"
	#include <math.h>
	#include "float.h" 
	#include "limits.h" 
	#include "windowsx.h" 
	#include "resource.h"       // main symbols
	void AFXAPI DDX_Text2(CDataExchange* pDX, int nIDC, float& value);
	void AFXAPI DDX_Text2(CDataExchange* pDX, int nIDC, double& value);
#endif	// defined multispec_win 

#if defined multispec_lin
         // GDAL capability
	#ifdef NetBeansProject
		#define include_hdf_capability 0
		#define include_gdal_capability 0
		#define include_hdf5_capability 0
	#else	// ifndef NetBeansProject
		#ifdef multispec_wxmac
			#define include_hdf_capability 0
			#define include_gdal_capability 0
			#define include_hdf5_capability 0
		#else
			#define include_hdf_capability 0
			#define include_gdal_capability 0
			#define include_hdf5_capability 0
		#endif
	#endif
#endif	// end "defined multispec_lin"

#include "SConstants.h"
#include "SDefines.h" 
#include "SGraphic.h"
#include "SExternalGlobals.h"
#include "SResourceConstants.h"
#include "SPrototypes.h"  

#if defined multispec_lin
	//#include "SDeclareGlobals.h" // DO NOT include this. SExternalGlobals.h is sufficient
	#include "LResource.h"
	//#include "SGraphView.h"
	#include <time.h>
	#include <cmath>
	#include <cfloat>
	#include <float.h>
	#include <math.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <limits.h>
	#include <iostream>
	#include <string>
	#include "wx/gdicmn.h"
#endif	// defined multispec_lin

#endif // __SMulSpec__             
