//	 									MultiSpec
//
//					Laboratory for Applications of Remote Sensing
//									Purdue University
//								West Lafayette, IN 47907
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	File:						SProjectComputeStatistics.cpp
//
//	Authors:					Larry L. Biehl
//
//	Revision date:			02/07/2018
//
//	Language:				C
//
//	System:					Linux, Macintosh, and Windows Operating Systems	
//
//	Brief description:	This file contains functions that compute field and 
//								class statistics.
//
//	Functions in file:	void 					AddToClassChannelStatistics
//								Boolean 				CheckIfClassMaskStatsUpToDate
//								Boolean 				CheckIfMaskStatsUpToDate
//								Boolean 				CheckIfProjectMaskStatsUpToDate
//								void 					ClearClassStatisticsMemory
//								void 					ClearFieldStatisticsMemory
//								void 					ClearProjectStatisticsMemory
//								void 					CombineFieldChannelStatistics
//								void 					CombineFieldStatistics
//								void 					ComputeCorrelationFromCovMatrix
//								void 					ComputeVarianceVector
//								Boolean 				FinishClassStatsUpdate
// 							void 					FinishProjectStatsUpdate
//								void 					FinishClassMaskStatsUpdate
//								void 					FinishFieldMaskStatsUpdate
// 							void 					FinishProjectMaskStatsUpdate
//								Boolean				GetClassChannelStatistics
//								void 					GetClassCovarianceMatrix
//								void 					GetClassMeanVector
//								Boolean 				GetClassSumsSquares
//								Boolean 				GetCommonCovariance
//								Boolean 				GetEigenStatisticsFeatures
//								UInt32 				GetNumberOfPixelsLoadedInClass
//								void 					GetTransformedClassCovarianceMatrix
//								void					InitializeChannelMaximums
//								void					InitializeChannelMinimums
//								void 					ReduceChanStatsVector
//								void 					ReduceStdDevVector
//								void 					ResetZeroVariances
//								void 					SetClassCovarianceStatsToUse
//								void					SetProjectCovarianceStatsToUse
//								Boolean 				SetupModifiedStatsMemory
//								Boolean 				SetupStatsMemory
//								SInt16	 			UpdateClassAreaStats
//								SInt16	 			UpdateFieldAreaStats
//								SInt16	 			UpdateProjectAreaStats
//								SInt16	 			UpdateStatsControl
//
//	Include files:			"MultiSpecHeaders"
//								"multiSpec.h"
//
//------------------------------------------------------------------------------------

#include "SMultiSpec.h" 

#if defined multispec_lin
	#include "SMultiSpec.h"
#endif
  
#if defined multispec_mac
#endif	// defined multispec_mac  
  	
#if defined multispec_win
#endif	// defined multispec_win    

//#include "SExtGlob.h"
#include <map>
#include <random>
#include "svm.h"

typedef dlib::matrix<double,0,1> feature_vector_type; // Must be dlib::matrix or some kind of sparse
class three_class_classifier_problem : public dlib::structural_svm_problem_threaded<column_vector, feature_vector_type>
{
   /*!
    Now we arrive at the meat of this example program.  To use dlib's structural SVM
    solver you need to define an object which tells the structural SVM solver what to
    do for your problem.  In this example, this is done by defining the three_class_classifier_problem
    object which inherits from structural_svm_problem_threaded.  Before we get into the
    details, we first discuss some background information on structural SVMs.
    
    A structural SVM is a supervised machine learning method for learning to predict
    complex outputs.  This is contrasted with a binary classifier which makes only simple
    yes/no predictions.  A structural SVM, on the other hand, can learn to predict
    complex outputs such as entire parse trees or DNA sequence alignments.  To do this,
    it learns a function F(x,y) which measures how well a particular data sample x
    matches a label y, where a label is potentially a complex thing like a parse tree.
    However, to keep this example program simple we use only a 3 category label output.
    
    At test time, the best label for a new x is given by the y which maximizes F(x,y).
    To put this into the context of the current example, F(x,y) computes the score for
    a given sample and class label.  The predicted class label is therefore whatever
    value of y which makes F(x,y) the biggest.  This is exactly what predict_label()
    does.  That is, it computes F(x,0), F(x,1), and F(x,2) and then reports which label
    has the biggest value.
    
    At a high level, a structural SVM can be thought of as searching the parameter space
    of F(x,y) for the set of parameters that make the following inequality true as often
    as possible:
    F(x_i,y_i) > max{over all incorrect labels of x_i} F(x_i, y_incorrect)
    That is, it seeks to find the parameter vector such that F(x,y) always gives the
    highest score to the correct output.  To define the structural SVM optimization
    problem precisely, we first introduce some notation:
    - let PSI(x,y)    == the joint feature vector for input x and a label y.
    - let F(x,y|w)    == dot(w,PSI(x,y)).
    (we use the | notation to emphasize that F() has the parameter vector of
    weights called w)
    - let LOSS(idx,y) == the loss incurred for predicting that the idx-th training
    sample has a label of y.  Note that LOSS() should always be >= 0 and should
    become exactly 0 when y is the correct label for the idx-th sample.  Moreover,
    it should notionally indicate how bad it is to predict y for the idx'th sample.
    - let x_i == the i-th training sample.
    - let y_i == the correct label for the i-th training sample.
    - The number of data samples is N.
    
    Then the optimization problem solved by dlib's structural SVM solver is the following:
    Minimize: h(w) == 0.5*dot(w,w) + C*R(w)
    
    Where R(w) == sum from i=1 to N: 1/N * sample_risk(i,w)
    and sample_risk(i,w) == max over all Y: LOSS(i,Y) + F(x_i,Y|w) - F(x_i,y_i|w)
    and C > 0
    
    You can think of the sample_risk(i,w) as measuring the degree of error you would make
    when predicting the label of the i-th sample using parameters w.  That is, it is zero
    only when the correct label would be predicted and grows larger the more "wrong" the
    predicted output becomes.  Therefore, the objective function is minimizing a balance
    between making the weights small (typically this reduces overfitting) and fitting the
    training data.  The degree to which you try to fit the data is controlled by the C
    parameter.
    
    For a more detailed introduction to structured support vector machines you should
    consult the following paper:
    Predicting Structured Objects with Support Vector Machines by
    Thorsten Joachims, Thomas Hofmann, Yisong Yue, and Chun-nam Yu
    
    !*/
   
public:
   
   // Finally, we come back to the code.  To use dlib's structural SVM solver you need to
   // provide the things discussed above.  This is the number of training samples, the
   // dimensionality of PSI(), as well as methods for calculating the loss values and
   // PSI() vectors.  You will also need to write code that can compute: max over all Y:
   // LOSS(i,Y) + F(x_i,Y|w).  In particular, the three_class_classifier_problem class is
   // required to implement the following four virtual functions:
   //   - get_num_dimensions()
   //   - get_num_samples()
   //   - get_truth_joint_feature_vector()
   //   - separation_oracle()
   
   
   // But first, we declare a constructor so we can populate our three_class_classifier_problem
   // object with the data we need to define our machine learning problem.  All we do here
   // is take in the training samples and their labels as well as a number indicating how
   // many threads the structural SVM solver will use.  You can declare this constructor
   // any way you like since it is not used by any of the dlib tools.
   three_class_classifier_problem (
                                   const std::vector<sample_type>& samples_,
                                   const std::vector<int>& labels_,
                                   const unsigned long num_threads
                                   ) :
   structural_svm_problem_threaded<column_vector, feature_vector_type>(num_threads),
   samples(samples_),
   labels(labels_)
   {}
   
   feature_vector_type make_psi (
                                 const sample_type& x,
                                 const int label
                                 ) const
   /*!
    ensures
    - returns the vector PSI(x,label)
    !*/
   {
      // All we are doing here is taking x, which is a 3 dimensional sample vector in this
      // example program, and putting it into one of 3 places in a 9 dimensional PSI
      // vector, which we then return.  So this function returns PSI(x,label).  To see why
      // we setup PSI like this, recall how predict_label() works.  It takes in a 9
      // dimensional weight vector and breaks the vector into 3 pieces.  Each piece then
      // defines a different classifier and we use them in a one-vs-all manner to predict
      // the label.  So now that we are in the structural SVM code we have to define the
      // PSI vector to correspond to this usage.  That is, we need to setup PSI so that
      // argmax_y dot(weights,PSI(x,y)) == predict_label(weights,x).  This is how we tell
      // the structural SVM solver what kind of problem we are trying to solve.
      //
      // It's worth emphasizing that the single biggest step in using a structural SVM is
      // deciding how you want to represent PSI(x,label).  It is always a vector, but
      // deciding what to put into it to solve your problem is often not a trivial task.
      // Part of the difficulty is that you need an efficient method for finding the label
      // that makes dot(w,PSI(x,label)) the biggest.  Sometimes this is easy, but often
      // finding the max scoring label turns into a difficult combinatorial optimization
      // problem.  So you need to pick a PSI that doesn't make the label maximization step
      // intractable but also still well models your problem.
      //
      // Finally, note that make_psi() is a helper routine we define in this example.  In
      // general, you are not required to implement it.  That is, all you must implement
      // are the four virtual functions defined below.
      
      
      // So let's make an empty 9-dimensional PSI vector
      feature_vector_type psi(get_num_dimensions());
      psi = 0; // zero initialize it
      
      // Now put a copy of x into the right place in PSI according to its label.  So for
      // example, if label is 1 then psi would be:  [0 0 0 x(0) x(1) x(2) 0 0 0]
      int startIdx = label*samples[0].size();
      int endIdx = (label+1)*samples[0].size()-1;
      set_rowm(psi,dlib::range(startIdx,endIdx)) = x;
      //if (label == 0)
        // set_rowm(psi,dlib::range(0,2)) = x;
      //else if (label == 1)
        // set_rowm(psi,dlib::range(3,5)) = x;
      //else // the label must be 2
       //  set_rowm(psi,dlib::range(6,8)) = x;
      
      return psi;
   }
   
   // We need to declare the dimensionality of the PSI vector (this is also the
   // dimensionality of the weight vector we are learning).  Similarly, we need to declare
   // the number of training samples.  We do this by defining the following virtual
   // functions.
   virtual long get_num_dimensions () const { return samples[0].size() * labels.size(); }
   virtual long get_num_samples ()    const { return samples.size(); }
   
   // In get_truth_joint_feature_vector(), all you have to do is output the PSI() vector
   // for the idx-th training sample when it has its true label.  So here it outputs
   // PSI(samples[idx], labels[idx]).
   virtual void get_truth_joint_feature_vector (
                                                long idx,
                                                feature_vector_type& psi
                                                ) const
   {
      psi = make_psi(samples[idx], labels[idx]);
   }
   
   // separation_oracle() is more interesting.  dlib's structural SVM solver will call
   // separation_oracle() many times during the optimization.  Each time it will give it
   // the current value of the parameter weights and separation_oracle() is supposed to
   // find the label that most violates the structural SVM objective function for the
   // idx-th sample.  Then the separation oracle reports the corresponding PSI vector and
   // loss value.  To state this more precisely, the separation_oracle() member function
   // has the following contract:
   //   requires
   //       - 0 <= idx < get_num_samples()
   //       - current_solution.size() == get_num_dimensions()
   //   ensures
   //       - runs the separation oracle on the idx-th sample.  We define this as follows:
   //           - let X           == the idx-th training sample.
   //           - let PSI(X,y)    == the joint feature vector for input X and an arbitrary label y.
   //           - let F(X,y)      == dot(current_solution,PSI(X,y)).
   //           - let LOSS(idx,y) == the loss incurred for predicting that the idx-th sample
   //             has a label of y.  Note that LOSS() should always be >= 0 and should
   //             become exactly 0 when y is the correct label for the idx-th sample.
   //
   //               Then the separation oracle finds a Y such that:
   //                   Y = argmax over all y: LOSS(idx,y) + F(X,y)
   //                   (i.e. It finds the label which maximizes the above expression.)
   //
   //               Finally, we can define the outputs of this function as:
   //               - #loss == LOSS(idx,Y)
   //               - #psi == PSI(X,Y)
   virtual void separation_oracle (
                                   const long idx,
                                   const column_vector& current_solution,
                                   scalar_type& loss,
                                   feature_vector_type& psi
                                   ) const
   {
      // Note that the solver will use multiple threads to make concurrent calls to
      // separation_oracle(), therefore, you must implement it in a thread safe manner
      // (or disable threading by inheriting from structural_svm_problem instead of
      // structural_svm_problem_threaded).  However, if your separation oracle is not
      // very fast to execute you can get a very significant speed boost by using the
      // threaded solver.  In general, all you need to do to make your separation oracle
      // thread safe is to make sure it does not modify any global variables or members
      // of three_class_classifier_problem.  So it is usually easy to make thread safe.
      
      column_vector scores;
      scores.set_size(labels.size());
      
      // compute scores for each of the three classifiers
      for(int i = 0; i < labels.size(); i++)
      {
         int startIdx = i*samples[0].size();
         int endIdx = (i+1)*samples[0].size()-1;
         scores(i) = dot(rowm(current_solution, dlib::range(startIdx,endIdx)),  samples[idx]);
      }
      
      //scores = dot(rowm(current_solution, dlib::range(0,2)),  samples[idx]),
      //dot(rowm(current_solution, dlib::range(3,5)),  samples[idx]),
      //dot(rowm(current_solution, dlib::range(6,8)),  samples[idx]);
      
      // Add in the loss-augmentation.  Recall that we maximize LOSS(idx,y) + F(X,y) in
      // the separate oracle, not just F(X,y) as we normally would in predict_label().
      // Therefore, we must add in this extra amount to account for the loss-augmentation.
      // For our simple multi-class classifier, we incur a loss of 1 if we don't predict
      // the correct label and a loss of 0 if we get the right label.
      
      for(int i = 0; i < labels.size(); i++)
      {
         if (labels[idx] != i)
            scores(i) += 1;
      }
      //if (labels[idx] != 0)
        // scores(0) += 1;
      //if (labels[idx] != 1)
       //  scores(1) += 1;
     // if (labels[idx] != 2)
       //  scores(2) += 1;
      
      // Now figure out which classifier has the largest loss-augmented score.
      const int max_scoring_label = index_of_max(scores);
      // And finally record the loss that was associated with that predicted label.
      // Again, the loss is 1 if the label is incorrect and 0 otherwise.
      if (max_scoring_label == labels[idx])
         loss = 0;
      else
         loss = 1;
      
      // Finally, compute the PSI vector corresponding to the label we just found and
      // store it into psi for output.
      psi = make_psi(samples[idx], max_scoring_label);
   }
   
private:
   
   // Here we hold onto the training data by reference.  You can hold it by value or by
   // any other method you like.
   const std::vector<sample_type>& samples;
   const std::vector<int>& labels;
};

// ----------------------------------------------------------------------------------------

// This function puts it all together.  In here we use the three_class_classifier_problem
// along with dlib's oca cutting plane solver to find the optimal weights given our
// training data.
column_vector train_three_class_classifier (
                                            const std::vector<sample_type>& samples,
                                            const std::vector<int>& labels,
                                            float convergence_rate
                                            )
{
   std::clock_t start;
   double duration;
   start = std::clock();
   const unsigned long num_threads = 8;
   three_class_classifier_problem problem(samples, labels, num_threads);
   
   // Before we run the solver we set up some general parameters.  First,
   // you can set the C parameter of the structural SVM by calling set_c().
   problem.set_c(1);
   
   // The epsilon parameter controls the stopping tolerance.  The optimizer will run until
   // R(w) is within epsilon of its optimal value. If you don't set this then it defaults
   // to 0.001.
   //problem.set_epsilon(0.0001);
   problem.set_epsilon(convergence_rate);
   // Uncomment this and the optimizer will print its progress to standard out.  You will
   // be able to see things like the current risk gap.  The optimizer continues until the
   // risk gap is below epsilon.
   problem.be_verbose();
   
   // The optimizer uses an internal cache to avoid unnecessary calls to your
   // separation_oracle() routine.  This parameter controls the size of that cache.
   // Bigger values use more RAM and might make the optimizer run faster.  You can also
   // disable it by setting it to 0 which is good to do when your separation_oracle is
   // very fast.  If you don't call this function it defaults to a value of 5.
   problem.set_max_cache_size(20);
   
   
   column_vector weights;
   // Finally, we create the solver and then run it.
   dlib::oca solver;
   solver(problem, weights);
   duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
   
   std::cout<<"DURATION: "<< duration <<'\n';
   // Alternatively, if you wanted to require that the learned weights are all
   // non-negative then you can call the solver as follows and it will put a constraint on
   // the optimization problem which causes all elements of weights to be >= 0.
   //solver(problem, weights, problem.get_num_dimensions());
   
   return weights;
}

// random forest functions

/* Counts the number of items in each class(label) in the training dataset*/
/*theMap<label, labelCount>*/
void class_counts(std::map<double, double>& theMap,
                  std::vector<double*>& sample)
{
    int numChannels = gClassifySpecsPtr->numberChannels;
    std::map<double,double>::iterator it;
    for(int i = 0; i < sample.size(); i++)
    {
        // the last item of sample array is the class number
        it = theMap.find(sample[i][numChannels]);
        if(it == theMap.end())
        {
            theMap.insert(std::pair<double,double>(sample[i][numChannels], 1));
        }
        else
        {
            it->second ++;
        }
    }
}

bool Question(int column, double value, double *data)
{
    if(data[column] >= value)
        return true;
    else
        return false;
}

void partition(std::vector<double*>& sample,
               std::vector<double*>&true_rows,
               std::vector<double*>&false_rows,
               int column, double value)
{
    for(int i = 0; i < sample.size(); i++)
    {
        if(Question(column, value, sample[i]))
        {
            true_rows.push_back(sample[i]);
        }
        else
        {
            false_rows.push_back(sample[i]);
        }
    }
}

double gini(std::vector<double*>& sample)
{
    std::map <double, double> counts;
    class_counts(counts, sample);
    double impurity = 1.0;
    std::map<double,double>::iterator it;
    //std::cout << "sSize:" << sample.size() << ",cSize:" << counts.size() << std::endl;
    for(it = counts.begin(); it != counts.end(); it++)
    {
        //std::cout << "label:" << it->first << ",count:" << it->second << std::endl;
        double prob_of_lbl = it->second / (double)(sample.size());
        impurity -= ((prob_of_lbl) * (prob_of_lbl));
    }
    counts.clear();
    return impurity;
}

double info_gain(std::vector<double*>& left,
                std::vector<double*>& right,
                double current_uncertainty)
{
    double p = double(left.size()) / (left.size() + right.size());
    return current_uncertainty - p * gini(left) - (1 - p) * gini(right);
}

double find_best_split(std::vector<double*>& data,double *rVal, int *rCol)
{
    double best_gain = 0;
    double current_uncertainty = gini(data);
    int n_feature = gClassifySpecsPtr->numberChannels;
    double gain = 0;
    //std::cout << "gini:" << current_uncertainty;
    //std::cout << ",dataSize:" << data.size() << std::endl;
    std::set<double> values;
    std::vector<double*> true_rows;
    std::vector<double*> false_rows;
    std::set<double>::iterator it;
    
    for(int col = 0; col < n_feature; col++)
    {
        for(int i = 0; i < data.size(); i++)
        {
            //std::cout << "value:" << data[i][col] << ",label:" << data[i][n_feature] << ",col:" << col << std::endl;
            values.insert(data[i][col]);
        }

        for(it = values.begin(); it != values.end(); it++)
        {
            // try splitting the dataset
            partition(data, true_rows,false_rows,
                      col, *it);

            //std::cout << "true:" << true_rows.size() << ",false:" << false_rows.size() << std::endl;
            if(true_rows.size() == 0 || false_rows.size() == 0)
            {
                continue;
            }
            gain = info_gain(true_rows, false_rows, current_uncertainty);
            
            if(gain >= best_gain)
            {
                best_gain = gain;
                *rCol = col;
                *rVal = *it;
                //std::cout << "col:" << col << ",val:" << *it << ",gain:"<< gain;
                //std::cout << ",tSize:" << true_rows.size() << ",fSize:"<< false_rows.size() << std::endl;
            }
            //std::cout << "true:" << true_rows.size() << "," << false_rows.size() << ", gain:"<< gain << std::endl;
            true_rows.clear();
            false_rows.clear();
        }
        values.clear();
    }
    //std::cout << "best_gain:" << best_gain << "," << *rCol << "," << *rVal<< std::endl;
    return best_gain;
}


RFNode* build_tree(RFNode *root, std::vector<double*> data,
                        double val, int col,
                        std::vector<double*>true_rows,
                        std::vector<double*>false_rows)
{
    double gain = 0;
    true_rows.clear();
    false_rows.clear();
    
    gain = find_best_split(data, &val, &col);
#if 1
    partition(data, true_rows,false_rows,
              col, val);
    // reach leaf node
    if(gain == 0)
    {
        //std::cout << "FIN2:" << data[0][gClassifySpecsPtr->numberChannels] << ",val:" << val <<std::endl;
        root = new RFNode(data[0][gClassifySpecsPtr->numberChannels]);
        return root;
        
    }
    root = new RFNode(val);
    //std::cout << "val:" << val <<std::endl;
    
    data = true_rows;
    root->left = build_tree(root->left, data,
                            val, col,
                            true_rows,
                            false_rows);
    
    data = false_rows;
    root->right = build_tree(root->right, data,
                             val, col,
                             true_rows,
                             false_rows);
#endif
    return root;
}


