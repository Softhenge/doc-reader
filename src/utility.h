#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

namespace utility {

enum class Direction
{
    LEFT,
    RIGHT,
    UP,
    BOTTOM
};

struct Contains
{
    Contains(const std::string& str = std::string())
        : m_string(str)
    {
    }

public:
    bool operator()(const std::string& str) const
    {
        return (str.find(m_string) != std::string::npos);
    }

private:
    std::string m_string;
};

/**
 * class CompareByX this comparator role is to compare rects by theire 'x' coordinate
 */
struct CompareByX
{
    bool operator () (const cv::Rect& lhs, const cv::Rect& rhs) const
    {
        if (lhs.y == rhs.y) {
            return lhs.x < rhs.x;
        } else {
            return lhs.y < rhs.y;
        }
    }
};

/**
 * class CompareByCoordinates this comparator role is to compare rects by theire 'x' ans 'y' coordinates
 */
struct CompareByCoordinates
{
    bool operator() (const cv::Rect& lhs, const cv::Rect& rhs) const
    {
        bool res = (lhs.x < rhs.x);
        if (!res) {
            res = (lhs.y < rhs.y + 6) && (lhs.y < rhs.y - 6);
        }
        return res;
    }
};

//! shows is the lhs rect inside rhs rect or not
inline bool inside(const cv::Rect& lhs, const cv::Rect& rhs)
{
    return lhs.area() == (lhs & rhs).area();
}

/**
 * @brief  isInSameLevel function sould point is rhs and lhs rects
 *                       (probaly key/value or value/value)
 *                       are in same level or not
 * E.g.
 *       ___________
 *  home:|____r1___| -> true  | in provided example 'home:' and r1 rect are in same level
 *       -----------          | but 'home:" and r2 rec are not in the same level
 *       |____r2___| -> false |
 * @return returns true is they are in the same level false otherwise
 **/
inline bool isInSameLevel(const cv::Rect& lhs, const cv::Rect& rhs)
{
    if (lhs.y == rhs.y) {
        return true;
    }
    int lhsY = lhs.y + lhs.height;
    int rhsY = rhs.y + rhs.height;
    return ((lhs.y < rhs.y) && (lhsY >= rhs.y)) ||
           ((rhs.y < lhs.y) && (rhsY >= lhs.y));
}

inline bool isInSameColumn(const cv::Rect& lhs, const cv::Rect& rhs)
{
    if (lhs.x == rhs.x) {
        return true;
    }
    int lhsX = lhs.x + lhs.width;
    int rhsX = rhs.x + rhs.width;
    return ((lhs.x < rhs.x) && (lhsX >= rhs.x)) ||
           ((rhs.x < lhs.x) && (rhsX >= lhs.x));
}

//! this function is checking is that rect check box or not
//! for check boxes we use simple logic if rect height and width are close each other or not
inline bool isCheckBox(const cv::Rect& rect)
{

    if (rect.height == rect.width) {
        return true;
    }
    auto max = std::max(rect.height, rect.width);
    auto min = std::min(rect.height, rect.width);
    if ((static_cast<double>(min) / max) >  0.9) {
        return true;
    }
    return false;
}

//! shows rect validation state
inline bool isValidRect(const cv::Rect& rect)
{
    return rect.width > 6;
}

inline cv::Rect getBoundingRect(const std::vector<cv::Rect>& candidates)
{
    bool ignore = true;
    for (const auto& item : candidates) {
        if (item.height > 11) {
            ignore = false;
        }
    }
    if (ignore) {
        return cv::Rect(0, 0, 0, 0);
    }

    //! TODO here need small inprovement connected with height
    const auto pair = std::minmax_element(candidates.begin(), candidates.end(), []
                                                (const cv::Rect& lhs, const cv::Rect& rhs) {
                                                    return (lhs.x < rhs.x);
                                                    });
    int minY = (*std::min_element(candidates.begin(), candidates.end(), []
                                        (const cv::Rect& lhs, const cv::Rect& rhs) {
                                            return (lhs.y < rhs.y);
                                         })).y;
    auto height = (*std::max_element(candidates.begin(), candidates.end(), []
                                             (const cv::Rect& lhs, const cv::Rect& rhs) {
                                                return (lhs.height + lhs.y < rhs.height + rhs.y);
                                             }));
    int minX = (*pair.first).x;
    int width = (*pair.second).x - (*pair.first).x + (*pair.second).width;
    return cv::Rect(minX, minY, width, (height.y + height.height - minY));
}

inline std::vector<std::vector<cv::Rect> > extractWords(const std::vector<cv::Rect>& line,
                                                        int diff)
{
    std::vector<std::vector<cv::Rect> > result;
    auto bRect = getBoundingRect(line);
    if (bRect.height < 2) {
        return std::move(result);
    }
    cv::Rect median = *line.begin();
    std::vector<cv::Rect> currentWord;
    currentWord.push_back(median);
    for (const auto& item : line) {
        if ((item.x == median.x) && (item.y == median.y) &&
                (item.height == median.height) && (item.width == median.width)) {
            continue;
        }
        if (item.x - (median.x + median.width) > diff) {
            median = item;
            result.push_back(currentWord);
            currentWord.clear();
            currentWord.push_back(item);
        } else {
            median = item;
            currentWord.push_back(item);
        }
    }
    result.push_back(currentWord);
    return std::move(result);
}


} //namespace utility

