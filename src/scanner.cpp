#include <iterator>
#include <algorithm>
#include <thread>
#include <future>
#include <fstream>
#include <memory>

#include <iomanip>

#include "scanner.h"
#include "extractor.h"

using json = nlohmann::json;

namespace cv {

//! write to json object
void to_json(json& js, const Rect& rect)
{
    static unsigned index;
    static const std::string rectName = std::string("rectangle");
    json array = {rect.x, rect.y, rect.width, rect.height};
    js.push_back(json::object_t::value_type(rectName + std::to_string(++index), array));
}

}

using namespace scan;
using namespace parser;
using namespace utility;

Scanner::Scanner(const std::string& image)
    : m_filePath(image)
{
    if (!std::experimental::filesystem::exists(image)) {
        std::cout << "\033[1;31mThe Image path is wrong \033[0m" << std::endl;
        Helper::helpMessage();
        Parser::removeInstance();
        exit(0);
    }
    try {
        m_image = cv::imread(m_filePath);
        if (m_image.empty()) {
            std::cout << "ERROR provided image empty" << std::endl;
            exit(0);
        }
    } catch (const std::runtime_error& ex) {
        std::cout << "ERROR: please provide input image" << std::endl;
        throw;
    }
    pyrUp(m_image, m_image);
    //const std::string& tmp = Parser::getInstance()->getCmdArg("--type");
    //if (tmp.empty() || tmp == std::string("standart")) {
    //    m_type = FormType::STANDART;
    //} else if (tmp == std::string("rect")) {
    //    m_type = FormType::RECT;
    //} else if (tmp == std::string("column")) {
    //    m_type = FormType::COLUMN;
    //} else if (tmp == "line") {
    //    m_type = FormType::LINE;
    //} else if (tmp == "barcode") {
    //    m_type = FormType::BARCODE;
    //} else {
    //    std::cout << "\033[1;31mSpecified form type does not registered\033[0m" << std::endl;
    //    Helper::helpMessage();
    //    Parser::removeInstance();
    //    exit(0);

    //}
    APISingleton::getInstance()->m_api->Init(nullptr, "eng", tesseract::OEM_TESSERACT_CUBE_COMBINED);
    APISingleton::getInstance()->m_image = m_image;
}

template <typename ContainerType>
auto Scanner::extractWords(const ContainerType& symbolsContainer, int diff) const -> std::vector<WORD>
{
    if (symbolsContainer.empty()) {
        return std::vector<WORD>();
    }
    std::vector<cv::Rect> symContainer(symbolsContainer.begin(), symbolsContainer.end());
    auto linesContainer = Helper::extractLines(symContainer);
    std::sort(linesContainer.begin(), linesContainer.end(), yComp);
    return HelperBase::extractWords(linesContainer, funcObj, diff);
}

void Scanner::makePairForCheckBox(const cv::Rect& rect,
        const std::set<cv::Rect, CompareByCoordinates>& keysSet,
        std::map<cv::Rect, std::pair<std::string, std::string>, CompareByCoordinates>& result)
{
    std::vector<cv::Rect> valueCandidates = Helper::getNearRightKeyCondidate(rect, m_outsideWordsSet);
    //! To improve our matching result we should avoid all kind of noises in our matched text boxes
    //! for that reason in this code unit we should merge rects it will improve our OCR result
    //! here we think think that keys are placed in one line
    //! for another case if they are placed in multiple lines we should iterate over values
    std::string value = valueCandidates.empty() ? std::string() : getTextFromCandidateVector(valueCandidates);

    std::string key = std::string();
    for (const auto& item : keysSet) {
        key.append(recognizeOne(item) + std::string(" "));
    }
    //! there are several places where we have keys without values it means we have unfilled place
    if (value.empty()) {
        result.insert(std::make_pair(rect, std::make_pair(key, "No value")));
    } else {
        result.insert(std::make_pair(rect, std::make_pair(key, value)));
    }
}

void Scanner::makePairForUsualRects(const cv::Rect& rect,
        std::map<cv::Rect, std::pair<std::string, std::string>, CompareByCoordinates>& result)
{
    std::vector<cv::Rect> keyCandidates = Helper::getNearLeftKeyCondidate(rect, m_outsideWordsSet);
    //! To improve our matching result we should avoid all kind of noises in our matched text boxes
    //! for that reason in this code unit we should merge rects it will improve our OCR result
    std::string key =  keyCandidates.empty() ? std::string() : getTextFromCandidateVector(keyCandidates);

    std::string value = std::string();
    value = recognizeOne(cv::Rect(rect.x, rect.y, rect.width, rect.height));
    //! there are several places where we have values without keys such as data etc.
    if (key.empty()) {
        if (value.empty()) {
            return;
        }
        result.insert(std::make_pair(rect, std::make_pair("No key", value)));
    } else {
        result.insert(std::make_pair(rect, std::make_pair(key, value)));
    }
}