extern Boolean ResetForAllVariancesEqual (
				HCovarianceStatisticsPtr		covariancePtr,
				Boolean								squareMatrixFlag, 
				SInt16								outputStatisticsCode, 
				UInt32								numberFeatures);


			// Prototypes for routines in this file that are only called by		
			// other routines in this file.													
			
void 		AddToClassChannelStatistics (
				UInt16								numberOutputChannels,
				HChannelStatisticsPtr			outChannelStatsPtr, 
				UInt16								numberInputChannels, 
				UInt16*								channelListPtr, 
				HChannelStatisticsPtr			inChannelStatsPtr, 
				Boolean								initializeFlag);

Boolean 	CheckIfClassMaskStatsUpToDate (
				UInt32								classNumber);

Boolean 	CheckIfMaskStatsUpToDate (
				SInt16								statsUpdateCode,
				UInt32								classNumber,
				UInt32								fieldNumber);

Boolean 	CheckIfProjectMaskStatsUpToDate (void);

void 		ClearClassStatisticsMemory (
				UInt32								classNumber);
							
void 		ClearFieldStatisticsMemory (
				UInt32								fieldNumber);
							
void 		ClearProjectStatisticsMemory (void);
			
void 		CombineFieldChannelStatistics (
				UInt16 								numberOutputChannels,
				UInt16* 								channelListPtr, 
				HChannelStatisticsPtr 			classChannelStatsPtr, 
				UInt16 								classNumber);

void 		CombineFieldStatistics (
				UInt16 								numberOutputChannels,
				UInt16* 								outputChannelListPtr, 
				HChannelStatisticsPtr 			outputChannelStatsPtr, 
				HSumSquaresStatisticsPtr 		outputSumSquaresPtr, 
				UInt16 								classNumber, 
				Boolean 								squareOutputMatrixFlag, 
				SInt16		 						outputStatisticsCode);

void 		ComputeVarianceVector (
				HChannelStatisticsPtr 			channelStatsPtr,
				HSumSquaresStatisticsPtr 		sumSquaresPtr, 
				HDoublePtr 							variancePtr, 
				UInt16 								numberChannels, 
				SInt64	 							numberPixels, 
				SInt16	 							statCode);

Boolean 	FinishClassStatsUpdate (
				UInt32								classNumber);
							
void 		FinishProjectStatsUpdate (void);
							
void 		FinishClassMaskStatsUpdate (
				UInt32								classNumber);
							
void 		FinishFieldMaskStatsUpdate (
				UInt32								fieldNumber);
							
void 		FinishProjectMaskStatsUpdate (void);

void 		ReduceChanStatsVector (
				HChannelStatisticsPtr 			inputMeansPtr,
				HChannelStatisticsPtr 			outputMeansPtr, 
				UInt16	 							numOutFeatures, 
				UInt16* 								featureListPtr);

void 		ReduceStdDevVector (
				HChannelStatisticsPtr			inputChannelStatsPtr,
				HDoublePtr							outputStdDevPtr, 
				SInt16								numOutFeatures, 
				SInt16*								featureListPtr);

SInt16	 UpdateClassAreaStats (
				FileIOInstructionsPtr			fileIOInstructionsPtr,
				UInt32								classNumber);

SInt16	 UpdateFieldAreaStats (
				FileIOInstructionsPtr			fileIOInstructionsPtr,
				UInt16								fieldNumber);

SInt16	 UpdateProjectAreaStats (
				FileIOInstructionsPtr			fileIOInstructionsPtr);

SInt16 	UpdateProjectMaskStats (
				SInt16								statsUpdateCode,
				FileIOInstructionsPtr			fileIOInstructionsPtr, 
				SInt32								classNumber,
				SInt32								fieldNumber,
				Boolean								checkForBadDataFlag,
				SInt16								statCode);



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void AddToClassChannelStatistics
//
//	Software purpose:	The purpose of this routine is to add the statistics
//							from the input field to that for the output class.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			GetClassChannelStatistics in statCompute.c
//
//	Coded By:			Larry L. Biehl			Date: 02/07/1992
//	Revised By:			Larry L. Biehl			Date: 02/07/1992	
														
void AddToClassChannelStatistics (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			outChannelStatsPtr, 
				UInt16								numberInputChannels, 
				UInt16*								channelListPtr, 
				HChannelStatisticsPtr			inChannelStatsPtr, 
				Boolean								initializeFlag)

{
	HChannelStatisticsPtr			lInChannelStatsPtr;
	
	UInt32								channel,
											channelListIndex,
											channelListIndexLimit,
											channelNum;
	
	
			// Initialize local variables.													
				
	channelListIndexLimit = numberOutputChannels - 1;
	channelNum = 0;
	channelListIndex = 0;
	lInChannelStatsPtr = inChannelStatsPtr;
																						
	if (numberOutputChannels == numberInputChannels)
		channelListPtr = NULL;
	
	for (channel=0; channel<numberInputChannels; channel++)
		{
		if (channelListPtr)
			{
			channelNum = channelListPtr[channelListIndex];
			lInChannelStatsPtr = &inChannelStatsPtr[channelNum];
			
			}	// end "if (channelListPtr)" 
			
		if (channel == channelNum)
			{
			if (initializeFlag)
				{
				outChannelStatsPtr->sum = lInChannelStatsPtr->sum;
				
				outChannelStatsPtr->standardDev = -1;
				
				outChannelStatsPtr->minimum = lInChannelStatsPtr->minimum;
				
				outChannelStatsPtr->maximum = lInChannelStatsPtr->maximum;
				
				}	// end "if (initializeFlag)" 
				
			else	// !initializeFlag 
				{
				outChannelStatsPtr->sum += lInChannelStatsPtr->sum;
				
				outChannelStatsPtr->minimum = 
							MIN (outChannelStatsPtr->minimum, lInChannelStatsPtr->minimum);
				
				outChannelStatsPtr->maximum = 
							MAX (outChannelStatsPtr->maximum, lInChannelStatsPtr->maximum);
					
				}	// end "else !initializeFlag" 
			
			if (channelListIndex < channelListIndexLimit)
				channelListIndex++;
				
			outChannelStatsPtr++;
			
			}	// end "if (channel == channelNum1)" 
			
		channelNum++;
		lInChannelStatsPtr++;
			
		}	// end "for (channel=0; channel<=..." 

}	// end "AddToClassChannelStatistics" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean CheckIfClassMaskStatsUpToDate
//
//	Software purpose:	The purpose of this routine is to update the
//							statistics for the given class.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:	
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 12/17/1998	

Boolean CheckIfClassMaskStatsUpToDate (
				UInt32								classNumber)

{
	HPClassNamesPtr					classNamesPtr;
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	UInt32								classStorage;
											
	SInt16								fieldNumber;
	
	
			// Initialize local variables.													
			
	classNamesPtr = gProjectInfoPtr->classNamesPtr;	
	classStorage = gProjectInfoPtr->storageClass[classNumber];
	
			// Continue only if the number of fields in the class is one or more	
			// and if the class statistics are not up-to-date							
	
	if (classNamesPtr[classStorage].numberOfTrainFields > 0)
		{
		fieldNumber = classNamesPtr[classStorage].firstFieldNumber;
		while (fieldNumber != -1)
			{
			fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];
	
					// Make certain that field is training field							
					
			if (fieldIdentPtr->fieldType == kTrainingType &&
														fieldIdentPtr->pointType == kMaskType)
				{
						// Check if field "fieldNumber" statistics are up to date
						
				if (!fieldIdentPtr->statsUpToDate)
																							return (FALSE);
																						
				}	// end "if (...->fieldType == kTrainingType && ..."
				
			fieldNumber = fieldIdentPtr->nextField;
			
			}	// end "while (fieldNumber != -1)"
			
		}	// end "if (classNamesPtr[classStorage].numberOfTrainFields > 0)"
	
			// Indicate that the class mask stats are up to date.								
			
	return (TRUE);
		
}	// end "CheckIfClassMaskStatsUpToDate" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean CheckIfMaskStatsUpToDate
//
//	Software purpose:	The purpose of this routine is to update the
//							statistics for the project.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	1 = Update completed okay
//							0 = Update did not complete because of a problem
// 
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 12/17/1998	

Boolean CheckIfMaskStatsUpToDate (
				SInt16								statsUpdateCode,
				UInt32								classNumber,
				UInt32								fieldNumber)

{
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	Boolean								returnFlag = TRUE;
	
					
	switch (statsUpdateCode)
		{
		case kUpdateProject:
			returnFlag = CheckIfProjectMaskStatsUpToDate ();
			break;
			
		case kUpdateClass:
			returnFlag = CheckIfClassMaskStatsUpToDate (classNumber);
			break;
			
		case kUpdateField:
			
			if (fieldNumber < (UInt32)gProjectInfoPtr->numberStorageFields)
				{
				fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];					
				if (fieldIdentPtr->pointType == kMaskType &&
															!fieldIdentPtr->loadedIntoClassStats)
					returnFlag = FALSE;
					
				}	// end "if (fieldNumber < (UInt32)...->numberStorageFields)"
			break;
			
		}	// end "switch (statsWindowMode)"
		
	return (returnFlag);
		
}	// end "CheckIfMaskStatsUpToDate"



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean CheckIfProjectMaskStatsUpToDate
//
//	Software purpose:	The purpose of this routine is to update the
//							statistics for the project.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	1 = Update completed okay
//							0 = Update did not complete because of a problem
// 
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 12/17/1998	

Boolean CheckIfProjectMaskStatsUpToDate ()

{
	UInt32								classIndex,
											numberClasses;
	
	
			// Initialize local variables.													
			
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	
			// Continue only if number of classes is one or more.													
	
	if (numberClasses > 0)
		{
		for (classIndex=0; classIndex<numberClasses; classIndex++)
			{
					// Check if class "class" statistics are up to date				
						
			if (!CheckIfClassMaskStatsUpToDate ((UInt32)classIndex)) 
																							return (FALSE);
			
			}	// end "for (classIndex=0; ... 
			
		}	// end "if (numberClasses > 0)" 
	
			// Indicate that project mask stats are up to date.								
			
	return (TRUE);
		
}	// end "CheckIfProjectMaskStatsUpToDate" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void CheckMatrix
//
//	Software purpose:	The purpose of this routine is to check the input matrix for
//							zero variances and all variances and covariances being equal.
//							Changes are made in the input covariance matrix so that it
//							can be inverted.
//
//	Parameters in:		
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:			GetTransformedClassCovarianceMatrix
//							GetTransformedCommonCovariance in SClassfy.cpp
//
//	Coded By:			Larry L. Biehl			Date: 02/21/2003
//	Revised By:			Larry L. Biehl			Date: 07/13/2009	

Boolean CheckMatrix (
				HDoublePtr							covariancePtr,
				Boolean								squareOutputMatrixFlag,
				SInt16								statisticsCode,
				UInt16								numberFeatures,
				UInt16								statClassNumber,
				SInt16								zeroVarianceStringIndex,
				SInt16								allCovariancesEqualIndex,
				Boolean								useListOneMessagePerClassFlag)

{
	CMFileStream* 						resultsFileStreamPtr;
	
	UInt32								classStorage;
	
	Boolean								continueFlag = TRUE,
											messageListedFlag = FALSE;
	
											
 	resultsFileStreamPtr = GetResultsFileStreamPtr (0);
	
	classStorage = gProjectInfoPtr->storageClass[statClassNumber];	
										
	if (useListOneMessagePerClassFlag && !gListOnlyOneMessagePerClassFlag)
		gProjectInfoPtr->classNamesPtr[classStorage].listMessageFlag = TRUE;
	
	if (gProjectInfoPtr->setZeroVarianceFlag)
		{
				// Set any zero variance to requested factor.						
				// This is done so that the matrix may be inverted.				
				
		if (ResetZeroVariances (covariancePtr, 
											squareOutputMatrixFlag,
											statisticsCode, 
											numberFeatures))
			{
					// List message that at least one zero variance for this class 		
					// was set to the user specified value.												
					
			continueFlag = ListClassInformationMessage (kProjectStrID,
																		zeroVarianceStringIndex,
																		resultsFileStreamPtr, 
																		gOutputForce1Code,
																		statClassNumber, 
																		continueFlag);
			
			messageListedFlag = TRUE;
											
			}	// end "if (ResetZeroVariances (covariancePtr, ..."
											
		}	// end "if (gProjectInfoPtr->setZeroVarianceFlag)"

			// Check for all variances and covariances being equal.
		
	if (continueFlag)
		{
		if (ResetForAllVariancesEqual (covariancePtr, 
													squareOutputMatrixFlag,
													statisticsCode,
													numberFeatures))
			{
				// List message that that the covariances were set to 0 because all
				// of the channels represent the same data. One could do just as well
				// using any one channel.												
				
			continueFlag = ListClassInformationMessage (kProjectStrID,
																		allCovariancesEqualIndex,
																		resultsFileStreamPtr, 
																		gOutputForce1Code,
																		statClassNumber, 
																		continueFlag);
			
			messageListedFlag = TRUE;
			
			}	// end "if (ResetForAllVariancesEqual ("
			
		}	// end "if (continueFlag)"
						
	if (useListOneMessagePerClassFlag && 
						gListOnlyOneMessagePerClassFlag && 
										messageListedFlag)
		gProjectInfoPtr->classNamesPtr[classStorage].listMessageFlag = FALSE;
		
	return (continueFlag);
		
}	// end "CheckMatrix" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ClearClassStatisticsMemory
//
//	Software purpose:	The purpose of this routine is to clear the statistics memory 
//							for the requested class.
//
//	Parameters in:		Class number (0 based).
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:	
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 12/17/1998	

void ClearClassStatisticsMemory (
				UInt32								classNumber)

{
	HChannelStatisticsPtr			classChanPtr;
	HSumSquaresStatisticsPtr		classSumSquaresPtr;
	
	HPClassNamesPtr					classNamesPtr;
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	UInt32								classStorage;
											
	SInt16								fieldNumber;
	
	
			// Make certain that input value makes sense		
	
	if (classNumber >= gProjectInfoPtr->numberStatisticsClasses)
																									return;
	
			// Initialize local variables.													
			
	classNamesPtr = gProjectInfoPtr->classNamesPtr;	
	classStorage = gProjectInfoPtr->storageClass[classNumber];
	
			// Continue only if the number of fields in the class is one or more	
			// and if the class statistics are not up-to-date							
	
	if (classNamesPtr[classStorage].numberOfTrainFields > 0 && 
										!classNamesPtr[classStorage].statsUpToDate)
		{
				// Verify that we have a correct count for the number of training
				// pixels that have been loaded into the class. It may be wrong if
				// the project has just been read from disk.
		
		classNamesPtr[classStorage].numberStatisticsPixels =  
						GetNumberOfPixelsLoadedInClass (
															&classNamesPtr[classStorage], 
															gProjectInfoPtr->fieldIdentPtr);
				
		if (gProjectInfoPtr->keepClassStatsOnlyFlag &&
									classNamesPtr[classStorage].numberStatisticsPixels == 0)
			{
			GetProjectStatisticsPointers (kClassStatsOnly, 
													classStorage, 
													&classChanPtr, 
													&classSumSquaresPtr,
													NULL,
													NULL);
													
					// Initialize the memory for the class statistics if needed. 	
		
			ZeroStatisticsMemory (classChanPtr, 
											classSumSquaresPtr,
											gProjectInfoPtr->numberStatisticsChannels,
											gProjectInfoPtr->statisticsCode,
											kTriangleOutputMatrix);	
								
			}	// end "if (gProjectInfoPtr->keepClassStatsOnlyFlag)"
		
		else	// !gProjectInfoPtr->keepClassStatsOnlyFlag
			{
			fieldNumber = classNamesPtr[classStorage].firstFieldNumber;
			
			while (fieldNumber != -1)
				{
				fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];
							
				ClearFieldStatisticsMemory (fieldNumber);
					
				fieldNumber = fieldIdentPtr->nextField;
				
				}	// end "while (fieldNumber != -1)"
				
			}	// end "else !gProjectInfoPtr->keepClassStatsOnlyFlag"
			
		}	// end "if (classNamesPtr[classStorage].numberOfTrainFields > 0)"
		
}	// end "ClearClassStatisticsMemory" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ClearFieldStatisticsMemory
//
//	Software purpose:	The purpose of this routine is to clear the statistics memory 
//							for the requested field.
//
//	Parameters in:		Field number (0 based).
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:	
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 01/07/1999	

void ClearFieldStatisticsMemory (
				UInt32								fieldNumber)

{
	HChannelStatisticsPtr			fieldChanPtr;
	HSumSquaresStatisticsPtr		fieldSumSquaresPtr;
	
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	
			// Make certain that input value makes sense									
			
	if (fieldNumber >= (UInt32)gProjectInfoPtr->numberStorageFields) 	
																									return;
																						
			// Initialize local variables.	
																						
	fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];	
					
	if (fieldIdentPtr->fieldType != kTrainingType)		
																									return;
	if (fieldIdentPtr->pointType == kClusterType)		
																									return;
																						
			// Note that if only the class statistics are being kept the memory
			// for the statistics for a specific field is initialized in the
			// 'UpdateFieldStats' routine for the rectangle and polygon defined fields.
			// For the mask defined field case, the statistics are added
			// directly to the class statistics.
																						
	if (!gProjectInfoPtr->keepClassStatsOnlyFlag && !fieldIdentPtr->statsUpToDate)
		{
		GetProjectStatisticsPointers (kFieldStatsOnly, 
												fieldIdentPtr->trainingStatsNumber,		
												&fieldChanPtr, 
												&fieldSumSquaresPtr,
												NULL,
												NULL);
								
				// Initialize the memory for the field statistics							
		
		ZeroStatisticsMemory (fieldChanPtr, 
										fieldSumSquaresPtr,
										gProjectInfoPtr->numberStatisticsChannels,
										gProjectInfoPtr->statisticsCode,
										kTriangleOutputMatrix);
									
		}	// end "if (!gProjectInfoPtr->keepClassStatsOnlyFlag && ..."
		
}	// end "ClearFieldStatisticsMemory" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ClearProjectStatisticsMemory
//
//	Software purpose:	The purpose of this routine is to update the
//							statistics for the project.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 12/17/1998	

void ClearProjectStatisticsMemory ()

{
	UInt32								classIndex,
											numberClasses;
	
	
			// Initialize local variables.													
			
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	
			// Continue only if number of classes is one or more.													
	
	if (numberClasses > 0)
		{
		for (classIndex=0; classIndex<numberClasses; classIndex++)
			ClearClassStatisticsMemory ((UInt32)classIndex);
			
		}	// end "if (numberClasses > 0)" 
		
}	// end "ClearProjectStatisticsMemory" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void CombineFieldChannelStatistics
//
//	Software purpose:	The purpose of this routine is to obtain the class
//							channels sums from the fields that belong to the
//							input class.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
//
// Called By:			GetClassChannelStatistics in statCompute.c
//
//	Coded By:			Larry L. Biehl			Date: 03/18/1993
//	Revised By:			Larry L. Biehl			Date: 03/18/1993	

void CombineFieldChannelStatistics (
				UInt16								numberOutputChannels, 
				UInt16*								channelListPtr, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				UInt16								classNumber)

{
	HChannelStatisticsPtr			fieldChanPtr;
	HPClassNamesPtr					classNamesPtr;
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	SInt32								classStorage,
											fieldStatsNumber;
											
	SInt16								fieldNumber,
											numberInputChannels;
											
	Boolean								continueFlag,
											initializeFlag;
	
	
			// Check input values.  Continue only if input parameters are within	
			// proper rangers																		
			
	continueFlag = TRUE;
	if (numberOutputChannels <= 0)  
		continueFlag = FALSE;
		
	if (classChannelStatsPtr == NULL)  
		continueFlag = FALSE;
																						
	if (!continueFlag)
																								return;
			
	numberInputChannels = gProjectInfoPtr->numberStatisticsChannels;
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	fieldIdentPtr = gProjectInfoPtr->fieldIdentPtr;
	initializeFlag = TRUE;
	
			// Get the class storage number.													
						
	classStorage = gProjectInfoPtr->storageClass[classNumber];
	
	fieldNumber = classNamesPtr[classStorage].firstFieldNumber;
	
	while (fieldNumber != -1)
		{
				// Make certain that field is training field							
				
		if (fieldIdentPtr[fieldNumber].fieldType == kTrainingType)
			{
					// Get field statistics set number.									
					
			fieldStatsNumber = fieldIdentPtr[fieldNumber].trainingStatsNumber;
			
					// Check if field "fieldNumber" statistics are up to date	
				
			if (fieldIdentPtr[fieldNumber].statsUpToDate)
				{
						// Get pointers to the memory for the first order field 	
						// statistics.															
	    		
				fieldChanPtr =  &gProjectInfoPtr->fieldChanStatsPtr[
															fieldStatsNumber*numberInputChannels];
						
				AddToClassChannelStatistics (numberOutputChannels, 
														classChannelStatsPtr,
														numberInputChannels, 
														channelListPtr, 
														fieldChanPtr,
														initializeFlag);
				
				initializeFlag = FALSE;
				
				}	// end "if (fieldIdentPtr[fieldNumber].statsUpToDate)" 
							
			}	// end "if (fieldIdentPtr[fieldNumber].field..." 
			
		fieldNumber = fieldIdentPtr[fieldNumber].nextField;
		
		}	// end "while (fieldNumber != -1)" 
	
}	// end "CombineFieldChannelStatistics" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void CombineFieldStatistics
//
//	Software purpose:	The purpose of this routine is to combine the field
//							statistics for the input class and requested channels
//							into the output matrix and vector.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			GetClassSumsSquares in statCompute.c
//
//	Coded By:			Larry L. Biehl			Date: 03/19/1993
//	Revised By:			Larry L. Biehl			Date: 03/19/1993	

