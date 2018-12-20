#pragma once

#include <map>

#include <experimental/filesystem>
#include <iostream>
#include <functional>

#include "json.hpp"
#include "helper.h"
#include "parser.h"
#include "api.h"

namespace scan {

class Scanner
{
    /// @brief constructors and destructors
    /// @{
public:
    /**
     *  @brief constructor the Scanner constructor takes the input image
     *         after that it loads the provided image and intialize OCR
     */
    Scanner(const std::string& image);

    Scanner(const Scanner& other) = default;
    Scanner(Scanner&& other) = default;
    Scanner& operator=(const Scanner& other) = default;
    Scanner& operator=(Scanner&& other) = default;
    ~Scanner()
    {
        APISingleton::removeInstance();
    }
    /// @}

private:

    /**
     * @brief extractWords function role is to extract words from provided symbols set
     * @param symbolsContainer extracted symbols
     * @return extracted words vector
     */
    template <typename ContainerType>
    std::vector<WORD> extractWords(const ContainerType& symbolsContainer, int diff =  10) const;

    /**
     * @brief collectWordsFromImage function role is to collect all words from the image
     *        after collecting we keep outside and inside words in apropriate containers
     * @brief outsideRects this vector keeps outsite symbols set
     * @brief hierarchicTree this structure keeps from rects to their symbols set
     */
    template <typename ContainerType>
    void collectWordsFromImage(const ContainerType& outsideRects,
                               const std::map<cv::Rect, std::set<cv::Rect, utility::CompareByCoordinates>, utility::CompareByX>& hierarchicTree);

private:
    /**
     * @brief recognizeOne This function is takeing one rect and trying to
     *        recognize the text inside this rectangle
     *        Recognition process.
     *              1. crop from original image provided rect
     *              2. change the cropped image background to rgba to improve recognition level
     * @param rect input rectangle
     * @return returns the recognized string
     */
    std::string recognizeOne(const cv::Rect& rect) const;

    /**
     * @brief this function is designed to recognize the text from image
     *        Recognition process.
     *        1. iterate over the rects vector
     *        2. for each rect repeat the recognizeOne process
     * @param boundingRects recognized text bounding boxes
     *
     */
    std::string recognize(const std::vector<cv::Rect>& boundingRects) const;

    /// @brief pair making functions
    ///{
private:

    /**
     * @brief makePairForCheckBox function is helper function to find and match key -> value pairs
     *        as for check boxes values are placed in the right side of checkbox we should find it's candidate values
     *        on right side and put them in theire places
     * @param rect the rect for which we are trying to find values
     * @param result this map is our result map we should fill this map with matched key -> values
     */
    void makePairForCheckBox(const cv::Rect& rect,
            const std::set<cv::Rect, utility::CompareByCoordinates>& keysSet,
            std::map<cv::Rect, std::pair<std::string, std::string>, utility::CompareByCoordinates>& result);

    /**
     * @brief makePairForUsualRects function is helper function to find and match key -> value pairs
     *        for rects which are not check boxes
     * @param rect the rect for which we are trying to find values
     * @param result this map is our result map we should fil this map with mached key -> values
     * @note this and makePairForCheckBox function are the same the differece is that for this version we are trying to find
     *       candidate values on the left side of proveded rect and for second version on he right side
     */
    void makePairForUsualRects(const cv::Rect& rect,
            std::map<cv::Rect, std::pair<std::string, std::string>, utility::CompareByCoordinates>&result);

    ///}

   /**
    * @brief getTextFromCandidateVector function is returning the string ehich is saved on that provided vector rects
    * @param candidates candidae (key or value) vector
    */
    std::string getTextFromCandidateVector(const std::vector<cv::Rect>& candidates) const;

public:
    void updateLine(const cv::Vec4i& l, cv::Vec4i& averageLine, const double& tangent);

    void findLines();
    /**
     * @brief findTextRects this function is finding all text rects
     * @return returns a vector of text rects
     */
    std::vector<cv::Rect> findTextRects() const;
    std::vector<cv::Rect> findTextRects(const cv::Mat& image, bool fake) const;

    /**
     * @brief findRects finds all unique rects on cv::Mat image
     *        unique rect finding algorithm is following
     *        1 find all rects
     *        2 remove extra components the menaing see on removeExtra function description
     * @return returns the rects vector
     */
    const std::vector<cv::Rect>& findRects() const;

    /**
     * @brief matchDatas this fcuntion role is to find out key/value datas from our provided image
     *        matching algorithm
     *        1. get all text rects from image
     *        2. get all rects from image
     *        3. identify outside / inside rects (rects inside the rects and outside)
     *        4. find probable condidate keys based on image details
     *        5. find values based key direction
     * @return returns std::map<std::string, std::string> key/value pairs
     */
    void matchDatas();

private:
    parser::FormType                m_type;
    std::string                     m_filePath;
    cv::Mat                         m_image;
    mutable std::vector<cv::Rect>   m_rectsVector;
    mutable std::set<cv::Rect, utility::CompareByCoordinates> m_insideWordsSet;
    mutable std::set<cv::Rect, utility::CompareByCoordinates> m_outsideWordsSet;
};

} // namespace scan