std::string Scanner::getTextFromCandidateVector(const std::vector<cv::Rect>& candidates) const
{
    cv::Rect tmpRect = getBoundingRect(candidates);
    if (tmpRect.x < 0 || tmpRect.y < 0 || tmpRect.width < 0 || tmpRect.height < 0) {
        return std::string();
    }
    return recognizeOne(tmpRect);
}

std::vector<cv::Rect> Scanner::findTextRects(const cv::Mat& image, bool fake) const
{
    using namespace std;
    std::vector<cv::Rect> symbols;
    cv::Mat gray = image;
    cv::Mat grad, blackWhite;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cvtColor(gray, gray, CV_BGR2GRAY);
    cv::Mat morphKernel = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    morphologyEx(gray, grad, cv::MORPH_GRADIENT, morphKernel);
    if (fake) {
        cv::fastNlMeansDenoising(grad, grad, 20);
    }
    threshold(grad, blackWhite, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
    // find contours
    if (fake) {
        findContours(blackWhite, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    } else {
        cv::Mat connected;
        // connect horizontally oriented regions
        morphKernel = getStructuringElement(cv::MORPH_RECT, cv::Size(9, 1));
        morphologyEx(blackWhite, connected, cv::MORPH_CLOSE, morphKernel);
        findContours(connected, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    }

    int imageSection = m_image.cols / 3.2; //m_image.cols / (m_type == FormType::DRIVING_ALBERTA) ? m_image.cols / 3.2 : m_image.cols / 3.3;
    int headerSection = m_image.rows / 8;
    int maxAllowedHeight = m_image.rows / 15;
    int maxAllowedWidth  = m_image.cols / 5;
    int minAllowedHeight = m_image.rows / 50;
    int minAllowedWidth  = m_image.cols / 70;
    // filter contours
    for(int idx = 0; idx >= 0; idx = hierarchy[idx][0]) {
        cv::Rect rect = boundingRect(contours[idx]);
        if (rect.height > maxAllowedHeight || rect.width > maxAllowedWidth ||
                rect.x < imageSection || rect.y < headerSection) {
            continue;
        }
        if ((rect.height > minAllowedHeight) || (rect.width > minAllowedWidth)) {
            symbols.push_back(rect);
        }
    }
    return std::move(symbols);
}

std::vector<cv::Rect> Scanner::findTextRects() const
{
    std::vector<cv::Rect> symbols;

    auto compareFunc =  [] (const cv::Rect& lhs, const cv::Rect& rhs) {
        return ((std::abs(lhs.x - rhs.x) <= 11) &&
                (std::abs(lhs.y - rhs.y) <= 11));
    };
    //!Here we run async rect detection part as this function result need us
    auto findTextFuture = std::async(std::launch::async, &Scanner::findRects, this);

    ///@brief in this part we are trying to find all symbols on the image
    ///such as characters and shapes
    ///{
    cv::Mat img = m_image.clone();
    cv::Mat gray;
    cvtColor(img, gray, CV_BGR2GRAY);
    cv::Mat blackWhite;
    cv::bitwise_not(gray, blackWhite);
    std::vector<std::vector<cv::Point> > contours;
    threshold(blackWhite, blackWhite, 100, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    findContours(blackWhite, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    for (size_t i = 0; i < contours.size(); ++i) {
        cv::Rect rect = boundingRect(contours[i]);
        if ((rect.width < 8 ) && (rect.height > 50)) {
            continue;
        }
        if ((rect.height < 200) && (rect.width < 200)) {
            symbols.emplace_back(rect);
        }
    }
    ///}
    findTextFuture.get();
    if (m_rectsVector.empty() || symbols.empty()) {
        return symbols;
    }
    for (const auto & item : m_rectsVector) {
        for (size_t i = 0; i < symbols.size(); ) {
            if (compareFunc(item, symbols[i])) {
                symbols.erase(symbols.begin() + i);
            } else {
                ++i;
            }
        }
    }
    Helper::removeExtras(symbols);
    //! DEBUG code
    return std::move(symbols);
}

std::string Scanner::recognize(const std::vector<cv::Rect>& boundingRects) const
{
    std::string result;
    for (const auto& rect : boundingRects) {
        result += recognizeOne(rect) + ' ';
    }
    return result;
}

std::string Scanner::recognizeOne(const cv::Rect& rect) const
{
    return APISingleton::getInstance()->recognize(rect);
}

const std::vector<cv::Rect>& Scanner::findRects() const
{
    if (!m_rectsVector.empty()) {
        return m_rectsVector;
    }
    cv::Mat inputImage = m_image.clone();
    cv::Mat gray;
    cvtColor(inputImage, gray, CV_BGR2GRAY);

    cv::Mat grad;
    cv::Mat morphKernel = getStructuringElement(cv::MORPH_RECT, cv::Size(8, 2));
    cv::morphologyEx(gray, grad, cv::MORPH_RECT, morphKernel);

    cv::Mat blackWhite;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;
    threshold(grad, blackWhite, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
    findContours(blackWhite, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    for(int idx = 0; idx >= 0; idx = hierarchy[idx][0]) {
        cv::Rect rect = boundingRect(contours[idx]);
        if (((rect.height > 14) &&
            (rect.width > 20)) &&
                rect.height < 350) {
            m_rectsVector.emplace_back(rect);
        }
    }
    //! this part is Debug information to see our algorithm result visually
    //! in release vesrion we should remove this part
    //for (const auto& rect : m_rectsVector) {
    //    rectangle(inputImage, rect, cv::Scalar(0, 255, 0), 5);
    //}
    //imwrite("rects.jpg", inputImage);
    return m_rectsVector;
}


void Scanner::matchDatas()
{
    std::map<cv::Rect, std::set<cv::Rect, CompareByCoordinates>, CompareByX> hierarchicTree;
    std::unique_ptr<Extractor> extPtr;
    std::vector<cv::Rect> textRects = findTextRects();
    extractWords(textRects);
//    //! this map is keeps from outside rect to its values
//    //! the values are all text boxes inside this rect in a ordered form
//    //! outsideRects set keeps all rects which are outside, sorted by theire coordinates
//    std::set<cv::Rect, CompareByCoordinates> outsideRects;
//    //! we use these loops to find all text rects which are outside bounding rects
//    //! and to fill hierarchicTree multimap
//    //! TODO look to this part of the code to avoid erase operation and remove bacause here we have O(n^2) complexity
//    for (const auto& item : textRects) {
//        bool outside = true;
//        for (const auto& rect : m_rectsVector) {
//            if (inside(item, rect)) {
//                outside = false;
//                hierarchicTree[rect].insert(item);
//            }
//        }
//        if (outside) {
//            outsideRects.insert(item);
//        }
//    }
//    collectWordsFromImage(outsideRects, hierarchicTree);
//
//    std::map<cv::Rect, std::pair<std::string, std::string>, utility::CompareByCoordinates> result;
//    for (const auto& item : hierarchicTree) {
//        if (isCheckBox(item.first)) {
//            makePairForCheckBox(item.first, item.second, result);
//        } else {
//            makePairForUsualRects(item.first, result);
//        }
//    }
//    m_rectsVector.clear();
//    return;
}

template <typename ContainerType>
void Scanner::collectWordsFromImage(const ContainerType& outsideRects,
                                    const std::map<cv::Rect, std::set<cv::Rect, CompareByCoordinates>, CompareByX>& hierarchicTree)
{
    m_outsideWordsSet.clear();
    m_insideWordsSet.clear();
    auto outWords = extractWords(outsideRects);
    for (const auto& out : outWords) {
        cv::Rect rect = getBoundingRect(out);
        m_outsideWordsSet.insert(rect);
    }
    for (const auto& item : hierarchicTree) {
        auto wordInHier = extractWords(item.second);
        for (const auto& in : wordInHier) {
            cv::Rect rect = getBoundingRect(in);
            m_insideWordsSet.insert(rect);
        }
    }

    std::ofstream outFile("output.json", std::ofstream::out);
    json jsonOutputObject;
    for (const auto& word : m_outsideWordsSet) {
        to_json(jsonOutputObject, word);
    }
    for (const auto& word : m_insideWordsSet) {
        to_json(jsonOutputObject, word);
    }
    outFile << std::setw(4) << jsonOutputObject;
}