void CombineFieldStatistics (
				UInt16								numberOutputChannels, 
				UInt16*								outputChannelListPtr, 
				HChannelStatisticsPtr			outputChannelStatsPtr, 
				HSumSquaresStatisticsPtr		outputSumSquaresPtr, 
				UInt16								classNumber, 
				Boolean								squareOutputMatrixFlag, 
				SInt16								outputStatisticsCode)

{
	HChannelStatisticsPtr			fieldChanPtr;
	HPClassNamesPtr					classNamesPtr;
	HPFieldIdentifiersPtr			fieldIdentPtr;
	HSumSquaresStatisticsPtr		fieldSumSquaresPtr;
	
	SInt32								classStorage,
											fieldStatsNumber;
											
	SInt16								fieldNumber,
											numberInputChannels;
	
	
			// Check input values.  Continue only if input parameters are within	
			// proper rangers																		
			
	if (numberOutputChannels <= 0 || !outputChannelStatsPtr || 
									numberOutputChannels <= 0 || !outputSumSquaresPtr)
																									return;
			
	numberInputChannels = gProjectInfoPtr->numberStatisticsChannels;
	fieldIdentPtr = gProjectInfoPtr->fieldIdentPtr;
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	
			// Get the class storage number.													
						
	classStorage = gProjectInfoPtr->storageClass[classNumber];
		
	fieldNumber = classNamesPtr[classStorage].firstFieldNumber;
	while (fieldNumber != -1)
		{
				// Make certain that field is training field							
				
		if (fieldIdentPtr[fieldNumber].fieldType == kTrainingType)
			{
					// Get field statistics set number.									
					
			fieldStatsNumber = fieldIdentPtr[fieldNumber].trainingStatsNumber;
			
					// Check if field "fieldNumber" statistics are up to date	
				
			if (fieldIdentPtr[fieldNumber].statsUpToDate)
				{
						// Get pointers to the memory for the first order and 	
						// second order field statistics									
						
				GetProjectStatisticsPointers (kFieldStatsOnly, 
														fieldStatsNumber, 
														&fieldChanPtr, 
														&fieldSumSquaresPtr,
														NULL,
														NULL);
						
				AddToClassStatistics (numberOutputChannels, 
												outputChannelStatsPtr, 
												outputSumSquaresPtr, 
												numberInputChannels, 
												outputChannelListPtr, 
												fieldChanPtr, 
												fieldSumSquaresPtr, 
												squareOutputMatrixFlag,
												gProjectInfoPtr->statisticsCode,
												outputStatisticsCode);
				
				}	// end "if (fieldIdentPtr[fieldNumber].statsUpToDate)" 
							
			}	// end "if (fieldIdentPtr[fieldNumber].field..." 
			
		fieldNumber = fieldIdentPtr[fieldNumber].nextField;
		
		}	// end "while (fieldNumber != -1)" 
	
}	// end "CombineFieldStatistics" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ComputeCorrelationFromCovMatrix
//
//	Software purpose:	The purpose of this routine is to compute a
//							correlation matrix (lower triangular form) from 
//							the input covariance matrix.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			ListStatistics in statList.c
//
//	Coded By:			Larry L. Biehl			Date: 12/07/1993
//	Revised By:			Larry L. Biehl			Date: 10/09/1997	

void ComputeCorrelationFromCovMatrix (
				UInt16								numberOutputChannels, 
				HCovarianceStatisticsPtr		correlationPtr, 
				UInt16								numberInputChannels, 
				UInt16*								channelListPtr,
				HDoublePtr							stdDevVectorPtr,
				Boolean								squareOutputMatrixFlag)

{
	double								channelStdDev1,
											channelStdDev2;
											
	HCovarianceStatisticsPtr		savedCorrelationPtr;
	
	HDoublePtr							lStdDevVectorPtr1,
											lStdDevVectorPtr2;
											
	SInt32								lowerLeftIndexSkip;
	
	UInt32								channel,
											covChan;
	
	
			// Check input values.  Continue only if input parameters are within	
			// proper rangers																		
			
	if (numberOutputChannels == 0 || numberInputChannels == 0 || 
								stdDevVectorPtr == NULL || correlationPtr == NULL) 
																								return;
																						
	savedCorrelationPtr = correlationPtr;
	
			// Compute the correlation matrix.												
			
			// If the number of input and output channels are the same then		
			// all the channels will be used.  Indicate that the channel			
			// list will not need to be used.												
			
	if (numberOutputChannels == numberInputChannels)
		channelListPtr = NULL;
		
	lStdDevVectorPtr1 = stdDevVectorPtr;
	lowerLeftIndexSkip = numberOutputChannels - 1;
	
	for (channel=0; channel<numberOutputChannels; channel++)
		{
		if (channelListPtr)
			lStdDevVectorPtr1 = &stdDevVectorPtr[channelListPtr[channel]];
			
		channelStdDev1 = *lStdDevVectorPtr1;
		
		if (channelStdDev1 > 0)
			{
			lStdDevVectorPtr2 = stdDevVectorPtr;
			
			for (covChan=0; covChan<=channel; covChan++)
				{	
				channelStdDev2 = *lStdDevVectorPtr2;
				if (channelStdDev2 > 0)
					{
					if (channelListPtr)
						lStdDevVectorPtr2 = &stdDevVectorPtr[channelListPtr[covChan]];
																			
					*correlationPtr /= (channelStdDev1 * channelStdDev2);
		  						
		  			}	// end "if (channelStdDev2 > 0)" 
		 								
			 	else	// channelStdDev2 == 0 
					*correlationPtr = 0;
			 							
			 	correlationPtr++;
			 	
			 	lStdDevVectorPtr2++;
			
				}	// end "for (covChan=0; covChan<=..." 
				
			}	// end "if (channelStdDev1 > 0)" 
		
		else	// channelStdDev1 == 0 
			{
			for (covChan=0; covChan<=channel; covChan++)
				{
				*correlationPtr = 0;
			 	correlationPtr++;
				
				}	// end "for (covChan=channelIndex; ..." 
			
			}	// end "else channelStdDev1 == 0" 
			
		if (squareOutputMatrixFlag)
			{
			correlationPtr += lowerLeftIndexSkip;
			lowerLeftIndexSkip--; 
			
			}	// end "if (squareOutputMatrixFlag)" 
		
		lStdDevVectorPtr1++;
			
		}	// end "for (channel=0; channel<numberOutputChannels; ..." 
	
			// Now copy the lower left part of the matrix to the upper right		
			// part if the output is to be a square matrix.								
				
	if (squareOutputMatrixFlag)
		{
		correlationPtr = savedCorrelationPtr;	
		CopyLowerToUpperSquareMatrix (numberOutputChannels, correlationPtr);
		
		}	// end "if (squareOutputMatrixFlag)" 
					
}	// end "ComputeCorrelationFromCovMatrix" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ComputeVarianceVector
//
//	Software purpose:	The purpose of this routine is to compute the
//							variance vector for the input intermediate statistics.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			GetClassCovarianceMatrix in statCompute.c
//
//	Coded By:			Larry L. Biehl			Date: 07/30/1992
//	Revised By:			Larry L. Biehl			Date: 08/23/2010	

void ComputeVarianceVector (
				HChannelStatisticsPtr			channelStatsPtr, 
				HSumSquaresStatisticsPtr		sumSquaresPtr, 
				HDoublePtr							variancePtr, 
				UInt16								numberChannels, 
				SInt64								numberPixels, 
				SInt16								inputStatCode)

{
	SInt64								numberPixelsLessOne;
	
	UInt32								channel,
											indexSkip;
	
	
			// Check input values.  Continue only if input parameters are within	
			// proper rangers																		
			
	if (numberChannels > 0 && channelStatsPtr && sumSquaresPtr && 
																variancePtr && numberPixels > 0)
		{
				// Compute the mean vector if needed.										
		
		if (channelStatsPtr[0].standardDev < 0)
			ComputeMeanStdDevVector (channelStatsPtr, 
												sumSquaresPtr,
												numberChannels, 
												numberPixels, 
												inputStatCode,
												kTriangleInputMatrix);
					
		numberPixelsLessOne = numberPixels - 1;
		
		if (numberPixels > 1)
			{
			indexSkip = 1;
			
			for (channel=0; channel<numberChannels; channel++)
				{
				*variancePtr = 
								channelStatsPtr->standardDev * channelStatsPtr->standardDev;
				
				channelStatsPtr++;
				variancePtr++;		
				
				}	// end "for (channel=0; channel<numberChannels; channel++)" 
				
			}	// end "if (numberPixels > 1)" 
			
		else	// numberPixels <= 1 
			{
			for (channel=0; channel<numberChannels; channel++)
				{															
				*variancePtr = 0;
				variancePtr++;
				
				}	// end "for (channel=0; channel<numberChannels; channel++)" 
			
			}	// end "else numberPixels <= 1 " 
			
		}	// end "numberChannels > 0 ..." 
					
}	// end "ComputeVarianceVector" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean DetermineIfSpecifiedStatisticsExist
//
//	Software purpose:	The purpose of this routine is to determine if the specified
//							statistics exist for the input class.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			CheckClassStats in SProjUtl.cpp
//							ListClassInformation in SStatLst.cpp
//
//	Coded By:			Larry L. Biehl			Date: 09/26/1997
//	Revised By:			Larry L. Biehl			Date: 08/26/2010	

Boolean DetermineIfSpecifiedStatisticsExist (
				HPClassNamesPtr					classNamesPtr,
				SInt16								covarianceStatsToUse,
				Boolean*								computeCommonCovarianceFlagPtr)

{
	SInt64								numberOfPixelsLoadedInClass;
	
	Boolean								existFlag = FALSE;
	
	
	*computeCommonCovarianceFlagPtr = FALSE;
	
	numberOfPixelsLoadedInClass = GetNumberOfPixelsLoadedInClass (
																	classNamesPtr,
																	gProjectInfoPtr->fieldIdentPtr);
	
	if (numberOfPixelsLoadedInClass > 0)
		{
		if (covarianceStatsToUse == kEnhancedStats)
			{
					// Note if enhanced statistics are not available then the orginal
					// statistics are used.
					
			existFlag = TRUE;
				
			}	// end "if (covarianceStatsToUse == kEnhancedStats)"
			
		else if (covarianceStatsToUse == kLeaveOneOutStats)
			{
			if (classNamesPtr->mixingParameterCode == kComputedOptimum)
				{
				if (classNamesPtr->looCovarianceValue >= 0)
					{
					existFlag = TRUE;
					
					if (classNamesPtr->looCovarianceValue > 1 && 
											gProjectInfoPtr->numberCommonCovarianceClasses == 0)
						*computeCommonCovarianceFlagPtr = TRUE;
					
					}	// end "if (classNamesPtr->looCovarianceValue >= 0)"
					
				}	// end "if (...->mixingParameterCode == kComputedOptimum)"
				
			else	// ...->mixingParameterCode == kUserSet || kIdentityMatrix
				existFlag = TRUE;
				
			}	// end "else if (covarianceStatsToUse == kLeaveOneOutStats)"
		
		else	// covarianceStatsToUse == kOriginalStats
			existFlag = TRUE;
			
		}	// end "if (numberOfPixelsLoadedInClass > 0)"
		
	return (existFlag);
					
}	// end "DetermineIfSpecifiedStatisticsExist" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void FinishClassStatsUpdate
//
//	Software purpose:	The purpose of this routine is to finish the update for the
//							class stats.
//
//	Parameters in:		Class number (0 based).
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:	
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 02/07/2018

Boolean FinishClassStatsUpdate (
				UInt32								classNumber)

{
	HChannelStatisticsPtr			classChanPtr;
	HSumSquaresStatisticsPtr		classSumSquaresPtr;
	
	HPClassNamesPtr					classNamesPtr;
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	UInt32								classStorage;
											
	SInt16								fieldNumber;
	
	Boolean								upToDateFlag;
	
	
			// Make certain that input value makes sense		
	
	if (classNumber >= gProjectInfoPtr->numberStatisticsClasses)
																						return (FALSE);
	
			// Initialize local variables.													
			
	classNamesPtr = gProjectInfoPtr->classNamesPtr;	
	classStorage = gProjectInfoPtr->storageClass[classNumber];
	
			// Continue only if the number of fields in the class is one or more	
			// and if the class statistics are not up-to-date							
	
   upToDateFlag = FALSE;
	if (classNamesPtr[classStorage].numberOfTrainFields > 0 && 
													!classNamesPtr[classStorage].statsUpToDate)
		{
		upToDateFlag = TRUE;
		fieldNumber = classNamesPtr[classStorage].firstFieldNumber;
		
		while (fieldNumber != -1)
			{
			fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];
			
					// Verify that all of the training fields have been added to
					// the class information before indicating that the class
					// statistics are up to date.	
					
			if (fieldIdentPtr->fieldType == kTrainingType &&
															!fieldIdentPtr->loadedIntoClassStats)
				{
				upToDateFlag = FALSE;
				break;
				
				}	// end "if (fieldIdentPtr->fieldType == kTrainingType && ..."
				
			fieldNumber = fieldIdentPtr->nextField;
			
			}	// end "while (fieldNumber != -1)"
			
				// Compute the first order statistics for the class stats only case.																		
		
		if (gProjectInfoPtr->keepClassStatsOnlyFlag)
			{
			GetProjectStatisticsPointers (kClassStatsOnly, 
													classStorage, 
													&classChanPtr, 
													&classSumSquaresPtr,
													NULL,
													NULL);
													
			ComputeMeanStdDevVector (
									classChanPtr,
									classSumSquaresPtr, 
									gProjectInfoPtr->numberStatisticsChannels, 
									(UInt32)classNamesPtr[classStorage].numberStatisticsPixels,
									gProjectInfoPtr->statisticsCode,
									kTriangleInputMatrix);
				
			}	// end "else !gProjectInfoPtr->keepClassStatsOnlyFlag"
			
		}	// end "if (classNamesPtr[classStorage].numberOfTrainFields > 0)"
		
	classNamesPtr[classStorage].statsUpToDate = upToDateFlag;
	
	return (upToDateFlag);
		
}	// end "FinishClassStatsUpdate" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void FinishProjectStatsUpdate
//
//	Software purpose:	The purpose of this routine is to finish the update for the
//							project mask stats.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 12/17/1998	

void FinishProjectStatsUpdate ()

{
	UInt32								classIndex,
											numberClasses;
											
	Boolean								statsUpToDateFlag = TRUE;
	
	
			// Initialize local variables.													
			
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	
			// Continue only if number of classes is one or more.													
	
	if (numberClasses > 0)
		{
		for (classIndex=0; classIndex<numberClasses; classIndex++)
			{
			if (!FinishClassStatsUpdate ((UInt32)classIndex))
				statsUpToDateFlag = FALSE;
			
			}	// end "if (numberClasses > 0)"
			
		}	// end "if (numberClasses > 0)"
		
	gProjectInfoPtr->statsUpToDate = statsUpToDateFlag;
		
	gUpdateProjectMenuItemsFlag = TRUE;
		
}	// end "FinishProjectStatsUpdate" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void FinishClassMaskStatsUpdate
//
//	Software purpose:	The purpose of this routine is to finish the update for the
//							class mask stats.
//
//	Parameters in:		Class number (0 based).
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:	
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 01/07/1999	

void FinishClassMaskStatsUpdate (
				UInt32								classNumber)

{
	HPClassNamesPtr					classNamesPtr;
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	UInt32								classStorage;
											
	SInt16								fieldNumber;
	
	
			// Make certain that input value makes sense		
	
	if (classNumber >= gProjectInfoPtr->numberStatisticsClasses)
																								return;
	
			// Initialize local variables.													
			
	classNamesPtr = gProjectInfoPtr->classNamesPtr;	
	classStorage = gProjectInfoPtr->storageClass[classNumber];
	
			// Continue only if the number of fields in the class is one or more	
			// and if the class statistics are not up-to-date							
	
	if (classNamesPtr[classStorage].numberOfTrainFields > 0 && 
											!classNamesPtr[classStorage].statsUpToDate)
		{	
		fieldNumber = classNamesPtr[classStorage].firstFieldNumber;
		
		while (fieldNumber != -1)
			{
			fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];
						
			FinishFieldMaskStatsUpdate (fieldNumber);
			
					// Add the field statistics to the class statistics if it	
					// hasn't been done.														
					
			if (fieldIdentPtr->pointType == kMaskType &&
							fieldIdentPtr->fieldType == kTrainingType &&
												!fieldIdentPtr->loadedIntoClassStats)
				{							
				classNamesPtr[classStorage].numberStatisticsPixels += 
														fieldIdentPtr->numberPixelsUsedForStats;
											
				fieldIdentPtr->loadedIntoClassStats = TRUE;
				
				}	// end "if (fieldIdentPtr->pointType == kMaskType && ..." 
				
			fieldNumber = fieldIdentPtr->nextField;
			
			}	// end "while (fieldNumber != -1)"
			
		}	// end "if (classNamesPtr[classStorage].numberOfTrainFields > 0 && ..."
		
}	// end "FinishClassMaskStatsUpdate" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void FinishFieldStatsUpdate
//
//	Software purpose:	The purpose of this routine is to finish the update for the
//							field mask stats.
//
//	Parameters in:		Field number (0 based).
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:	
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 12/17/1998	

void FinishFieldMaskStatsUpdate (
				UInt32								fieldNumber)

{
	HChannelStatisticsPtr			fieldChanPtr;
	HSumSquaresStatisticsPtr		fieldSumSquaresPtr;
	
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	
			// Make certain that input value makes sense									
			
	if (fieldNumber >= (UInt32)gProjectInfoPtr->numberStorageFields) 	
																								return;
																						
			// Initialize local variables.	
																						
	fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];	
					
	if (fieldIdentPtr->fieldType != kTrainingType ||
												fieldIdentPtr->pointType != kMaskType)		
																								return;
																								
	if (fieldIdentPtr->statsUpToDate)
																								return;
																						
			// Note that if only the class statistics are being kept then the
			// mean and standard deviation for the field does not need to be
			// computed.
																						
	if (!gProjectInfoPtr->keepClassStatsOnlyFlag)
		{
		GetProjectStatisticsPointers (kFieldStatsOnly, 
												fieldIdentPtr->trainingStatsNumber, 
												&fieldChanPtr, 
												&fieldSumSquaresPtr,
												NULL,
												NULL);
										
				// Compute the first order statistics.																		
		
		ComputeMeanStdDevVector (fieldChanPtr, 
											fieldSumSquaresPtr, 
											gProjectInfoPtr->numberStatisticsChannels, 
											fieldIdentPtr->numberPixels,
											gProjectInfoPtr->statisticsCode,
											kTriangleInputMatrix);
	
				// Indicate that project information has changed.						
	 
		gProjectInfoPtr->changedFlag = TRUE;
		
		fieldIdentPtr->statsUpToDate = TRUE;
									
		}	// end "if (!gProjectInfoPtr->keepClassStatsOnlyFlag)"
					
			// Indicate that statistics have been loaded into the project.		
		
	gProjectInfoPtr->statsLoaded = TRUE;
			
			// Note.							
			// The 'fieldIdentPtr->statsUpToDate' flag is kept as false for the case
			// when only the class statistics are kept.
		
}	// end "FinishFieldMaskStatsUpdate" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void FinishProjectMaskStatsUpdate
//
//	Software purpose:	The purpose of this routine is to finish the update for the
//							project mask stats.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 12/17/1998
//	Revised By:			Larry L. Biehl			Date: 12/17/1998	

void FinishProjectMaskStatsUpdate ()

