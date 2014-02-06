#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdlib.h>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
    if( argc != 3)
    {
     cout <<" Usage: threholding <ImageToLoadAndDisplay> <threshold_value>" << endl;
     return -1;
    }

    Mat image;
    image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);   // Read the file

    if(! image.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    Mat bin_image(image.size(),image.type());

    threshold(image, bin_image, atoi(argv[2]), 255, THRESH_BINARY);

    namedWindow( "Display window", CV_WINDOW_NORMAL );// Create a window for display.
    imshow( "Display window", bin_image );                   // how our image inside it.

    waitKey(0);                                          // Wait for a keystroke in the window
    return 0;
}
