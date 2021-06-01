// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_RUNTIME_ACE_RUNTIME_WINDOW_MANAGER_H_
#define FLUTTER_RUNTIME_ACE_RUNTIME_WINDOW_MANAGER_H_

#include <map>
#include <mutex>

#include "flutter/fml/macros.h"
#include "flutter/lib/ui/window/window.h"

namespace flutter {

class WindowManager {
public:
    static void AddWindow(int32_t instance_id, std::unique_ptr<Window> window);
    static Window* GetWindow(int32_t instance_id);
    static void RemoveWindow(int32_t instance_id);
private:
    static std::map<int32_t, std::unique_ptr<Window>> window_map_;
    static std::mutex mutex_;
    FML_DISALLOW_COPY_AND_ASSIGN(WindowManager);
};

} // flutter

#endif // FLUTTER_RUNTIME_ACE_RUNTIME_WINDOW_MANAGER_H_