{
	UInt32								classIndex,
											numberClasses;
	
	
			// Initialize local variables.													
			
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	
			// Continue only if number of classes is one or more.													
	
	if (numberClasses > 0)
		{
		for (classIndex=0; classIndex<numberClasses; classIndex++)
			FinishClassMaskStatsUpdate ((UInt32)classIndex);
			
		}	// end "if (numberClasses > 0)" 
		
}	// end "FinishProjectMaskStatsUpdate" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean GetClassChannelStatistics
//
//	Software purpose:	The purpose of this routine is to obtain the class
//							channels sums from the fields that belong to the
//							input class.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			GetGAIAClassMeans in SGaiaUtl.cpp
//							GetClassCovarianceMatrix in SStatCom.cpp
//							GetClassMeanVector in SStatCom.cpp
//
//	Coded By:			Larry L. Biehl			Date: 02/07/1992
//	Revised By:			Larry L. Biehl			Date: 01/23/1998	

Boolean GetClassChannelStatistics (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				UInt16*								channelListPtr, 
				UInt16								classNumber)

{
	HChannelStatisticsPtr			inputChannelStatsPtr;
	
	HPClassNamesPtr					classNamesPtr;
	
	SInt32								classStorage;
	
	
			// Check input values.  Continue only if input parameters are within	
			// proper rangers																		
			
	if (numberOutputChannels <= 0 || 
								classChannelStatsPtr == NULL)  
																						return (FALSE);
			
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	classStorage = gProjectInfoPtr->storageClass[classNumber];
	
			// Continue only if the number of fields in the class is one or 		
			// more.																					
				
	if (classNamesPtr[classStorage].numberOfTrainFields <= 0)
																						return (FALSE);
	
	if (classNamesPtr[classStorage].covarianceStatsToUse == kEnhancedStats &&
												classNamesPtr[classStorage].modifiedStatsFlag)
		{
				// Get pointers to the memory for the first order and 				
				// second order field statistics												
				
		GetProjectStatisticsPointers (kClassStatsOnly, 
												classStorage, 
												NULL, 
												NULL,
												&inputChannelStatsPtr,
												NULL);
												
		ReduceChanStatsVector (inputChannelStatsPtr, 
										classChannelStatsPtr, 
										numberOutputChannels, 
										channelListPtr);
										
		}	// end "if (...covarianceStatsToUse == kEnhancedStats && ...)" 
		
	else	// ...covarianceStatsToUse != kEnhancedStats || ... 
		{
		if (gProjectInfoPtr->keepClassStatsOnlyFlag)
			AddToClassChannelStatistics (
							numberOutputChannels, 
							classChannelStatsPtr, 
							gProjectInfoPtr->numberStatisticsChannels, 
							channelListPtr, 
							&gProjectInfoPtr->classChanStatsPtr[
									classStorage*gProjectInfoPtr->numberStatisticsChannels],
							TRUE);
		
		else	// !gProjectInfoPtr->keepClassStatsOnlyFlag 
			CombineFieldChannelStatistics (numberOutputChannels,
														channelListPtr, 
														classChannelStatsPtr, 
														classNumber);
		
		ComputeMeanVector (classChannelStatsPtr, 
									numberOutputChannels, 
									classNamesPtr[classStorage].numberStatisticsPixels);
			
		}	// end "else ...covarianceStatsToUse != kEnhancedStats || ..." 
	
	return (TRUE);
	
}	// end "GetClassChannelStatistics" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void GetClassCovarianceMatrix
//
//	Software purpose:	The purpose of this routine is to compute a
//							covariance matrix for the input class.  Note that
//							no error message is currently returned if the
//							matrix cannot be computed.  A check is in the routines
//							that call this routine to make certain that this
//							routine is not called if no class covariance matrix
//							exists.  (This needs to be changed).
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			UpdateClassLOOStats in SLOOCov.cpp
//							CreateFalseColorPalette in SPalette.cpp
//							Preprocess in SProjPur.cpp
//							GetCommonCovariance in SStatCom.cpp
//							GetTransformedClassCovarianceMatrix in SStatCom.cpp
//							ListClassStats in SStatLst.cpp
//							LoadStatEnhanceClassStatistics in statisticsEnhancement.c
//
//	Coded By:			Larry L. Biehl			Date: 02/07/1992
//	Revised By:			Larry L. Biehl			Date: 05/04/2017	

void GetClassCovarianceMatrix (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				HCovarianceStatisticsPtr		classCovariancePtr, 
				UInt16*								channelListPtr, 
				UInt16								statClassNumber, 
				Boolean								squareOutputMatrixFlag,
				SInt16								outputStatisticsCode,
				UInt16								covarianceStatsToUse)

{
	SInt64								numberPixels;
	
	HCovarianceStatisticsPtr		commonCovariancePtr,
											inputCommonCovariancePtr,
											inputCovariancePtr;
											
	HPClassNamesPtr					classNamesPtr;
	
	SInt16								classStorage;
	
	Boolean								continueFlag;
	
	
			// Check input values.  Continue only if input parameters are within	
			// proper rangers																		
			
	if (numberOutputChannels <= 0 || !classChannelStatsPtr || !classCovariancePtr)
																									return;
																						
	if (outputStatisticsCode > gProjectInfoPtr->statisticsCode)
																									return;
	
	continueFlag = TRUE;
	
			// Get the index for the storage location of the data.					
																									
	classStorage = gProjectInfoPtr->storageClass[statClassNumber];
	
	classNamesPtr = &gProjectInfoPtr->classNamesPtr[classStorage];
	
			// Note that if enhanced statistics are requested for a class but		
			// none are available then the original statistics are returned.		
			
	if (covarianceStatsToUse == kEnhancedStats && classNamesPtr->modifiedStatsFlag)
		{
				// Get pointers to the memory for the first order and 				
				// second order field statistics												
				
		GetProjectStatisticsPointers (kClassStatsOnly, 
												classStorage, 
												NULL, 
												NULL,
												NULL,
												&inputCovariancePtr);
												
		continueFlag = GetClassChannelStatistics (numberOutputChannels,
																classChannelStatsPtr, 
																channelListPtr, 
																statClassNumber);
		
		ReduceInputMatrix (numberOutputChannels,
									classCovariancePtr,
									gProjectInfoPtr->numberStatisticsChannels,
									channelListPtr, 
									inputCovariancePtr, 
									squareOutputMatrixFlag);		
	
				// Now copy the lower left part of the matrix to the upper 			
				// right part if the output is to be a square matrix.					
		
		if (squareOutputMatrixFlag)
			CopyLowerToUpperSquareMatrix (numberOutputChannels, classCovariancePtr);
			
		}	// end "if (covarianceStatsToUse == kEnhancedStats && ...)" 
		
	else	// covarianceStatsToUse != kEnhancedStats || ...
		{
				// First get the sums and sum of squares for the class.				
				
		continueFlag = GetClassSumsSquares (numberOutputChannels, 
														classChannelStatsPtr,
														classCovariancePtr, 
														channelListPtr, 
														statClassNumber, 
														squareOutputMatrixFlag,
														outputStatisticsCode);
		
				// First compute the lower left triangular portion of the matrix.	
				
		numberPixels = classNamesPtr->numberStatisticsPixels;
		
		if (continueFlag)
			{
			if (outputStatisticsCode == kMeanCovariance)
				{						
				ComputeCovarianceMatrix (numberOutputChannels, 
													classCovariancePtr,
													numberOutputChannels, 
													NULL, 
													classChannelStatsPtr, 
													classCovariancePtr, 
													numberPixels, 
													squareOutputMatrixFlag);
			
				if (covarianceStatsToUse == kLeaveOneOutStats)
					{						
					if (gProjectInfoPtr->useCommonCovarianceInLOOCFlag &&
										gProjectInfoPtr->numberCommonCovarianceClasses == 0)
							// Get the common covariance for leave-one-out statistics.
						UpdateProjectLOOStats (kUpdateProject, NULL);
			
							// Get pointer to memory (already defined) to store the common 
							// covariance in.
							// Handle is already locked.
					inputCommonCovariancePtr = (HCovarianceStatisticsPtr)GetHandlePointer (
														gProjectInfoPtr->commonCovarianceStatsHandle);
					
					if (inputCommonCovariancePtr != NULL)							
						commonCovariancePtr = 
								&inputCommonCovariancePtr[gProjectInfoPtr->numberCovarianceEntries];
												
							// Reduce the common covariance to just the channels that are 
							// being used.
							
					if (gProjectInfoPtr->useCommonCovarianceInLOOCFlag && 
														!gProjectInfoPtr->localCommonCovarianceLoadedFlag)
						{
						ReduceInputMatrix (numberOutputChannels,
													commonCovariancePtr,
													gProjectInfoPtr->numberStatisticsChannels,
													channelListPtr, 
													inputCommonCovariancePtr, 
													kTriangleOutputMatrix);
													
						gProjectInfoPtr->localCommonCovarianceLoadedFlag = TRUE;
												
						}	// end "if (gProjectInfoPtr->useCommonCovarianceInLOOCFlag && ..."
												
					if (inputCommonCovariancePtr != NULL &&
									(!gProjectInfoPtr->useCommonCovarianceInLOOCFlag ||
													gProjectInfoPtr->numberCommonCovarianceClasses > 0))
								// Get the leave-one-out covariance
						GetLOOCovariance (classNamesPtr->mixingParameterCode,
												classNamesPtr->looCovarianceValue,
												classNamesPtr->userMixingParameter,
												numberOutputChannels,
												classCovariancePtr,
												commonCovariancePtr,
												classCovariancePtr,
												squareOutputMatrixFlag);
							
					}	// end "if (covarianceStatsToUse == kLeaveOneOutStats && ..."
												
				}	// end "if (outputStatisticsCode == kMeanCovariance)"
					
			else	// outputStatisticsCode != kMeanCovariance 
				ComputeVarianceVector (classChannelStatsPtr, 
												classCovariancePtr,
												classCovariancePtr, 
												numberOutputChannels, 
												numberPixels, 
												kMeanStdDevOnly);
							
			}	// end "if (continueFlag)"
			
		}	// end "else covarianceStatsToUse != kEnhancedStats || ..." 
			
	if (continueFlag && outputStatisticsCode == kMeanCovariance &&
															gProjectInfoPtr->setZeroVarianceFlag)
				// Set any zero variances to requested factor.							
				// This is done so that the matrix may be inverted.
		if (ResetZeroVariances (classCovariancePtr, 
											squareOutputMatrixFlag,
											outputStatisticsCode, 
											numberOutputChannels))
			{
					// List message that at least one zero variance for this class 		
					// was set to the user specified value.	
					
			CMFileStream* resultsFileStreamPtr = GetResultsFileStreamPtr (0);												
				
			continueFlag = ListClassInformationMessage (kProjectStrID,
																		IDS_Project67,
																		resultsFileStreamPtr, 
																		gOutputForce1Code,
																		statClassNumber, 
																		continueFlag);
			
			}	// end "if (ResetZeroVariances (classCovariancePtr, ..."
					
}	// end "GetClassCovarianceMatrix" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void GetClassMaximumVector
//
//	Software purpose:	The purpose of this routine is to return the input class
//							maximum vector.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			ParallelPipedClsfierControl in SClassify.cpp
//
//	Coded By:			Larry L. Biehl			Date: 03/29/2012
//	Revised By:			Larry L. Biehl			Date: 03/29/2012	

void GetClassMaximumVector (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				HDoublePtr							outputClassMaximumPtr, 
				UInt16*								channelListPtr, 
				UInt16								classNumber)

{	
	Boolean								continueFlag;
	
												
	continueFlag = GetClassChannelStatistics (numberOutputChannels,
															classChannelStatsPtr, 
															channelListPtr, 
															classNumber);
	
	if (continueFlag)
		ReduceMaximumVector (classChannelStatsPtr, 
									outputClassMaximumPtr,
									numberOutputChannels,
									NULL);
	
}	// end "GetClassMaximumVector" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void GetClassMeanVector
//
//	Software purpose:	The purpose of this routine is to return the input class
//							mean vector.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			L12ClsfierControl in classify.c
//
//	Coded By:			Larry L. Biehl			Date: 07/03/1996
//	Revised By:			Larry L. Biehl			Date: 07/03/1996	

void GetClassMeanVector (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				HDoublePtr							outputClassMeanPtr, 
				UInt16*								channelListPtr, 
				UInt16								classNumber)

{	
	Boolean								continueFlag;
	
												
	continueFlag = GetClassChannelStatistics (numberOutputChannels,
															classChannelStatsPtr, 
															channelListPtr, 
															classNumber);
	
	if (continueFlag)
		ReduceMeanVector (classChannelStatsPtr, 
								outputClassMeanPtr,
								numberOutputChannels,
								NULL);
	
}	// end "GetClassMeanVector" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void GetClassMinimumVector
//
//	Software purpose:	The purpose of this routine is to return the input class
//							maximum vector.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			ParallelPipedClsfierControl in SClassify.cpp
//
//	Coded By:			Larry L. Biehl			Date: 03/29/2012
//	Revised By:			Larry L. Biehl			Date: 03/29/2012	

void GetClassMinimumVector (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				HDoublePtr							outputClassMinimumPtr, 
				UInt16*								channelListPtr, 
				UInt16								classNumber)

{	
	Boolean								continueFlag;
	
												
	continueFlag = GetClassChannelStatistics (numberOutputChannels,
															classChannelStatsPtr, 
															channelListPtr, 
															classNumber);
	
	if (continueFlag)
		ReduceMinimumVector (classChannelStatsPtr, 
									outputClassMinimumPtr,
									numberOutputChannels,
									NULL);
	
}	// end "GetClassMinimumVector" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void GetClassStdDevVector
//
//	Software purpose:	The purpose of this routine is to return the input class
//							mean vector.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			L12ClsfierControl in classify.c
//
//	Coded By:			Larry L. Biehl			Date: 03/30/2012
//	Revised By:			Larry L. Biehl			Date: 03/30/2012	

void GetClassStdDevVector (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				HDoublePtr							outputClassStdDevPtr, 
				UInt16*								channelListPtr, 
				UInt16								classNumber)

{	
	Boolean								continueFlag;
	
												
	continueFlag = GetClassChannelStatistics (numberOutputChannels,
															classChannelStatsPtr, 
															channelListPtr, 
															classNumber);
	
	if (continueFlag)
		ReduceStdDevVector (classChannelStatsPtr, 
									outputClassStdDevPtr,
									numberOutputChannels,
									NULL);
	
}	// end "GetClassStdDevVector" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean GetClassSumsSquares
//
//	Software purpose:	The purpose of this routine is to obtain the class
//							sums of squares from the fields that belong to the
//							input class.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			GetClassCovarianceMatrix in statCompute.c
//							ListClassStats in statList.c
//
//	Coded By:			Larry L. Biehl			Date: 02/07/1992
//	Revised By:			Larry L. Biehl			Date: 10/21/1994	

Boolean GetClassSumsSquares (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				HSumSquaresStatisticsPtr		classSumSquaresPtr, 
				UInt16*								outputChannelListPtr, 
				UInt16								classNumber, 
				Boolean								squareOutputMatrixFlag, 
				SInt16								outputStatisticsCode)

{
	HPClassNamesPtr					classNamesPtr;
	
	SInt32								classStorage;
	
	
			// Check input values.  Continue only if input parameters are within	
			// proper rangers																		
			
	if (numberOutputChannels <= 0 ||
			!classChannelStatsPtr || 
				numberOutputChannels <= 0 ||
					!classSumSquaresPtr)
																					return (FALSE);
			
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	
			// Get the class storage number.													
						
	classStorage = gProjectInfoPtr->storageClass[classNumber];
	
			// Continue only if the number of fields in the class is one or more	
			// and if the class statistics are not up-to-date							
	
	if (classNamesPtr[classStorage].numberOfTrainFields > 0)
		{
				// Initialize class statistics memory.										
				
		ZeroStatisticsMemory (classChannelStatsPtr, 
										classSumSquaresPtr,
										numberOutputChannels,
										outputStatisticsCode,
										squareOutputMatrixFlag);
									
  		if (gProjectInfoPtr->keepClassStatsOnlyFlag)
  			{
  			if (classNamesPtr[classStorage].numberStatisticsPixels > 0)
				AddToClassStatistics (
								numberOutputChannels,
								classChannelStatsPtr,
								classSumSquaresPtr, 
								gProjectInfoPtr->numberStatisticsChannels, 
								outputChannelListPtr, 
								&gProjectInfoPtr->classChanStatsPtr[
										classStorage*gProjectInfoPtr->numberStatisticsChannels], 
								&gProjectInfoPtr->classSumSquaresStatsPtr[
										classStorage*gProjectInfoPtr->numberCovarianceEntries],  
								squareOutputMatrixFlag,
								gProjectInfoPtr->statisticsCode,
								outputStatisticsCode);
			
			else	// classNamesPtr[classStorage].numberStatisticsPixels <= 0 
																							return (FALSE);
																				
			}	// end "if (gProjectInfoPtr->keepClassStatsOnlyFlag)" 
  		
  		else	// !gProjectInfoPtr->keepClassStatsOnlyFlag 
			CombineFieldStatistics (numberOutputChannels,  
											outputChannelListPtr,
											classChannelStatsPtr, 
											classSumSquaresPtr,
											classNumber, 
											squareOutputMatrixFlag, 
											outputStatisticsCode);
		
		}	// end "if (classNamesPtr[classStorage].number ..." 
		
	else	// classNamesPtr[classStorage].numberOfTrainFields <= 0 
																							return (FALSE);
	
	return (TRUE);
	
}	// end "GetClassSumsSquares" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean GetCommonCovariance
//
//	Software purpose:	The purpose of this routine is to compute the common
//							covariance for the input set of classes and class
//							weights.
//
//	Parameters in:				
//
//	Parameters out:	None
//
// Value Returned:	None			
// 
// Called By:			GetTransformedCommonCovariance in SClassfy.cpp
//							SetupClsfierStats in SEClssfy.cpp
//							UpdateProjectLOOStats in SLooCov.cpp
//
//	Coded By:			Larry L. Biehl			Date: 03/24/1997
//	Revised By:			Larry L. Biehl			Date: 11/24/1999

Boolean GetCommonCovariance (
				HDoublePtr							covariancePtr,
				HDoublePtr							tempMatrixPtr,
				HChannelStatisticsPtr			classChannelStatsPtr,
				UInt16*								classPtr,
				UInt16*								statFeaturePtr,  
				UInt32								numberClasses,
				UInt16								numberFeatureChannels, 
				Boolean								squareOutputMatrixFlag,
				SInt16								outputStatisticsCode,
				UInt16								inputCovarianceStatsToUse,
				Boolean								projectCommonCovarianceFlag)
	
{ 
	double								weightValue;
	float									totalProbability;
	
	UInt32								classIndex,
											index,
											numberCommonCovarianceClasses,
											numberIndices,
											statClassNumber,
											weightsIndex;
											
	SInt16								classStorage;
	
	UInt16								covarianceStatsToUse;
	
	Boolean								continueFlag = TRUE;
	
						
			// Normalize a priori probabilities.										
			
	weightsIndex = GetCommonCovarianceWeightsIndex ();
			
	totalProbability = (float)GetTotalProbability (classPtr,
																	numberClasses,
																	weightsIndex);
	
			// Zero the covariance matrix
																	
	ZeroMatrix (covariancePtr, 
					numberFeatureChannels, 
					numberFeatureChannels,
					squareOutputMatrixFlag);
		
	numberIndices = (UInt32)numberFeatureChannels * numberFeatureChannels;
	if (!squareOutputMatrixFlag)
		numberIndices = (UInt32)(numberFeatureChannels+1)*numberFeatureChannels/2;
		
			// If the input covarianceStatsToUse parameter is set to 'kMixedStats',
			// then use that indicated by the specified project class. Otherwise use
			// as the input parameter specifies.
			
	if (inputCovarianceStatsToUse != kMixedStats)
		covarianceStatsToUse = inputCovarianceStatsToUse;
	
			// We will count the number of classes used to compute the common
			// covariance matrix. It is needed for algorithms such as the
			// leave-one-out covariance.
				
	numberCommonCovarianceClasses = 0;
			
	for (classIndex=0; 
			classIndex<numberClasses; 
			classIndex++)
		{
		if (classPtr != NULL)
			statClassNumber = classPtr[classIndex] - 1;
			
		else	// classPtr == NULL.  Assume all classes in order.
			statClassNumber = classIndex;
					
				// Get the constant for the class									
				
		weightValue = GetClassWeightValue (
									(UInt16)statClassNumber, weightsIndex, totalProbability);
		
		if (weightValue > 0)
			{
			numberCommonCovarianceClasses++;
			
					// Set the covariance to use parameter if needed.

			if (inputCovarianceStatsToUse == kMixedStats)
				{
				classStorage = gProjectInfoPtr->storageClass[statClassNumber];
				covarianceStatsToUse = 
						gProjectInfoPtr->classNamesPtr[classStorage].covarianceStatsToUse;
				
				}	// end "if (inputCovarianceStatsToUse == kMixedStats)"
			
					// Get the class covariance matrix	
					
			GetClassCovarianceMatrix ((UInt16)numberFeatureChannels,
												classChannelStatsPtr, 
												tempMatrixPtr, 
												statFeaturePtr, 
												(UInt16)statClassNumber,
												squareOutputMatrixFlag,
												outputStatisticsCode,
												covarianceStatsToUse);
			
			for (index=0; index<numberIndices; index++)
				{						
				*covariancePtr += weightValue * *tempMatrixPtr;
				tempMatrixPtr++;
				covariancePtr++;
				
				}	// end "for (index=0; index<numberIndices; index++)"
				
			tempMatrixPtr -= numberIndices;
			covariancePtr -= numberIndices;
								
			if (gOperationCanceledFlag)
				{
				continueFlag = FALSE;
				break;
				
				}	// end "if (gOperationCanceledFlag)"
				
			}	// end "if (weightValue > 0)"
			
		}	// end "for (class=0; class<..."
		
	if (continueFlag && projectCommonCovarianceFlag)
		gProjectInfoPtr->numberCommonCovarianceClasses = 
															(UInt16)numberCommonCovarianceClasses;
			
	return (continueFlag); 
			
}	// end "GetCommonCovariance" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean GetEigenStatisticsFeatures
//
//	Software purpose:	The purpose of this routine is to load a feature
//							vector which includes the index within the statistics
//							matrices/vectors that is being used in the input
//							eigenvector channels.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
//
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 04/28/1993
//	Revised By:			Larry L. Biehl			Date: 05/04/2017	

