// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/trace_event.h"

#include <algorithm>
#include <atomic>
#include <utility>

#include "flutter/fml/build_config.h"
#include "flutter/fml/logging.h"

namespace fml {
namespace tracing {

size_t TraceNonce() {
  static std::atomic_size_t gLastItem;
  return ++gLastItem;
}

void TraceBegin(const char* label,
                intptr_t count,
                const char** names,
                const char** values) {
}

void TraceEnd() {
}

void TimelineEvent(const char* label,
                   int64_t timestamp0,
                   int64_t timestamp1_or_async_id,
                   Event_Type type,
                   intptr_t argument_count,
                   const char** argument_names,
                   const char** argument_values) {
}

void TraceTimelineEvent(TraceArg category_group,
                        TraceArg name,
                        TraceIDArg identifier,
                        Event_Type type,
                        const std::vector<const char*>& c_names,
                        const std::vector<std::string>& values) {
  const auto argument_count = std::min(c_names.size(), values.size());

  std::vector<const char*> c_values;
  c_values.resize(argument_count, nullptr);

  for (size_t i = 0; i < argument_count; i++) {
    c_values[i] = values[i].c_str();
  }

  TimelineEvent(name,            // label
                0,               // timestamp0
                identifier,      // timestamp1_or_async_id
                type,            // event type
                argument_count,  // argument_count
                const_cast<const char**>(c_names.data()),  // argument_names
                c_values.data()                            // argument_values
  );
}

void TraceEvent0(TraceArg category_group, TraceArg name) {
  TimelineEvent(name,         // label
                0,            // timestamp0
                0,            // timestamp1_or_async_id
                Event_Begin,  // event type
                0,            // argument_count
                nullptr,      // argument_names
                nullptr       // argument_values
  );
}

void TraceEvent1(TraceArg category_group,
                 TraceArg name,
                 TraceArg arg1_name,
                 TraceArg arg1_val) {
  const char* arg_names[] = {arg1_name};
  const char* arg_values[] = {arg1_val};
  TimelineEvent(name,         // label
                0,            // timestamp0
                0,            // timestamp1_or_async_id
                Event_Begin,  // event type
                1,            // argument_count
                arg_names,    // argument_names
                arg_values    // argument_values
  );
}

void TraceEvent2(TraceArg category_group,
                 TraceArg name,
                 TraceArg arg1_name,
                 TraceArg arg1_val,
                 TraceArg arg2_name,
                 TraceArg arg2_val) {
  const char* arg_names[] = {arg1_name, arg2_name};
  const char* arg_values[] = {arg1_val, arg2_val};
  TimelineEvent(name,         // label
                0,            // timestamp0
                0,            // timestamp1_or_async_id
                Event_Begin,  // event type
                2,            // argument_count
                arg_names,    // argument_names
                arg_values    // argument_values
  );
}

void TraceEventEnd(TraceArg name) {
  TimelineEvent(name,       // label
                0,          // timestamp0
                0,          // timestamp1_or_async_id
                Event_End,  // event type
                0,          // argument_count
                nullptr,    // argument_names
                nullptr     // argument_values
  );
}

void TraceEventAsyncComplete(TraceArg category_group,
                             TraceArg name,
                             TimePoint begin,
                             TimePoint end) {
  auto identifier = TraceNonce();

  if (begin > end) {
    std::swap(begin, end);
  }

  TimelineEvent(name,                                   // label
                begin.ToEpochDelta().ToMicroseconds(),  // timestamp0
                identifier,    // timestamp1_or_async_id
                Event_UnKnow,  // event type
                0,             // argument_count
                nullptr,       // argument_names
                nullptr        // argument_values
  );
  TimelineEvent(name,                                 // label
                end.ToEpochDelta().ToMicroseconds(),  // timestamp0
                identifier,                           // timestamp1_or_async_id
                Event_UnKnow,                         // event type
                0,                                    // argument_count
                nullptr,                              // argument_names
                nullptr                               // argument_values
  );
}

void TraceEventAsyncBegin0(TraceArg category_group,
                           TraceArg name,
                           TraceIDArg id) {
  TimelineEvent(name,          // label
                0,             // timestamp0
                id,            // timestamp1_or_async_id
                Event_UnKnow,  // event type
                0,             // argument_count
                nullptr,       // argument_names
                nullptr        // argument_values
  );
}

void TraceEventAsyncEnd0(TraceArg category_group,
                         TraceArg name,
                         TraceIDArg id) {
  TimelineEvent(name,          // label
                0,             // timestamp0
                id,            // timestamp1_or_async_id
                Event_UnKnow,  // event type
                0,             // argument_count
                nullptr,       // argument_names
                nullptr        // argument_values
  );
}

void TraceEventAsyncBegin1(TraceArg category_group,
                           TraceArg name,
                           TraceIDArg id,
                           TraceArg arg1_name,
                           TraceArg arg1_val) {
  const char* arg_names[] = {arg1_name};
  const char* arg_values[] = {arg1_val};
  TimelineEvent(name,          // label
                0,             // timestamp0
                id,            // timestamp1_or_async_id
                Event_UnKnow,  // event type
                1,             // argument_count
                arg_names,     // argument_names
                arg_values     // argument_values
  );
}

void TraceEventAsyncEnd1(TraceArg category_group,
                         TraceArg name,
                         TraceIDArg id,
                         TraceArg arg1_name,
                         TraceArg arg1_val) {
  const char* arg_names[] = {arg1_name};
  const char* arg_values[] = {arg1_val};
  TimelineEvent(name,          // label
                0,             // timestamp0
                id,            // timestamp1_or_async_id
                Event_UnKnow,  // event type
                1,             // argument_count
                arg_names,     // argument_names
                arg_values     // argument_values
  );
}

void TraceEventInstant0(TraceArg category_group, TraceArg name) {
  TimelineEvent(name,          // label
                0,             // timestamp0
                0,             // timestamp1_or_async_id
                Event_UnKnow,  // event type
                0,             // argument_count
                nullptr,       // argument_names
                nullptr        // argument_values
  );
}

void TraceEventFlowBegin0(TraceArg category_group,
                          TraceArg name,
                          TraceIDArg id) {
  TimelineEvent(name,          // label
                0,             // timestamp0
                id,            // timestamp1_or_async_id
                Event_UnKnow,  // event type
                0,             // argument_count
                nullptr,       // argument_names
                nullptr        // argument_values
  );
}

void TraceEventFlowStep0(TraceArg category_group,
                         TraceArg name,
                         TraceIDArg id) {
  TimelineEvent(name,          // label
                0,             // timestamp0
                id,            // timestamp1_or_async_id
                Event_UnKnow,  // event type
                0,             // argument_count
                nullptr,       // argument_names
                nullptr        // argument_values
  );
}

void TraceEventFlowEnd0(TraceArg category_group, TraceArg name, TraceIDArg id) {
  TimelineEvent(name,          // label
                0,             // timestamp0
                id,            // timestamp1_or_async_id
                Event_UnKnow,  // event type
                0,             // argument_count
                nullptr,       // argument_names
                nullptr        // argument_values
  );
}

}  // namespace tracing
}  // namespace fml
