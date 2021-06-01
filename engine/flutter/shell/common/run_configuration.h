// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_RUN_CONFIGURATION_H_
#define FLUTTER_SHELL_COMMON_RUN_CONFIGURATION_H_

#include <memory>
#include <string>

#include "flutter/common/settings.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"
#include "flutter/fml/task_runner.h"
#include "flutter/fml/unique_fd.h"
#include "flutter/shell/common/rasterizer.h"

namespace flutter {

//------------------------------------------------------------------------------
/// @brief      Specifies all the configuration required by the runtime library
///             to launch the root isolate. This object may be created on any
///             thread but must be given to the |Run| call of the |Engine| on
///             the UI thread. The configuration object is used to specify how
///             the root isolate finds its snapshots, assets, root library and
///             the "main" entrypoint.
///
class RunConfiguration {
 public:
  //----------------------------------------------------------------------------
  /// @brief      Attempts to infer a run configuration from the settings
  ///             object. This tries to create a run configuration with sensible
  ///             defaults for the given Dart VM runtime mode. In JIT mode, this
  ///             will attempt to look for the VM and isolate snapshots in the
  ///             assets directory (must be specified in settings). In AOT mode,
  ///             it will attempt to look for known snapshot symbols in the
  ///             currently currently loaded process. The entrypoint defaults to
  ///             the "main" method in the root library.
  ///
  /// @param[in]  settings   The settings object used to look for the various
  ///                        snapshots and settings. This is usually initialized
  ///                        from command line arguments.
  /// @param[in]  io_worker  An optional IO worker. Resolving and reading the
  ///                        various snapshots may be slow. Providing an IO
  ///                        worker will ensure that realization of these
  ///                        snapshots happens on a worker thread instead of the
  ///                        calling thread. Note that the work done to realize
  ///                        the snapshots may occur after this call returns. If
  ///                        is the embedder's responsibility to make sure the
  ///                        serial worker is kept alive for the lifetime of the
  ///                        shell associated with the engine that this run
  ///                        configuration is given to.
  ///
  /// @return     A run configuration. Depending on the completeness of the
  ///             settings, This object may potentially be invalid.
  ///
  static RunConfiguration InferFromSettings(
      const Settings& settings,
      fml::RefPtr<fml::TaskRunner> io_worker = nullptr);

  RunConfiguration() = default;

  //----------------------------------------------------------------------------
  /// @brief      Run configurations cannot be copied because it may not always
  ///             be possible to copy the underlying isolate snapshots. If
  ///             multiple run configurations share the same underlying
  ///             snapshots, creating a configuration from isolate snapshots
  ///             sharing the same underlying buffers is recommended.
  ///
  /// @param      config  The run configuration to move.
  ///
  RunConfiguration(RunConfiguration&& config);

  //----------------------------------------------------------------------------
  /// @brief      There are no threading restrictions on the destruction of the
  ///             run configuration.
  ///
  ~RunConfiguration();

  //----------------------------------------------------------------------------
  /// @brief      A valid run configuration only guarantees that the engine
  ///             should be able to find the assets and the isolate snapshots
  ///             when it attempts to launch the root isolate. The validity of
  ///             the snapshot cannot be determined yet. That determination can
  ///             only be made when the configuration is used to run the root
  ///             isolate in the engine. However, the engine will always reject
  ///             an invalid run configuration.
  ///
  /// @attention  A valid run configuration does not mean that the root isolate
  ///             will always be launched. It only indicates that the various
  ///             snapshots are isolate snapshots and asset managers are present
  ///             and accounted for. The validity of the snapshots will only be
  ///             checked when the engine attempts to launch the root isolate.
  ///
  /// @return     Returns whether the snapshots and asset manager registrations
  ///             are valid.
  ///
  bool IsValid() const;

  //----------------------------------------------------------------------------
  /// @brief      Updates the main application entrypoint. If this is not set,
  /// the
  ///             "main" method is used as the entrypoint.
  ///
  /// @param[in]  entrypoint  The entrypoint to use.
  void SetEntrypoint(std::string entrypoint);

  //----------------------------------------------------------------------------
  /// @brief      Specifies the main Dart entrypoint and the library to find
  ///             that entrypoint in. By default, this is the "main" method in
  ///             the root library. The root library may be specified by
  ///             entering the empty string as the second argument.
  ///
  /// @see        SetEntrypoint()
  ///
  /// @param[in]  entrypoint  The entrypoint
  /// @param[in]  library     The library
  ///
  void SetEntrypointAndLibrary(std::string entrypoint, std::string library);

  //----------------------------------------------------------------------------
  /// @return     The main Dart entrypoint to be used for the root isolate.
  ///
  const std::string& GetEntrypoint() const;

  //----------------------------------------------------------------------------
  /// @return     The name of the library in which the main entrypoint resides.
  ///             If empty, the root library is used.
  ///
  const std::string& GetEntrypointLibrary() const;

 private:
  std::string entrypoint_ = "main";
  std::string entrypoint_library_ = "";

  FML_DISALLOW_COPY_AND_ASSIGN(RunConfiguration);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_RUN_CONFIGURATION_H_