Boolean GetEigenStatisticsFeatures (
				UInt16*								statEigenFeaturePtr,
				UInt16*								outputFeaturePtr,
				SInt16								numberFeatures)

{
	UInt16*								eigenFeaturePtr;
	
	UInt32								index,
											index2,
											numberEigenFeatures,
											numberStatisticsChannels;
											
	Boolean								continueFlag;
	
	
	continueFlag = FALSE;
	numberEigenFeatures = gTransformationMatrix.numberChannels;
	
	if (gTransformationMatrix.eigenFeatureHandle && 
												numberEigenFeatures > 0 && gProjectInfoPtr)
		{
		numberStatisticsChannels = gProjectInfoPtr->numberStatisticsChannels;
		eigenFeaturePtr = (UInt16*)GetHandlePointer (
													gTransformationMatrix.eigenFeatureHandle);
		
		index2 = 0;
		if (gTransformationMatrix.createdByCode < 16)
			{
			for (index=0; index<numberEigenFeatures; index++)
				{
				while (eigenFeaturePtr[index] !=
									(UInt16)gProjectInfoPtr->channelsPtr[index2] &&
																index2 < numberStatisticsChannels)
					index2++;
					
				if (index2 < numberStatisticsChannels)
					statEigenFeaturePtr[index] = (UInt16)index2;
					
				else	// index2 >= numberStatisticsChannels 
					statEigenFeaturePtr[index] = (UInt16)(numberStatisticsChannels - 1);
				
				}	// end "for (index=0; ..." 
				
			continueFlag = (index2 < numberStatisticsChannels);
			
			}	// end "if (gTransformationMatrix.createdByCode < 16)"
			
		else	// gTransformationMatrix.createdByCode >= 16
			{
					// This section is for the offset-gain type of transformation. The
					// feature vector will include all project channels.
			
			//UInt32 processorNumberFeatures = numberFeatures;
					
			for (index=0; index<(UInt32)numberFeatures; index++)
				{
				while (outputFeaturePtr[index] !=
									(UInt16)gProjectInfoPtr->channelsPtr[index2] &&
																index2 < numberStatisticsChannels)
					index2++;
					
				if (index2 < numberStatisticsChannels)
					statEigenFeaturePtr[index] = (UInt16)index2;
					
				else	// index2 >= numberStatisticsChannels 
					statEigenFeaturePtr[index] = (UInt16)(numberStatisticsChannels - 1);
				
				}	// end "for (index=0; ..." 
				
			continueFlag = TRUE;
			
			}	// end "else gTransformationMatrix.createdByCode >= 16"
		
		}	// end "if (...eigenFeatureHandle && ..." 
		
	return (continueFlag);
			
}	// end "GetEigenStatisticsFeatures" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		UInt32 GetNumberOfPixelsLoadedInClass
//
//	Software purpose:	The purpose of this routine is to get a pointer
//							to the statistics feature vector.  The vector
//							will depend upon whether a transformation matrix
//							is being used.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
//
// Called By:			DetermineIfSpecifiedStatisticsExist in SStatCom.cpp
//							UpdateClassStats in SStatCom.cpp
//							
//	Coded By:			Larry L. Biehl			Date: 03/05/1998
//	Revised By:			Larry L. Biehl			Date: 04/24/2013	

SInt64 GetNumberOfPixelsLoadedInClass (
				HPClassNamesPtr					classNamesPtr,
				HPFieldIdentifiersPtr			fieldIdentPtr)

{									
	SInt64								numberOfPixelsLoadedInClass = 0;
	
	SInt16								fieldNumber;
	
		
	fieldNumber = classNamesPtr->firstFieldNumber;
	while (fieldNumber != -1)
		{
				// Make certain that field is training field							
				
		if (fieldIdentPtr[fieldNumber].fieldType == kTrainingType)
			{
			if (fieldIdentPtr[fieldNumber].loadedIntoClassStats)
				numberOfPixelsLoadedInClass +=
										fieldIdentPtr[fieldNumber].numberPixelsUsedForStats;
											
			}	// end "if (fieldIdentPtr[fieldNumber].fieldType == kTrainingType)"
			
		fieldNumber = fieldIdentPtr[fieldNumber].nextField;
		
		}	// end "while (fieldNumber != -1)"
	
	return (numberOfPixelsLoadedInClass); 
			
}	// end "GetNumberOfPixelsLoadedInClass"



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void GetProjectChannelMinMaxes
//
//	Software purpose:	The purpose of this routine is to find the overall project
//							minimum and maximum values for the selected project channels.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
//	 Value Returned:	None
// 
// Called By:			
//
//	Coded By:			Larry L. Biehl			Date: 01/28/2006
//	Revised By:			Larry L. Biehl			Date: 02/01/2006	

Boolean GetProjectChannelMinMaxes (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				UInt16*								channelListPtr, 
				HDoublePtr							channelMinPtr, 
				HDoublePtr							channelMaxPtr,
				double*								overallMinPtr,
				double*								overallMaxPtr)

{ 
	UInt32								classIndex,
											classStorage,
											index,
											numberClasses;
											
	Boolean								returnFlag = FALSE;
	
	
			// Initialize local variables.													
			
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	
			// Continue only if number of classes is one or more and if project		
			//	statistics are up-to-date													
	
	if (numberClasses > 0)
		{	
				// Initialize the channel min and max vectors.
				
		for (index=0; index<numberOutputChannels; index++)
			{
			channelMinPtr[index] = DBL_MAX;
			channelMaxPtr[index] = -DBL_MAX;
			
			}	// end "for (index=0; index<numberOutputChannels; index++)"
			
		*overallMinPtr = DBL_MAX;
		*overallMaxPtr = -DBL_MAX;
					
		for (classIndex=0; classIndex<numberClasses; classIndex++)
			{
					// Get the class storage number.											
						
			classStorage = gProjectInfoPtr->storageClass[classIndex];
			
					// Check if class "class" statistics are up to date				
						
			if (gProjectInfoPtr->classNamesPtr[classStorage].statsUpToDate)
				{
				GetClassChannelStatistics (numberOutputChannels, 
													classChannelStatsPtr, 
													channelListPtr, 
													(UInt16)classIndex);
				
				for (index=0; index<numberOutputChannels; index++)
					{
					channelMinPtr[index] = MIN (channelMinPtr[index], 
															classChannelStatsPtr[index].minimum);
					channelMaxPtr[index] = MAX (channelMaxPtr[index], 
															classChannelStatsPtr[index].maximum);
					
					}	// end "for (index=0; index<numberOutputChannels; index++)"
					
				returnFlag = TRUE;
						
				}	// end "if (...->classNamesPtr[classStorage].statsUpToDate)"
			
			}	// end "for (classIndex=0; ... 
			
		if (returnFlag)
			{
			for (index=0; index<numberOutputChannels; index++)
				{						
				*overallMinPtr = MIN (*overallMinPtr, channelMinPtr[index]);
				*overallMaxPtr = MAX (*overallMaxPtr, channelMaxPtr[index]);
				
				}	// end "for (index=0; index<numberOutputChannels; index++)"
			
			}	// end "if (returnFlag)"
			
		}	// end "if (numberClasses > 0)"
		
	return (returnFlag);
			
}	// end "GetProjectChannelMinMaxes" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void GetTransformedClassCovarianceMatrix
//
//	Software purpose:	The purpose of this routine is to compute a
//							transformed covariance matrix for the input class.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
//	 Value Returned:	None
// 
// Called By:			MaxLikeClsfierControl in classify.c
//							SetupClsfierStats in echo_classify.c
//							EvaluateCovariancesControl in other.c
//							LoadSeparabilityStatistics in separability.c
//							CreateStatisticsImages in statisticsImage.c
//
//	Coded By:			Larry L. Biehl			Date: 04/14/1993
//	Revised By:			Larry L. Biehl			Date: 07/13/2009	

void GetTransformedClassCovarianceMatrix (
				UInt16								numberOutputChannels, 
				HChannelStatisticsPtr			classChannelStatsPtr, 
				HCovarianceStatisticsPtr		classCovariancePtr, 
				UInt16*								channelListPtr, 
				UInt16								statClassNumber, 
				Boolean								squareOutputMatrixFlag, 
				SInt16								outputStatisticsCode,
				HDoublePtr							eigenVectorPtr, 
				HDoublePtr							tempMatrixPtr,
				UInt16								numberFeatures)

{
	UInt32								classStorage;
	
	Boolean								lOutputMatrixFlag,
											savedZeroVarianceFlag;
	
	
			// Reset 'set zero variance flag' to FALSE so that this operation will
			// not be done in the 'GetClassCovarianceMatrix' routine. We want to
			// wait until after the matrix has been transformed (if requested) before
			// this operation is done.
			
	savedZeroVarianceFlag = gProjectInfoPtr->setZeroVarianceFlag;
	gProjectInfoPtr->setZeroVarianceFlag = FALSE;
	
	lOutputMatrixFlag = squareOutputMatrixFlag;
	if (eigenVectorPtr != NULL && tempMatrixPtr != NULL)
		lOutputMatrixFlag = kSquareOutputMatrix;
						
	classStorage = gProjectInfoPtr->storageClass[statClassNumber];
	
			// Get the class covariance matrix.										

	GetClassCovarianceMatrix (
						numberOutputChannels, 
						classChannelStatsPtr, 
						classCovariancePtr, 
						channelListPtr, 
						statClassNumber,
						lOutputMatrixFlag,
						outputStatisticsCode,
						gProjectInfoPtr->classNamesPtr[classStorage].covarianceStatsToUse);
										
	if (eigenVectorPtr != NULL && tempMatrixPtr != NULL)
		TransformSymmetricMatrix (eigenVectorPtr,
											classCovariancePtr, 
											tempMatrixPtr, 
											classCovariancePtr, 
											numberFeatures,
											numberOutputChannels,
											squareOutputMatrixFlag);
	
	gProjectInfoPtr->setZeroVarianceFlag = savedZeroVarianceFlag;

	CheckMatrix (classCovariancePtr,
						squareOutputMatrixFlag,
						outputStatisticsCode,
						numberFeatures,
						statClassNumber,
						IDS_Project67,
						IDS_Project78,
						kUseListOneMessagePerClassFlag);

}	// end "GetTransformedClassCovarianceMatrix" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void GetStdDevVectorFromCovariance
//
//	Software purpose:	The purpose of this routine is to reduce the input standard 
//							deviation vector to the requested channels and transform
//							the data if requested.  The output will be a vector of double 
//							elements
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			ListClassStats in SStatLst.cpp
//							ListFieldStats in SStatLst.cpp
//
//	Coded By:			Larry L. Biehl			Date: 02/04/1999
//	Revised By:			Larry L. Biehl			Date: 02/04/1999	

void GetStdDevVectorFromCovariance (
				HDoublePtr							covariancePtr,
				UInt32								numberFeatures, 
				Boolean								squareInputMatrixFlag, 
				SInt16								inputStatisticsCode,
				HDoublePtr							outputStdDevPtr)

{			
	UInt32								index,
											indexSkip;
	
	
	if (outputStdDevPtr != NULL)
		{
		if (squareInputMatrixFlag)
			indexSkip = numberFeatures + 1;
		
		else	// !squareInputMatrixFlag
			indexSkip = 2;
				                                                      
		for (index=0; index<numberFeatures; index++)
			{
					// Make sure that the variances are not negative.
					 
			*outputStdDevPtr = sqrt (fabs (*covariancePtr));
			
			if (inputStatisticsCode == kMeanCovariance)
				{
				if (squareInputMatrixFlag)
					covariancePtr += indexSkip;
				
				else	// !squareInputMatrixFlag
					{
					covariancePtr += indexSkip;
					indexSkip++;
					
					}	// end "else !squareInputMatrixFlag"
				
				}	// end "if (inputStatisticsCode == kMeanCovariance)"
				
			else	// inputStatisticsCode != kMeanCovariance
				covariancePtr++;
			                                         
			outputStdDevPtr++;
			
			}	// end "for (index=0; index<..."
									
		}	// end "if (outputStdDevPtr != NULL)"

}	// end "GetStdDevVectorFromCovariance" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void InitializeChannelMaximums
//
//	Software purpose:	The purpose of this routine is set the memory for the
//							channel maximums to -DBL_MAX.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			ReadFieldStatistics in project.c
//
//	Coded By:			Larry L. Biehl			Date: 02/09/1992
//	Revised By:			Larry L. Biehl			Date: 12/20/2005	

void InitializeChannelMaximums (
				HChannelStatisticsPtr			channelStatsPtr, 
				SInt16								numberChannels)

{
	SInt32								channel;
	

	if (channelStatsPtr != NULL && numberChannels > 0)
		{
				// Initialize the memory for the statistics information					
		
		for (channel=0; channel<numberChannels; channel++)
			{
			channelStatsPtr->maximum = -DBL_MAX;
				
			channelStatsPtr++;
				
			}	// end "for (channel=0; channel<..." 
			
		}	// end "if (channelStatsPtr != NULL && numberChannels > 0)"

}	// end "InitializeChannelMaximums" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void InitializeChannelMinimums
//
//	Software purpose:	The purpose of this routine is set the memory for the
//							channel minimums to DBL_MAX.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 02/09/1992
//	Revised By:			Larry L. Biehl			Date: 12/31/2005	

void InitializeChannelMinimums (
				HChannelStatisticsPtr			channelStatsPtr, 
				SInt16								numberChannels)

{
	SInt32								channel;
	

	if (channelStatsPtr != NULL && numberChannels > 0)
		{
				// Initialize the memory for the statistics information					
		
		for (channel=0; channel<numberChannels; channel++)
			{
			channelStatsPtr->minimum = DBL_MAX;
				
			channelStatsPtr++;
				
			}	// end "for (channel=0; channel<..."  
			
		}	// end "if (channelStatsPtr != NULL && numberChannels > 0)"

}	// end "InitializeChannelMinimums" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ReduceChanStatsVector
//
//	Software purpose:	The purpose of this routine is to copy the channel
//							statistics vector for the requested channels from the input 
//							vector into the output vector.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned: 	None
//
// Called By:			GetClassChannelStatistics in statCompute.c
//
//	Coded By:			Larry L. Biehl			Date: 12/07/1993
//	Revised By:			Larry L. Biehl			Date: 12/07/1993

void ReduceChanStatsVector (
				HChannelStatisticsPtr			inputChanStatsPtr, 
				HChannelStatisticsPtr			outputChanStatsPtr, 
				UInt16								numOutFeatures, 
				UInt16*								featureListPtr)

{
	UInt16								channel,
											channelIndex;
	
	
	channelIndex = 0;
	
	for (channel=0; channel<numOutFeatures; channel++)
		{
		if (featureListPtr)
			channelIndex = featureListPtr[channel];
			
		*outputChanStatsPtr = inputChanStatsPtr[channelIndex];
		
		outputChanStatsPtr++;
		channelIndex++;
		
		}	// end "for (channel=0; channel<numOutFeatures; channel++)" 

}	// end "ReduceChanStatsVector" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ReduceStdDevVector
//
//	Software purpose:	The purpose of this routine is to copy the standard deviations
//							for those channels that will be used from the input 
//							vector into the output vector.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned: 	None
//
// Called By:			
//
//	Coded By:			Larry L. Biehl			Date: 10/09/1997
//	Revised By:			Larry L. Biehl			Date: 10/09/1997

void ReduceStdDevVector (
				HChannelStatisticsPtr			inputChannelStatsPtr, 
				HDoublePtr							outputStdDevPtr, 
				SInt16								numOutFeatures, 
				SInt16*								featureListPtr)

{
	UInt32								channel,
											channelIndex;
	
	
	if (inputChannelStatsPtr != NULL && outputStdDevPtr != NULL)
		{
		channelIndex = 0;
		
		for (channel=0; channel<(UInt32)numOutFeatures; channel++)
			{
			if (featureListPtr != NULL)
				channelIndex = featureListPtr[channel];
				
			*outputStdDevPtr = inputChannelStatsPtr[channelIndex].standardDev;
			
			outputStdDevPtr++;
			channelIndex++;
			
			}	// end "for (channel=0; channel<numOutFeatures; channel++)" 
			
		}	// end "if (inputChannelStatsPtr != NULL && outputStdDevPtr != NULL)"

}	// end "ReduceStdDevVector"



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ResetZeroVariances
//
//	Software purpose:	The purpose of this routine is to set any zero
//							variances to the user specified value.  This would
//							be done so that a covariance matrix that contained
//							one or more channels with 0 variance could be
//							inverted.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	True if a zero variance was found and set to a small value.
//							False if no zero variance was found.
// 
// Called By:			GetTransformedCommonCovariance in SClassfy.cpp
//							GetClassCovarianceMatrix in SStatCom.cpp
//							GetTransformedClassCovarianceMatrix in SStatCom.cpp
//							behzad_ModifyStatistics in SStatEnh.cpp
//
//	Coded By:			Larry L. Biehl			Date: 07/16/1993
//	Revised By:			Larry L. Biehl			Date: 10/21/1999	

Boolean ResetZeroVariances (
				HCovarianceStatisticsPtr		covariancePtr, 
				Boolean								squareMatrixFlag, 
				SInt16								outputStatisticsCode, 
				UInt32								numberFeatures)

{
	UInt32								feature,
											indexSkip;
											
	Boolean								varianceResetFlag = FALSE;
	
	
	if (!covariancePtr || numberFeatures == 0)
																							return (FALSE);
	
	indexSkip = 2;		
	if (squareMatrixFlag)																				
		indexSkip = numberFeatures + 1;
		
	for (feature=0; feature<numberFeatures; feature++)
		{
		if (*covariancePtr == 0.)
			{
			*covariancePtr = (double)gProjectInfoPtr->zeroVarianceFactor;
			varianceResetFlag = TRUE;
			
			}	// end "if (*covariancePtr == 0.)"
			
		if (outputStatisticsCode == kMeanStdDevOnly)
			covariancePtr++;
			
		else if (squareMatrixFlag)
			covariancePtr += indexSkip;
			
		else	// !squareMatrixFlag 
			{
			covariancePtr += indexSkip;
			indexSkip++;
			
			}	// end "else !squareMatrixFlag" 
				
		}	// end "for (feature=0; feature<numberFeatures; feature++)"
		
	return (varianceResetFlag); 
			
}	// end "ResetZeroVariances" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void ResetForAllVariancesEqual
//
//	Software purpose:	The purpose of this routine is to check for case when all
//							variances and covariances are equal which implies that the 
//							channels are identical. 
//
//							If all variances and covariances are equal, the covariances
//							are set to 0.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	True if all variances and covariances are equal.
//							False if variances and covariance are not equal.
// 
// Called By:			GetTransformedCommonCovariance in SClassfy.cpp
//							GetClassCovarianceMatrix in SStatCom.cpp
//							GetTransformedClassCovarianceMatrix in SStatCom.cpp
//							behzad_ModifyStatistics in SStatEnh.cpp
//
//	Coded By:			Larry L. Biehl			Date: 02/21/2003
//	Revised By:			Larry L. Biehl			Date: 02/21/2003	

Boolean ResetForAllVariancesEqual (
				HCovarianceStatisticsPtr		covariancePtr, 
				Boolean								squareMatrixFlag, 
				SInt16								statisticsCode, 
				UInt32								numberFeatures)

