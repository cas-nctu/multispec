//                               MultiSpec
//
//               Laboratory for Applications of Remote Sensing
//                         Purdue University
//                        West Lafayette, IN 47907
//                         Copyright (2009-2018)
//                     (c) Purdue Research Foundation
//                           All rights reserved.
//
//   File:                  LClassifyCorrelationDialog.cpp : class implementation file
//   Class Definition:      LClassifyCorrelationDialog.h
//
//   Authors:               Larry L. Biehl
//
//   Revision date:         11/16/2018
//
//   Language:            C++
//
//   System:               Linux Operating System
//
//   Brief description:   This file contains functions that relate to the
//                        CMCorrelationClassifyDialog class.
//
//------------------------------------------------------------------------------------

#include "SMultiSpec.h"

#include "LMultiSpec.h"
#include "LClassifySVMDialog.h"

BEGIN_EVENT_TABLE (CMSVMClassifyDialog, CMDialog)
EVT_INIT_DIALOG (CMSVMClassifyDialog::OnInitDialog)
EVT_COMBOBOX (IDC_CovarianceCombo, CMSVMClassifyDialog::OnSelendokCovarianceCombo)
END_EVENT_TABLE ()



CMSVMClassifyDialog::CMSVMClassifyDialog (
                                                          wxWindow*                     pParent,
                                                          wxWindowID                     id,
                                                          const wxString&               title)
: CMDialog (id, pParent, title)

{
   m_gamma = 0.001;
   m_cost = 10;
   m_svm_type = 0;
   m_kernel_type = 2;
   m_initializedFlag = CMDialog::m_initializedFlag;
   
   CreateControls ();
   SetSizerAndFit (bSizer166);
   
}   // end "CMCorrelationClassifyDialog"



void CMSVMClassifyDialog::CreateControls ()

{
   this->SetSizeHints (wxDefaultSize, wxDefaultSize);
   
   //wxBoxSizer* bSizer166;
   bSizer166 = new wxBoxSizer (wxVERTICAL);
   wxBoxSizer* bSizer167;
   bSizer167 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText183 = new wxStaticText (this,
                                       wxID_ANY,
                                       wxT("Gamma Value:"),
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       0);
   m_staticText183->Wrap (-1);
   bSizer167->Add (m_staticText183, 0, wxALIGN_CENTER|wxALL, 5);

   m_text27 = new wxTextCtrl( this,
                              IDC_SVM_GAMMA,
                              wxEmptyString,
                              wxDefaultPosition,
                              wxSize (180,-1),
                              0 );
   
   bSizer167->Add (m_text27, 0, wxALIGN_CENTER|wxALL, 5);
   
   m_staticText184 = new wxStaticText (this,
                                       wxID_ANY,
                                       wxT("(default: 1/num_features)"),
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       0);
   m_staticText184->Wrap (-1);
   bSizer167->Add (m_staticText184, 0, wxALIGN_CENTER|wxALL, 5);

   bSizer166->Add (bSizer167, 1, wxALL|wxEXPAND, 12);
    
    wxBoxSizer* bSizer170;
    bSizer170 = new wxBoxSizer (wxHORIZONTAL);
    
    m_staticText187 = new wxStaticText (this,
                                        wxID_ANY,
                                        wxT("Cost Value:"),
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        0);
   m_staticText183->Wrap (-1);
   bSizer170->Add (m_staticText187, 0, wxALIGN_CENTER|wxALL, 5);
    
   m_text28 = new wxTextCtrl( this,
                              IDC_SVM_COST,
                              wxEmptyString,
                              wxDefaultPosition,
                              wxSize (180,-1),
                              0 );
    
   bSizer170->Add (m_text28, 0, wxALIGN_CENTER|wxALL, 5);
   bSizer166->Add (bSizer170, 1, wxALL|wxEXPAND, 12);
  
   wxBoxSizer* bSizer168;
   bSizer168 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText185 = new wxStaticText (this,
                                        wxID_ANY,
                                        wxT("SVM Types:"),
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        0);
   m_staticText185->Wrap (-1);
   bSizer168->Add (m_staticText185, 0, wxALIGN_CENTER|wxALL, 5);
   m_comboBox6 = new wxComboBox (this, IDC_SVM_TYPE, wxT("C-SVC"), wxDefaultPosition, wxSize(-1,28), 0, NULL, 0);
   m_comboBox6->Append(wxT("C-SVC"));
   m_comboBox6->Append(wxT("nu-SVC"));
   m_comboBox6->Append(wxT("one-class SVM"));
   m_comboBox6->Append(wxT("epsilon-SVR"));
   m_comboBox6->Append(wxT("nu-SVR"));

   bSizer168->Add (m_comboBox6, 0, wxALIGN_CENTER|wxALL, 5);
   bSizer166->Add (bSizer168, 1, wxALL|wxEXPAND, 12);
   wxBoxSizer* bSizer169;
   bSizer169 = new wxBoxSizer (wxHORIZONTAL);
   m_staticText186 = new wxStaticText (this,
                                        wxID_ANY,
                                        wxT("Kernel Type:"),
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        0);
   m_staticText186->Wrap (-1);
   bSizer169->Add (m_staticText186, 0, wxALIGN_CENTER|wxALL, 5);
    
   m_comboBox7 = new wxComboBox (this, IDC_SVM_KERNEL_TYPE, wxT("Radial Basis Function"), wxDefaultPosition, wxSize(-1,28), 0, NULL, 0);
   m_comboBox7->Append(wxT("Linear"));
   m_comboBox7->Append(wxT("Polynomial"));
   m_comboBox7->Append(wxT("Radial Basis Function"));
   m_comboBox7->Append(wxT("Sigmoid"));
    
   bSizer169->Add (m_comboBox7, 0, wxALIGN_CENTER|wxALL, 5);

   bSizer166->Add (bSizer169, 1, wxALL|wxEXPAND, 12);
   CreateStandardButtons (bSizer166);
   
   this->SetSizer (bSizer166);
   this->Layout ();
   
   this->Centre (wxBOTH);
   
}   // end "CreateControls"



