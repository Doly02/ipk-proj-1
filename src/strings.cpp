/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      strings.cpp
 *  Author:         Tomas Dolak
 *  Date:           13.03.2024
 *  Description:    Implements Handling Program Arguments.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           strings.cpp
 *  @author         Tomas Dolak
 *  @date           13.03.2024
 *  @brief          Implements Handling Program Arguments.
 * ****************************/


#include "../include/strings.hpp"

    /**
     * @brief Compares Content Of Vector And String
     * @param vec Vector To Compare
     * @param str String To Compare
     * 
     * @return True If The Content Of Vector And String Are The Same, Otherwise False
    */
   
    bool compareVectorAndString(const std::vector<char>& vec, const std::string& str) 
    {
        std::string vecAsString(vec.begin(), vec.end());
        return vecAsString == str;
    }

    bool areAllDigitsOrLettersOrDash(const std::vector<char>& vec) {
        return std::all_of(vec.begin(), vec.end(), [](char c) { 
            return std::isdigit(c) || std::isalpha(c) || c == '-'; 
        });
    }
    bool areAllDigitsOrLettersOrDashOrDot(const std::vector<char>& vec) {
        return std::all_of(vec.begin(), vec.end(), [](char c) { 
            return std::isdigit(c) || std::isalpha(c) || c == '-'|| c == '.'; 
        });
    }
    bool areAllPrintableCharacters(const std::vector<char>& vec) 
    {
        return std::all_of(vec.begin(), vec.end(), [](char c) { return c >= 0x21 && c <= 0x7E; });
    }

    bool areAllPrintableCharactersOrSpace(const std::vector<char>& vec) 
    {
        return std::all_of(vec.begin(), vec.end(), [](char c) { return c >= 0x20 && c <= 0x7E; });
    }


    std::string convertToString(const std::vector<char>& inputVector)
    {   
    // Vytvoření stringu z vektoru
    return std::string(inputVector.begin(), inputVector.end());
    }

    bool compare(const std::vector<char>& vec, const std::string& pattern) {
        std::string str(vec.begin(), vec.end());
        std::regex regexPattern(pattern);

        return std::regex_search(str, regexPattern);
    }
    

