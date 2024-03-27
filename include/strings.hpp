/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      strings.hpp
 *  Author:         Tomas Dolak
 *  Date:           13.03.2024
 *  Description:    Implements Handling Program Arguments.
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           strings.hpp
 *  @author         Tomas Dolak
 *  @date           13.03.2024
 *  @brief          Implements Handling Program Arguments.
 * ****************************/

#ifndef STRINGS_H
#define STRINGS_H

#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <regex>

    /**
     * @brief Compares Content Of Vector And String
     * @param vec Vector To Compare
     * @param str String To Compare
     * 
     * @return True If The Content Of Vector And String Are The Same, Otherwise False
    */
    bool compareVectorAndString(const std::vector<char>& vec, const std::string& str); 


    bool areAllDigitsOrLettersOrDash(const std::vector<char>& vec);
    
    bool areAllDigitsOrLettersOrDashOrDot(const std::vector<char>& vec);
    bool areAllPrintableCharacters(const std::vector<char>& vec);

    bool areAllPrintableCharactersOrSpace(const std::vector<char>& vec);

    std::string convertToString(const std::vector<char>& inputVector);

    bool compare(const std::vector<char>& vec, const std::string& pattern);
    
    
#endif // STRINGS_H