{
	double								compareVariance;
	
	HCovarianceStatisticsPtr		savedCovariancePtr;
	
	UInt32								feature1,
											feature2,
											upperRightIndexSkip;
											
	Boolean								variancesEqualFlag = TRUE;
	
	
	if (covariancePtr == NULL || 
				numberFeatures <= 1 || 
						statisticsCode == kMeanStdDevOnly)
																							return (FALSE);
	
	savedCovariancePtr = covariancePtr;
	upperRightIndexSkip = numberFeatures - 1;
		
	compareVariance = *covariancePtr;
	for (feature1=0; feature1<numberFeatures; feature1++)
		{
		for (feature2=0; feature2<=feature1; feature2++)
			{
			if (compareVariance != *covariancePtr)
				{
				variancesEqualFlag = FALSE;
				break;
				
				}	// end "if (compareVariance != *covariancePtr)"
				
			covariancePtr++;
				
			}	// end "for (feature2=0; feature2<=feature1; feature2++)"
			
		if (squareMatrixFlag)
			{
			covariancePtr += upperRightIndexSkip;
			upperRightIndexSkip--;
			
			}	// end "if (squareMatrixFlag)"
				
		}	// end "for (feature1=0; feature1<numberFeatures; feature1++)"
		
	if (variancesEqualFlag)
		{
				// The variance are all equal. Set the covariance values to 0.
				
		covariancePtr = savedCovariancePtr;
		upperRightIndexSkip = numberFeatures - 1;
		for (feature1=0; feature1<numberFeatures; feature1++)
			{
			for (feature2=0; feature2<feature1; feature2++)
				{
				*covariancePtr = 0.;
				covariancePtr++;
				
				}	// end "for (feature2=0; feature2<feature1; feature2++)"
			
					// Skip the diagonal position.
						
			covariancePtr++;
			
			if (squareMatrixFlag)
				{
						// Skip the upper right portion of matrix. It will be handled
						// below with "CopyLowerToUpperSquareMatrix".
						
				covariancePtr += upperRightIndexSkip;
				upperRightIndexSkip--;
				
				}	// end "if (squareMatrixFlag)"
					
			}	// end "for (feature=0; feature<numberFeatures; feature++)"
	
				// Now copy the lower left part of the matrix to the upper right		
				// part if the output is to be a square matrix.								
					
		if (squareMatrixFlag)
			{
			covariancePtr = savedCovariancePtr;	
			CopyLowerToUpperSquareMatrix ((UInt16)numberFeatures, covariancePtr);
			
			}	// end "if (squareMatrixFlag)" 
		
		}	// end "if (variancesEqualFlag)"
		
	return (variancesEqualFlag); 
			
}	// end "ResetForAllVariancesEqual" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void SetClassCovarianceStatsToUse
//
//	Software purpose:	The purpose of this routine is to set the covariance stats
//							to be used for the specified class to the input setting.
//
//	Parameters in:		None
//
//	Parameters out:	
//
// Value Returned:	None
// 
// Called By:			StatisticsWMouseDn in SStatist.cpp
//
//	Coded By:			Larry L. Biehl			Date: 12/18/1997
//	Revised By:			Larry L. Biehl			Date: 02/24/2000	

void SetClassCovarianceStatsToUse (
				UInt16								covarianceStatsToUse)

{
	HPClassNamesPtr					classNamesPtr;
	
	UInt32								classIndex,
											classStorage,
											numberClasses;
	
	
	if (covarianceStatsToUse <= 0 || covarianceStatsToUse > kEnhancedStats)
																									return;
																							
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	
	classStorage = gProjectInfoPtr->storageClass[gProjectInfoPtr->currentClass];
	classNamesPtr[classStorage].covarianceStatsToUse = covarianceStatsToUse;
	
			// Initialize the project covariance stats to the input class setting.
			
	gProjectInfoPtr->covarianceStatsToUse = covarianceStatsToUse;
	
			// Now make sure that the project setting is representative of all
			// of the class covariance-to-use settings.
		
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	for (classIndex=0; classIndex<numberClasses; classIndex++)
		{
				// Get the class storage number.	
		
		classStorage = gProjectInfoPtr->storageClass[classIndex];
		if (covarianceStatsToUse != classNamesPtr[classStorage].covarianceStatsToUse)
			{
			gProjectInfoPtr->covarianceStatsToUse = kMixedStats;
			break;
			
			}	// end "if (covarianceStatsToUse != ..."
									
		}	// end "for (classIndex=0; classIndex<numberClasses; classIndex++)"
		
			// Force the projet menu to be updated in case it was changed.
			
	gUpdateProjectMenuItemsFlag = TRUE;
		
			// Determine if the project statistics need to be updated.
		
	classStorage = gProjectInfoPtr->storageClass[gProjectInfoPtr->currentClass];
	if (covarianceStatsToUse == kLeaveOneOutStats)
		{
		if (classNamesPtr[classStorage].mixingParameterCode == kComputedOptimum &&
											classNamesPtr[classStorage].looCovarianceValue < 0)
			{
			gProjectInfoPtr->statsUpToDate = FALSE;
			gProjectInfoPtr->classNamesPtr[classStorage].statsUpToDate = FALSE;
				
			}	// end "if (classNamesPtr[classStorage].looCovarianceValue < 0)"
			
		}	// end "if (covarianceStatsToUse == kLeaveOneOutStats)"
	
			// Hilite the 'update statistics' control if needed.						
	
	if (!gProjectInfoPtr->statsUpToDate && gProjectInfoPtr->updateControlH)
		MHiliteControl (gProjectWindow, gProjectInfoPtr->updateControlH, 0);
					
}	// end "SetClassCovarianceStatsToUse" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void SetClassListMessageFlag
//
//	Software purpose:	The purpose of this routine is to set the list message flag for
//							each class to the input value.
//
//	Parameters in:		True or False value to set the list message flag to.
//
//	Parameters out:	None
//
// Value Returned:	None
// 
// Called By:			CalculateSeparabilityControl in SFeatSel.cpp
//
//	Coded By:			Larry L. Biehl			Date: 02/22/2001
//	Revised By:			Larry L. Biehl			Date: 02/22/2001	

void SetClassListMessageFlag (
				Boolean								listMessageFlag)

{
	HPClassNamesPtr					classNamesPtr;
	
	UInt32								classIndex,
											classStorage,
											numberClasses;
	
	
			// Set the list message flag in each class.
		
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	
	for (classIndex=0; classIndex<numberClasses; classIndex++)
		{
		classStorage = gProjectInfoPtr->storageClass[classIndex];
		classNamesPtr[classStorage].listMessageFlag = listMessageFlag;
									
		}	// end "for (classIndex=0; classIndex<numberClasses; classIndex++)"
					
}	// end "SetClassListMessageFlag" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		void SetProjectCovarianceStatsToUse
//
//	Software purpose:	The purpose of this routine is to set the covariance stats
//							to be used for all classes to the input setting.
//
//	Parameters in:		None
//
//	Parameters out:	
//
// Value Returned:	None
// 
// Called By:			Menus in menus.cpp
//							AddFieldStatsToClassStats in SEdtStat.cpp
//							CutClass in SEdtStat.cpp
//							RemoveFieldStatsFromClassStats in SEdtStat.cpp
//							UndoCutClass in SEdtStat.cpp
//							ReadProjectFile in SProject.cpp
//							CopyEnhancedStatsToProject in statisticsEnhancement
//
//	Coded By:			Larry L. Biehl			Date: 09/22/1997
//	Revised By:			Larry L. Biehl			Date: 05/04/2017	

void SetProjectCovarianceStatsToUse (
				UInt16								covarianceStatsToUse)

{
	HPClassNamesPtr					classNamesPtr;
	
	UInt32								classIndex,
											classStorage,
											numberClasses;
											
	SInt16								projectCovarianceStatsToUse = kOriginalStats,
											savedCovarianceStatsToUse;
	
	Boolean								enhancedStatsExistFlag = FALSE;
	

			// Even if the input 'covarianceStatsToUse' is the same as the project
			// setting, verify that the class settings are consistant.
			// Also make sure that the class settings are not set to 'Mixed Stats'.
			// Also make sure that the project setting is consistant with the class
			// settings; i.e. project setting is not mixed if all classes have the
			// same setting.
				
	//if (gProjectInfoPtr->covarianceStatsToUse == covarianceStatsToUse)
	//																								return;
																							
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	
			// Set the covariance-to-use code for the project
			
	gProjectInfoPtr->covarianceStatsToUse = covarianceStatsToUse;
		
	for (classIndex=0; classIndex<numberClasses; classIndex++)
		{
				// Get the class storage number.										
					
		classStorage = gProjectInfoPtr->storageClass[classIndex];
		
				// Just make sure that the class covariance to use is not set to mixed.
				
		if (classNamesPtr[classStorage].covarianceStatsToUse == kMixedStats)
			classNamesPtr[classStorage].covarianceStatsToUse = kOriginalStats;
		
		savedCovarianceStatsToUse = classNamesPtr[classStorage].covarianceStatsToUse;
		
				// Do not make any changes in the class stats to use if the input option
				// is mixed stats. Just check if everything makes sense.
				
		if (covarianceStatsToUse != kMixedStats)
			classNamesPtr[classStorage].covarianceStatsToUse = covarianceStatsToUse;
			
		if (classNamesPtr[classStorage].modifiedStatsFlag)
			enhancedStatsExistFlag = TRUE;
		
		if (classNamesPtr[classStorage].covarianceStatsToUse == kEnhancedStats)
			{
			if (!classNamesPtr[classStorage].modifiedStatsFlag)
				{
						// Enhanced statistics do not exist for this class. Indicate
						// that the statistics being used for the project are mixed
						// (as long as training pixels exist for the class)
						// continue to use the previous stats-to-use setting.
				
				if (classNamesPtr[classStorage].numberTrainPixels > 0)
					gProjectInfoPtr->covarianceStatsToUse = kMixedStats;
					
				classNamesPtr[classStorage].covarianceStatsToUse =
																			savedCovarianceStatsToUse;
				if (savedCovarianceStatsToUse == kEnhancedStats)
					classNamesPtr[classStorage].covarianceStatsToUse = kOriginalStats;
				
				}	// end "if (covarianceStatsToUse == kLeaveOneOutStats && ..."
				
			}	// end "if (covarianceStatsToUse == kEnhancedStats)"
		
				// Determine if the project statistics need to be updated.
		
		else if (classNamesPtr[classStorage].covarianceStatsToUse == kLeaveOneOutStats)
			{
			if (classNamesPtr[classStorage].mixingParameterCode == kComputedOptimum &&
												classNamesPtr[classStorage].looCovarianceValue < 0)
				{
				gProjectInfoPtr->statsUpToDate = FALSE;
				classNamesPtr[classStorage].statsUpToDate = FALSE;
			
						// Force the commom covariance for leave-one-out stats to 
						// be recalculated.
						
				gProjectInfoPtr->numberCommonCovarianceClasses = 0;
					
				}	// end "if (covarianceStatsToUse == ..."
				
			}	// end "else if (...covarianceStatsToUse == kLeaveOneOutStats)"
			
		if (classIndex == 0)
			projectCovarianceStatsToUse = 
											classNamesPtr[classStorage].covarianceStatsToUse;
			
		else if (projectCovarianceStatsToUse != kMixedStats)
			{
			if (projectCovarianceStatsToUse != 
									(SInt16)classNamesPtr[classStorage].covarianceStatsToUse)
				projectCovarianceStatsToUse = kMixedStats;
			
			}	// end "else if (projectCovarianceStatsToUse != kMixedStats)"
									
		}	// end "for (classIndex=0; classIndex<numberClasses; classIndex++)"
		
	gProjectInfoPtr->covarianceStatsToUse = projectCovarianceStatsToUse;
		
	gProjectInfoPtr->enhancedStatsExistFlag = enhancedStatsExistFlag;
	
	if (covarianceStatsToUse == kEnhancedStats)
		{
		if (!enhancedStatsExistFlag)
			gProjectInfoPtr->covarianceStatsToUse = kOriginalStats;
		
		}	// end "if (covarianceStatsToUse == kEnhancedStats)"

			// Force the statistics window to be updated if needed.
			
	InvalPopUpCovarianceToUse ();
	
			// Hilite the 'update statistics' control if needed.						
	
	if (!gProjectInfoPtr->statsUpToDate && gProjectInfoPtr->updateControlH)
		MHiliteControl (gProjectWindow, gProjectInfoPtr->updateControlH, 0);
		
	gUpdateProjectMenuItemsFlag = TRUE;
					
}	// end "SetProjectCovarianceStatsToUse" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean SetupModifiedStatsMemory
//
//	Software purpose:	The purpose of this routine is to update the 
//							memory allocated for the modified class statistics
//							if needed.  
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None
//
// Called By:			CopyEnhancedStatsToProject in statisticsEnhancement.c
//
//	Coded By:			Larry L. Biehl			Date: 12/16/1993
//	Revised By:			Larry L. Biehl			Date: 07/31/1997	

Boolean SetupModifiedStatsMemory (
				UInt32		 						numberClasses)

{
	double								doubleBytesNeeded;
	
	UInt32								bytesNeeded;
	
	Boolean								changedFlag,
											continueFlag;
	
	
			// Initialize local variables.													
			
	continueFlag = TRUE;
	
			// Change size of handle for modified class covariance 					
			// statistics if needed.															
		
	bytesNeeded = numberClasses * gProjectInfoPtr->numberCovarianceEntries *
																	sizeof (CovarianceStatistics);
															
	doubleBytesNeeded = (double)numberClasses * 
									(double)gProjectInfoPtr->numberCovarianceEntries *
																	sizeof (CovarianceStatistics);
	
	continueFlag = (doubleBytesNeeded < LONG_MAX);
	
	if (continueFlag)
		{
				// Unlock project statistics memory.										
			
  		UnlockProjectMemory (&gProjectInfoPtr, 3, NULL);	
  														
		gProjectInfoPtr->modifiedClassCovStatsPtr = 
						(HCovarianceStatisticsPtr)CheckHandleSize (
													&gProjectInfoPtr->modifiedClassCovStatsHandle,
													&continueFlag,  
													&changedFlag, 
													bytesNeeded);
		
		if (continueFlag)
			{
					// Change size of handle for modified class means if needed.		
					
			bytesNeeded = numberClasses * 
										gProjectInfoPtr->numberStatisticsChannels * 
																			sizeof (ChannelStatistics);
							
			gProjectInfoPtr->modifiedClassChanStatsPtr = 
						(HChannelStatisticsPtr)CheckHandleSize (
												&gProjectInfoPtr->modifiedClassChanStatsHandle,
												&continueFlag,  
												&changedFlag, 
												bytesNeeded);
			
			}	// end "if (continueFlag)" 
			
		gProjectInfoPtr->moveMemoryFlag = TRUE;
						
		LockProjectMemory (NULL, 0, &gProjectInfoPtr);
						
		}	// end "if (continueFlag)" 
		
	return (continueFlag);
		
}	// end "SetupModifiedStatsMemory" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		Boolean SetupStatsMemory
//
//	Software purpose:	The purpose of this routine is to update the 
//							memory allocated for the field and class statistics
//							if needed.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:			SaveClusterStatistics in cluster.c
//							SaveProjectFile in project.c
//							ReadProjectFile in project.c
//							UpdateStatsControl in statCompute.c
//
//	Coded By:			Larry L. Biehl			Date: 11/16/1988
//	Revised By:			Larry L. Biehl			Date: 01/13/2004

Boolean SetupStatsMemory (void)

{
	double								doubleBytesNeeded;
	
	Ptr									spareMemoryPtr;
	
	UInt32								longBytesNeeded,
											numberChannels,
											numberCovarianceChannels,
											numberStorageSets;
								
	Boolean								changedFlag, 
											continueFlag,
											tooLargeMemoryBlockFlag;
	
	
			// Initialize local variables.													
			
	continueFlag = TRUE;
	tooLargeMemoryBlockFlag = FALSE;
	numberChannels = gProjectInfoPtr->numberStatisticsChannels;
	numberCovarianceChannels = gProjectInfoPtr->numberCovarianceEntries;
	
			// Unlock project statistics memory.											
			
  	UnlockProjectMemory (&gProjectInfoPtr, 3, NULL);
	
			// First we will get a block of memory that we want to be sure is available
			// after allocating memory for the statistics. The block requested is
			// the same size as that needed for a full segment of code.
	
	spareMemoryPtr = (char*)MNewPointer ((SInt32)gSpareCodeSize);
	continueFlag = (spareMemoryPtr != NULL);
	
	if (continueFlag)
		{
	  	numberStorageSets = gProjectInfoPtr->numberStorageStatFields;
	  	if (gProjectInfoPtr->keepClassStatsOnlyFlag)
	  		numberStorageSets = 1;
	  	
				// Change size of handle for field first order training					
				// statistics (mean, std dev and sum) if needed.							
				
		longBytesNeeded = 
						numberStorageSets * numberChannels * sizeof (ChannelStatistics);
																
		doubleBytesNeeded =(double)numberStorageSets * 
										(double)numberChannels * sizeof (ChannelStatistics);
						
		if (doubleBytesNeeded < LONG_MAX)
			gProjectInfoPtr->fieldChanStatsPtr = (HChannelStatisticsPtr)CheckHandleSize (
															&gProjectInfoPtr->fieldChanStatsHandle,
															&continueFlag,  
															&changedFlag, 
															longBytesNeeded);
		
		else	// doubleBytesNeeded >= LONG_MAX 
			{
			continueFlag = FALSE;
			tooLargeMemoryBlockFlag = TRUE;
			
			}	// end "else doubleBytesNeeded >= LONG_MAX"
										
		}	// end "if (continueFlag)"
		
	if (continueFlag)
		{
				// Change size of handle for field sums of squares training 		
				// statistics if needed.														
				
		longBytesNeeded = numberStorageSets * 
									numberCovarianceChannels * sizeof (SumSquaresStatistics);
																
		doubleBytesNeeded =(double)numberStorageSets * 
						(double)numberCovarianceChannels * sizeof (SumSquaresStatistics);
		
		if (doubleBytesNeeded < LONG_MAX)														
			gProjectInfoPtr->fieldSumSquaresStatsPtr = 
									(SumSquaresStatisticsPtr)CheckHandleSize (
													&gProjectInfoPtr->fieldSumSquaresStatsHandle,
													&continueFlag,   
													&changedFlag, 
													longBytesNeeded);
		
		else	// doubleBytesNeeded >= LONG_MAX 
			{
			continueFlag = FALSE;
			tooLargeMemoryBlockFlag = TRUE;
			
			}	// end "else doubleBytesNeeded >= LONG_MAX"
			
		}	// end "if (continueFlag)" 
		
	if (continueFlag && gProjectInfoPtr->keepClassStatsOnlyFlag)
		{
	  	numberStorageSets = gProjectInfoPtr->numberStorageClasses;
	  	
				// Change size of handle for class first order training					
				// statistics (mean, std dev and sum) if needed.							
				
		longBytesNeeded = 
						numberStorageSets * numberChannels * sizeof (ChannelStatistics);
						
		gProjectInfoPtr->classChanStatsPtr = 
						(HChannelStatisticsPtr)CheckHandleSize (
															&gProjectInfoPtr->classChanStatsHandle,
															&continueFlag,   
															&changedFlag, 
															longBytesNeeded);
		
		}	// end "if (continueFlag && ...->keepClassStatsOnlyFlag)" 
		
	if (continueFlag && gProjectInfoPtr->keepClassStatsOnlyFlag)
		{
				// Change size of handle for class sums of squares training 	
				// statistics if needed.													
				
		longBytesNeeded = numberStorageSets * 
									numberCovarianceChannels * sizeof (SumSquaresStatistics);
																
		doubleBytesNeeded =(double)numberStorageSets * 
						(double)numberCovarianceChannels * sizeof (SumSquaresStatistics);
		
		if (doubleBytesNeeded < LONG_MAX)														
			gProjectInfoPtr->classSumSquaresStatsPtr = 
								(SumSquaresStatisticsPtr)CheckHandleSize (
													&gProjectInfoPtr->classSumSquaresStatsHandle,
													&continueFlag,    
													&changedFlag, 
													longBytesNeeded);
		
		else	// doubleBytesNeeded >= LONG_MAX 
			{
			continueFlag = FALSE;
			tooLargeMemoryBlockFlag = TRUE;
			
			}	// end "else doubleBytesNeeded >= LONG_MAX"
			
		}	// end "if (continueFlag && ...->keepClassStatsOnlyFlag)"
		
			// Change size of handles for common covariance statistics if needed.
			
	if (continueFlag && DetermineIfLOOCProjectMemoryNeeded ())
		{
					// Change size of handle for common class means if needed.		
					
		longBytesNeeded = numberChannels * sizeof (ChannelStatistics);
								
		CheckHandleSize (&gProjectInfoPtr->commonChannelStatsHandle, 
								&continueFlag,  
								&changedFlag, 
								longBytesNeeded);
		
		if (continueFlag)
			{	
					// Need to allow for the common covariance and a subset of the common
					// covariance which will be used by the various processors.
					
			longBytesNeeded = 2 * numberCovarianceChannels * sizeof (CovarianceStatistics);
																
			doubleBytesNeeded = 2 * 
							(double)numberCovarianceChannels * sizeof (CovarianceStatistics);
  												
			if (doubleBytesNeeded < LONG_MAX)			
				CheckHandleSize (&gProjectInfoPtr->commonCovarianceStatsHandle,
										&continueFlag,  
										&changedFlag, 
										longBytesNeeded);
										 		
			else	// doubleBytesNeeded >= LONG_MAX 
				{
				continueFlag = FALSE;
				tooLargeMemoryBlockFlag = TRUE;
				
				}	// end "else doubleBytesNeeded >= LONG_MAX"
								
			}	// end "if (continueFlag)" 
		
		}	// end "if (continueFlag && DetermineIfLOOCProjectMemoryNeeded ())"
		
	CheckAndDisposePtr (spareMemoryPtr);
			
	gProjectInfoPtr->moveMemoryFlag = TRUE;
	
			// Lock project statistics memory.												
			
	LockProjectMemory (NULL, 0, &gProjectInfoPtr);
	
	if (tooLargeMemoryBlockFlag)
		DisplayAlert (kErrorAlertID, 
							kStopAlert, 
							kAlertStrID, 
							IDS_Alert124,
							0, 
							NULL);
		
	return (continueFlag);
		
}	// end "SetupStatsMemory" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		SInt16 UpdateClassAreaStats
//
//	Software purpose:	The purpose of this routine is to update the
//							statistics for the given class.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:			UpdateProjectAreaStats in SStatCom.cpp
//							UpdateStatsControl in SStatCom.cpp
//
//	Coded By:			Larry L. Biehl			Date: 11/16/1988
//	Revised By:			Larry L. Biehl			Date: 10/26/1999	

