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

#if !defined __LSVMLG_H__
#define   __LSVMLG_H__

#include "LDialog.h"

// CMCorrelationClassifyDialog dialog

class CMSVMClassifyDialog : public CMDialog
{
public:
   CMSVMClassifyDialog (
                                wxWindow*                        parent = NULL,
                                wxWindowID                        id = IDD_SVMParameters,
                                const wxString&                  title = wxT("Specify SVM Classifier Parameters"));
   
   Boolean DoDialog ();
   
   float m_gamma;
   float m_cost;
   int m_svm_type;
   int m_kernel_type;
   
   
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
   wxStaticText*                   m_staticText185;
   wxStaticText*                   m_staticText186;
   wxStaticText*                   m_staticText187;
   wxComboBox*                     m_comboBox27;
   wxButton*                       m_button37;
   wxButton*                       m_button38;
   wxTextCtrl*                     m_text27;
   wxTextCtrl*                     m_text28;
    
   wxComboBox*                     m_comboBox6;
   wxComboBox*                     m_comboBox7;
   
};   // end "CMSVMClassifyDialog dialog"

#endif   // !defined __LCLSFDLG_H__

