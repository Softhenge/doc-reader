#pragma once

#include <vector>
#include <string>

namespace parser {

enum class FormType : unsigned char
{
};

/**
 * @brief file parser.h
 * @class Parser
 * @description programm input variable straightforward implementation
 * @param m_tokens this is argumens vector is keeps input argumens and thire values
 * @param m_emptyString this is the empty string, we keep this this empty string to
 *                      return this istring in case if the provided option does not speciied
 *                      or if provided options values does not specified
 *                      we keep this dummy string to avoid local addres returning issues
 */
class Parser
{
public:
    /**
     * @brief constructor
     * @param argc argument count this will be 1 plus the number of arguments
     * @param argv argument vector
     */
    Parser(const int& argc, char** const argv)
    {
        for (int i = 1; i < argc; ++i) {
            m_tokens.push_back(std::string(argv[i]));
        }
    }

    ///@ check is the argument and its options are registered or not
    bool argOptionExists(const std::string& option) const
    {
        return std::find(m_tokens.begin(), m_tokens.end(), option) != m_tokens.end();
    }

    ///@ returns the specified option value
    const std::string& getCmdArg(const std::string& option) const
    {
        auto iter = std::find(m_tokens.begin(), m_tokens.end(), option);
        if (iter != m_tokens.end() && ++iter != m_tokens.end()) {
            return *iter;
        }
        return m_emptyString;
    }

    ///@brief static interfaces
    ///{
public:
    static Parser* createInstance(const int& argc, char** const argv)
    {
        if (s_instance == nullptr) {
            s_instance = new Parser(argc, argv);
        }
        return s_instance;
    }

    static Parser* getInstance()
    {
        return s_instance;
    }

    static void removeInstance()
    {
        delete s_instance;
        s_instance = nullptr;
    }

private:
    static Parser* s_instance;
    ///}

private:
    std::string m_emptyString;
    std::vector<std::string> m_tokens;
};

} //namespace parser
