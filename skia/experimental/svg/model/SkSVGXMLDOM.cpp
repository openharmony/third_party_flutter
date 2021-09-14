/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSVGXMLDOM.h"

#include <cstdlib>

#include "CssStyleParser.h"
#include "include/core/SkStream.h"
#include "include/private/SkTo.h"
#include "src/xml/SkDOM.cpp"
#include "src/xml/SkXMLParser.h"
#include "src/xml/SkXMLWriter.h"

union SkColorEx {
    struct {
        SkColor  color    : 32;
        bool     valid    : 1;
        uint32_t reserved : 31; // reserved
    };
    uint64_t value = 0;
};

class SkSVGDOMParser : public SkDOMParser {
public:
    SkSVGDOMParser(SkArenaAlloc* chunk) : SkDOMParser(chunk) {}
    /** Returns true for success
    */
    bool parse(SkStream& docStream, uint64_t svgThemeColor) {
        fSvgThemeColor = svgThemeColor;
        return SkXMLParser::parse(docStream);
    };

protected:
    void flushAttributes() override {
        SkASSERT(fLevel > 0);
        SkDOM::Node* node = fAlloc->make<SkSVGXMLDOM::Node>();
        flushAttributesWithNode(node);
    }

    bool onStartElement(const char elem[]) override {
        this->startCommon(elem, SkDOM::kElement_Type);
        if (!strcmp(elem, "style")) {
            fProcessingStyle = true;
        }
        return false;
    }

    bool setSVGColorAndOpacity(
        SkDOM::Attr* attr, const char name[], const char value[], const SkColorEx& svgThemeColor) {
        if (svgThemeColor.valid && (((strcmp(name, "fill") == 0) && (strcmp(value, "none") != 0)) ||
            ((strcmp(name, "stroke") == 0) && (strcmp(value, "none") != 0))) && isPureColor(value)) {
            char colorBuffer[8];
            int res = snprintf(colorBuffer, sizeof(colorBuffer), "#%06x", (svgThemeColor.color & 0xFFFFFF));
            attr->fValue = dupstr(fAlloc, (res < 0) ? value : colorBuffer);
            return false;
        }
        if ((svgThemeColor.valid == 1) && (strcmp(name, "opacity") == 0)) {
            char opacityBuffer[4];
            // the opacity is stored in svgThemeColor[24:31], so shift right by 24 bits after extracting it,
            // for e.g., (0x33FFFFFF & 0xFF000000) >> 24 = 0x33.
            // the target string of opacity is like "0.1", so normalize 0x33 to 1, for e.g., 0x33 / 255 = 0.13.
            int res = snprintf(
                opacityBuffer, sizeof(opacityBuffer), "%2.1f", ((svgThemeColor.color & 0xFF000000) >> 24) / 255.0);
            attr->fValue = dupstr(fAlloc, (res < 0) ? value : opacityBuffer);
            return false;
        }
        return true;
    }

    bool onAddAttribute(const char name[], const char value[]) override {
        SkDOM::Attr* attr = fAttrs.append();
        attr->fName = dupstr(fAlloc, name);
        SkColorEx svgThemeColor;
        svgThemeColor.value = fSvgThemeColor;
        if (!setSVGColorAndOpacity(attr, name, value, svgThemeColor)) {
            return false;
        }
        attr->fValue = dupstr(fAlloc, value);
        // add attributes in style classes.
        if (!strcmp(attr->fName, "class")) {
            auto styleClassMap = fStyleParser.getArributesMap(attr->fValue);
            if (!styleClassMap.empty()) {
                for (auto& arr: styleClassMap) {
                    SkDOM::Attr* attr = fAttrs.append();
                    attr->fName = dupstr(fAlloc, arr.first.c_str());
                    if (!setSVGColorAndOpacity(attr, attr->fName, arr.second.c_str(), svgThemeColor)) {
                        continue;
                    }
                    attr->fValue = dupstr(fAlloc, arr.second.c_str());
                }
            }
        }
        return false;
    }

    bool onEndElement(const char elem[]) override {
        if (SkDOMParser::onEndElement(elem)) {
            return true;
        }
        if (!strcmp(elem, "style")) {
            fProcessingStyle = false;
        }
        return false;
    }

    bool onText(const char text[], int len) override {
        SkString str(text, len);
        this->startCommon(str.c_str(), SkDOM::kText_Type);
        this->SkSVGDOMParser::onEndElement(str.c_str());
        if (fProcessingStyle) {
            std::string style(str.c_str());
            if (!style.empty() && style.front() == '.') {
                fStyleParser.parseCssStyle(style);
            }
        }

        return false;
    }

    bool isPureColor(const char value[]) const {
        std::string color(value);
        if (color.empty()) {
            return true;
        }

        auto pos = color.find_first_not_of(' ');
        if (pos != std::string::npos) {
            color = color.substr(pos);
        }

        if (color.length() > urlLength && color.substr(0, urlLength - 1) == "url(#") {
            return false;
        }
        return true;
    }

private:
    // for parse css style svg files.
    bool fProcessingStyle = false;
    CssStyleParser fStyleParser;
    uint64_t fSvgThemeColor = 0;
    static const int urlLength = 6;
};


SkSVGXMLDOM::SkSVGXMLDOM() = default;
SkSVGXMLDOM::~SkSVGXMLDOM() = default;

const SkDOM::Node* SkSVGXMLDOM::build(SkStream& docStream) {
    SkSVGDOMParser parser(&fAlloc);
    if (!parser.parse(docStream, fSvgThemeColor)) {
        SkDEBUGCODE(SkDebugf("xml parse error, line %d\n", parser.fParserError.getLineNumber());)
        fRoot = nullptr;
        fAlloc.reset();
        return nullptr;
    }
    fRoot = parser.getRoot();
    return fRoot;
}

const SkDOM::Node* SkSVGXMLDOM::build(SkStream& docStream, uint64_t svgThemeColor) {
    fSvgThemeColor = svgThemeColor;
    return SkSVGXMLDOM::build(docStream);
}

const SkDOM::Node* SkSVGXMLDOM::copy(const SkDOM& dom, const SkDOM::Node* node) {
    SkSVGDOMParser parser(&fAlloc);

    walk_dom(dom, node, &parser);

    fRoot = parser.getRoot();
    return fRoot;
}

SkXMLParser* SkSVGXMLDOM::beginParsing() {
    SkASSERT(!fParser);
    fParser.reset(new SkSVGDOMParser(&fAlloc));

    return fParser.get();
}