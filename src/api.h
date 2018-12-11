#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <string>
#include <baseapi.h>

/**
 * @class APISingleton
 * @brief singleton wrapper class which keeps reference to the image
 *        and pointer to TessBaseAPI class instance. This class provides two functions which allows us to
 *        recognize the text in the provided area
 * @file api.h
 * @author Softhenge LLC info@softhenge.com
 **/

struct APISingleton
{
public:
    /**
     * @brief create the singleton object if it is not exist and returns this object
     *         if it is exist returns it.
     **/
     static APISingleton* getInstance();

     /**
      * @brief removes the singleton object
      **/
     static void removeInstance();

public:
     /**
      * @brief recognize text from image by provided rect
      */
     std::string recognize(const cv::Rect& rect) const;

     /**
      * @brief recognize text from provided image by provided rect
      */
     std::string recognize(const cv::Mat& image, const cv::Rect& rect) const;

     //! TODO add functionality for RotatedRects

private:
    //! @brief default constructor
    APISingleton();
    //! @brief destructor
    ~APISingleton();

    //! @brief copy constructor copy assignment operator
    APISingleton(const APISingleton& other) = delete;
    const APISingleton& operator=(const APISingleton& other) = delete;

    //! @brief move constructor and move assignment
    APISingleton(APISingleton&& other) = delete;
    const APISingleton& operator=(APISingleton&& other) = delete;

public:
     tesseract::TessBaseAPI* m_api;
     cv::Mat m_image;

private:
    static APISingleton* s_instance;
};

