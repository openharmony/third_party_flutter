// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.4.30 platform view adapt ohos.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#ifndef FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SHELL_HOLDER_H_
#define FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SHELL_HOLDER_H_

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/fml/unique_fd.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/shell/common/run_configuration.h"
#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/shell/platform/ohos/platform_view_ohos.h"

namespace flutter {

class OhosShellHolder {
 public:
  OhosShellHolder(flutter::Settings settings,
                     bool is_background_view);

  ~OhosShellHolder();

  bool IsValid() const;

  void Launch(RunConfiguration configuration);

  const flutter::Settings& GetSettings() const;

  fml::WeakPtr<PlatformViewOhos> GetPlatformView();

  Rasterizer::Screenshot Screenshot(Rasterizer::ScreenshotType type,
                                    bool base64_encode);

  void UpdateAssetManager(fml::RefPtr<flutter::AssetManager> asset_manager);

 private:
  const flutter::Settings settings_;
  fml::WeakPtr<PlatformViewOhos> platform_view_;
  ThreadHost thread_host_;
  std::unique_ptr<Shell> shell_;
  bool is_valid_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(OhosShellHolder);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_OHOS_OHOS_SHELL_HOLDER_H_
