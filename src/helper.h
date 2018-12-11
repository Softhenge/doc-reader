#pragma once

#include <vector>
#include <iostream>
#include <functional>

#include "utility.h"

struct HelperBase
{

static std::vector<std::vector<cv::Rect> > extractWords(const std::vector<std::vector<cv::Rect> >& linesContainer,
                                                 std::function<bool(const cv::Rect&, const cv::Rect&)> funcObj,
                                                 int diff = 10)
{
    std::vector<std::vector<cv::Rect> > result;
    for (auto line : linesContainer) {
        std::sort(line.begin(), line.end(), funcObj);
        auto subRes = utility::extractWords(line, diff);
        std::move(subRes.begin(), subRes.end(), std::back_inserter(result));
    }
    return std::move(result);
}

};

using LINE     = typename std::vector<cv::Rect>;
using WORD     = typename std::vector<cv::Rect>;
using COLUMN   = typename std::vector<cv::Rect>;

///@brief function object to sort rect by their x coordinate
static std::function<bool(const cv::Rect&, const cv::Rect&)> funcObj = [](const cv::Rect& lhs, const cv::Rect& rhs) {
    return lhs.x < rhs.x;
};

///@brief function object to sort lines by thir y coordinates
static std::function<bool(const LINE&, const LINE&)> yComp = [] (const LINE& lhs, const LINE& rhs) {
    return (lhs.front()).y < (rhs.front()).y;
};

struct Helper
{
/**
 * @brief helpMessage
 */
static void helpMessage();


/**
 * @brief extractWords function role is to extract words from provided symbols set
 * @param symbolsContainer extracted symbols
 * @return extracted words vector
 */
template <typename ContainerType>
std::vector<WORD> extractWords(const ContainerType& symbolsContainer, int diff =  10) const;

/**
 * removeExtras this helper function role is removeing extra rects from rect vector
 * _______________
 * |r1___________ | here we need to remove r2 rect
 * | |          | |
 * | |    r2    | |
 * | |__________| |
 * |______________|
 * after loops we should remove duplicate rects from our vector and for that reason
 * we move datas from vetor to set and after that move back
 * invesigation results has shown that std::erase/std::unique paradigm is more
 * heavy than vector->set, set->vector movement
 */
static void removeExtras(std::vector<cv::Rect>& inputRects);

static cv::Rect getVerticalRect(const std::vector<cv::Rect>& candidates);

/**
 * @brief getNearLeftKeyCondidate function role is to return key rects for provided value rect
 * E.g  web site:        |-----------------| here this function should return vector with two rects
 *                       |______rect_______| these rects are 'web' and 'site'
 * @param rect this is the key rect for which we are trying to find appropriate values
 * @param outsideRects this vector keeps the candidate key rects
 */
static std::vector<cv::Rect> getNearLeftKeyCondidate(const cv::Rect& rect,
                                          const std::set<cv::Rect, utility::CompareByCoordinates>& outsideRects);

/**
 * @brief getNearRightKeyCondidate function role is to return value rects for provided key rect
 * E.g |--------|           here this function should return vector with one elementh 'English'
 *     |________| English
 * @param rect this is the key rect for which we are trying to find appopriate value
 * @param outsideRects this vector keeps the candidate value
 */
static std::vector<cv::Rect> getNearRightKeyCondidate(const cv::Rect& rect,
                                               const std::set<cv::Rect, utility::CompareByCoordinates>& outsideRects);
/**
 * @brief extractLines function extracts all lines from provided datas
 * @param data
 * @return extracted lines vector
 */
static std::vector<LINE> extractLines(std::vector<cv::Rect>& data);
};

