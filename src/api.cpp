#include "api.h"

APISingleton* APISingleton::s_instance = nullptr;

APISingleton* APISingleton::getInstance()
{
    if (nullptr == s_instance) {
        s_instance = new APISingleton();
    }
    return s_instance;
}

void APISingleton::removeInstance()
{
    delete s_instance;
    s_instance = 0;
}

APISingleton::APISingleton()
    : m_api (new tesseract::TessBaseAPI())
{
}

APISingleton::~APISingleton()
{
    delete m_api;
    m_api = nullptr;
}

std::string APISingleton::recognize(const cv::Rect& rect) const
{
    if ((rect.height == 0) || (rect.width == 0)) {
        return std::string();
    }
    cv::Mat sub = m_image(rect);
    m_api->SetImage((uchar*)sub.data,
                    sub.size().width,
                    sub.size().height,
                    sub.channels(),
                    sub.step1());
    std::string txt = m_api->GetUTF8Text();
    txt.erase(std::remove(txt.begin(), txt.end(), '\n'), txt.end());
    return std::move(txt);
}

std::string APISingleton::recognize(const cv::Mat& image, const cv::Rect& rect) const
{
    if ((rect.height == 0) || (rect.width == 0)) {
        return std::string();
    }
    cv::Mat sub = image(rect);
    m_api->SetImage((uchar*)sub.data,
                    sub.size().width,
                    sub.size().height,
                    sub.channels(),
                    sub.step1());
    std::string txt = m_api->GetUTF8Text();
    txt.erase(std::remove(txt.begin(), txt.end(), '\n'), txt.end());
    return std::move(txt);
}

