/******************************
 *  Project:        IPK Project 1 - Client for Chat Servers
 *  File Name:      strings.hpp
 *  Author:         Tomas Dolak
 *  Date:           27.03.2024
 *  Description:    Implements Operations Over Strings And String Vector. 
 *
 * ****************************/

/******************************
 *  @package        IPK Project 1 - Client for Chat Servers
 *  @file           strings.hpp
 *  @author         Tomas Dolak
 *  @date           27.03.2024
 *  @brief          Implements Operations Over Strings And String Vector.
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

    /**
     * @brief Checks If All Characters In Vector Are Digits, Letters Or Dash
     * @param vec Vector To Check
     * 
     * @return True If All Characters In Vector Are Digits, Letters Or Dash, Otherwise False
    */
    bool areAllDigitsOrLettersOrDash(const std::vector<char>& vec);
    /**
     * @brief Checks If All Characters In Vector Are Digits, Letters, Dash Or Dot
     * @param vec Vector To Check
     * 
     * @return True If All Characters In Vector Are Digits, Letters, Dash Or Dot, Otherwise False
    */    
    bool areAllDigitsOrLettersOrDashOrDot(const std::vector<char>& vec);
    /**
     * @brief Checks If All Characters In Vector Are Printable Characters
     * @param vec Vector To Check
     * 
     * @return True If All Characters In Vector Are Printable Characters, Otherwise False
    */    
    bool areAllPrintableCharacters(const std::vector<char>& vec);
    /**
     * @brief Checks If All Characters In Vector Are Printable Characters Or Space
     * @param vec Vector To Check
     * 
     * @return True If All Characters In Vector Are Printable Characters Or Space, Otherwise False
    */
    bool areAllPrintableCharactersOrSpace(const std::vector<char>& vec);
    /**
     * @brief Converts Vector Of Characters To String
     * @param inputVector Vector To Convert
     * 
     * @return String Created From Vector
    */
    std::string convertToString(const std::vector<char>& inputVector);
    /**
     * @brief Compares Content Of Vector And String
     * @param vec Vector To Compare
     * @param pattern Pattern To Compare
     * 
     * @return True If The Content Of Vector And String Are The Same, Otherwise False
    */
    bool compare(const std::vector<char>& vec, const std::string& pattern);
    
    
#endif // STRINGS_H