SInt16 UpdateClassAreaStats (
				FileIOInstructionsPtr			fileIOInstructionsPtr, 
				UInt32								classNumber)

{
	HChannelStatisticsPtr			classChanPtr;
	HSumSquaresStatisticsPtr		classSumSquaresPtr;
	
	HPClassNamesPtr					classNamesPtr;
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	UInt32								classStorage,
											fieldCount;
											
	SInt16								fieldNumber,
											numberChannels;
	
	
			// Initialize local variables.													
			
	numberChannels = gProjectInfoPtr->numberStatisticsChannels;
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	
			// Get the class storage number.													
						
	classStorage = gProjectInfoPtr->storageClass[classNumber];
	
			// Continue only if the number of fields in the class is one or more	
			// and if the class statistics are not up-to-date							
	
	if (classNamesPtr[classStorage].numberOfTrainFields > 0 && 
														!classNamesPtr[classStorage].statsUpToDate)
		{
				// Set up status dialog. Class name, and number of						
				// training fields for the class.											
				
		LoadDItemString (gStatusDialogPtr, 
								IDC_Status6, 
								(Str255*)&classNamesPtr[classStorage].name);
		LoadDItemValue (gStatusDialogPtr, 
								IDC_Status10,
								(SInt32)classNamesPtr[classStorage].numberOfTrainFields);
				
		if (gProjectInfoPtr->keepClassStatsOnlyFlag)
			GetProjectStatisticsPointers (kClassStatsOnly, 
													classStorage, 
													&classChanPtr, 
													&classSumSquaresPtr,
													NULL,
													NULL);
		
		fieldNumber = classNamesPtr[classStorage].firstFieldNumber;
		fieldCount = 1;
		while (fieldNumber != -1)
			{
			fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];
	
					// Make certain that field is training field							
					
			if (fieldIdentPtr->fieldType == kTrainingType &&
															fieldIdentPtr->pointType != kMaskType)
				{
						// Set up field count in status dialog.							
				
				LoadDItemValue (gStatusDialogPtr, IDC_Status8, fieldCount);
				
						// Check if field "fieldNumber" statistics are up to date	
					
				if (!fieldIdentPtr->statsUpToDate)
					if (UpdateFieldAreaStats (fileIOInstructionsPtr, fieldNumber) <= 0) 
																								return (0);
				
						// Add the field statistics to the class statistics if it	
						// hasn't been done.														
						
				if (!fieldIdentPtr->loadedIntoClassStats)
					{			
					if (gProjectInfoPtr->keepClassStatsOnlyFlag)
						AddToClassStatistics (numberChannels,
														classChanPtr, 
														classSumSquaresPtr, 
														numberChannels, 
														NULL, 
														gProjectInfoPtr->fieldChanStatsPtr, 
														gProjectInfoPtr->fieldSumSquaresStatsPtr, 
														kTriangleOutputMatrix,
														gProjectInfoPtr->statisticsCode,
														gProjectInfoPtr->statisticsCode);
						
					classNamesPtr[classStorage].numberStatisticsPixels += 
															fieldIdentPtr->numberPixelsUsedForStats;
												
					fieldIdentPtr->loadedIntoClassStats = TRUE;
					
					}	// end "if (!fieldIdentPtr->loadedIntoClassStats)" 
				
				fieldCount++;
								
				}	// end "fieldIdentPtr->fieldType == kTrainingType && ..." 
				
			fieldNumber = fieldIdentPtr->nextField;
			
			}	// end "while (fieldNumber != -1)" 
		
				// Do not set this flag for now. Will allow the possibility for user
				// to have enhanced statstistics that are not based on current
				// 'original' statistics.
					
		//classNamesPtr[classStorage].modifiedStatsFlag = FALSE;
			
		}	// end "if (classNamesPtr[classStorage].number ..."
	
			// Indicate that routine completed normally.								
			
	return (1);
		
}	// end "UpdateClassAreaStats" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		SInt16 UpdateFieldAreaStats
//
//	Software purpose:	The purpose of this routine is to update the
//							statistics for the given field.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	None	
// 
// Called By:			UpdateClassAreaStats in SStatCom.cpp
//							UpdateStatsControl in SStatCom.cpp
//
//	Coded By:			Larry L. Biehl			Date: 11/16/1988
//	Revised By:			Larry L. Biehl			Date: 04/30/2013	

SInt16 UpdateFieldAreaStats (
				FileIOInstructionsPtr			fileIOInstructionsPtr, 
				UInt16								fieldNumber)

{
	HChannelStatisticsPtr			fieldChanPtr;
	HSumSquaresStatisticsPtr		fieldSumSquaresPtr;
	
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	UInt32								fieldStatsIndex;
	UInt16								numberChannels;
	SInt16								returnCode;
	
	
			// Make certain that input values make sense									
			
	if (fieldNumber >= (UInt16)gProjectInfoPtr->numberStorageFields) 	
																								return (0);
	if (fileIOInstructionsPtr->fileInfoPtr == NULL)													
																								return (0);
																						
	fieldIdentPtr = &gProjectInfoPtr->fieldIdentPtr[fieldNumber];					
	if (fieldIdentPtr->fieldType != kTrainingType)		
																								return (0);
	if (fieldIdentPtr->pointType == kClusterType)		
																								return (0);
																						
			// If this is a mask type field then those statistics will be
			// computed later.
										
	if (fieldIdentPtr->pointType == kMaskType)		
																								return (1);
	
			// Initialize local variables.													
			
	numberChannels = gProjectInfoPtr->numberStatisticsChannels;
	
			// Get field statistics set number.												
			
	fieldStatsIndex = fieldIdentPtr->trainingStatsNumber;
		
	GetProjectStatisticsPointers (kFieldStatsOnly, 
											fieldStatsIndex, 
											&fieldChanPtr, 
											&fieldSumSquaresPtr,
											NULL,
											NULL);
							
			// Initialize the memory for the field statistics when only the class
			// stats are being kept. For this case the same location in memory is
			// being used for each field and then it is added into the class
			// statistics.
										
	if (gProjectInfoPtr->keepClassStatsOnlyFlag)
		ZeroStatisticsMemory (fieldChanPtr, 
										fieldSumSquaresPtr, 
										numberChannels,
										gProjectInfoPtr->statisticsCode,
										kTriangleOutputMatrix);
	
			// Determine if field is described by a polygon or a rectangle.  If	
			// the field is described by a polygon then create a region for the	
			// field.																				
	
	GetFieldBoundary (gProjectInfoPtr, &gAreaDescription, fieldNumber);
															
	returnCode = GetAreaStats (fileIOInstructionsPtr, 
										fieldChanPtr, 
										fieldSumSquaresPtr, 
										(UInt16*)gProjectInfoPtr->channelsPtr, 
										numberChannels, 
										fileIOInstructionsPtr->fileInfoPtr->noDataValueFlag, 
										gProjectInfoPtr->statisticsCode,
										NULL,
										NULL);
   
	if (returnCode	== 1)
		{
		fieldIdentPtr->numberPixelsUsedForStats = gAreaDescription.numSamplesPerChan;
		
				// Compute the first order statistics.																		
		
		if (!gProjectInfoPtr->keepClassStatsOnlyFlag)
			ComputeMeanStdDevVector (fieldChanPtr,
												fieldSumSquaresPtr, 
												numberChannels, 
												fieldIdentPtr->numberPixelsUsedForStats,
												gProjectInfoPtr->statisticsCode,
												kTriangleInputMatrix);
										
		if (gProjectInfoPtr->keepClassStatsOnlyFlag)
			fieldIdentPtr->statsUpToDate = FALSE;
			
		else	// !gProjectInfoPtr->keepClassStatsOnlyFlag
			fieldIdentPtr->statsUpToDate = TRUE;
	
				// Indicate that statistics have been loaded into the project.		
			
		gProjectInfoPtr->statsLoaded = TRUE;
	
				// Indicate that project information has changed.						
	 
		gProjectInfoPtr->changedFlag = TRUE;
	
		}	// end "if (returnCode	== 1)" 
		
			// Dispose of the region handle if needed.									
	
	CloseUpAreaDescription (&gAreaDescription);
	
			// Indicate that routine completed normally.								
			
	return (returnCode);
		
}	// end "UpdateFieldAreaStats" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		SInt16 UpdateProjectAreaStats
//
//	Software purpose:	The purpose of this routine is to update the
//							statistics for the project.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	1 = Update completed okay
//							0 = Update did not complete because of a problem
// 
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 11/16/1988
//	Revised By:			Larry L. Biehl			Date: 10/26/1999	

SInt16 UpdateProjectAreaStats (
				FileIOInstructionsPtr			fileIOInstructionsPtr)

{
	UInt32								classIndex,
											classStorage,
											numberClasses;
	
	
			// Initialize local variables.													
			
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	
			// Continue only if number of classes is one or more and if project		
			//	statistics are not up-to-date													
	
	if (numberClasses > 0 && !gProjectInfoPtr->statsUpToDate)
		{
				// Set up status dialog.  Load in number of classes.					
				
		LoadDItemValue (gStatusDialogPtr, IDC_Status5, (SInt32)numberClasses);
						
		for (classIndex=0; classIndex<numberClasses; classIndex++)
			{
					// Set up status dialog.  Load in class count.						
				
			LoadDItemValue (gStatusDialogPtr, IDC_Status3, (SInt32)classIndex+1);
			
					// Get the class storage number.											
						
			classStorage = gProjectInfoPtr->storageClass[classIndex];
			
					// Check if class "class" statistics are up to date				
						
			if (!gProjectInfoPtr->classNamesPtr[classStorage].statsUpToDate)
				if (UpdateClassAreaStats (fileIOInstructionsPtr, (UInt32)classIndex) <= 0) 
																								return (0);
			
			}	// end "for (classIndex=0; ... 
			
		}	// end "if (numberClasses > 0 && !gProjectInfoPtr->..."
	
			// Indicate that routine completed normally.								
			
	return (1);
		
}	// end "UpdateProjectAreaStats" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		SInt16 UpdateProjectMaskStats
//
//	Software purpose:	The purpose of this routine is to update the statistics for the
//							project that are defined by a mask file.
//
//	Parameters in:		None
//
//	Parameters out:	None
//
// Value Returned:	1 = Update completed okay
//							0 = Update did not complete because of a problem
// 
// Called By:
//
//	Coded By:			Larry L. Biehl			Date: 10/28/1998
//	Revised By:			Larry L. Biehl			Date: 02/16/2014	

SInt16 UpdateProjectMaskStats (
				SInt16								statsUpdateCode,
				FileIOInstructionsPtr			fileIOInstructionsPtr, 
				SInt32								requestedClassNumber, 
				SInt32								requestedFieldNumber,
				Boolean								checkForBadDataFlag,
				SInt16								statCode)

{
	double								dValue,
											maxDataValue,
											minDataValue,
											noDataValue;
	
	HDoublePtr							bufferPtr,
											bufferPtr2,
											tOutputBufferPtr;
	
	HChannelStatisticsPtr			areaChanPtr,
											lAreaChanPtr;
	
	HPClassNamesPtr					classNamesPtr;
	HPFieldIdentifiersPtr			fieldIdentPtr;
	
	HSumSquaresStatisticsPtr		areaSumSquaresPtr,
											lAreaSumSquaresPtr;
	
	HUInt16Ptr							maskBufferPtr,
											maskValueToFieldPtr;
											
	SInt32								classNumber,
											fieldNumber;
//											value;
	
	UInt32								classStorage,
											channel,
											column,
											columnEnd,
											columnInterval,
											columnStart,
											covChan,
											line,
											lineCount,
											lineEnd,
											lineInterval,
											lineStart,
											maskIndex,
											maskColumnStart,
											maskLineStart,
											numberClasses,
											numberMaskColumns,
											numberMaskLines,
											numberSamples;
											
	SInt32								errCode;
	
	SInt16								returnCode;
	
	UInt16								fieldClassStatsCode,
											lastMaskValue, 
											numberChannels,
											storage;
	
	Boolean								checkForNoDataFlag,
											continueFlag,
											dataOkayFlag,
											lCheckForBadDataFlag,
											updateNumberLinesFlag,
											updateProjectFlag,
											usePixelFlag;
	
	
			// Continue only if we have a training mask file.
			
	if (gProjectInfoPtr->trainingMask.maskHandle == NULL)
																								return (1);
	
			// Continue only if number of classes is one or more and if project		
			//	statistics are not up-to-date													
	
	if (gProjectInfoPtr->numberStatisticsClasses <= 0 || 
															gProjectInfoPtr->statsUpToDate)
																								return (1);
	
			// Continue only if there are mask fields that need to be updated.											
																					
	if (CheckIfMaskStatsUpToDate (statsUpdateCode, 
												requestedClassNumber, 
												requestedFieldNumber))
																								return (1);
																					
	maskValueToFieldPtr = (HUInt16Ptr)GetHandlePointer (
										gProjectInfoPtr->trainingMask.maskValueToFieldHandle,
										kLock);
	
	maskBufferPtr = (HUInt16Ptr)GetHandlePointer (
														gProjectInfoPtr->trainingMask.maskHandle,
														kLock);
	
			// Continue only if we have valid pointers to the mask buffer,
			// and value to class vector													
	
	if (maskBufferPtr == NULL || maskValueToFieldPtr == NULL)
																								return (0);
		
			// Hide the class information for the status dialog.
			
	HideStatusDialogItemSet (kStatusClassA);
	HideStatusDialogItemSet (kStatusField);
	
			// Initialize local variables.													
	
	continueFlag = TRUE;	
	returnCode = 0;	
	numberClasses = gProjectInfoPtr->numberStatisticsClasses;
	fieldIdentPtr = gProjectInfoPtr->fieldIdentPtr;
	classNamesPtr = gProjectInfoPtr->classNamesPtr;
	numberChannels = gProjectInfoPtr->numberStatisticsChannels;
	maxDataValue = gImageWindowInfoPtr->maxUsableDataValue;
	lastMaskValue = 0;
	
	fieldClassStatsCode = kFieldStatsOnly;
	if (gProjectInfoPtr->keepClassStatsOnlyFlag)
		fieldClassStatsCode = kClassStatsOnly;
		
	updateProjectFlag = (statsUpdateCode == kUpdateProject);
	
			// Get the area of the image to be read.
	
	lineStart = 1;
	lineEnd = gImageFileInfoPtr->numberLines;
	lineInterval = 1;
	
	columnStart = 1;
	columnEnd = gImageFileInfoPtr->numberColumns;
	columnInterval = 1;
		
			// Get the mask buffer variables.
			// Remember that the first column of the mask stored in memory is a flag
			// indicating whether there are any mask values for the respective line. 
		
	numberMaskLines = gProjectInfoPtr->trainingMask.numberLines;
	numberMaskColumns = gProjectInfoPtr->trainingMask.numberColumns + 1;
	
			// Determine if any of the requested lines and columns are within
			// the area represented by the mask. The lines and columns returned 
			// represent just those lines represented by the mask.
	
	continueFlag = GetMaskArea (NULL, 
											kTrainingType,
											gProjectInfoPtr,
											NULL,
											gProjectInfoPtr->startLine,
											gProjectInfoPtr->startColumn, 
											lineInterval, 
											columnInterval, 
											&lineStart, 
											&lineEnd, 
											&columnStart, 
											&columnEnd, 
											&maskLineStart, 
											&maskColumnStart);
	
	if (continueFlag)
		{
		checkForNoDataFlag = FALSE;
		
				// Get the number of columns (or samples) represented in one line of data.
				
		numberSamples = (columnEnd - columnStart + columnInterval)/columnInterval;
	
				// Position mask buffer to start at same relative location that lineStart
				// represents. 'lineStart' represents the line start in the image file.
															
		maskBufferPtr += (maskLineStart - 1) * numberMaskColumns;
		
				// Set the number of mask columns to represent the index to skip when 
				// going from one line to the next allowing for lines to be skipped.
				
		numberMaskColumns *= lineInterval;
			
				// Load some of the File IO Instructions structure that pertain
				// to the specific area being used.
				
		SetUpFileIOInstructions (fileIOInstructionsPtr,
											NULL,
											lineStart,
											lineEnd,
											lineInterval,
											columnStart,
											columnEnd,
											columnInterval, 
											gProjectInfoPtr->numberStatisticsChannels,
											gProjectInfoPtr->channelsPtr,
											kDetermineSpecialBILFlag);
										
				// Determine if data values need to be checked.								
				
		lCheckForBadDataFlag = checkForBadDataFlag;
		if (lCheckForBadDataFlag)
			{
			if (fileIOInstructionsPtr->fileInfoPtr->noDataValueFlag)
				{
						// Want to ignore these data values.
						
				noDataValue = fileIOInstructionsPtr->fileInfoPtr->noDataValue;
				checkForNoDataFlag = TRUE;
				lCheckForBadDataFlag = FALSE;
			 
				if (noDataValue >= 0)
					{
					maxDataValue = 1.00000001 * noDataValue;
					minDataValue = 0.99999999 * noDataValue;
					
					}	// end "if (noDataValue) >= 0)"
				 
				else if (noDataValue < 0)
					{
					minDataValue = 1.00000001 * noDataValue;
					maxDataValue = 0.99999999 * noDataValue;
					
					}	// end "if (noDataValue) < 0)"
				
				}	// end "if (fileIOInstructionsPtr->fileInfoPtr->noDataValueFlag)"
				
			else	// !...->noDataValueFlag
				{		
						// Don't really need to check if the number of bits is not less
						// than the number bits possible within the number of bytes.
						
				if (gImageWindowInfoPtr->numberBytes * 8 == gImageWindowInfoPtr->numberBits)
					lCheckForBadDataFlag = FALSE;
				/*
				if (gImageWindowInfoPtr->numberBytes == 1 && 
																gImageWindowInfoPtr->numberBits == 8)
					lCheckForBadDataFlag = FALSE;
					
				else if (gImageWindowInfoPtr->numberBytes == 2 && 
																gImageWindowInfoPtr->numberBits == 16)
					lCheckForBadDataFlag = FALSE;
					
				else if (gImageWindowInfoPtr->numberBytes == 8)
					lCheckForBadDataFlag = FALSE;
				*/
				}	// end "else !...->noDataValueFlag"
			
			}	// end "if (checkForBadDataFlag)"
	
				// Loop through the lines for the project image.										
		
		updateNumberLinesFlag = TRUE;
		lineCount = 0;
				
		for (line=lineStart; line<=lineEnd; line+=lineInterval)
			{
					// Load the line count into the status dialog.							
			
			lineCount++;
			
			if (TickCount () >= gNextStatusTime)
				{
				if (updateNumberLinesFlag)
					{		
					LoadDItemValue (gStatusDialogPtr, 
											IDC_Status20,
											(lineEnd-lineStart+lineInterval)/lineInterval);
					updateNumberLinesFlag = FALSE;
					
					}	// end "if (updateNumberLinesFlag)"
					
				LoadDItemValue (gStatusDialogPtr, IDC_Status18, (SInt32)lineCount);
				gNextStatusTime = TickCount () + gNextStatusTimeOffset;
				
				}	// end "if (TickCount () >= gNextStatusTime)"
				
			if (*maskBufferPtr > 0)
				{
						// Get all channels for the line of image data.  Return if			
						// there is a file IO error.													
					
				errCode = GetLineOfData (fileIOInstructionsPtr,
													line, 
													columnStart,
													columnEnd,
													columnInterval,
													(HUCharPtr)gInputBufferPtr,
													(HUCharPtr)gOutputBufferPtr);
							
				if (errCode != noErr)
					{
					continueFlag = FALSE;
					break;
					
					}	// end "if (errCode != noErr)"
			    
			   tOutputBufferPtr = (HDoublePtr)gOutputBufferPtr;
				maskIndex = maskColumnStart;
			   
				for (column=0; column<numberSamples; column++)
					{
					if (maskBufferPtr[maskIndex] != 0)
						{		
			   		if (lastMaskValue != maskBufferPtr[maskIndex])
			   			{
									// Get the field number and class number that this mask 
									//	value is being assigned to.
									
							fieldNumber = maskValueToFieldPtr[maskBufferPtr[maskIndex]];
							classStorage = fieldIdentPtr[fieldNumber].classStorage;
							classNumber = classNamesPtr[classStorage].classNumber - 1;
								
							storage = fieldIdentPtr[fieldNumber].trainingStatsNumber;
							if (gProjectInfoPtr->keepClassStatsOnlyFlag)
								storage = (UInt16)classStorage;
								
				   				// Make sure that we have the correct statistics pointers 
				   				// for the current class or field
							
							GetProjectStatisticsPointers (fieldClassStatsCode,
																	storage, 
																	&areaChanPtr, 
																	&areaSumSquaresPtr,
																	NULL,
																	NULL);
							
							lastMaskValue = maskBufferPtr[maskIndex];
							
							usePixelFlag = (updateProjectFlag || 
													classNumber == requestedClassNumber || 
																fieldNumber == requestedFieldNumber);
							
									// Verify that this field has not already been computed and
									// loaded into the class statistics.
																
							if (fieldIdentPtr[fieldNumber].loadedIntoClassStats)
								usePixelFlag = FALSE;
																
							}	// end "if (lastMaskValue != maskBufferPtr[maskIndex])"
							
				   	dataOkayFlag = usePixelFlag;
				   	
				   			// Check for bad data if requested.										
				   			
				   	if (dataOkayFlag)
							{
									// Now check for any bad or no data values.
									
							if (lCheckForBadDataFlag)
								{
								bufferPtr = tOutputBufferPtr;
								for (channel=0; channel<numberChannels; channel++)
									{
									if (*bufferPtr > maxDataValue)
										{
										dataOkayFlag = FALSE;
										break;
										
										}	// end "if (*bufferPtr > maxDataValue)" 
										
									bufferPtr++;
										
									}	// end "for (channel=0; channel<numberChannels; ..." 
								
								}	// end "if (dataOkayFlag && lCheckForBadDataFlag)" 								
								
							else if (checkForNoDataFlag)
								{
										// From other Processors
										
								bufferPtr = tOutputBufferPtr;
								for (channel=0; channel<numberChannels; channel++)
									{
									if (*bufferPtr > minDataValue && *bufferPtr < maxDataValue)
										{
										dataOkayFlag = FALSE;
										break;
										
										}	// end "if (*bufferPtr > minDataValue && ..." 
										
									bufferPtr++;
										
									}	// end "for (channel=0; channel<numberChannels; ..."
									
								}	// end "else if (checkForNoDataFlag)"
										
							}	// end "if (dataOkayFlag)"
				   	
				   	if (dataOkayFlag)
				   		{
				   		bufferPtr = tOutputBufferPtr;
							
							lAreaChanPtr = areaChanPtr;
							lAreaSumSquaresPtr = areaSumSquaresPtr;
							
							for (channel=0; channel<numberChannels; channel++)
								{
								dValue = *bufferPtr;
													
										// Get the minimum and maximum value and sum.				
										
								lAreaChanPtr->minimum = MIN (lAreaChanPtr->minimum, dValue);
								lAreaChanPtr->maximum = MAX (lAreaChanPtr->maximum, dValue);
								lAreaChanPtr->sum += dValue;
								
								lAreaChanPtr++;
								
								if (statCode == kMeanCovariance)
									{	
											// Accumulate the channel covariance statistics
						      	
					      		bufferPtr2 = tOutputBufferPtr;
						      		
									for (covChan=0; covChan<channel; covChan++)
										{
										*lAreaSumSquaresPtr += dValue * *bufferPtr2;
						      		bufferPtr2++;
						      		lAreaSumSquaresPtr++;
										
										}	// end "for (covChan=channel+1; ..."
											
									}	// end "if (statCode == kMeanCovariance)" 
								
								*lAreaSumSquaresPtr += dValue * dValue;
					      	lAreaSumSquaresPtr++;
								
					      	bufferPtr++;
									
								}	// end "for (channel=1; channel<numberChannels..." 
								
							fieldIdentPtr[fieldNumber].numberPixelsUsedForStats++;
							
							}	// end "if (dataOkayFlag)" 
						
						}	// end "if (maskBufferPtr[maskIndex] != 0)" 
					
							// Exit routine if user has "command period" down.					
							
					if (TickCount () >= gNextTime)
						{
						if (!CheckSomeEvents (
												osMask+keyDownMask+updateMask+mDownMask+mUpMask))
							{
							returnCode = -1;
							break;
							
							}	// end "if (!CheckSomeEvents (osMask + ..." 
							
						}	// end "if (TickCount () >= nextTime)" 
					
					tOutputBufferPtr += numberChannels;
					maskIndex += columnInterval;
							
					}	// end "for (columnPtr=0; columnPtr<..."
					
				if (returnCode < 0)
					break;
					
				}	// end "if (MaskLineUsed (line))"
				
			maskBufferPtr += numberMaskColumns;
		      
			}	// end "for (line=lineStart; line<=lineEnd; line++)" 
		
				// Close up any File IO Instructions structure that pertain to the 
				// specific area used.
				
		CloseUpFileIOInstructions (fileIOInstructionsPtr, NULL);
			
		}	// end "if (continueFlag)"
		
	if (continueFlag && returnCode == 0)
		{	
		LoadDItemValue (gStatusDialogPtr, IDC_Status18, (SInt32)lineCount);
		
				// Finish updating mask statistics parameters.	
					
		switch (statsUpdateCode)
			{
			case kUpdateProject:
				FinishProjectMaskStatsUpdate ();
				break;
				
			case kUpdateClass:
				FinishClassMaskStatsUpdate (requestedClassNumber);
				break;
				
			case kUpdateField:
				FinishClassMaskStatsUpdate (requestedFieldNumber);
				break;
				
			}	// end "switch (statsWindowMode)"
			
		returnCode = 1;
			
		}	// end "if (continueFlag)"
		
			// Unlock the mask handles.
			
	CheckAndUnlockHandle (gProjectInfoPtr->trainingMask.maskHandle);
	CheckAndUnlockHandle (gProjectInfoPtr->trainingMask.maskValueToFieldHandle);
	
			// Indicate that routine completed normally.								
			
	return (returnCode);
		
}	// end "UpdateProjectMaskStats" 



