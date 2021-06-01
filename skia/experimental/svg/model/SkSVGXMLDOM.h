/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGXMLDOM_DEFINED
#define SkSVGXMLDOM_DEFINED

#include "src/xml/SkDOM.h"

class SkSVGDOMParser;

class SkSVGXMLDOM : public SkDOM {
public:
    SkSVGXMLDOM();
    ~SkSVGXMLDOM() override;

    typedef SkDOMNode Node;
    typedef SkDOMAttr Attr;

    /** Returns null on failure
    */
    const Node* build(SkStream&) override;
    const Node* build(SkStream& docStream, uint64_t svgThemeColor);
    const Node* copy(const SkDOM& dom, const Node* node) override;

    virtual SkXMLParser* beginParsing() override;

private:
    uint64_t fSvgThemeColor = 0;
    std::unique_ptr<SkSVGDOMParser> fParser;
};

#endif
