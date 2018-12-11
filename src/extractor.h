#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "helper.h"
#include "json.hpp"

struct Extractor
{
    //! @brief virtual function wich is extracting the data and writing to the <json> file
    virtual void extract() const = 0;
};