//------------------------------------------------------------------------------------
//								 Copyright (1988-2018)
//								(c) Purdue Research Foundation
//									All rights reserved.
//
//	Function name:		SInt16 UpdateStatsControl
//
//	Software purpose:	The purpose of this routine is to handle the
//							"Update Statistics Control" event depending on
//							the statistics window mode.
//
//	Parameters in:		Parameter indicating the what is to be updated
//								2 (kUpdateProject) = update all of project
//								3 (kUpdateClass) = update active class
//								4 (kUpdateField) = update active field
//
//	Parameters out:	None
//
// Value Returned:	1: Update done
//							2: Cancel operation
//							3: Do not update statistics
// 
// Called By:			VerifyProjectStatsUpToDate in SProjUtl.cpp
//							StatisticsWControlEvent in statistics.c
//							ListStatsControl in statPrint.c
//
//	Coded By:			Larry L. Biehl			Date: 11/17/1988
//	Revised By:			Larry L. Biehl			Date: 03/15/2017	

SInt16 UpdateStatsControl (
				SInt16								statsWindowMode, 
				Boolean								requestFlag)

{
	time_t								startTime;
	
	FileInfoPtr							fileInfoPtr;
	FileIOInstructionsPtr			fileIOInstructionsPtr;
	
	SignedByte							handleStatus;
	SInt16								returnCode;
	
	Boolean								continueFlag;
	

	continueFlag = TRUE;
	returnCode = -1;
	
	if (statsWindowMode >= kUpdateProject && statsWindowMode <= kUpdateField)
		{
				// Present dialog box to user if needed to request if statistics	
				// are to be updated.															
		
		if (requestFlag)
			{
			MInitCursor ();
			if (statsWindowMode == kUpdateProject)  
				CopyPToP (gTextString3, (UCharPtr)"\0project\0");
			else if (statsWindowMode == kUpdateClass)  
				CopyPToP (gTextString3, (UCharPtr)"\0class\0");
			else if (statsWindowMode == kUpdateField)  
				CopyPToP (gTextString3, (UCharPtr)"\0field\0");
			
					// Update statistics before continuing?
			if (LoadSpecifiedStringNumberStringP (kAlertStrID,
																IDS_Alert42,
																(char*)gTextString,
																(char*)gTextString2,
																TRUE,
																(char*)&gTextString3[1]))
				returnCode = DisplayAlert (kUpdateCancelAlertID, 2, 0, 0, 0, gTextString);
			
			MSetCursor (kWait);
			
			if (returnCode != 1)											
																						return (returnCode);
			
			}	// end "if (requestFlag)" 

				// Change cursor to watch cursor until done with process.
			
		MSetCursor (kWait);
		
		returnCode = 0;
		
					// Get status of window information handle.  We will leave the 
					// handle locked at end if it	was locked upon entry.				
					// Then unlock it so that we do not fragment the memory when	
					// getting memory for the statistics.									
		
		handleStatus = MHGetState (gProjectInfoPtr->windowInfoHandle);
		
				// Check handle to image file information.  If handle to image		
				// file information doesn't exists. Find the	image file and get	
				// the information for it.														
					
		if (GetProjectImageFileInfo (kPrompt, kSetupGlobalInfoPointers))
			{
			UnlockProjectWindowInfoHandles ();
					
			startTime = time (NULL);
			
					// Force text selection to start from end of present text.		
					
			ForceTextToEnd ();	
			
					// List the processor name, date, time and project info.			

			if (gProcessorCode == kComputeStatsProcessor)
				continueFlag = ListHeaderInfo (
										NULL,
										kLImageInfo+kLProjectName+kLProjectImageName+kLStatType, 
										&gOutputForce1Code, 
										gProjectInfoPtr->covarianceStatsToUse, 
										continueFlag);
			
					// Make certain that memory for statistics information is 		
					// sufficient.																	
					 
			if (continueFlag)
				{
				continueFlag = SetupStatsMemory ();		

				if (!continueFlag)
					ListSpecifiedStringNumber (kAlertStrID, 
														IDS_Alert125,
														(unsigned char*)gTextString,
														NULL, 
														gOutputForce1Code,
														TRUE);
				
				}	// end "if (continueFlag)"
			
					// Lock handle to file information and get pointer to it			
			
			if (continueFlag)		
				continueFlag = GetProjectImageFileInfo (kDoNotPrompt, 
																		kSetupGlobalInfoPointers);
			
			fileInfoPtr = gImageFileInfoPtr;
			fileIOInstructionsPtr = NULL;
			
					// Get buffers to read data from image file into.					
				 				
			if (continueFlag)
				continueFlag = GetIOBufferPointers (
														&gFileIOInstructions[0],
														gImageWindowInfoPtr,
														gImageLayerInfoPtr,
														gImageFileInfoPtr,
														&gInputBufferPtr, 
														&gOutputBufferPtr,
														1,
														gImageWindowInfoPtr->maxNumberColumns,
														1,
														gProjectInfoPtr->numberStatisticsChannels,
														(UInt16*)gProjectInfoPtr->channelsPtr,
														kDoNotPackData,
														kForceBISFormat,
														kForceReal8Bytes,
														kDoNotAllowForThreadedIO,
														&fileIOInstructionsPtr);
			
			if (continueFlag)
				{	
						// Get updating statistics status dialog box.					
					
				gStatusDialogPtr = GetStatusDialog (kUpdateStatsInfoID, FALSE);
				
				if (gStatusDialogPtr != NULL)
					{
							// Load "Updating Statistics For:" in the status dialog
							
					LoadDItemStringNumber (kProjectStrID, 
													IDS_Project40,
													gStatusDialogPtr, 
													IDC_Status11, 
													(Str255*)gTextString);
					
					ShowStatusDialogItemSet (kStatusField);
					ShowStatusDialogItemSet (kStatusLine);
					ShowStatusDialogItemSet (kStatusCommand);
					
					switch (statsWindowMode)
						{
						case kUpdateProject:
							ShowStatusDialogItemSet (kStatusClassA);
							break;
							
						case kUpdateClass:
							ShowStatusDialogItemSet (kStatusClassA);
							LoadDItemValue (gStatusDialogPtr, IDC_Status3, (SInt32)1);
							LoadDItemValue (gStatusDialogPtr, IDC_Status5, (SInt32)1);
							break;
							
						case kUpdateField:
							LoadDItemValue (gStatusDialogPtr, IDC_Status8, (SInt32)1);
							LoadDItemValue (gStatusDialogPtr, IDC_Status10, (SInt32)1);
							break;
							
						}	// end "switch (statsWindowMode)" 
						
					ShowDialogWindow (gStatusDialogPtr, 
											kUpdateStatsInfoID, 
											kDoNotSetUpDFilterTable);
					CheckSomeEvents (updateMask + activMask);

							// Turn spin cursor on
			
					gPresentCursor = kSpin;
					/*
							// Force text selection to start from end of present text.		
							
					ForceTextToEnd ();	
					
							// List the processor name, date, time and project info.			
		
					if (gProcessorCode == kComputeStatsProcessor)
						continueFlag = ListHeaderInfo (
									NULL, 
									kLImageInfo+kLProjectName+kLProjectImageName+kLStatType, 
									&gOutputForce1Code, 
									gProjectInfoPtr->covarianceStatsToUse, 
									continueFlag);
					*/
							// Make sure that the line and column intervals are '1'.	
							
					InitializeAreaDescription (&gAreaDescription);
					
							// Intialize the nextTime variables to indicate when the next check	
							// should occur for a command-. and status information.

					gNextTime = TickCount ();
					gNextStatusTime = TickCount ();
					
					switch (statsWindowMode)
						{
						case kUpdateProject:
									// Make sure that the statistics vectors and arrays for
									// those classes or field which have not been initialized
									// are cleared;
						
							ClearProjectStatisticsMemory ();
						
									// Update project stats defined by rectangles/polygons.
									
							returnCode = UpdateProjectAreaStats (fileIOInstructionsPtr);
							
									// Update project stats defined by mask file.
							
							if (returnCode == 1)
								returnCode = UpdateProjectMaskStats (
																	statsWindowMode,
																	fileIOInstructionsPtr, 
																	-1, 
																	-1,
																	fileInfoPtr->noDataValueFlag, 
																	gProjectInfoPtr->statisticsCode);
											
							if (returnCode == 1)						
								FinishProjectStatsUpdate ();
							break;
							
						case kUpdateClass:
							ClearClassStatisticsMemory (gProjectInfoPtr->currentClass);
						
									// Update class stats defined by rectangles/polygons.
									
							returnCode = UpdateClassAreaStats (
																	fileIOInstructionsPtr,
																	gProjectInfoPtr->currentClass);
							
									// Update class stats defined by mask file.
							
							if (returnCode == 1)
								returnCode = UpdateProjectMaskStats (
																	statsWindowMode,
																	fileIOInstructionsPtr,
																	gProjectInfoPtr->currentClass,
																	-1, 
																	fileInfoPtr->noDataValueFlag, 
																	gProjectInfoPtr->statisticsCode);
											
							if (returnCode == 1)						
								FinishClassStatsUpdate (gProjectInfoPtr->currentClass);
							break;
							
						case kUpdateField:
							ClearFieldStatisticsMemory (gProjectInfoPtr->currentField);
							
							returnCode = UpdateFieldAreaStats (
																	fileIOInstructionsPtr,
																	gProjectInfoPtr->currentField);
																
							if (returnCode == 1)
								returnCode = UpdateProjectMaskStats (
																	statsWindowMode,
																	fileIOInstructionsPtr,
																	-1,
																	gProjectInfoPtr->currentField, 
																	fileInfoPtr->noDataValueFlag, 
																	gProjectInfoPtr->statisticsCode);
							break;
							
						}	// end "switch (statsWindowMode)"
						
							// Update any leave one out covariance parameters if needed.
					
					if (returnCode == 1)	
						{
						returnCode = UpdateProjectLOOStats (statsWindowMode,
																		fileIOInstructionsPtr);	
						
								//	Other routines expect UpdateStatsControl to return a 1 to 
								// indicate that everything worked okay.
								
						if (returnCode == 2)
							returnCode = 1;
														
						}	// end "if (returnCode == 1)"	
					/*
							// List the CPU time taken for the statistics.
					
					if (gProcessorCode == kComputeStatsProcessor)
						{	
						ListCPUTimeInformation (NULL, TRUE, startTime);
						
								// Scroll output window to the selection and adjust the 	
								// scroll bar.															
					
						UpdateOutputWScrolls (gOutputWindow, 1, kDisplayMessage);
						
						}	// end "if (gProcessorCode == kComputeStatsProcessor)"
					*/
							// Dispose of updating statistics status dialog box.			
						
					CloseStatusDialog (TRUE);

							// Turn spin cursor off
				
					gPresentCursor = kWait;
					
					}	// end "if (gStatusDialogPtr != NULL)" 
				
						// Dispose of the IO buffer.		
											
				DisposeIOBufferPointers (fileIOInstructionsPtr,
													&gInputBufferPtr, 
													&gOutputBufferPtr);
				
						// If statistics update completed normally, then update		
						// variables and menu items.											
					                	
				if (returnCode == 1)
					{
					if (gProjectWindow != NULL)
						MHiliteControl (gProjectWindow, 
												gProjectInfoPtr->updateControlH, 
												255);
						
					}	// end "if (returnCode == 1)"
				
				}	// end "if (continueFlag)" 	
					
					// List the CPU time taken for the statistics.					
			
			if (gProcessorCode == kComputeStatsProcessor)
				{	
				ListCPUTimeInformation (NULL, TRUE, startTime);
				
						// Scroll output window to the selection and adjust the 	
						// scroll bar.															
			
				UpdateOutputWScrolls (gOutputWindow, 1, kDisplayMessage);
				
				}	// end "if (gProcessorCode == kComputeStatsProcessor)"
			
					// Indicate that project information has changed and 			
					// update menu items.			

			gUpdateProjectMenuItemsFlag = TRUE;
				
					// Unlock the file information handle if needed.					

			if (handleStatus >= 0)
				UnlockProjectWindowInfoHandles ();
			
			}	// end "if (GetProjectImageFileInfo (..."
			
		MInitCursor ();
			
		}	// end "if (statsWindowMode >= 2 && statsWindowMode <= 4)"
    
      if(gClassifySpecsPtr->mode == kDecisionTreeMode)
      {
         // get SVM training weight
         struct svm_parameter param;
         param.svm_type = gProjectInfoPtr->svm_type;
         param.kernel_type = gProjectInfoPtr->svm_kernel_type;
         param.degree = 3;
         param.gamma = gProjectInfoPtr->svm_gamma;    // 1/num_features
         param.coef0 = 0;
         param.nu = 0.5;
         param.cache_size = 100;
         param.C = gProjectInfoPtr->svm_cost;
         param.eps = 0.1;
         param.p = 0.0001;
         param.shrinking = 1;
         param.probability = 0;
         param.nr_weight = 0;
         param.weight_label = NULL;
         param.weight = NULL;

         struct svm_problem prob;
         struct svm_node *x_space;
         int pixelNum = gProjectInfoPtr->svm_labels.size();
         int featureNum = gClassifySpecsPtr->numberChannels;
         prob.l = pixelNum;
         // FIXME: free prob.y prob.x x_space at right place
         prob.y = (double*)malloc(pixelNum * sizeof(double));
         prob.x = (struct svm_node**)malloc(pixelNum * sizeof(struct svm_node*));
         x_space = (struct svm_node*)malloc(pixelNum * (featureNum+1) * sizeof(struct svm_node));
         // copy the label
         int jj = 0;
         for(int i = 0; i < pixelNum; i++)
         {
            prob.y[i] = (double)(gProjectInfoPtr->svm_labels[i]);
            prob.x[i] = &x_space[jj];
            for(int j = 0; j < featureNum; j ++)
            {
                // copy index and value of features in each channel
                x_space[jj].index = j + 1;
                x_space[jj].value = gProjectInfoPtr->svm_sample[i][j];
                ++jj;
            }
            x_space[jj++].index = -1;
         }
         // SVM training
         gProjectInfoPtr->svmModel = svm_train(&prob,&param);

         gProjectInfoPtr->svm_x = (struct svm_node*)malloc((featureNum+1)* sizeof(struct svm_node));

         gProjectInfoPtr->svm_labels.clear();
      }
	return (returnCode);

}	// end "UpdateStatsControl"