//------------------------------------------------------------------------------------
//                         Copyright (2012-2018)
//                     (c) Purdue Research Foundation
//                           All rights reserved.
//
//   Function name:      void DoDialog
//
//   Software purpose:   The purpose of this routine is to present the Correlation
//                     specification dialog box to the user and copy the
//                     revised back to the classify specification structure if
//                     the user selected OK.
//                     This code was adapted from the Windows version of the same class.
//
//   Parameters in:      None
//
//   Parameters out:   None
//
//   Value Returned:   None
//
//   Called By:
//
//   Coded By:         Larry L. Biehl         Date: 04/09/1998
//   Revised By:         Larry L. Biehl         Date: 04/09/1998

Boolean CMSVMClassifyDialog::DoDialog ()

{
   SInt16                        returnCode;
   
   bool                           continueFlag = false;
   
   
   // Make sure intialization has been completed.
   
   if (!m_initializedFlag)
      return (false);
   
   //m_covarianceEstimate = *covarianceEstimatePtr - 1;
   
   returnCode = ShowModal ();
   
   if (returnCode == wxID_OK)
   {
      continueFlag = true;
      
      //*covarianceEstimatePtr = m_covarianceEstimate + 1;
      gProjectInfoPtr->svm_gamma = m_gamma;
      gProjectInfoPtr->svm_cost = m_cost;
      gProjectInfoPtr->svm_type = m_svm_type;
      gProjectInfoPtr->svm_kernel_type = m_kernel_type;
   }   // end "if (returnCode == IDOK)"
   
   return (continueFlag);
   
}   // end "DoDialog"



void CMSVMClassifyDialog::OnInitDialog (
                                                wxInitDialogEvent&            event)

{
   CMDialog::OnInitDialog (event);
   
}   // end "OnInitDialog"



void CMSVMClassifyDialog::OnSelendokCovarianceCombo (
                                                             wxCommandEvent&               event)

{
   // Add your control notification handler code here
   
}   // end "OnSelendokCovarianceCombo"



bool CMSVMClassifyDialog::TransferDataFromWindow ()

{
   //wxComboBox* covEst = (wxComboBox*)FindWindow (IDC_CovarianceCombo);
   //m_covarianceEstimate = covEst->GetSelection ();
   wxTextCtrl* convEst = (wxTextCtrl*)FindWindow (IDC_SVM_GAMMA);
   wxTextCtrl* costEst = (wxTextCtrl*)FindWindow (IDC_SVM_COST);
   wxComboBox* svmType = (wxComboBox*)FindWindow(IDC_SVM_TYPE);
   wxComboBox* kernelType = (wxComboBox*)FindWindow(IDC_SVM_KERNEL_TYPE);
   m_gamma = wxAtof(convEst->GetValue());
   m_cost = wxAtof(costEst->GetValue());
   m_svm_type = svmType->GetSelection();
   m_kernel_type = kernelType->GetSelection();
   return true;
   
}   // end "TransferDataFromWindow"



bool CMSVMClassifyDialog::TransferDataToWindow ()

{
   //wxComboBox* covEst = (wxComboBox*)FindWindow (IDC_CovarianceCombo);
   //covEst->SetSelection (m_covarianceEstimate);
   wxTextCtrl* convEst = (wxTextCtrl*)FindWindow (IDC_SVM_GAMMA);
   wxTextCtrl* costEst = (wxTextCtrl*)FindWindow (IDC_SVM_COST);
   wxComboBox* svmType = (wxComboBox*)FindWindow(IDC_SVM_TYPE);
   wxComboBox* kernelType = (wxComboBox*)FindWindow(IDC_SVM_KERNEL_TYPE);
   convEst->SetValue(wxString::Format(wxT("%f"), m_gamma));
   costEst->SetValue(wxString::Format(wxT("%f"), m_cost));
   svmType->SetSelection(m_svm_type);
   kernelType->SetSelection(m_kernel_type);
   return true;
   
}   // end "TransferDataToWindow"

