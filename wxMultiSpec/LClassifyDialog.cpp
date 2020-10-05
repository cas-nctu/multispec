//                               MultiSpec
//
//               Laboratory for Applications of Remote Sensing
//                         Purdue University
//                        West Lafayette, IN 47907
//                         Copyright (2009-2018)
//                     (c) Purdue Research Foundation
//                           All rights reserved.
//
//   File:                  LClassifyDialog.cpp : class implementation file
//   Class Definition:      LClassifyDialog.h
//
//   Authors:               Larry L. Biehl
//
//   Revision date:         02/13/2019
//
//   Language:            C++
//
//   System:               Linux Operating System
//
//   Brief description:   This file contains functions that relate to the
//                        CMClassifyDialog class.
//
//------------------------------------------------------------------------------------

#include "SMultiSpec.h"

#include "LClassifyDialog.h"
#include "LImage_dialog.cpp"


CMClassifyDialog::CMClassifyDialog (
                                    wxWindow*                     parent,
                                    wxWindowID                     id,
                                    const wxString&               title)
: CMDialog (CMClassifyDialog::IDD, parent, title)

{
   m_trainingAreaFlag = FALSE;
   m_imageAreaFlag = FALSE;
   m_thresholdResultsFlag = FALSE;
   m_classWeightsSelection = -1;
   m_thresholdPercent = (float)0.;
   m_diskFileFlag = FALSE;
   m_createImageOverlayFlag = FALSE;
   m_classifyListSelection = -1;
   m_covarianceEstimate = 1;
   m_classAreaSelection = 0;
   m_createProbabilityFileFlag = FALSE;
   m_testAreaFlag = FALSE;
   m_fileNamesSelection = -1;
   m_outputFormatCode = 1;
   m_paletteSelection = 0;
   m_trainingAreaLOOFlag = FALSE;
   m_cemThreshold = (float)0.;
   m_angleThreshold = (float)0.;
   m_correlationThreshold = (float)0.;
   m_selectImageOverlaySelection = 1;
   
   m_selectImageOverlaySelection = 1;
   
   m_createImageOverlayFlag = FALSE;
   
   m_classWeightsPtr = NULL;
   m_classAreaListPtr = NULL;
   m_optionKeyFlag = FALSE;
   
   m_classifyProcedureEnteredCode = 0;
   
   m_initializedFlag = CMDialog::m_initializedFlag;
   
   if (m_initializedFlag)
      m_initializedFlag = GetDialogLocalVectors (&m_localFeaturesPtr,
                                                 &m_localTransformFeaturesPtr,
                                                 &m_classListPtr,
                                                 &m_classAreaListPtr,
                                                 &m_classWeightsPtr,
                                                 NULL,
                                                 NULL,
                                                 NULL);
   CreateControls ();
   //SetSizerAndFit (bSizer110);
   
}   // end "CMClassifyDialog"



CMClassifyDialog::~CMClassifyDialog (void)

{
   ReleaseDialogLocalVectors (m_localFeaturesPtr,
                              m_localTransformFeaturesPtr,
                              m_classListPtr,
                              m_classAreaListPtr,
                              m_classWeightsPtr,
                              NULL,
                              NULL,
                              NULL);
   
}   // end "~CMClassifyDialog"



BEGIN_EVENT_TABLE(CMClassifyDialog, CMDialog)
EVT_INIT_DIALOG(CMClassifyDialog::OnInitDialog)
EVT_COMBOBOX(IDC_ChannelCombo, CMClassifyDialog::OnSelendokChannelCombo)
EVT_COMBOBOX_DROPDOWN(IDC_ChannelCombo, CMClassifyDialog::OnSelendokChannelComboDropDown)
EVT_COMBOBOX(IDC_ClassificationProcedure, CMClassifyDialog::OnSelendokClassificationProcedure)
EVT_COMBOBOX(IDC_DiskCombo, CMClassifyDialog::OnSelendokDiskCombo)
EVT_COMBOBOX(IDC_AreasCombo, CMClassifyDialog::OnSelendokAreasCombo)
EVT_COMBOBOX_DROPDOWN(IDC_AreasCombo, CMClassifyDialog::OnSelendokAreasComboDropDown)
EVT_COMBOBOX(IDC_PaletteCombo, CMClassifyDialog::OnSelendokPaletteCombo)
//EVT_COMBOBOX_DROPDOWN(IDC_PaletteCombo, CMClassifyDialog::OnDropdownPaletteCombo)
EVT_COMBOBOX_DROPDOWN(IDC_ClassificationProcedure, CMClassifyDialog::OnDropdownClassificationProcedure)
EVT_COMBOBOX(IDC_ClassCombo, CMClassifyDialog::OnSelendokClassCombo)
EVT_COMBOBOX_DROPDOWN(IDC_ClassCombo, CMClassifyDialog::OnSelendokClassComboDropDown)
EVT_COMBOBOX(IDC_TargetCombo, CMClassifyDialog::OnSelendokTargetCombo)
EVT_COMBOBOX(IDC_WeightCombo, CMClassifyDialog::OnSelendokClassWeightsCombo)
EVT_COMBOBOX_DROPDOWN(IDC_WeightCombo, CMClassifyDialog::OnSelendokClassWeightsComboDropDown)
EVT_COMBOBOX(IDC_ImageOverlayCombo, CMClassifyDialog::OnSelendokImageOverlayCombo)
//EVT_COMBOBOX_CLOSEUP(IDC_ClassificationProcedure, CMClassifyDialog::OnCloseupClassificationProcedure)
EVT_CHECKBOX(IDC_DiskFile, CMClassifyDialog::OnDiskFile)
EVT_CHECKBOX(IDC_ThresholdResults, CMClassifyDialog::OnThresholdResults)
EVT_CHECKBOX(IDC_TextWindow, CMClassifyDialog::OnTextWindow)
EVT_CHECKBOX(IDC_Training, CMClassifyDialog::OnTraining)
EVT_CHECKBOX(IDC_TestAreas, CMClassifyDialog::OnTestAreas)
EVT_CHECKBOX(IDC_ImageArea, CMClassifyDialog::OnImageArea)
EVT_CHECKBOX(IDC_FeatureTransformation, CMClassifyDialog::OnFeatureTransformation)
EVT_CHECKBOX(IDC_CreateProbabilityFile, CMClassifyDialog::OnCreateProbabilityFile)
EVT_BUTTON(IDC_ListOptions, CMClassifyDialog::OnListOptions)
EVT_CHECKBOX(IDC_TrainingLOO, CMClassifyDialog::OnTrainingLOO)
EVT_CHECKBOX(IDC_ImageWindowOverlay, CMClassifyDialog::OnImageOverlay)
EVT_TEXT(IDC_CorrelationCoefficient, CMClassifyDialog::OnChangeCorrelationCoefficient)
EVT_TEXT(IDC_CorrelationThresold, CMClassifyDialog::OnChangeCorrelationThresold)
EVT_TEXT(IDC_ColumnEnd, CMClassifyDialog::CheckColumnEnd)
EVT_TEXT(IDC_ColumnStart, CMClassifyDialog::CheckColumnStart)
EVT_TEXT(IDC_LineEnd, CMClassifyDialog::CheckLineEnd)
EVT_TEXT(IDC_LineStart, CMClassifyDialog::CheckLineStart)
EVT_TEXT(IDC_LineInterval, CMClassifyDialog::CheckLineInterval)
EVT_TEXT(IDC_ColumnInterval, CMClassifyDialog::CheckColumnInterval)
EVT_BUTTON(IDEntireImage, CMClassifyDialog::ToEntireImage)
EVT_BUTTON(IDSelectedImage, CMClassifyDialog::ToSelectedImage)
//EVT_CHAR_HOOK(CMClassifyDialog::OnButtonPress)
END_EVENT_TABLE()



void CMClassifyDialog::CreateControls ()

