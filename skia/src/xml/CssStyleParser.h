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

#ifndef FOUNDATION_ACE_CORE_CSS_STYLE_PARSER_H
#define FOUNDATION_ACE_CORE_CSS_STYLE_PARSER_H

#include <string>
#include <unordered_map>
#include <vector>

using ClassStyleMap = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;

class CssStyleParser {
public:
    void parseCssStyle(const std::string& style);
    const std::unordered_map<std::string, std::string>& getArributesMap(const std::string& key) const;
    static std::vector<std::string> splitString(const std::string& srcString, const std::string& splitString);

private:
    ClassStyleMap fStyleMap;
};

#endif