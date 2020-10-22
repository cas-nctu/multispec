//                                     MultiSpec
//
//                   Copyright 1988-2020 Purdue Research Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use
// this file except in compliance with the License. You may obtain a copy of the
// License at:  https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
// language governing permissions and limitations under the License.
//
// MultiSpec is curated by the Laboratory for Applications of Remote Sensing at
// Purdue University in West Lafayette, IN and licensed by Larry Biehl.
//
//	File:						WEvaluateCovarianceDialog.h
//	Implementation:		WEvaluateCovarianceDialog.cpp
//
//	Authors:					Larry L. Biehl
//
//	Language:				C++
//
//	System:					Windows Operating System
//
//	Brief description:	Header file for the CMEvalCovarianceDialog class.
//
//	Written By:				Larry L. Biehl			Date: ??/??/1995?
//	Revised By:				Larry L. Biehl			Date: 12/05/2019
//
//------------------------------------------------------------------------------------

#pragma once
	     
#include "WDialog.h"


class CMEvalCovarianceDialog : public CMDialog
{
	// Construction
	public:
		CMEvalCovarianceDialog (	// standard constructor
				CWnd* 								pParent = NULL);
	
		~CMEvalCovarianceDialog ();	// standard desctructor
	
		Boolean DoDialog (
				EvaluateCovarianceSpecsPtr		evaluateCovarianceSpecsPtr);

	// Dialog Data
		//{{AFX_DATA (CMEvalCovarianceDialog)
		enum { IDD = IDD_EvalCovariances };
	
		BOOL									m_featureTransformationFlag,
												m_listInvertedMatrixFlag,
												m_listInvertedInvertedMatrixFlag,
												m_listOriginalMatrixFlag,
												m_listOriginalXInvertedMatrixFlag;
		//}}AFX_DATA

	// Implementation
	protected:
		virtual void DoDataExchange (
				CDataExchange* 					pDX);	// DDX/DDV support

		// Generated message map functions
		//{{AFX_MSG (CMEvalCovarianceDialog)
		virtual BOOL OnInitDialog ();
	
		afx_msg void OnSelendokChannelCombo ();
	
		afx_msg void OnSelendokClassCombo ();
	
		afx_msg void OnTransformation ();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP ()
	
	
		EvaluateCovarianceSpecsPtr		m_evaluateCovarianceSpecsPtr;
	
		UInt16								m_numberEigenvectors;
		
		Boolean		  						m_featureTransformAllowedFlag,
												m_initializedFlag;
	
};	// end class CMEvalCovarianceDialog
