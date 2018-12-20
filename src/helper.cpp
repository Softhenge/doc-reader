#include <vector>
#include <iostream>
#include <opencv2/core/core.hpp>

#include "helper.h"

namespace {

/**
 * @brief isNearlySame this function is to determine is 'lhs' rect lnearly same as 'rhs' rect
 * E.g. |----------------|    Here we R1 and R2 rectsa are nearly same
 *      |   |------------|-|
 *      |R1_|__R2________|_|
 */
inline bool isNearlySame(const cv::Rect& lhs, const cv::Rect& rhs)
{
    return ((std::abs(lhs.x - rhs.x) < 3) &&
            (std::abs(lhs.y - rhs.y) < 3) &&
            (std::abs(lhs.height - rhs.height) < 3) &&
            (std::abs(lhs.width - rhs.width) < 3));
}

} // unnamed namespace

using namespace utility;

void Helper::helpMessage()
{
    std::cout
             << std::endl
             << "\033[1;31mUsage: " << "<executable>"
             << std::endl << "\t --image <input_image> Requiered"
             << std::endl << "\t --type \033[0m"
             << std::endl;
}

void Helper::removeExtras(std::vector<cv::Rect>& inputRects)
{
    if (inputRects.empty()) {
        return;
    }

    std::set<cv::Rect, CompareByX> uniqueSet(inputRects.begin(), inputRects.end());
    inputRects.clear();
    std::copy(uniqueSet.begin(), uniqueSet.end(), std::back_inserter(inputRects));

    //! here also need to avoid erase operation as it is expensive operation in our case
    for (size_t i = 0; i < inputRects.size(); ++i) {
        for (size_t j = 0; j < inputRects.size(); ) {
            if (i == j) {
                j++;
                continue;
            }
            if (inside(inputRects[j], inputRects[i])) {
                inputRects.erase(inputRects.begin() + j);
            } else {
                ++j;
            }
        }
    }
    for (size_t i = 0; i < inputRects.size() - 1; ++i) {
        for (size_t j = i + 1; j < inputRects.size(); ) {
            if (isNearlySame(inputRects[i], inputRects[j])) {
                inputRects.erase(inputRects.begin() + j);
            } else {
                ++j;
            }
        }
    }
}

cv::Rect Helper::getVerticalRect(const std::vector<cv::Rect>& candidates)
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
    int minX = (*std::min_element(candidates.begin(), candidates.end(),
                                             [] (const cv::Rect& lhs, const cv::Rect& rhs) {
                                                return (lhs.x < rhs.x);
                                             })).x;
    int minY = (*std::min_element(candidates.begin(), candidates.end(),
                                            [] (const cv::Rect& lhs, const cv::Rect& rhs) {
                                                return (lhs.y < rhs.y);
                                            })).y;
    auto widthIter = *std::max_element(candidates.begin(), candidates.end(),
                                            [] (const cv::Rect& lhs, const cv::Rect& rhs) {
                                                return ((lhs.x + lhs.width) < (rhs.x + rhs.width));
                                            });

    auto height = (*std::max_element(candidates.begin(), candidates.end(),
                                            [] (const cv::Rect& lhs, const cv::Rect& rhs) {
                                                return (lhs.height + lhs.y < rhs.height + rhs.y);
                                            }));
    int width = (widthIter).x - minX + (widthIter).width;
    return cv::Rect(minX, minY, width, (height.y + height.height - minY));
}

std::vector<cv::Rect> Helper::getNearLeftKeyCondidate(const cv::Rect& rect,
                                          const std::set<cv::Rect, CompareByCoordinates>& outsideRects)
{
    std::vector<cv::Rect> result;
    cv::Rect median = rect;
    for (const auto& iter : outsideRects) {
        if (iter.x >= rect.x) {
            continue;
        }
        if ((iter & rect).area() > 0) {
            continue;
        }
        if (isInSameLevel(rect, iter)) {
            if ((median.x - (iter.x + iter.x + iter.width)) > 10) {
                continue;
            }
            median = iter;
            result.emplace_back(iter);
        }
    }
    return std::move(result);
}

std::vector<cv::Rect> Helper::getNearRightKeyCondidate(const cv::Rect& rect,
                                               const std::set<cv::Rect, CompareByCoordinates>& outsideRects)
{
    std::vector<cv::Rect> result;
    cv::Rect median = rect;
    for (const auto& iter : outsideRects) {
        if (iter.x <= rect.x) {
            continue;
        }
        if ((iter & rect).area() > 0) {
            continue;
        }
        if (isInSameLevel(rect, iter)) {
            if (median.x - (iter.x + iter.width) > 10) {
                continue;
            }
            median = iter;
            result.emplace_back(iter);
        }
    }
    return std::move(result);
}

void Helper::substractSubLines(std::vector<LINE>& result, const LINE& currentLine)
{
    auto xCoord = currentLine.front().x;
    LINE line;
    for (auto item : currentLine) {
        if (item.x - xCoord > 120) {
            result.push_back(line);
            line.clear();
        }
        line.push_back(item);
        xCoord = item.x;
    }
    result.push_back(line);
}

auto Helper::extractLines(std::vector<cv::Rect>& data) -> std::vector<LINE>
{
    std::vector<LINE> result;
    if (data.empty()) {
        return result;
    }
    auto compareByX = [](const cv::Rect& lhs, const cv::Rect& rhs) {
        return lhs.x < rhs.x;
    };

    while (!data.empty()) {
        LINE currentLine;
        if (data.size() == 1) {
            currentLine.push_back(data.front());
            result.push_back(currentLine);
            return result;
        }
        cv::Rect median = data.front();
        currentLine.push_back(median);
        data.erase(data.begin());
        for(size_t i = 0; i < data.size(); ) {
            if (isInSameLevel(data[i], median)) {
                currentLine.push_back(data[i]);
                if (median.height < data[i].height) {
                    median = data[i];
                }
                data.erase(data.begin() + i);
            } else {
                ++i;
            }
        }
        std::sort(currentLine.begin(), currentLine.end(), compareByX);
        substractSubLines(result, currentLine);
//        result.push_back(currentLine);
    }
    return std::move(result);
}

template <typename ContainerType>
auto Helper::extractWords(const ContainerType& symbolsContainer, int diff) const -> std::vector<WORD>
{
    if (symbolsContainer.empty()) {
        return std::vector<WORD>();
    }
    std::vector<cv::Rect> symContainer(symbolsContainer.begin(), symbolsContainer.end());
    auto linesContainer = Helper::extractLines(symContainer);
    std::sort(linesContainer.begin(), linesContainer.end(), yComp);
    return HelperBase::extractWords(linesContainer, funcObj, diff);
}
