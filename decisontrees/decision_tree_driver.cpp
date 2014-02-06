// Example : decision tree learning
// usage: prog training_data_file testing_data_file

// For use with test / training datasets : speech_ex

// Author : Toby Breckon, toby.breckon@cranfield.ac.uk

// Copyright (c) 2011 School of Engineering, Cranfield University
// License : LGPL - http://www.gnu.org/licenses/lgpl.html

#include <cv.h>       // opencv general include file
#include <ml.h>		  // opencv machine learning include file

using namespace cv; // OpenCV API is in the C++ "cv" namespace

#include <stdio.h>

/******************************************************************************/

#define NUMBER_OF_TRAINING_SAMPLES 1400
#define ATTRIBUTES_PER_SAMPLE 5
#define NUMBER_OF_TESTING_SAMPLES 328
#define NUMBER_OF_CLASSES 4

#define DEBUG

// N.B. classes are spoken alphabetric letters A-Z labelled 1 -> 26

/******************************************************************************/

// loads the sample database from file (which is a CSV text file)

int read_data_from_csv(const char* filename, Mat data, Mat classes, int n_samples )
{
    float tmp;

    // if we can't read the input file then return 0
    FILE* f = fopen( filename, "r" );
    if( !f )
    {
        printf("ERROR: cannot read file %s\n",  filename);
        return 0; // all not OK
    }

    // for each sample in the file

    for(int line = 0; line < n_samples; line++)
    {

        // for each attribute on the line in the file
        for(int attribute = 0; attribute < (ATTRIBUTES_PER_SAMPLE + 1); attribute++)
        {
            if (attribute < ATTRIBUTES_PER_SAMPLE)
            {
                // first 617 elements (0-616) in each line are the attribute
                fscanf(f, "%f,", &tmp);
                data.at<float>(line, attribute) = tmp;
            }
            else if (attribute == ATTRIBUTES_PER_SAMPLE)
            {
                // attribute 617 is the class label {1 ... 26} == {A-Z}
                fscanf(f, "%f,", &tmp);
                classes.at<float>(line, 0) = tmp;
            }
        }
    }
    fclose(f);
    return 1; // all OK
}

/******************************************************************************/

int main( int argc, char** argv )
{
    // lets just check the version first

    printf ("OpenCV version %s (%d.%d.%d)\n",
            CV_VERSION,
            CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION);

    // define training data storage matrices (one for attribute examples, one
    // for classifications)
    Mat training_data = Mat(NUMBER_OF_TRAINING_SAMPLES, ATTRIBUTES_PER_SAMPLE, CV_32FC1);
    Mat training_classifications = Mat(NUMBER_OF_TRAINING_SAMPLES, 1, CV_32FC1);

    //define testing data storage matrices
    Mat testing_data = Mat(NUMBER_OF_TESTING_SAMPLES, ATTRIBUTES_PER_SAMPLE, CV_32FC1);
    Mat testing_classifications = Mat(NUMBER_OF_TESTING_SAMPLES, 1, CV_32FC1);

    // define all the attributes as numerical
    // alternatives are CV_VAR_CATEGORICAL or CV_VAR_ORDERED(=CV_VAR_NUMERICAL)
    // that can be assigned on a per attribute basis

    Mat var_type = Mat(ATTRIBUTES_PER_SAMPLE + 1, 1, CV_8U );
    var_type.setTo(Scalar(CV_VAR_NUMERICAL) ); // all inputs are numerical

    // this is a classification problem (i.e. predict a discrete number of class
    // outputs) so reset the last (+1) output var_type element to CV_VAR_CATEGORICAL

    var_type.at<uchar>(ATTRIBUTES_PER_SAMPLE, 0) = CV_VAR_CATEGORICAL;

    CvDTreeNode* resultNode; // node returned from a prediction

    // load training and testing data sets
    #ifdef DEBUG
        printf("Argv[1] is %s\n",argv[1]);
    #endif
    if (read_data_from_csv(argv[1], training_data, training_classifications, NUMBER_OF_TRAINING_SAMPLES) &&
            read_data_from_csv(argv[2], testing_data, testing_classifications, NUMBER_OF_TESTING_SAMPLES))
    {
        // define the parameters for training the decision tree

        float *priors = NULL;  // weights of each classification for classes
        // (all equal as equal samples of each character)

        CvDTreeParams params = CvDTreeParams(100, // max depth
                                             3, // min sample count
                                             0, // regression accuracy: N/A here
                                             false, // compute surrogate split, no missing data
                                             4, // max number of categories (use sub-optimal algorithm for larger numbers)
                                             5, // the number of cross-validation folds
                                             false, // use 1SE rule => smaller tree
                                             false, // throw away the pruned tree branches
                                             priors // the array of priors
                                            );

        // train decision tree classifier (using training data)
        printf( "\nUsing training database: %s\n\n", argv[1]);
        CvDTree* dtree = new CvDTree;
        dtree->train(training_data, CV_ROW_SAMPLE, training_classifications,
                     Mat(), Mat(), var_type, Mat(), params);

        // perform classifier testing and report results

        Mat test_sample;
        int correct_class = 0;
        int wrong_class = 0;
        int false_positives [NUMBER_OF_CLASSES];
        char class_labels[NUMBER_OF_CLASSES];

        // zero the false positive counters in a simple loop

        for (int i = 0; i < NUMBER_OF_CLASSES; i++)
        {
            false_positives[i] = 0;
            class_labels[i] = (char) 65 + i; // ASCII 65 = A
        }

        printf( "\nUsing testing database: %s\n\n", argv[2]);

        for (int tsample = 0; tsample < NUMBER_OF_TESTING_SAMPLES; tsample++)
        {

            // extract a row from the testing matrix

            test_sample = testing_data.row(tsample);

            // run decision tree prediction

            resultNode = dtree->predict(test_sample, Mat(), false);

            printf("Testing Sample %i -> class result (character %c)\n", tsample,
                   class_labels[((int) (resultNode->value)) - 1]);

            // if the prediction and the (true) testing classification are the same
            // (N.B. openCV uses a floating point decision tree implementation!)

            if (fabs(resultNode->value - testing_classifications.at<float>(tsample, 0))
                    >= FLT_EPSILON)
            {
                // if they differ more than floating point error => wrong class

                wrong_class++;

                false_positives[((int) (resultNode->value)) - 1]++;

            }
            else
            {

                // otherwise correct

                correct_class++;
            }
        }

        printf( "\nResults on the testing database: %s\n"
                "\tCorrect classification: %d (%g%%)\n"
                "\tWrong classifications: %d (%g%%)\n",
                argv[2],
                correct_class, (double) correct_class*100/NUMBER_OF_TESTING_SAMPLES,
                wrong_class, (double) wrong_class*100/NUMBER_OF_TESTING_SAMPLES);

        for (int i = 0; i < NUMBER_OF_CLASSES; i++)
        {
            printf( "\tClass (character %c) false postives 	%d (%g%%)\n", class_labels[i],
                    false_positives[i],
                    (double) false_positives[i]*100/NUMBER_OF_TESTING_SAMPLES);
        }


        // all matrix memory free by destructors


        // all OK : main returns 0

        return 0;
    }

    // not OK : main returns -1

    return -1;
}
/******************************************************************************/