{
   /*
    wxBitmap entireimi = wxBITMAP_PNG_FROM_DATA(entireim);
    wxBitmap toentirei = wxBITMAP_PNG_FROM_DATA(toentire);
    wxBitmap selectedi = wxBITMAP_PNG_FROM_DATA(selected);
    wxBitmap bmp4i = wxBITMAP_PNG_FROM_DATA(bmp4);
    */
   this->SetSizeHints (wxDefaultSize, wxDefaultSize);
   
   wxBoxSizer* bVSizerMain = new wxBoxSizer (wxVERTICAL);
   
   bFlexGrid110 = new wxFlexGridSizer (0, 2, 0, 0);
   bFlexGrid110->SetFlexibleDirection (wxBOTH);
   
   wxBoxSizer* bSizer111;
   bSizer111 = new wxBoxSizer (wxVERTICAL);
   
   m_staticText145 = new wxStaticText (this,
                                       wxID_ANY,
                                       wxT("Procedure:"),
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       0);
   m_staticText145->Wrap (-1);
   SetUpToolTip (m_staticText145, IDS_ToolTip175);
   bSizer111->Add (m_staticText145, 0, wxALL, 5);
   
   m_comboBox15 = new wxComboBox (this,
                                  IDC_ClassificationProcedure,
                                  wxT("Quadratic Likelihood"),
                                  wxDefaultPosition,
                                  wxSize (250, -1),
                                  0,
                                  NULL,
                                  wxCB_READONLY);
   m_comboBox15->Append (wxT("Maximum Likelihood"));
   m_comboBox15->Append (wxT("Mahalanobis"));
   m_comboBox15->Append (wxT("Fisher Linear Likelihood"));
   m_comboBox15->Append (wxT("Minimum Euclidean Distance"));
   m_comboBox15->Append (wxT("ECHO Spectral-spatial..."));
   m_comboBox15->Append (wxT("Correlation (SAM)"));
   m_comboBox15->Append (wxT("Matched Filter (CEM)..."));
   m_comboBox15->Append (wxT("Parallel Piped"));
   m_comboBox15->Append (wxT("Support Vector Machine (SVM)..."));
   m_comboBox15->Append (wxT("k Nearest Neighbor(KNN)..."));
   SetUpToolTip (m_comboBox15, IDS_ToolTip175);
   bSizer111->Add (m_comboBox15, 0, wxALL, 5);
   
   m_checkBox8 = new wxCheckBox (this,
                                 IDC_FeatureTransformation,
                                 wxT("Use feature transformation"),
                                 wxDefaultPosition,
                                 wxDefaultSize,
                                 0);
   SetUpToolTip (m_checkBox8, IDS_ToolTip150);
   bSizer111->Add (m_checkBox8, 0, wxALL, 5);
   
   wxBoxSizer* bSizer113;
   bSizer113 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText146 = new wxStaticText (this,
                                       IDC_ChannelPrompt,
                                       wxT("Channels:"),
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       0);
   m_staticText146->Wrap (-1);
   SetUpToolTip (m_staticText146, IDS_ToolTip52);
   bSizer113->Add (m_staticText146, 0, wxALIGN_CENTER | wxALL, 5);
   
   m_comboBox16 = new wxComboBox (this,
                                  IDC_ChannelCombo,
                                  wxT("All available"),
                                  wxDefaultPosition,
                                  wxDefaultSize,
                                  0,
                                  NULL,
                                  wxCB_READONLY);
   m_comboBox16->Append (wxT("All available"));
   m_comboBox16->Append (wxT("Subset..."));
   SetUpToolTip (m_comboBox16, IDS_ToolTip52);
   bSizer113->Add (m_comboBox16, 0, wxRESERVE_SPACE_EVEN_IF_HIDDEN|wxALL, 5);
   
   
   bSizer111->Add (bSizer113, 0, 0, 5);
   
   
   bSizer114 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText147 = new wxStaticText (this,
                                       wxID_ANY,
                                       wxT("Target:"),
                                       wxDefaultPosition,
                                       wxDefaultSize,
                                       0);
   m_staticText147->Wrap (-1);
   SetUpToolTip (m_staticText147, IDS_ToolTip177);
   bSizer114->Add (m_staticText147, 0, wxALIGN_CENTER | wxALL, 5);
   
   m_staticText175 = new wxStaticText (this, IDC_TargetBase, wxT("Base Image"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText175->Wrap (-1);
   SetUpToolTip (m_staticText175, IDS_ToolTip177);
   bSizer114->Add (m_staticText175, 0, wxALIGN_CENTER | wxALL, 5);
   
   m_comboBox17 = new wxComboBox (this, IDC_TargetCombo, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
   SetUpToolTip (m_comboBox17, IDS_ToolTip177);
   bSizer114->Add (m_comboBox17, 0, wxALL, 5);
   
   
   bSizer111->Add (bSizer114, 0, 0, 5);
   
   wxBoxSizer* bSizer115;
   bSizer115 = new wxBoxSizer (wxVERTICAL);
   
   wxBoxSizer* bSizer116;
   bSizer116 = new wxBoxSizer (wxVERTICAL);
   
   m_staticText148 = new wxStaticText (this, wxID_ANY, wxT("Classify:"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText148->Wrap (-1);
   bSizer116->Add (m_staticText148, 0, wxALL, 5);
   
   wxBoxSizer* bSizer132;
   bSizer132 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText149 = new wxStaticText (this, wxID_ANY, wxT("Class areas:"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText149->Wrap (-1);
   SetUpToolTip (m_staticText149, IDS_ToolTip188);
   bSizer132->Add (m_staticText149, 0, wxALIGN_CENTER | wxLEFT, 15);
   
   m_comboBox24 = new wxComboBox (this, IDC_AreasCombo, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
   m_comboBox24->Append (wxT("All"));
   m_comboBox24->Append (wxT("Subset..."));
   SetUpToolTip (m_comboBox24, IDS_ToolTip103);
   bSizer132->Add (m_comboBox24, 0, wxLEFT, 5);
   
   
   bSizer116->Add (bSizer132, 1, 0, 5);
   
   m_checkBox9 = new wxCheckBox (this, IDC_Training, wxT("Training (resubstitution)"), wxDefaultPosition, wxDefaultSize, 0);
   SetUpToolTip (m_checkBox9, IDS_ToolTip189);
   bSizer116->Add (m_checkBox9, 0, wxLEFT, 25);
   
   m_checkBox11 = new wxCheckBox (this, IDC_TrainingLOO, wxT("Training (leave-one-out)"), wxDefaultPosition, wxDefaultSize, 0);
   SetUpToolTip (m_checkBox11, IDS_ToolTip193);
   bSizer116->Add (m_checkBox11, 0, wxLEFT, 25);
   
   m_checkBox10 = new wxCheckBox (this, IDC_TestAreas, wxT("Test areas (holdout)"), wxDefaultPosition, wxDefaultSize, 0);
   SetUpToolTip (m_checkBox10, IDS_ToolTip168);
   bSizer116->Add (m_checkBox10, 0, wxLEFT, 25);
   
   
   bSizer115->Add (bSizer116, 0, wxEXPAND, 5);
   
   wxBoxSizer* bSizer117;
   bSizer117 = new wxBoxSizer (wxVERTICAL);
   
   m_checkBox12 = new wxCheckBox (this, IDC_ImageArea, wxT("Image selection"), wxDefaultPosition, wxDefaultSize, 0);
   bSizer117->Add (m_checkBox12, 0, wxLEFT, 15);
   
   wxStaticBoxSizer* sbSizer8;
   //wxStaticBox* areaStaticBox = new wxStaticBox (this, IDC_LineColFrame, wxT("Area to Classify"));
   sbSizer8 = new wxStaticBoxSizer (new wxStaticBox (this,
                                                     IDC_LineColFrame,
                                                     wxT("Area to Classify"),
                                                     wxDefaultPosition,
                                                     wxDefaultSize,
                                                     wxTAB_TRAVERSAL),
                                    wxHORIZONTAL);
   
   CreateLineColumnControls (sbSizer8);
   /*
    wxBoxSizer* bSizer791;
    bSizer791 = new wxBoxSizer (wxHORIZONTAL);
    
    m_bpButton4 = new wxBitmapButton (areaStaticBox, IDEntireImage, entireimi, wxDefaultPosition, wxSize (40,40), wxBU_AUTODRAW);
    
    m_bpButton4->SetBitmapDisabled (toentirei);
    bSizer791->Add (m_bpButton4, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);
    
    m_bpButton5 = new wxBitmapButton (areaStaticBox, IDSelectedImage, selectedi, wxDefaultPosition, wxSize (40,40), wxBU_AUTODRAW);
    m_bpButton5->Hide ();
    m_bpButton5->SetBitmapDisabled (selectedi);
    bSizer791->Add (m_bpButton5, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT | wxALL, 5);
    
    
    sbSizer8->Add (bSizer791, 0, wxEXPAND, 5);
    
    wxBoxSizer* bSizer121;
    bSizer121 = new wxBoxSizer (wxVERTICAL);
    
    m_staticText177 = new wxStaticText (sbSizer8->GetStaticBox (), IDC_StartEndInterval, wxT("                      Start       \tEnd          Interval"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText177->Wrap (-1);
    bSizer121->Add (m_staticText177, 0, wxALL, 5);
    
    wxGridSizer* gSizer1;
    gSizer1 = new wxGridSizer (2, 4, 0, 0);
    
    m_staticText60 = new wxStaticText (sbSizer8->GetStaticBox (), IDC_LinePrompt, wxT("Lines"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText60->Wrap (-1);
    gSizer1->Add (m_staticText60, 0, wxALIGN_CENTER | wxALL, 2);
    
    m_linestartctrl = new wxTextCtrl (sbSizer8->GetStaticBox (), IDC_LineStart, wxEmptyString, wxDefaultPosition, wxSize (70, -1), 0);
    m_linestartctrl->SetMaxLength (0);
    m_linestartctrl->SetValidator (wxTextValidator (wxFILTER_DIGITS, &m_LineStartString));
    
    gSizer1->Add (m_linestartctrl, 0, wxALIGN_CENTER | wxALL, 2);
    
    m_lineendctrl = new wxTextCtrl (sbSizer8->GetStaticBox (), IDC_LineEnd, wxEmptyString, wxDefaultPosition, wxSize (70, -1), 0);
    m_lineendctrl->SetMaxLength (0);
    m_lineendctrl->SetValidator (wxTextValidator (wxFILTER_DIGITS, &m_LineEndString));
    
    gSizer1->Add (m_lineendctrl, 0, wxALIGN_CENTER | wxALL, 2);
    
    m_lineintctrl = new wxTextCtrl (sbSizer8->GetStaticBox (), IDC_LineInterval, wxEmptyString, wxDefaultPosition, wxSize (70, -1), 0);
    m_lineintctrl->SetMaxLength (0);
    m_lineintctrl->SetValidator (wxTextValidator (wxFILTER_DIGITS, &m_LineIntervalString));
    
    gSizer1->Add (m_lineintctrl, 0, wxALIGN_CENTER | wxALL, 2);
    
    m_staticText62 = new wxStaticText (sbSizer8->GetStaticBox (), IDC_ColumnPrompt, wxT("Columns"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText62->Wrap (-1);
    gSizer1->Add (m_staticText62, 0, wxALIGN_CENTER | wxALL, 2);
    
    m_colstartctrl = new wxTextCtrl (sbSizer8->GetStaticBox (), IDC_ColumnStart, wxEmptyString, wxDefaultPosition, wxSize (70, -1), 0);
    m_colstartctrl->SetMaxLength (0);
    m_colstartctrl->SetValidator (wxTextValidator (wxFILTER_DIGITS, &m_ColumnStartString));
    
    gSizer1->Add (m_colstartctrl, 0, wxALIGN_CENTER | wxALL, 2);
    
    m_colendctrl = new wxTextCtrl (sbSizer8->GetStaticBox (), IDC_ColumnEnd, wxEmptyString, wxDefaultPosition, wxSize (70, -1), 0);
    m_colendctrl->SetMaxLength (0);
    m_colendctrl->SetValidator (wxTextValidator (wxFILTER_DIGITS, &m_ColumnEndString));
    
    gSizer1->Add (m_colendctrl, 0, wxALIGN_CENTER | wxALL, 2);
    
    m_colintctrl = new wxTextCtrl (sbSizer8->GetStaticBox (), IDC_ColumnInterval, wxEmptyString, wxDefaultPosition, wxSize (70, -1), 0);
    m_colintctrl->SetMaxLength (0);
    m_colintctrl->SetValidator (wxTextValidator (wxFILTER_DIGITS, &m_ColumnIntervalString));
    
    SetUpToolTip (m_bpButton4, IDS_ToolTip40);
    SetUpToolTip (m_linestartctrl, IDS_ToolTip19);
    SetUpToolTip (m_lineendctrl, IDS_ToolTip20);
    SetUpToolTip (m_lineintctrl, IDS_ToolTip21);
    SetUpToolTip (m_colstartctrl, IDS_ToolTip22);
    SetUpToolTip (m_colendctrl, IDS_ToolTip23);
    SetUpToolTip (m_colintctrl, IDS_ToolTip24);
    
    gSizer1->Add (m_colintctrl, 0, wxALIGN_CENTER | wxALL, 2);
    
    bSizer121->Add (gSizer1, 1, wxEXPAND, 5);
    sbSizer8->Add (bSizer121, 1, wxEXPAND, 5);
    */
   bSizer117->Add (sbSizer8, 1, wxEXPAND, 5);
   
   bSizer115->Add (bSizer117, 0, 0, 5);
   bSizer111->Add (bSizer115, 0, 0, 5);
   //bSizer110->Add (bSizer111, 1, wxALL, 12);
   bFlexGrid110->Add (bSizer111, 1, wxALL, 12);
   
   wxBoxSizer* bSizer112;
   bSizer112 = new wxBoxSizer (wxVERTICAL);
   
   wxBoxSizer* bSizer120;
   bSizer120 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText176 = new wxStaticText (this, IDC_ClassPrompt, wxT("Classes:"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText176->Wrap (-1);
   SetUpToolTip (m_staticText176, IDS_ToolTip103);
   bSizer120->Add (m_staticText176, 0, wxALIGN_CENTER, 5);
   
   
   m_comboBox18 = new wxComboBox (this, IDC_ClassCombo, wxT("All"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
   m_comboBox18->Append (wxT("All"));
   m_comboBox18->Append (wxT("Subset..."));
   SetUpToolTip (m_comboBox18, IDS_ToolTip103);
   bSizer120->Add (m_comboBox18, 0, wxALIGN_CENTER|wxLEFT, 5);
   
   
   bSizer112->Add (bSizer120, 0, 0, 5);
   
   //   wxBoxSizer* bSizer122;
   bSizer122 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText178 = new wxStaticText (this, IDC_WeightsPrompt, wxT("Class weights:"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText178->Wrap (-1);
   SetUpToolTip (m_staticText178, IDS_ToolTip156);
   bSizer122->Add (m_staticText178, 0, wxALIGN_CENTER, 5);
   
   m_staticText186 = new wxStaticText (this, IDC_WeightsEqual, wxT("Equal"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText186->Wrap (-1);
   bSizer122->Add (m_staticText186, 0, wxALIGN_CENTER|wxLEFT, 5);
   
   m_comboBox19 = new wxComboBox (this, IDC_WeightCombo, wxT("Equal"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
   m_comboBox19->Append (wxT("Equal"));
   m_comboBox19->Append (wxT("Unequal..."));
   SetUpToolTip (m_comboBox19, IDS_ToolTip157);
   bSizer122->Add (m_comboBox19, 0, wxRESERVE_SPACE_EVEN_IF_HIDDEN|wxALIGN_CENTER|wxLEFT, 5);
   
   
   bSizer112->Add (bSizer122, 0, wxBOTTOM|wxTOP, 5);
   
   //   wxBoxSizer* bSizer123;
   bSizer123 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText179 = new wxStaticText (this, IDC_SymbolPrompt, wxT("Symbols:"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText179->Wrap (-1);
   SetUpToolTip (m_staticText179, IDS_ToolTip157);
   bSizer123->Add (m_staticText179, 0, wxALIGN_CENTER, 5);
   
   m_staticText187 = new wxStaticText (this, IDC_SymbolsDefaultSet, wxT("Default set"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText187->Wrap (-1);
   SetUpToolTip (m_staticText187, IDS_ToolTip157);
   bSizer123->Add (m_staticText187, 0, wxALIGN_CENTER|wxLEFT, 5);
   
   m_comboBox20 = new wxComboBox (this, IDC_SymbolCombo, wxT("Default Set"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
   m_comboBox20->Append (wxT("Default set"));
   m_comboBox20->Append (wxT("User defined..."));
   bSizer123->Add (m_comboBox20, 0, wxALIGN_CENTER|wxLEFT, 5);
   
   
   bSizer112->Add (bSizer123, 0, wxBOTTOM|wxTOP, 5);
   
   wxBoxSizer* bSizer124;
   bSizer124 = new wxBoxSizer (wxVERTICAL);
   
   m_staticText180 = new wxStaticText (this, wxID_ANY, wxT("Write classification results to:"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText180->Wrap (-1);
   bSizer124->Add (m_staticText180, 0, wxBOTTOM|wxTOP, 5);
   
   //   wxBoxSizer* bSizer125;
   bSizer125 = new wxBoxSizer (wxHORIZONTAL);
   
   m_checkBox13 = new wxCheckBox (this, IDC_DiskFile, wxT("Disk File:"), wxDefaultPosition, wxDefaultSize, 0);
   SetUpToolTip (m_checkBox13, IDS_ToolTip178);
   bSizer125->Add (m_checkBox13, 0, wxALIGN_CENTER|wxLEFT, 15);
   
   m_comboBox21 = new wxComboBox (this, IDC_DiskCombo, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
   m_comboBox21->Append (wxT("ASCII"));
   m_comboBox21->Append (wxT("ERDAS .GIS"));
   m_comboBox21->Append (wxT("GAIA"));
   m_comboBox21->Append (wxT("GeoTIFF"));
   SetUpToolTip (m_comboBox21, IDS_ToolTip215);
   bSizer125->Add (m_comboBox21, 0, wxALIGN_CENTER|wxLEFT, 5);
   
   
   bSizer124->Add (bSizer125, 0, wxALL, 5);
   
   //   wxBoxSizer* bSizer126;
   bSizer126 = new wxBoxSizer (wxVERTICAL);
   
   m_checkBox14 = new wxCheckBox (this, IDC_ImageWindowOverlay, wxT("Image window overlay"), wxDefaultPosition, wxDefaultSize, 0);
   SetUpToolTip (m_checkBox14, IDS_ToolTip179);
   bSizer126->Add (m_checkBox14, 0, wxLEFT, 15);
   
   m_comboBox22 = new wxComboBox (this, IDC_ImageOverlayCombo, wxT("Combo!"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
   m_comboBox22->Append (wxT("Add new overlay"));
   SetUpToolTip (m_comboBox22, IDS_ToolTip180);
   bSizer126->Add (m_comboBox22, 0, wxLEFT, 35);
   
   
   bSizer124->Add (bSizer126, 0, wxALL, 5);
   
   //   wxBoxSizer* bSizer127;
   bSizer127 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText182 = new wxStaticText (this, IDC_PalettePrompt, wxT("Palette:"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText182->Wrap (-1);
   bSizer127->Add (m_staticText182, 0, wxALIGN_CENTER|wxLEFT, 15);
   
   m_comboBox23 = new wxComboBox (this, IDC_PaletteCombo, wxT("Default Colors"), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
   m_comboBox23->Append (wxT("Default Colors"));
   m_comboBox23->Append (wxT("Default Gray Levels"));
   m_comboBox23->Append (wxT("Blue-Green-Red"));
   m_comboBox23->Append (wxT("AVHRR NDVI"));
   m_comboBox23->Append (wxT("MODIS NDVI"));
   m_comboBox23->Append (wxT("False Color..."));
   m_comboBox23->Append (wxT("ERDAS .trl file"));
   m_comboBox23->Append (wxT("User Defined"));
   SetUpToolTip (m_comboBox23, IDS_ToolTip181);
   bSizer127->Add (m_comboBox23, 0, wxALIGN_CENTER|wxLEFT, 5);
   
   
   bSizer124->Add (bSizer127, 0, wxALL, 5);
   
   //   wxBoxSizer* bSizer128;
   bSizer128 = new wxBoxSizer (wxHORIZONTAL);
   
   m_checkBox15 = new wxCheckBox (this, IDC_ThresholdResults, wxT("Threshold results at"), wxDefaultPosition, wxDefaultSize, 0);
   SetUpToolTip (m_checkBox15, IDS_ToolTip182);
   bSizer128->Add (m_checkBox15, 0, wxALIGN_CENTER|wxALL, 5);
   
   m_textCtrl85 = new wxTextCtrl (this, IDC_ThresholdValue, wxEmptyString, wxDefaultPosition, wxSize (40,-1), 0);
   m_textCtrl85->SetValidator (wxTextValidator (wxFILTER_NUMERIC, &m_ThresString));
   SetUpToolTip (m_textCtrl85, IDS_ToolTip183);
   bSizer128->Add (m_textCtrl85, 0, wxALL, 5);
   
   m_staticText183 = new wxStaticText (this, IDC_PercentSymbol, wxT("%"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText183->Wrap (-1);
   bSizer128->Add (m_staticText183, 0, wxALIGN_CENTER|wxALL, 5);
   
   m_textCtrl87 = new wxTextCtrl (this, IDC_CEMThreshold, wxEmptyString, wxDefaultPosition, wxSize (40,-1), 0);
   SetUpToolTip (m_textCtrl87, IDS_ToolTip184);
   m_textCtrl87->SetValidator (wxTextValidator (wxFILTER_NUMERIC, &m_cemThresString));
   
   bSizer128->Add (m_textCtrl87, 0, wxALL, 5);
   
   m_textCtrl88 = new wxTextCtrl (this, IDC_CorrelationThresold, wxEmptyString, wxDefaultPosition, wxSize (40,-1), 0);
   m_textCtrl88->SetValidator (wxTextValidator (wxFILTER_NUMERIC, &m_corrThresString));
   SetUpToolTip (m_textCtrl88, IDS_ToolTip148);
   bSizer128->Add (m_textCtrl88, 0, wxALL, 5);
   
   m_staticText185 = new wxStaticText (this, IDC_DegreeSymbol, wxT("o"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText185->Wrap (-1);
   bSizer128->Add (m_staticText185, 0, 0, 5);
   
   
   bSizer124->Add (bSizer128, 0, wxEXPAND, 5);
   
   //   wxBoxSizer* bSizer129;
   bSizer129 = new wxBoxSizer (wxHORIZONTAL);
   
   m_staticText184 = new wxStaticText (this, IDC_correlationPrompt, wxT("or correlation     \n coefficient of"), wxDefaultPosition, wxDefaultSize, 0);
   m_staticText184->Wrap (-1);
   SetUpToolTip (m_staticText184, IDS_ToolTip148);
   bSizer129->Add (m_staticText184, 0, wxALIGN_CENTER|wxLEFT, 25);
   
   m_textCtrl86 = new wxTextCtrl (this, IDC_CorrelationCoefficient, wxEmptyString, wxDefaultPosition, wxSize (80,-1), 0);
   SetUpToolTip (m_textCtrl86, IDS_ToolTip148);
   bSizer129->Add (m_textCtrl86, 0, wxALL, 5);
   
   
   bSizer124->Add (bSizer129, 0, wxEXPAND|wxLEFT, 5);
   
   wxBoxSizer* bSizer130;
   bSizer130 = new wxBoxSizer (wxHORIZONTAL);
   
   m_checkBox16 = new wxCheckBox (this, IDC_CreateProbabilityFile, wxT("Create Probability Results File"), wxDefaultPosition, wxDefaultSize, 0);
   SetUpToolTip (m_checkBox16, IDS_ToolTip186);
   bSizer130->Add (m_checkBox16, 0, wxALL, 5);
   
   
   bSizer124->Add (bSizer130, 0, wxEXPAND, 5);
   
   m_button23 = new wxButton (this, IDC_ListOptions, wxT("Results List Options..."), wxDefaultPosition, wxSize (200,-1), 0);
   SetUpToolTip (m_button23, IDS_ToolTip187);
   bSizer124->Add (m_button23, 0, wxALL, 5);
   /*
    wxBoxSizer* bSizer1311;
    bSizer1311 = new wxBoxSizer (wxHORIZONTAL);
    
    m_button24 = new wxButton (this, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer1311->Add (m_button24, 0, wxALIGN_RIGHT|wxALL, 5);
    
    m_button25 = new wxButton (this, wxID_OK, wxT("OK"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer1311->Add (m_button25, 0, wxALIGN_RIGHT|wxALL, 5);
    
    bSizer124->Add (bSizer1311, 0, wxALIGN_RIGHT, 5);
    */
   bSizer112->Add (bSizer124, 0, wxEXPAND, 5);
   
   bFlexGrid110->Add (bSizer112, 1, wxLEFT|wxTOP|wxRIGHT, 12);
   
   bVSizerMain->Add (bFlexGrid110, 1, wxLEFT|wxTOP|wxRIGHT, 12);
   
   //wxSizer* standardButtonSizer = CreateButtonSizer (wxOK | wxCANCEL);
   //bVSizerMain->Add (standardButtonSizer, wxSizerFlags(0).Right());
   CreateStandardButtons (bVSizerMain);
   
   m_textCtrl87->Hide ();
   m_textCtrl88->Hide ();
   m_staticText185->Hide ();
   //this->SetSizer (bSizer110);
   this->SetSizerAndFit (bVSizerMain);
   this->Layout ();
   //bSizer110->Fit (this);
   bVSizerMain->Fit (this);
   this->Centre (wxBOTH);
   
}   // end "CreateControls"



//------------------------------------------------------------------------------------
//                         Copyright (1988-2018)
//                        (c) Purdue Research Foundation
//                           All rights reserved.
//
//   Function name:      void DoDialog
//
//   Software purpose:   The purpose of this routine is to present the classification
//                     specification dialog box to the user and copy the
//                     revised back to the classify specification structure if
//                     the user selected OK.
//
//   Parameters in:      None
//
//   Parameters out:   None
//
//   Value Returned:   None
//
//   Called By:         Dialog in MDisMult.cpp
//
//   Coded By:         Larry L. Biehl         Date: 02/27/1996
//   Revised By:         Larry L. Biehl         Date: 04/10/1998

SInt16 CMClassifyDialog::DoDialog (void)

{
   SInt16 returnCode;
   
   Boolean continueFlag = FALSE;
   
   
   // Make sure intialization has been completed.
   
   if (!m_initializedFlag)
      return (FALSE);
   
   returnCode = ShowModal ();
   
   if (returnCode == wxID_OK)
   {
      continueFlag = TRUE;
      
      // Classification area
      
      m_dialogSelectArea.lineStart = m_LineStart;
      m_dialogSelectArea.lineEnd = m_LineEnd;
      m_dialogSelectArea.lineInterval = m_LineInterval;
      
      m_dialogSelectArea.columnStart = m_ColumnStart;
      m_dialogSelectArea.columnEnd = m_ColumnEnd;
      m_dialogSelectArea.columnInterval = m_ColumnInterval;
      
      ClassifyDialogOK (m_classificationProcedure,
                        m_covarianceEstimate,
                        m_featureTransformationFlag,
                        m_channelSelection,
                        (SInt16)m_localActiveNumberFeatures,
                        (SInt16*)m_localActiveFeaturesPtr,
                        m_targetWindowInfoHandle,
                        m_fileNamesSelection,
                        m_classAreaSelection,
                        (SInt16)m_localNumberClassAreas,
                        (SInt16*)m_classAreaListPtr,
                        m_trainingAreaFlag,
                        m_trainingAreaLOOFlag,
                        m_testAreaFlag,
                        m_imageAreaFlag,
                        &m_dialogSelectArea,
                        m_classSelection,
                        (SInt16)m_localNumberClasses,
                        (SInt16*)m_classListPtr,
                        m_classWeightsSelection + 1,
                        m_classWeightsPtr,
                        m_symbolSelection,
                        m_localSymbolsPtr,
                        //                           m_textWindowFlag,
                        m_diskFileFlag,
                        m_createImageOverlayFlag,
                        m_selectImageOverlaySelection + 1,
                        m_outputFormatCode + 1,
                        m_thresholdResultsFlag,
                        m_correlationThreshold,
                        m_angleThreshold,
                        m_cemThreshold,
                        m_thresholdPercent,
                        m_createProbabilityFileFlag,
                        m_paletteSelection + 1,
                        m_listResultsTestCode,
                        m_listResultsTrainingCode,
                        m_parallelPipedCode);
      
   }   // end "if (returnCode == IDOK)"
   
   // Set the variable containing the pointer to the overlay combo box
   // to NULL.
   
   gPopUpImageOverlayMenu = NULL;
   
   return (continueFlag);
   
}   // end "DoDialog"



void CMClassifyDialog::OnDropdownClassificationProcedure (
                                                          wxCommandEvent&                              event)

{
   wxMouseState         mousestate;
   
   
   if (mousestate.RightIsDown () || wxGetKeyState (WXK_SHIFT) ||
       m_covarianceEstimate != kNoCovarianceUsed)
   {
      SetComboItemText   (IDC_ClassificationProcedure,
                          m_correlationComboListItem,
                          (char*)"Correlation (SAM)...");
      
      m_optionKeyFlag = TRUE;
      
   }   // end "if (GetKeyState (VK_RBUTTON) < 0 || ..."
   
   
   else // no option requested
   {
      SetComboItemText (IDC_ClassificationProcedure,
                        m_correlationComboListItem,
                        (char*)"Correlation (SAM)");
      
      m_optionKeyFlag = FALSE;
      
   }   // end "else no option requested"
   
   wxComboBox* comboBoxPtr = (wxComboBox*)FindWindow (IDC_ClassificationProcedure);
   //comboBoxPtr->SetItemData (m_correlationComboListItem, kCorrelationMode);
   comboBoxPtr->SetClientData (m_correlationComboListItem, (void*)kCorrelationMode);
   
   //comboBoxPtr->SetSelection (m_classifyListSelection); // ?
   
   comboBoxPtr->SetSelection (-1);
   
}   // end "OnDropdownClassificationProcedure"



void CMClassifyDialog::OnInitDialog (
                                     wxInitDialogEvent& event)
{
   SInt16 channelSelection,
   classAreaSelection,
   classSelection,
   fileNamesSelection,
   outputFormatCode,
   paletteSelection,
   selectImageOverlaySelection,
   symbolSelection,
   weightsSelection;
   
   
   //wxDialog::OnInitDialog (event);
   
   // Make sure that we have the bitmaps for the entire image buttons.
   
   ClassifyDialogInitialize (this,
                             m_localFeaturesPtr,
                             m_localTransformFeaturesPtr,
                             m_classListPtr,
                             m_classAreaListPtr,
                             m_localSymbolsPtr,
                             &m_classificationProcedure,
                             &m_thresholdAllowedFlag,
                             &m_covarianceEstimate,
                             &m_numberEigenvectors,
                             &m_featureTransformAllowedFlag,
                             (Boolean*)& m_featureTransformationFlag,
                             &channelSelection,
                             &m_localActiveNumberFeatures,
                             &fileNamesSelection,
                             &m_targetWindowInfoHandle,
                             &classAreaSelection,
                             &m_localNumberClassAreas,
                             (Boolean*)& m_trainingAreaFlag,
                             &m_trainingFieldsExistFlag,
                             &m_savedLeaveOneOutFlag,
                             (Boolean*)& m_trainingAreaLOOFlag,
                             (Boolean*)& m_testAreaFlag,
                             (Boolean*)& m_imageAreaFlag,
                             &m_dialogSelectArea,
                             &classSelection,
                             &m_localNumberClasses,
                             &weightsSelection,
                             &symbolSelection,
                             &m_outputAsciiCode,
                             //(Boolean*)&m_textWindowFlag,
                             (Boolean*)& m_createImageOverlayFlag,
                             &selectImageOverlaySelection,
                             &outputFormatCode,
                             (Boolean*)& m_diskFileFlag,
                             &paletteSelection,
                             (Boolean*)& m_thresholdResultsFlag,
                             (Boolean*)& m_createProbabilityFileFlag,
                             &m_saveThresholdPercent,
                             &m_saveAngleThreshold,
                             &m_saveCorrelationThreshold,
                             &m_saveCEMThreshold,
                             &m_listResultsTestCode,
                             &m_listResultsTrainingCode,
                             &m_parallelPipedCode);
   
   // Set feature/transform feature parameters
   
   InitializeDialogFeatureParameters (m_featureTransformationFlag,
                                      m_localActiveNumberFeatures,
                                      gProjectInfoPtr->numberStatisticsChannels,
                                      gTransformationMatrix.numberFeatures,
                                      m_localFeaturesPtr,
                                      m_localTransformFeaturesPtr,
                                      &m_localNumberFeatures,
                                      &m_localNumberTransformFeatures,
                                      &m_localActiveFeaturesPtr);
   
   // Set up classification popup menu list
   
   m_correlationComboListItem = kCorrelationMode - 1;
   if (gProjectInfoPtr->statisticsCode != kMeanCovariance)
      m_correlationComboListItem = 1;
   
   // Get the classification list selection that matches the input
   // classification procedure.
   
   m_classifyListSelection = GetComboListSelection2 (this,
                                                     IDC_ClassificationProcedure,
                                                     m_classificationProcedure);
   
   if (m_classifyListSelection == -1)
      m_classifyListSelection = 0;
   
   //   Set the channels/features list item
   
   m_availableFeaturePtr = gProjectInfoPtr->channelsPtr;
   m_channelSelection = channelSelection;
   
   // Target File
   
   m_fileNamesSelection = fileNamesSelection - 1;
   
   // Class areas to classify.
   
   m_classAreaSelection = classAreaSelection;
   
   // Selected area to classify.
   
   m_LineStart = m_dialogSelectArea.lineStart;
   m_LineEnd = m_dialogSelectArea.lineEnd;
   m_LineInterval = m_dialogSelectArea.lineInterval;
   m_ColumnStart = m_dialogSelectArea.columnStart;
   m_ColumnEnd = m_dialogSelectArea.columnEnd;
   m_ColumnInterval = m_dialogSelectArea.columnInterval;
   
   // Classes to use.
   
   m_classSelection = classSelection;
   
   // Class weights.
   // Adjust for 0 base index.
   
   m_classWeightsSelection = weightsSelection - 1;
   
   if (weightsSelection > 0)
      HideDialogItem (this, IDC_WeightsEqual);
   
   else // weightsSelection <= 0
      HideDialogItem (this, IDC_WeightCombo);
   
   // Symbols to use.
   // Adjust for 0 base index.
   // User defined symbols are not available yet.
   
   m_symbolSelection = symbolSelection - 1;
   HideDialogItem (this, IDC_SymbolCombo);
   wxComboBox* symbolc = (wxComboBox*)FindWindow (IDC_SymbolCombo);
   symbolc->Delete (1);
   
   // Classification output to disk file.
   // Adjust for 0 base index.
   
   m_outputFormatCode = abs (outputFormatCode);
   m_outputFormatCode -= 1;
   
   if (!(gClassifySpecsPtr->outputStorageType & 0x0006))
      gOutputFormatCode = -gOutputFormatCode;
   
   if (!m_diskFileFlag)
      MHideDialogItem (this, IDC_DiskCombo);
   
   m_selectImageOverlaySelection = selectImageOverlaySelection - 1;
   
   // Palette to use.
   // Adjust for 0 base index.
   
   m_paletteSelection = paletteSelection - 1;
   
   // Threshold output data.
   // Threshold percent (to be used for output files).
   
   m_thresholdPercent = (float)m_saveThresholdPercent;
   
   // Thresholds for Correlation Classifier.
   
   m_angleThreshold = (float)m_saveAngleThreshold;
   m_correlationThreshold = (float)m_saveCorrelationThreshold;
   
   // Thresholds for CEM Classifier.
   
   m_cemThreshold = (float)m_saveCEMThreshold;
   
   if (TransferDataToWindow ())
      PositionDialogWindow ();
   
   // Set default text selection to first edit text item
   
   SelectDialogItemText (this, IDC_LineStart, 0, SInt16_MAX);
   //SetSizerAndFit (bSizer110);
   //SetSizerAndFit (bFlexGrid110);
   
   AdjustDlgLayout ();
   
   //this->Layout ();
   //this->Fit ();
   
}   // end "OnInitDialog"



void CMClassifyDialog::AdjustDlgLayout ()

{
   bSizer122->Layout ();
   bSizer123->Layout ();
   bSizer125->Layout ();
   bSizer126->Layout ();
   bSizer127->Layout ();
   bSizer128->Layout ();
   bSizer129->Layout ();
   bSizer114->Layout ();
   
}   // end "AdjustDlgLayout"



void CMClassifyDialog::HideShowClassAreasItem ()
{
   if (!m_trainingAreaFlag && !m_trainingAreaLOOFlag && !m_testAreaFlag)
      MHideDialogItem (this, IDC_AreasCombo);
   
   else   // m_trainingAreaFlag || m_trainingAreaLOOFlag ...
      MShowDialogItem (this, IDC_AreasCombo);
   
   
}   // end "ShowThresholdItems"



void CMClassifyDialog::OnSelendokChannelCombo (
                                               wxCommandEvent& event)

{
   HandleChannelsMenu (IDC_ChannelCombo,
                       m_featureTransformationFlag,
                       (SInt16)gProjectInfoPtr->numberStatisticsChannels,
                       2,
                       TRUE);
   
}   // end "OnSelendokChannelCombo"



void CMClassifyDialog::OnSelendokClassWeightsCombo (
                                                    wxCommandEvent& event)

{
   HandleClassWeightsMenu ((SInt16)m_localNumberClasses,
                           (SInt16*)m_classListPtr,
                           m_classWeightsPtr,
                           gProjectInfoPtr->covarianceStatsToUse == kEnhancedStats,
                           IDC_WeightCombo,
                           &m_classWeightsSelection);
   
}   // end "OnSelendokClassWeightsCombo"



void CMClassifyDialog::OnSelendokClassificationProcedure (
                                                          wxCommandEvent& event)

{
   wxComboBox* comboBoxPtr = (wxComboBox*)FindWindow (IDC_ClassificationProcedure);
   
   SInt16 classificationProcedure = 0,
   savedClassifyListSelection,
   weightsSelection;
   
   Boolean returnFlag = TRUE;
   
   
   savedClassifyListSelection = m_classifyListSelection;
   
   m_classifyListSelection= comboBoxPtr->GetSelection ();
   
   // Get the actual classification procedure code.
   
   // comboBoxPtr = (CComboBox*)GetDlgItem (IDC_ClassificationProcedure);
   //comboBoxPtr->SetSelection (m_classifyListSelection);
   SInt64 classifyListSelection64 = (SInt64)((int*)comboBoxPtr->GetClientData (m_classifyListSelection));
   classificationProcedure = (SInt16)classifyListSelection64;
   
   // Get the current weights selection. Force to 1 base.
   
   weightsSelection = m_classWeightsSelection + 1;
   
   classificationProcedure = ClassifyDialogOnClassificationProcedure (
                                                                      this,
                                                                      wxID_OK,
                                                                      &m_thresholdAllowedFlag,
                                                                      &m_featureTransformAllowedFlag,
                                                                      &weightsSelection,
                                                                      &m_parallelPipedCode,
                                                                      classificationProcedure,
                                                                      &m_covarianceEstimate,
                                                                      m_numberEigenvectors,
                                                                      &m_classifyProcedureEnteredCode,
                                                                      m_optionKeyFlag);
   
   m_classWeightsSelection = weightsSelection - 1;
   
   if (classificationProcedure != 0)
   {
      m_classificationProcedure = classificationProcedure;
      
      ClassifyDialogSetLeaveOneOutItems (this,
                                         m_classificationProcedure,
                                         m_fileNamesSelection,
                                         m_savedLeaveOneOutFlag,
                                         m_trainingFieldsExistFlag,
                                         (Boolean*)& m_trainingAreaLOOFlag);
      
      CheckFeatureTransformationDialog (this,
                                        m_featureTransformAllowedFlag,
                                        IDC_FeatureTransformation,
                                        IDC_ChannelPrompt,
                                        (SInt16*)& m_featureTransformationFlag);
      
      ClassifyDialogSetThresholdItems (this,
                                       m_classificationProcedure,
                                       m_imageAreaFlag,
                                       m_createProbabilityFileFlag,
                                       m_thresholdResultsFlag,
                                       m_thresholdAllowedFlag);
      
      if (weightsSelection > 0)
      {
         HideDialogItem (this, IDC_WeightsEqual);
         ShowDialogItem (this, IDC_WeightCombo);
         
         wxComboBox* weightc = (wxComboBox*)FindWindow (IDC_WeightCombo);
         weightc->SetSelection (m_classWeightsSelection);
         
      }   // end "if (weightsSelection > 0)"
      
      else // weightsSelection <= 0
      {
         ShowDialogItem (this, IDC_WeightsEqual);
         HideDialogItem (this, IDC_WeightCombo);
         
      }   // end "else weightsSelection <= 0"
      
   }   // end "if (classificationProcedure != 0)"
   
   else // classificationProcedure == 0
   {
      m_classifyListSelection = savedClassifyListSelection;
      
   }   // end "else m_classificationProcedure == 0"
   
   comboBoxPtr->SetSelection (m_classifyListSelection);
   
   SInt64 classificationProcedure64 =(SInt64)((int*)comboBoxPtr->GetClientData (m_classifyListSelection));
   m_classificationProcedure = (SInt16)classificationProcedure64;
   
   m_optionKeyFlag = FALSE;
   
   switch (m_classifyListSelection){
      case 5:
         SetUpToolTip (m_checkBox16, IDS_ToolTip232);
         break;
      case 6:
         SetUpToolTip (m_checkBox16, IDS_ToolTip232);
         break;
      default:
         SetUpToolTip (m_checkBox16, IDS_ToolTip186);
   }
   
   //   this->Layout ();
   //   this->Fit ();
   AdjustDlgLayout ();
}   // end "OnSelendokClassificationProcedure"

void CMClassifyDialog::CheckColumnEnd (wxCommandEvent& event)
{
   SignedByte handleStatus;
   
   
   if (!m_settingSelectedEntireButton)
   {
      m_dialogSelectArea.imageWindowInfoPtr =
      (WindowInfoPtr)GetHandleStatusAndPointer (m_targetWindowInfoHandle,
                                                &handleStatus,
                                                kNoMoveHi);
      
      CMDialog::CheckColumnEnd (event);
      
      MHSetState (m_targetWindowInfoHandle, handleStatus);
      
      m_dialogSelectArea.imageWindowInfoPtr = NULL;
      
   }   // end "if (!m_settingSelectedEntireButton)"
   
}   // end "CheckColumnEnd"

void
CMClassifyDialog::CheckColumnStart (wxCommandEvent& event){
   SignedByte handleStatus;
   
   
   if (!m_settingSelectedEntireButton){
      m_dialogSelectArea.imageWindowInfoPtr =
      (WindowInfoPtr)GetHandleStatusAndPointer (m_targetWindowInfoHandle,
                                                &handleStatus,
                                                kNoMoveHi);
      
      CMDialog::CheckColumnStart (event);
      
      MHSetState (m_targetWindowInfoHandle, handleStatus);
      
      m_dialogSelectArea.imageWindowInfoPtr = NULL;
      
   }   // end "if (!m_settingSelectedEntireButton)"
   
}   // end "CheckColumnStart"

void CMClassifyDialog::CheckLineEnd (wxCommandEvent& event){
   SignedByte handleStatus;
   
   
   if (!m_settingSelectedEntireButton){
      m_dialogSelectArea.imageWindowInfoPtr =
      (WindowInfoPtr)GetHandleStatusAndPointer (m_targetWindowInfoHandle,
                                                &handleStatus,
                                                kNoMoveHi);
      
      CMDialog::CheckLineEnd (event);
      
      MHSetState (m_targetWindowInfoHandle, handleStatus);
      
      m_dialogSelectArea.imageWindowInfoPtr = NULL;
      
   }   // end "if (!m_settingSelectedEntireButton)"
   
}   // end "CheckLineEnd"

void CMClassifyDialog::CheckLineStart (wxCommandEvent& event){
   SignedByte handleStatus;
   
   
   if (!m_settingSelectedEntireButton){
      m_dialogSelectArea.imageWindowInfoPtr =
      (WindowInfoPtr)GetHandleStatusAndPointer (m_targetWindowInfoHandle,
                                                &handleStatus,
                                                kNoMoveHi);
      
      CMDialog::CheckLineStart (event);
      
      MHSetState (m_targetWindowInfoHandle, handleStatus);
      
      m_dialogSelectArea.imageWindowInfoPtr = NULL;
      
   }   // end "if (!m_settingSelectedEntireButton)"
   
}   // end "CheckLineStart"



void CMClassifyDialog::CheckOutputFormatItems ()

{
   // Make certain that disk file output formats and thresholding
   // are consistent with the requested classification
   // specifications.
   
   wxComboBox* diskc = (wxComboBox*)FindWindow (IDC_DiskCombo);
   m_outputFormatCode = diskc->GetSelection ();
   
   m_outputFormatCode++;
   
   ClassifyDialogSetPaletteItems (this,
                                  m_outputFormatCode,
                                  m_createImageOverlayFlag);
   /*
    if (m_diskFileFlag && m_outputFormatCode == kClassifyERDAS74OutputFormat)
    {
    ShowDialogItem (this, IDC_PalettePrompt);
    ShowDialogItem (this, IDC_PaletteCombo);
    
    }      // end "if (m_outputFormatCode != 2)"
    
    else      // !m_diskFileFlag || m_outputFormatCode != 2
    {
    HideDialogItem (this, IDC_PalettePrompt);
    HideDialogItem (this, IDC_PaletteCombo);
    
    }      // end "else !m_diskFileFlag || m_outputFormatCode != 2"
    */
   if (m_outputFormatCode != 1)
      m_outputAsciiCode = (m_outputAsciiCode & 0xfffd);
   
   else // m_outputFormatCode == 1
      m_outputAsciiCode = (m_outputAsciiCode | 0x0002);
   
   // Check threshold items.
   
   ClassifyDialogSetThresholdItems (this,
                                    m_classificationProcedure,
                                    m_imageAreaFlag,
                                    m_createProbabilityFileFlag,
                                    m_thresholdResultsFlag,
                                    m_thresholdAllowedFlag);
   
}   // end "CheckOutputFormatItems"


/*
 void CMClassifyDialog::OnButtonPress (wxKeyEvent& event)
 {
 if (event.GetKeyCode () == WXK_RETURN)
 {
 if (TransferDataFromWindow ())
 EndModal (wxID_OK);
 }
 
 else
 {
 event.Skip ();
 }
 }
 */


void CMClassifyDialog::OnDiskFile (wxCommandEvent& event)

{
   wxCheckBox* diskfcb = (wxCheckBox*)FindWindow (IDC_DiskFile);
   m_diskFileFlag = diskfcb->GetValue ();
   
   if (!m_diskFileFlag)
      HideDialogItem (this, IDC_DiskCombo);
   
   else   // !m_diskFileFlag)
      ShowDialogItem (this, IDC_DiskCombo);
   
   CheckOutputFormatItems ();
   //this->Layout ();
   //this->Fit ();
   AdjustDlgLayout ();
   
}   // end "OnDiskFile"



void CMClassifyDialog::OnThresholdResults (wxCommandEvent& event)

{
   wxCheckBox* thresholdcb = (wxCheckBox*)FindWindow (IDC_ThresholdResults);
   m_thresholdResultsFlag = thresholdcb->GetValue ();
   CheckOutputFormatItems ();
   //    this->Layout ();
   //    this->Fit ();
   AdjustDlgLayout ();
   
}   // end "OnThresholdResults"



void CMClassifyDialog::OnSelendokDiskCombo (wxCommandEvent& event){
   CheckOutputFormatItems ();
   
}   // end "OnSelendokDiskCombo"



void CMClassifyDialog::OnTextWindow (wxCommandEvent& event)
{
   // Add your control notification handler code here
   
}   // end "OnTextWindow"



void CMClassifyDialog::OnTraining (wxCommandEvent& event)

{
   wxCheckBox* trainingcb = (wxCheckBox*)FindWindow (IDC_Training);
   m_trainingAreaFlag = trainingcb->GetValue ();
   HideShowClassAreasItem ();
   
   CheckAreaSettings ();
   //    this->Layout ();
   //    this->Fit ();
   AdjustDlgLayout ();
   
}   // end "OnTraining"



void CMClassifyDialog::OnTrainingLOO(wxCommandEvent& event)

{
   wxCheckBox* trainingloocb = (wxCheckBox*)FindWindow (IDC_TrainingLOO);
   m_trainingAreaLOOFlag = trainingloocb->GetValue ();
   
   HideShowClassAreasItem ();
   
   CheckAreaSettings ();
   //    this->Layout ();
   //    this->Fit ();
   AdjustDlgLayout ();
}   // end "OnTrainingLOO"



void CMClassifyDialog::OnTestAreas (wxCommandEvent& event){
   wxCheckBox* testareacb = (wxCheckBox*)FindWindow (IDC_TestAreas);
   m_testAreaFlag = testareacb->GetValue ();
   HideShowClassAreasItem ();
   
   CheckAreaSettings ();
   //    this->Layout ();
   //    this->Fit ();
   AdjustDlgLayout ();
}   // end "OnTestAreas"



void CMClassifyDialog::OnImageArea (wxCommandEvent& event)

{
   SignedByte handleStatus;
   
   
   wxCheckBox* imageareacb = (wxCheckBox*)FindWindow (IDC_ImageArea);
   m_imageAreaFlag = imageareacb->GetValue ();
   HideShowAreaItems (m_imageAreaFlag);
   
   // Determine if this is the entire area and set the
   // to entire image icon.
   
   if (m_imageAreaFlag){
      m_dialogSelectArea.imageWindowInfoPtr =
      (WindowInfoPtr)GetHandleStatusAndPointer (m_targetWindowInfoHandle,
                                                &handleStatus,
                                                kNoMoveHi);
      
      SetEntireImageButtons (
                             NULL,
                             m_LineStart,
                             m_LineEnd,
                             m_ColumnStart,
                             m_ColumnEnd);
      
      MHSetState (m_targetWindowInfoHandle, handleStatus);
      
      m_dialogSelectArea.imageWindowInfoPtr = NULL;
      
   }   // end ""
   
   
   ClassifyDialogSetThresholdItems (this,
                                    m_classificationProcedure,
                                    m_imageAreaFlag,
                                    m_createProbabilityFileFlag,
                                    m_thresholdResultsFlag,
                                    m_thresholdAllowedFlag);
   
   CheckAreaSettings ();
   
}   // end "OnImageArea"



void CMClassifyDialog::CheckAreaSettings (void){
   Boolean enableFlag = FALSE;
   
   
   if (m_trainingAreaFlag ||
       m_trainingAreaLOOFlag ||
       m_testAreaFlag ||
       m_imageAreaFlag)
      enableFlag = TRUE;
   
   wxButton* okbutton = (wxButton*)FindWindow (wxID_OK);
   okbutton->Enable (enableFlag);
   //GetDlgItem (IDOK)->EnableWindow (enableFlag);
   
}   // end "CheckAreaSettings"

void CMClassifyDialog::OnFeatureTransformation (wxCommandEvent& event)

{
   wxCheckBox* featurecb = (wxCheckBox*)FindWindow (IDC_FeatureTransformation);
   m_featureTransformationFlag = featurecb->GetValue ();
   
   CheckFeatureTransformationDialog (this,
                                     m_featureTransformAllowedFlag,
                                     IDC_FeatureTransformation,
                                     IDC_ChannelPrompt,
                                     (SInt16*)&m_featureTransformationFlag);
   
   m_channelSelection = UpdateDialogFeatureParameters (
                                                       m_featureTransformationFlag,
                                                       &m_localActiveNumberFeatures,
                                                       &m_localActiveFeaturesPtr,
                                                       m_localNumberFeatures,
                                                       m_localFeaturesPtr,
                                                       gProjectInfoPtr->numberStatisticsChannels,
                                                       m_localNumberTransformFeatures,
                                                       m_localTransformFeaturesPtr,
                                                       gTransformationMatrix.numberFeatures);
   
}   // end "OnFeatureTransformation"



void CMClassifyDialog::OnCreateProbabilityFile (wxCommandEvent& event)
{
   wxCheckBox* probfilecb = (wxCheckBox*)FindWindow (IDC_CreateProbabilityFile);
   m_createProbabilityFileFlag = probfilecb->GetValue ();
   
}   // end "OnCreateProbabilityFile"



void CMClassifyDialog::OnSelendokAreasCombo (wxCommandEvent& event)

{
   HandleClassesMenu (&m_localNumberClassAreas,
                      (SInt16*)m_classAreaListPtr,
                      1,
                      (SInt16)gProjectInfoPtr->numberStatisticsClasses,
                      IDC_AreasCombo,
                      &m_classAreaSelection);
   
}   // end "OnSelendokAreasCombo"



void CMClassifyDialog::OnSelendokPaletteCombo (wxCommandEvent& event)

{
   int lastPaletteSelection;
   
   lastPaletteSelection = m_paletteSelection;
   wxComboBox* palettec = (wxComboBox*)FindWindow (IDC_PaletteCombo);
   m_paletteSelection = palettec->GetSelection ();
   
   if (m_paletteSelection + 1 == kFalseColors){
      if (!FalseColorPaletteDialog ()){
         if (lastPaletteSelection != m_paletteSelection){
            m_paletteSelection = lastPaletteSelection;
            //DDX_CBIndex (m_dialogToPtr, IDC_PaletteCombo, m_paletteSelection);
            palettec->SetSelection (m_paletteSelection);
            
         }   // end "if (lastPaletteSelection != m_paletteSelection)"
         
         m_paletteSelection = lastPaletteSelection;
         
      }   // end "else !FalseColorPaletteDialog ()"
      
   }   // end "if (m_paletteSelection+1 == kFalseColors)"
   
}   // end "OnSelendokPaletteCombo"


void CMClassifyDialog::OnDropdownPaletteCombo (wxCommandEvent& event){
   SetUpPalettePopUpMenu (this);
   
   m_paletteSelection = gProjectInfoPtr->imagePalettePopupMenuSelection - 1;
   //DDX_CBIndex (m_dialogToPtr, IDC_PaletteCombo, m_paletteSelection);
   wxComboBox* palettec = (wxComboBox*)FindWindow (IDC_PaletteCombo);
   palettec->SetSelection (m_paletteSelection);
}   // end "OnDropdownPaletteCombo"



void CMClassifyDialog::OnListOptions (wxCommandEvent& event){
   SetDLogControlHilite (this, wxID_OK, 255);
   
   ListResultsOptionsDialog (
                             &m_listResultsTrainingCode,
                             &m_listResultsTestCode);
   
   SetDLogControlHilite (this, wxID_OK, 0);
   
}   // end "OnListOptions"

void CMClassifyDialog::OnChangeCorrelationCoefficient (wxCommandEvent& event)

{
   wxTextCtrl* correlationcoeff = (wxTextCtrl*)FindWindow (IDC_CorrelationCoefficient);
   wxString m_correlationThresholdString = correlationcoeff->GetValue ();
   m_correlationThresholdString.ToDouble (&m_correlationThreshold);
   
   //   if (thresholdValue > 1)
   //      RealNumberErrorAlert (saveCorrelationThreshold, dialogPtr, 46, 4);
   
   if (m_correlationThreshold >= 0 && m_correlationThreshold <= 1){
      m_saveCorrelationThreshold = m_correlationThreshold;
      m_angleThreshold = (float)(
                                 acos (m_saveCorrelationThreshold)*kRadiansToDegrees);
      m_saveAngleThreshold = m_angleThreshold;
      //DDX_Text (m_dialogToPtr, IDC_CorrelationThresold, m_angleThreshold);
      wxTextCtrl* correlationthres = (wxTextCtrl*)FindWindow (IDC_CorrelationThresold);
      correlationthres->ChangeValue (wxString::Format (wxT("%f"), m_angleThreshold));
      
   }   // end "if (m_correlationThreshold >= 0 && ..."
   
}   // end "OnChangeCorrelationCoefficient"



void CMClassifyDialog::OnChangeCorrelationThresold (wxCommandEvent& event)

{
   wxTextCtrl* correlationthres = (wxTextCtrl*)FindWindow (IDC_CorrelationThresold);
   wxString correlationthresString = correlationthres->GetValue ();
   correlationthresString.ToDouble (&m_angleThreshold);
   //   if (thresholdValue > 180)
   //      RealNumberErrorAlert (saveAngleThreshold, dialogPtr, 47, 1);
   
   if (m_angleThreshold >= 0 && m_angleThreshold <= 180){
      m_saveAngleThreshold = m_angleThreshold;
      m_correlationThreshold = (float)cos (
                                           m_angleThreshold * kDegreesToRadians);
      m_saveCorrelationThreshold = m_correlationThreshold;
      //DDX_Text (m_dialogToPtr, IDC_CorrelationCoefficient, m_correlationThreshold);
      wxTextCtrl* correlationcoef = (wxTextCtrl*)FindWindow (IDC_CorrelationCoefficient);
      correlationcoef->ChangeValue (wxString::Format (wxT("%f"), m_correlationThreshold));
   }   // end "if (thresholdValue >= 0 && ..."
   
}   // end "OnChangeCorrelationThresold"



void CMClassifyDialog::OnSelendokTargetCombo (wxCommandEvent& event){
   int savedFileNamesSelection;
   
   Boolean checkOKFlag,
   createImageOverlayFlag;
   
   
   savedFileNamesSelection = m_fileNamesSelection;
   wxComboBox* targetc = (wxComboBox*)FindWindow (IDC_TargetCombo);
   m_fileNamesSelection = targetc->GetSelection ();
   
   if (m_fileNamesSelection != savedFileNamesSelection){
      createImageOverlayFlag = m_createImageOverlayFlag;
      ClassifyDialogOnTargetFile (this,
                                  m_fileNamesSelection + 1,
                                  &m_targetWindowInfoHandle,
                                  &checkOKFlag,
                                  &m_dialogSelectArea,
                                  &createImageOverlayFlag);
      m_createImageOverlayFlag = createImageOverlayFlag;
      
      // Recheck the image area items. Some may have gotten displayed
      // when they should be hidden.
      
      if (!m_imageAreaFlag)
         HideShowAreaItems (m_imageAreaFlag);
      
      if (checkOKFlag)
      {
         wxCheckBox* traincb = (wxCheckBox*)FindWindow (IDC_Training);
         wxCheckBox* imagecb = (wxCheckBox*)FindWindow (IDC_TestAreas);
         wxCheckBox* diskfcb = (wxCheckBox*)FindWindow (IDC_TrainingLOO);
         wxCheckBox* probfilecb = (wxCheckBox*)FindWindow (IDC_ImageArea);
         
         m_trainingAreaFlag= traincb->GetValue ();
         m_testAreaFlag= imagecb->GetValue ();
         m_trainingAreaLOOFlag= diskfcb->GetValue ();
         m_imageAreaFlag= probfilecb->GetValue ();
         CheckAreaSettings ();
         
      }   // end "if (checkOKFlag)"
      
   }   // end "if (m_fileNamesSelection != savedFileNamesSelection)"
   
}   // end "OnSelendokTargetCombo"


void CMClassifyDialog::OnImageOverlay (wxCommandEvent& event)

{
   wxCheckBox* imageovercb = (wxCheckBox*)FindWindow (IDC_ImageWindowOverlay);
   m_createImageOverlayFlag= imageovercb->GetValue ();
   
   if (m_createImageOverlayFlag)
      ShowDialogItem (this, IDC_ImageOverlayCombo);
   else // m_createImageOverlayFlag
      HideDialogItem (this, IDC_ImageOverlayCombo);
   
   ClassifyDialogSetPaletteItems (this,
                                  m_outputFormatCode,
                                  m_createImageOverlayFlag);
   //    this->Layout ();
   //    this->Fit ();
   AdjustDlgLayout ();
}   // end "OnImageOverlay"



void CMClassifyDialog::OnSelendokImageOverlayCombo (wxCommandEvent& event)
{
   wxComboBox* imageoverc = (wxComboBox*)FindWindow (IDC_ImageOverlayCombo);
   m_selectImageOverlaySelection = imageoverc->GetSelection ();
}   // end "OnSelendokImageOverlayCombo"



void CMClassifyDialog::OnCloseupClassificationProcedure (wxCommandEvent& event)
{
   /*   if (m_optionKeyFlag)
    {
    SetComboItemText (IDC_ClassificationProcedure,
    m_correlationComboListItem,
    (char*)"Correlation (SAM)");
    
    CComboBox* comboBoxPtr = (CComboBox*)GetDlgItem (IDC_ClassificationProcedure);
    comboBoxPtr->SetItemData (m_correlationComboListItem, kCorrelationMode);
    
    m_optionKeyFlag = FALSE;
    
    }      // end "if (m_optionKeyFlag)"
    */
}   // end "OnCloseupClassificationProcedure"



void CMClassifyDialog::OnStnClickedStartendinterval ()
{
   // TODO: Add your control notification handler code here
   
}   // end "OnStnClickedStartendinterval"



void CMClassifyDialog::ToEntireImage (wxCommandEvent& event)
{
   SignedByte handleStatus;
   
   
   m_dialogSelectArea.imageWindowInfoPtr =
   (WindowInfoPtr)GetHandleStatusAndPointer (m_targetWindowInfoHandle,
                                             &handleStatus,
                                             kNoMoveHi);
   
   CMDialog::ToEntireImage (event);
   
   MHSetState (m_targetWindowInfoHandle, handleStatus);
   
   m_dialogSelectArea.imageWindowInfoPtr = NULL;
   
}   // end "ToEntireImage"



void CMClassifyDialog::ToSelectedImage (wxCommandEvent& event)

{
   SignedByte handleStatus;
   
   
   m_dialogSelectArea.imageWindowInfoPtr =
   (WindowInfoPtr)GetHandleStatusAndPointer (m_targetWindowInfoHandle,
                                             &handleStatus,
                                             kNoMoveHi);
   
   CMDialog::ToSelectedImage (event);
   
   MHSetState (m_targetWindowInfoHandle, handleStatus);
   
   m_dialogSelectArea.imageWindowInfoPtr = NULL;
   
}   // end "ToSelectedImage"



bool CMClassifyDialog::TransferDataFromWindow ()
{
   SInt16         returnCode = 0;
   
   
   wxTextCtrl* c_end = (wxTextCtrl*)FindWindow (IDC_ColumnEnd);
   wxTextCtrl* c_inter = (wxTextCtrl*)FindWindow (IDC_ColumnInterval);
   wxTextCtrl* c_start = (wxTextCtrl*)FindWindow (IDC_ColumnStart);
   wxTextCtrl* l_end = (wxTextCtrl*)FindWindow (IDC_LineEnd);
   wxTextCtrl* l_inter = (wxTextCtrl*)FindWindow (IDC_LineInterval);
   wxTextCtrl* l_start = (wxTextCtrl*)FindWindow (IDC_LineStart);
   
   wxTextCtrl* clsfythresh = (wxTextCtrl*)FindWindow (IDC_ThresholdValue);
   wxTextCtrl* clsfycorrcoef = (wxTextCtrl*)FindWindow (IDC_CorrelationThresold);
   wxTextCtrl* clsfycorrthresh = (wxTextCtrl*)FindWindow (IDC_CorrelationCoefficient);
   wxTextCtrl* clsfyCEMthresh = (wxTextCtrl*)FindWindow (IDC_CEMThreshold);
   
   wxCheckBox* traincb = (wxCheckBox*)FindWindow (IDC_Training);
   wxCheckBox* imagecb = (wxCheckBox*)FindWindow (IDC_ImageArea);
   wxCheckBox* diskfcb = (wxCheckBox*)FindWindow (IDC_DiskFile);
   wxCheckBox* probfilecb = (wxCheckBox*)FindWindow (IDC_CreateProbabilityFile);
   wxCheckBox* testareacb = (wxCheckBox*)FindWindow (IDC_TestAreas);
   wxCheckBox* trainloocb = (wxCheckBox*)FindWindow (IDC_TrainingLOO);
   wxCheckBox* featuretrancb = (wxCheckBox*)FindWindow (IDC_FeatureTransformation);
   wxCheckBox* thresholdcb = (wxCheckBox*)FindWindow (IDC_ThresholdResults);
   wxCheckBox* imagewindowcb = (wxCheckBox*)FindWindow (IDC_ImageWindowOverlay);
   
   wxComboBox* classc = (wxComboBox*)FindWindow (IDC_ClassCombo);
   wxComboBox* weightc = (wxComboBox*)FindWindow (IDC_WeightCombo);
   wxComboBox* symbolc = (wxComboBox*)FindWindow (IDC_SymbolCombo);
   wxComboBox* classprocc = (wxComboBox*)FindWindow (IDC_ClassificationProcedure);
   wxComboBox* areasc = (wxComboBox*)FindWindow (IDC_AreasCombo);
   wxComboBox* targetc = (wxComboBox*)FindWindow (IDC_TargetCombo);
   wxComboBox* diskc = (wxComboBox*)FindWindow (IDC_DiskCombo);
   wxComboBox* palettec = (wxComboBox*)FindWindow (IDC_PaletteCombo);
   wxComboBox* channelc = (wxComboBox*)FindWindow (IDC_ChannelCombo);
   wxComboBox* imageoverc = (wxComboBox*)FindWindow (IDC_ImageOverlayCombo);
   
   m_trainingAreaFlag = traincb->GetValue ();
   m_imageAreaFlag = imagecb->GetValue ();
   m_diskFileFlag = diskfcb->GetValue ();
   m_createProbabilityFileFlag = probfilecb->GetValue ();
   m_testAreaFlag = testareacb->GetValue ();
   m_trainingAreaLOOFlag = trainloocb->GetValue ();
   m_featureTransformationFlag= featuretrancb->GetValue ();
   m_thresholdResultsFlag = thresholdcb->GetValue ();
   m_createImageOverlayFlag = imagewindowcb->GetValue ();
   
   m_classSelection = classc->GetSelection ();
   m_classWeightsSelection = weightc->GetSelection ();
   m_symbolSelection =    symbolc->GetSelection ();
   m_classifyListSelection = classprocc->GetSelection ();
   m_classAreaSelection = areasc->GetSelection ();
   m_fileNamesSelection = targetc->GetSelection ();
   m_outputFormatCode = diskc->GetSelection ();
   m_paletteSelection = palettec->GetSelection ();
   m_channelSelection = channelc->GetSelection ();
   m_selectImageOverlaySelection = imageoverc->GetSelection ();
   
   
   m_LineStartString = l_start->GetValue ();
   m_LineEndString = l_end->GetValue ();
   m_LineIntervalString = l_inter->GetValue ();
   m_ColumnStartString = c_start->GetValue ();
   m_ColumnEndString = c_end->GetValue ();
   m_ColumnIntervalString = c_inter->GetValue ();
   
   if (m_thresholdResultsFlag)
   {
      wxComboBox* comboBoxPtr = (wxComboBox*)FindWindow (IDC_ClassificationProcedure);
      SInt64 classificationProcedure = (SInt64)((int*)comboBoxPtr->GetClientData (m_classifyListSelection));
      m_classificationProcedure = (SInt16)classificationProcedure;
      
      if (m_classificationProcedure == kCorrelationMode){
         wxString clcorrthresh = clsfycorrthresh->GetValue ();
         clcorrthresh.ToDouble (&m_correlationThreshold);
         wxString clcorrcoef = clsfycorrcoef->GetValue ();
         clcorrcoef.ToDouble (&m_angleThreshold);
      }else if (m_classificationProcedure == kCEMMode){
         wxString clCEMthresh = clsfyCEMthresh->GetValue ();
         clCEMthresh.ToDouble (&m_cemThreshold);
      }else{// m_classificationProcedure != kCEMMode || ...
         wxString clthresh = clsfythresh->GetValue ();
         clthresh.ToDouble (&m_thresholdPercent);
      }
   }
   
   //   if (m_imageAreaFlag){
   //            VerifyLineColumnStartEndValues ();
   //   }
   wxComboBox* comboBoxPtr = (wxComboBox*)FindWindow (IDC_ClassificationProcedure);
   if (m_classifyListSelection < 0)
      m_classifyListSelection = 0;
   
   if (m_classSelection < 0)
      m_classSelection = m_classSelection_Saved;
   if (m_classWeightsSelection < 0)
      m_classWeightsSelection = m_weightSelection_Saved;
   if (m_channelSelection < 0)
      m_channelSelection = m_channelSelection_Saved;
   if (m_classAreaSelection < 0)
      m_classAreaSelection = m_areaSelection_Saved;
   
   SInt64 classificationProcedure = (SInt64)((int*)comboBoxPtr->GetClientData (m_classifyListSelection));
   m_classificationProcedure = (SInt16)classificationProcedure;
   
   
   if (m_imageAreaFlag)
      returnCode = VerifyLineColumnValues (
                                           IDC_LineStart,
                                           IDC_ColumnStart,
                                           true);
   
   return (returnCode == 0);
   
}   // end "TransferDataFromWindow"



bool CMClassifyDialog::TransferDataToWindow ()

{
   wxTextCtrl* c_end = (wxTextCtrl*)FindWindow (IDC_ColumnEnd);
   wxTextCtrl* c_inter = (wxTextCtrl*)FindWindow (IDC_ColumnInterval);
   wxTextCtrl* c_start = (wxTextCtrl*)FindWindow (IDC_ColumnStart);
   wxTextCtrl* l_end = (wxTextCtrl*)FindWindow (IDC_LineEnd);
   wxTextCtrl* l_inter = (wxTextCtrl*)FindWindow (IDC_LineInterval);
   wxTextCtrl* l_start = (wxTextCtrl*)FindWindow (IDC_LineStart);
   
   wxTextCtrl* clsfythresh = (wxTextCtrl*)FindWindow (IDC_ThresholdValue);
   wxTextCtrl* clsfycorrcoef = (wxTextCtrl*)FindWindow (IDC_CorrelationThresold);
   wxTextCtrl* clsfycorrthresh = (wxTextCtrl*)FindWindow (IDC_CorrelationCoefficient);
   wxTextCtrl* clsfyCEMthresh = (wxTextCtrl*)FindWindow (IDC_CEMThreshold);
   
   wxCheckBox* traincb = (wxCheckBox*)FindWindow (IDC_Training);
   wxCheckBox* imagecb = (wxCheckBox*)FindWindow (IDC_ImageArea);
   wxCheckBox* diskfcb = (wxCheckBox*)FindWindow (IDC_DiskFile);
   wxCheckBox* probfilecb = (wxCheckBox*)FindWindow (IDC_CreateProbabilityFile);
   wxCheckBox* testareacb = (wxCheckBox*)FindWindow (IDC_TestAreas);
   wxCheckBox* trainloocb = (wxCheckBox*)FindWindow (IDC_TrainingLOO);
   wxCheckBox* featuretrancb = (wxCheckBox*)FindWindow (IDC_FeatureTransformation);
   wxCheckBox* thresholdcb = (wxCheckBox*)FindWindow (IDC_ThresholdResults);
   wxCheckBox* imagewindowcb = (wxCheckBox*)FindWindow (IDC_ImageWindowOverlay);
   
   wxComboBox* classc = (wxComboBox*)FindWindow (IDC_ClassCombo);
   wxComboBox* weightc = (wxComboBox*)FindWindow (IDC_WeightCombo);
   wxComboBox* symbolc = (wxComboBox*)FindWindow (IDC_SymbolCombo);
   wxComboBox* classprocc = (wxComboBox*)FindWindow (IDC_ClassificationProcedure);
   wxComboBox* areasc = (wxComboBox*)FindWindow (IDC_AreasCombo);
   wxComboBox* targetc = (wxComboBox*)FindWindow (IDC_TargetCombo);
   wxComboBox* diskc = (wxComboBox*)FindWindow (IDC_DiskCombo);
   wxComboBox* palettec = (wxComboBox*)FindWindow (IDC_PaletteCombo);
   wxComboBox* channelc = (wxComboBox*)FindWindow (IDC_ChannelCombo);
   wxComboBox* imageoverc = (wxComboBox*)FindWindow (IDC_ImageOverlayCombo);
   
   
   if (m_covarianceEstimate != kNoCovarianceUsed)
   {
      SetComboItemText (IDC_ClassificationProcedure,
                        m_correlationComboListItem,
                        (char*)"Correlation (SAM)...");
   }
   
   traincb->SetValue (m_trainingAreaFlag);
   imagecb->SetValue (m_imageAreaFlag);
   diskfcb->SetValue (m_diskFileFlag);
   probfilecb->SetValue (m_createProbabilityFileFlag);
   testareacb->SetValue (m_testAreaFlag);
   trainloocb->SetValue (m_trainingAreaLOOFlag);
   featuretrancb->SetValue (m_featureTransformationFlag);
   thresholdcb->SetValue (m_thresholdResultsFlag);
   imagewindowcb->SetValue (m_createImageOverlayFlag);
   
   classc->SetSelection (m_classSelection);
   if (m_classWeightsSelection >= 0)
      weightc->SetSelection (m_classWeightsSelection);
   if (m_symbolSelection >= 0)
      symbolc->SetSelection (m_symbolSelection);
   classprocc->SetSelection (m_classifyListSelection);
   areasc->SetSelection (m_classAreaSelection);
   
   int fileNamesSelection = 0;
   if (m_fileNamesSelection > 0)
      fileNamesSelection = m_fileNamesSelection;
   targetc->SetSelection (fileNamesSelection);
   
   diskc->SetSelection (m_outputFormatCode);
   palettec->SetSelection (m_paletteSelection);
   channelc->SetSelection (m_channelSelection);
   imageoverc->SetSelection (m_selectImageOverlaySelection);
   
   
   c_end->ChangeValue (wxString::Format (wxT("%i"), m_ColumnEnd));
   c_inter->ChangeValue (wxString::Format (wxT("%i"), m_ColumnInterval));
   c_start->ChangeValue (wxString::Format (wxT("%i"), m_ColumnStart));
   l_end->ChangeValue (wxString::Format (wxT("%i"), m_LineEnd));
   l_inter->ChangeValue (wxString::Format (wxT("%i"), m_LineInterval));
   l_start->ChangeValue (wxString::Format (wxT("%i"), m_LineStart));
   
   
   clsfythresh->ChangeValue (wxString::Format (wxT("%.1f"), m_thresholdPercent));
   clsfycorrthresh->ChangeValue (wxString::Format (wxT("%.4f"), m_correlationThreshold));
   clsfycorrcoef->ChangeValue (wxString::Format (wxT("%.1f"), m_angleThreshold));
   clsfyCEMthresh->ChangeValue (wxString::Format (wxT("%.1f"), m_cemThreshold));
   
   return true;
   
}   // end "TransferDataToWindow"
