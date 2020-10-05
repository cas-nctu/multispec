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
#include "LClassifyKNNDialog.h"

BEGIN_EVENT_TABLE (CMKNNClassifyDialog, CMDialog)
EVT_INIT_DIALOG (CMKNNClassifyDialog::OnInitDialog)
EVT_COMBOBOX (IDC_CovarianceCombo, CMKNNClassifyDialog::OnSelendokCovarianceCombo)
END_EVENT_TABLE ()



CMKNNClassifyDialog::CMKNNClassifyDialog (
                                          wxWindow*                     pParent,
                                          wxWindowID                     id,
                                          const wxString&               title)
: CMDialog (id, pParent, title)

{
   m_topK = 5;
   m_initializedFlag = CMDialog::m_initializedFlag;
   
   CreateControls ();
   SetSizerAndFit (bSizer166);
   
}   // end "CMCorrelationClassifyDialog"



void CMKNNClassifyDialog::CreateControls ()

{
   this->SetSizeHints (wxDefaultSize, wxDefaultSize);
   
   //wxBoxSizer* bSizer166;
   bSizer166 = new wxBoxSizer (wxVERTICAL);
   
   wxBoxSizer* bSizer167;
   bSizer167 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText183 = new wxStaticText (this,
                                       wxID_ANY,
                                       wxT("KNN topK value:"),
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       0);
   m_staticText183->Wrap (-1);
   bSizer167->Add (m_staticText183, 0, wxALIGN_CENTER|wxALL, 5);
   
   m_text27 = new wxTextCtrl( this,
                             IDC_CovariancePrecision,
                             wxEmptyString,
                             wxDefaultPosition,
                             wxSize (180,-1),
                             0 );
   
   bSizer167->Add (m_text27, 0, wxALIGN_CENTER|wxALL, 5);
   
   m_staticText184 = new wxStaticText (this,
                                       wxID_ANY,
                                       wxT("e.g. (3-10)"),
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       0);
   m_staticText184->Wrap (-1);
   bSizer167->Add (m_staticText184, 0, wxALIGN_CENTER|wxALL, 5);
   
   bSizer166->Add (bSizer167, 1, wxALL|wxEXPAND, 12);
   
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

Boolean CMKNNClassifyDialog::DoDialog ()

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
      gProjectInfoPtr->topK = m_topK;
   }   // end "if (returnCode == IDOK)"
   
   return (continueFlag);
   
}   // end "DoDialog"



void CMKNNClassifyDialog::OnInitDialog (
                                        wxInitDialogEvent&            event)

{
   CMDialog::OnInitDialog (event);
   
}   // end "OnInitDialog"



void CMKNNClassifyDialog::OnSelendokCovarianceCombo (
                                                     wxCommandEvent&               event)

{
   // Add your control notification handler code here
   
}   // end "OnSelendokCovarianceCombo"



bool CMKNNClassifyDialog::TransferDataFromWindow ()

{
   //wxComboBox* covEst = (wxComboBox*)FindWindow (IDC_CovarianceCombo);
   //m_covarianceEstimate = covEst->GetSelection ();
   wxTextCtrl* convEst = (wxTextCtrl*)FindWindow (IDC_CovariancePrecision);
   m_topK = wxAtof(convEst->GetValue());
   return true;
   
}   // end "TransferDataFromWindow"



bool CMKNNClassifyDialog::TransferDataToWindow ()

{
   //wxComboBox* covEst = (wxComboBox*)FindWindow (IDC_CovarianceCombo);
   //covEst->SetSelection (m_covarianceEstimate);
   wxTextCtrl* convEst = (wxTextCtrl*)FindWindow (IDC_CovariancePrecision);
   convEst->SetValue(wxString::Format(wxT("%f"), m_topK));
   return true;
   
}   // end "TransferDataToWindow"


