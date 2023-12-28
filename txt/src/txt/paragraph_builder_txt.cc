/*
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "paragraph_builder_txt.h"

#include "paragraph_txt.h"
#include <unicode/utf8.h>

namespace txt {

ParagraphBuilderTxt::ParagraphBuilderTxt(
    const ParagraphStyle& style,
    std::shared_ptr<FontCollection> font_collection)
    : font_collection_(std::move(font_collection)) {
  SetParagraphStyle(style);
}

ParagraphBuilderTxt::~ParagraphBuilderTxt() = default;

void ParagraphBuilderTxt::SetParagraphStyle(const ParagraphStyle& style) {
  paragraph_style_ = style;
  paragraph_style_index_ = runs_.AddStyle(style.GetTextStyle());
  runs_.StartRun(paragraph_style_index_, text_.size());
}

void ParagraphBuilderTxt::PushStyle(const TextStyle& style) {
  size_t style_index = runs_.AddStyle(style);
  style_stack_.push_back(style_index);
  runs_.StartRun(style_index, text_.size());
}

void ParagraphBuilderTxt::Pop() {
  if (style_stack_.empty()) {
    return;
  }
  style_stack_.pop_back();
  runs_.StartRun(PeekStyleIndex(), text_.size());
}

size_t ParagraphBuilderTxt::PeekStyleIndex() const {
  return style_stack_.size() ? style_stack_.back() : paragraph_style_index_;
}

const TextStyle& ParagraphBuilderTxt::PeekStyle() {
  return runs_.GetStyle(PeekStyleIndex());
}

void ParagraphBuilderTxt::AddText(const std::u16string& text) {
  text_.insert(text_.end(), text.begin(), text.end());
}

void ParagraphBuilderTxt::AddSymbol(const uint32_t& symbolId)
{
    std::vector<uint32_t> symbolUnicode = {symbolId};
    std::vector<uint16_t> symbolUnicode16 = SymbolToUTF16(symbolUnicode);
    text_.insert(text_.end(), symbolUnicode16.begin(), symbolUnicode16.end());
}

std::vector<uint16_t> ParagraphBuilderTxt::SymbolToUTF16(const std::vector<uint32_t> &utf32Text)
{
    size_t utf32Index = 0;
    size_t codePoint = 0;
    int error = 0;
    std::vector<uint16_t> utf16Text;
    while (utf32Index < utf32Text.size()) {
        UTF32_NEXT_CHAR_SAFE(utf32Text.data(), utf32Index, utf32Text.size(), codePoint, error);
        utf16Text.push_back(U16_LEAD(codePoint));
        utf16Text.push_back(U16_TRAIL(codePoint));
    }
    return utf16Text;
}

void ParagraphBuilderTxt::AddPlaceholder(PlaceholderRun& span) {
  obj_replacement_char_indexes_.insert(text_.size());
  runs_.StartRun(PeekStyleIndex(), text_.size());
  AddText(std::u16string(1ull, objReplacementChar));
  runs_.StartRun(PeekStyleIndex(), text_.size());
  inline_placeholders_.push_back(span);
}

std::unique_ptr<Paragraph> ParagraphBuilderTxt::Build() {
  runs_.EndRunIfNeeded(text_.size());

  std::unique_ptr<ParagraphTxt> paragraph = std::make_unique<ParagraphTxt>();
  paragraph->SetText(std::move(text_), std::move(runs_));
  paragraph->SetInlinePlaceholders(std::move(inline_placeholders_),
                                   std::move(obj_replacement_char_indexes_));
  paragraph->SetParagraphStyle(paragraph_style_);
  paragraph->SetFontCollection(font_collection_);
  SetParagraphStyle(paragraph_style_);
  return paragraph;
}

}  // namespace txt
