/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CssStyleParser.h"

const std::unordered_map<std::string, std::string>& CssStyleParser::getArributesMap(const std::string& key) const
{
    auto styleClassIter = fStyleMap.find(key);
    if (styleClassIter != fStyleMap.end()) {
        return styleClassIter->second;
    } else {
        static std::unordered_map<std::string, std::string> fEmptyMap;
        return fEmptyMap;
    }
}

void CssStyleParser::parseCssStyle(const std::string& style)
{
    // begin from 1 to skip first '.'
    auto styles = splitString(style.substr(1), "}.");
    for (auto& style : styles) {
        auto nameEnd = style.find_first_of('{');
        if (nameEnd != std::string::npos) {
            auto names = style.substr(0, nameEnd);
            if (names.empty()) {
                return;
            }
            auto splitNames = splitString(names, ",.");
            auto attributesString = style.substr(nameEnd + 1);
            auto attributesVector = splitString(attributesString, ";");
            for (auto& splitName : splitNames) {
                for (auto& attribute : attributesVector) {
                    auto arrPair = splitString(attribute, ":");
                    if (arrPair.size() == 2) {
                        auto arrMapIter = fStyleMap.find(splitName);
                        if (arrMapIter == fStyleMap.end()) {
                            std::unordered_map<std::string, std::string> arrMap;
                            arrMap.emplace(std::make_pair(arrPair[0], arrPair[1]));
                            fStyleMap.emplace(std::make_pair(splitName, arrMap));
                        } else {
                            arrMapIter->second.emplace(std::make_pair(arrPair[0], arrPair[1]));
                        }
                    }
                }
            }
        }
    }
}

std::vector<std::string> CssStyleParser::splitString(const std::string& srcString, const std::string& splitString)
{
    std::string::size_type pos1;
    std::string::size_type pos2;
    std::vector<std::string> res;
    pos2 = srcString.find(splitString);
    pos1 = 0;
    while (std::string::npos != pos2) {
        res.push_back(srcString.substr(pos1, pos2 - pos1));

        pos1 = pos2 + splitString.size();
        pos2 = srcString.find(splitString, pos1);
    }
    if (pos1 != srcString.length()) {
        res.push_back(srcString.substr(pos1));
    }
    return res;
}