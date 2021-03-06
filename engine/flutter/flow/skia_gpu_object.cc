// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/skia_gpu_object.h"

#include "flutter/fml/message_loop.h"

namespace flutter {

SkiaUnrefQueue::SkiaUnrefQueue(fml::RefPtr<fml::TaskRunner> task_runner,
                               fml::TimeDelta delay)
    : task_runner_(std::move(task_runner)),
      drain_delay_(delay),
      drain_pending_(false) {}

SkiaUnrefQueue::~SkiaUnrefQueue() {
  FML_DCHECK(objects_.empty());
}

void SkiaUnrefQueue::Unref(SkRefCnt* object) {
  std::scoped_lock lock(mutex_);
  objects_.push_back(object);
  if (invalid_) {
    FML_LOG(ERROR) << "Unref called after queue invalid!";
    return;
  }
  if (!drain_pending_) {
    drain_pending_ = true;
    task_runner_->PostDelayedTask(
        [strong = fml::Ref(this)]() { strong->Drain(); }, drain_delay_);
  }
}

void SkiaUnrefQueue::Drain(bool finish) {
  std::deque<SkRefCnt*> skia_objects;
  {
    std::scoped_lock lock(mutex_);
    objects_.swap(skia_objects);
    drain_pending_ = false;
    if (finish) {
      invalid_ = finish;
    }
  }

  for (SkRefCnt* skia_object : skia_objects) {
    skia_object->unref();
  }
}

}  // namespace flutter
