//                               MultiSpec
//
//               Laboratory for Applications of Remote Sensing
//                           Purdue University
//                        West Lafayette, IN 47907
//                         Copyright (2009-2018)
//                     (c) Purdue Research Foundation
//                           All rights reserved.
//
//   File:                  LClassifyCorrelationDialog.h
//   Implementation:      LClassifyCorrelationDialog.cpp
//
//   Authors:               Larry L. Biehl
//
//   Language:            C++
//
//   System:               Linux Operating System
//
//   Brief description:   Header file for the CMSVMClassifyDialog class
//
//   Written By:            Wei-Kang Hsu         Date: 04/09/2015
//   Revised By:            Larry L. Biehl         Date: 01/24/2018
//
//------------------------------------------------------------------------------------

#if !defined __LKNNLG_H__
#define   __LKNNLG_H__

#include "LDialog.h"

// CMCorrelationClassifyDialog dialog

class CMKNNClassifyDialog : public CMDialog
{
public:
   CMKNNClassifyDialog (
                        wxWindow*                        parent = NULL,
                        wxWindowID                        id = IDD_KNNParameters,
                        const wxString&                  title = wxT("Specify KNN Classifier Parameters"));
   
   Boolean DoDialog ();
   
   float m_topK;
   
   
   // Implementation
protected:
   void CreateControls ();
   void OnInitDialog (
                      wxInitDialogEvent&               event);
   void OnSelendokCovarianceCombo (
                                   wxCommandEvent&                  event);
   bool TransferDataFromWindow ();
   bool TransferDataToWindow ();
   
   DECLARE_EVENT_TABLE()
   
   bool                              m_initializedFlag;
   
   // Linux GUI controls
   
   wxBoxSizer*                     bSizer166;
   wxStaticText*                   m_staticText183;
   wxStaticText*                   m_staticText184;
   wxComboBox*                     m_comboBox27;
   wxButton*                       m_button37;
   wxButton*                       m_button38;
   wxTextCtrl*                     m_text27;
   
};   // end "CMKNNClassifyDialog dialog"

#endif   // !defined __LCLSFDLG_H__


