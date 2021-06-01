// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_ACE_SHELL_H_
#define SHELL_COMMON_ACE_SHELL_H_

#include <functional>
#include <string_view>
#include <unordered_map>

#include "flutter/common/settings.h"
#include "flutter/common/task_runners.h"
#include "flutter/flow/texture.h"
#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/fml/memory/thread_checker.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/fml/status.h"
#include "flutter/fml/synchronization/thread_annotations.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/thread.h"
#include "flutter/lib/ui/semantics/semantics_node.h"
#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/shell/common/animator.h"
#include "flutter/shell/common/engine.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/common/shell_io_manager.h"
#include "flutter/shell/common/surface.h"

namespace flutter {

class Shell final : public PlatformView::Delegate,
                    public Animator::Delegate,
                    public Engine::Delegate,
                    public Rasterizer::Delegate {
 public:
  template <class T>
  using CreateCallback = std::function<std::unique_ptr<T>(Shell&)>;

  static std::unique_ptr<Shell> Create(
      TaskRunners task_runners,
      Settings settings,
      CreateCallback<PlatformView> on_create_platform_view,
      CreateCallback<Rasterizer> on_create_rasterizer);

  //----------------------------------------------------------------------------
  /// @brief      Destroys the shell. This is a synchronous operation and
  ///             synchronous barrier blocks are introduced on the various
  ///             threads to ensure shutdown of all shell sub-components before
  ///             this method returns.
  ///
  ~Shell();

  //----------------------------------------------------------------------------
  /// @brief      Starts an isolate for the given RunConfiguration.
  ///
  void RunEngine(RunConfiguration run_configuration);

  //----------------------------------------------------------------------------
  /// @brief      Starts an isolate for the given RunConfiguration. The
  ///             result_callback will be called with the status of the
  ///             operation.
  ///
  void RunEngine(RunConfiguration run_configuration,
                 std::function<void(Engine::RunStatus)> result_callback);

  //------------------------------------------------------------------------------
  /// @return     The settings used to launch this shell.
  ///
  const Settings& GetSettings() const;

  //------------------------------------------------------------------------------
  /// @brief      If callers wish to interact directly with any shell
  ///             subcomponents, they must (on the platform thread) obtain a
  ///             task runner that the component is designed to run on and a
  ///             weak pointer to that component. They may then post a task to
  ///             that task runner, do the validity check on that task runner
  ///             before performing any operation on that component. This
  ///             accessor allows callers to access the task runners for this
  ///             shell.
  ///
  /// @return     The task runners current in use by the shell.
  ///
  const TaskRunners& GetTaskRunners() const;

  //----------------------------------------------------------------------------
  /// @brief      Rasterizers may only be accessed on the GPU task runner.
  ///
  /// @return     A weak pointer to the rasterizer.
  ///
  fml::WeakPtr<Rasterizer> GetRasterizer();

// TODO(dnfield): Remove this when either Topaz is up to date or flutter_runner
// is built out of this repo.
#ifdef OS_FUCHSIA
  //------------------------------------------------------------------------------
  /// @brief      Engines may only be accessed on the UI thread. This method is
  ///             deprecated, and implementers should instead use other API
  ///             available on the Shell or the PlatformView.
  ///
  /// @return     A weak pointer to the engine.
  ///
  fml::WeakPtr<Engine> GetEngine();
#endif  // OS_FUCHSIA

  //----------------------------------------------------------------------------
  /// @brief      Platform views may only be accessed on the platform task
  ///             runner.
  ///
  /// @return     A weak pointer to the platform view.
  ///
  fml::WeakPtr<PlatformView> GetPlatformView();

  // Embedders should call this under low memory conditions to free up
  // internal caches used.
  //
  // This method posts a task to the GPU threads to signal the Rasterizer to
  // free resources.

  //----------------------------------------------------------------------------
  /// @brief      Used by embedders to notify that there is a low memory
  ///             warning. The shell will attempt to purge caches. Current, only
  ///             the rasterizer cache is purged.
  void NotifyLowMemoryWarning() const;

  //----------------------------------------------------------------------------
  /// @brief      Used by embedders to check if all shell subcomponents are
  ///             initialized. It is the embedder's responsibility to make this
  ///             call before accessing any other shell method. A shell that is
  ///             not setup must be discarded and another one created with
  ///             updated settings.
  ///
  /// @return     Returns if the shell has been setup. Once set up, this does
  ///             not change for the life-cycle of the shell.
  ///
  bool IsSetup() const;

  //----------------------------------------------------------------------------
  /// @brief      Captures a screenshot and optionally Base64 encodes the data
  ///             of the last layer tree rendered by the rasterizer in this
  ///             shell.
  ///
  /// @param[in]  type           The type of screenshot to capture.
  /// @param[in]  base64_encode  If the screenshot data should be base64
  ///                            encoded.
  ///
  /// @return     The screenshot result.
  ///
  Rasterizer::Screenshot Screenshot(Rasterizer::ScreenshotType type,
                                    bool base64_encode);

  //----------------------------------------------------------------------------
  /// @brief   Pauses the calling thread until the first frame is presented.
  ///
  /// @return  'kOk' when the first frame has been presented before the timeout
  ///          successfully, 'kFailedPrecondition' if called from the GPU or UI
  ///          thread, 'kDeadlineExceeded' if there is a timeout.
  ///
  fml::Status WaitForFirstFrame(fml::TimeDelta timeout);

 private:
  const TaskRunners task_runners_;
  const Settings settings_;
  std::unique_ptr<PlatformView> platform_view_;  // on platform task runner
  std::unique_ptr<Engine> engine_;               // on UI task runner
  std::unique_ptr<Rasterizer> rasterizer_;       // on GPU task runner
  std::unique_ptr<ShellIOManager> io_manager_;   // on IO task runner

  fml::WeakPtr<Engine> weak_engine_;          // to be shared across threads
  fml::WeakPtr<Rasterizer> weak_rasterizer_;  // to be shared across threads
  fml::WeakPtr<PlatformView>
      weak_platform_view_;  // to be shared across threads

  bool is_setup_ = false;
  uint64_t next_pointer_flow_id_ = 0;

  bool first_frame_rasterized_ = false;
  std::atomic<bool> waiting_for_first_frame_ = true;
  std::mutex waiting_for_first_frame_mutex_;
  std::condition_variable waiting_for_first_frame_condition_;

  // Written in the UI thread and read from the GPU thread. Hence make it
  // atomic.
  std::atomic<bool> needs_report_timings_{false};

  // Whether there's a task scheduled to report the timings to Dart through
  // ui.Window.onReportTimings.
  bool frame_timings_report_scheduled_ = false;

  // Vector of FrameTiming::kCount * n timestamps for n frames whose timings
  // have not been reported yet. Vector of ints instead of FrameTiming is stored
  // here for easier conversions to Dart objects.
  std::vector<int64_t> unreported_timings_;

  // How many frames have been timed since last report.
  size_t UnreportedFramesCount() const;

  Shell(TaskRunners task_runners, Settings settings);

  static std::unique_ptr<Shell> CreateShellOnPlatformThread(
      TaskRunners task_runners,
      Settings settings,
      Shell::CreateCallback<PlatformView> on_create_platform_view,
      Shell::CreateCallback<Rasterizer> on_create_rasterizer);

  bool Setup(std::unique_ptr<PlatformView> platform_view,
             std::unique_ptr<Engine> engine,
             std::unique_ptr<Rasterizer> rasterizer,
             std::unique_ptr<ShellIOManager> io_manager);

  void ReportTimings();

  // |PlatformView::Delegate|
  void OnPlatformViewCreated(std::unique_ptr<Surface> surface) override;

  // |PlatformView::Delegate|
  void OnPlatformViewDestroyed() override;

  // |PlatformView::Delegate|
  void OnSetIdleNotificationCallback(
      const Engine::IdleCallback& idleCallback) override;

  // |PlatformView::Delegate|
  void OnPlatformViewSetViewportMetrics(
      const ViewportMetrics& metrics) override;

  // |PlatformView::Delegate|
  void OnPlatformViewDispatchPlatformMessage(
      fml::RefPtr<PlatformMessage> message) override;

  // |PlatformView::Delegate|
  void OnPlatformViewDispatchPointerDataPacket(
      std::unique_ptr<PointerDataPacket> packet) override;

  // |PlatformView::Delegate|
  void OnPlatformViewRegisterTexture(
      std::shared_ptr<flutter::Texture> texture) override;

  // |PlatformView::Delegate|
  void OnPlatformViewUnregisterTexture(int64_t texture_id) override;

  // |PlatformView::Delegate|
  void OnPlatformViewMarkTextureFrameAvailable(int64_t texture_id) override;

  // |PlatformView::Delegate|
  void OnPlatformViewSetNextFrameCallback(fml::closure closure) override;

  // |Animator::Delegate|
  void OnAnimatorBeginFrame(fml::TimePoint frame_time) override;

  // |Animator::Delegate|
  void OnAnimatorNotifyIdle(int64_t deadline) override;

  // |Animator::Delegate|
  void OnAnimatorDraw(
      fml::RefPtr<Pipeline<flutter::LayerTree>> pipeline) override;

  // |Animator::Delegate|
  void OnAnimatorDrawLastLayerTree() override;

  // |Engine::Delegate|
  void OnEngineHandlePlatformMessage(
      fml::RefPtr<PlatformMessage> message) override;

  void HandleEngineSkiaMessage(fml::RefPtr<PlatformMessage> message);

  // |Engine::Delegate|
  void OnPreEngineRestart() override;

  // |Engine::Delegate|
  void SetNeedsReportTimings(bool value) override;

  // |Rasterizer::Delegate|
  void OnFrameRasterized(const FrameTiming&) override;

  fml::WeakPtrFactory<Shell> weak_factory_;

  friend class testing::ShellTest;

  FML_DISALLOW_COPY_AND_ASSIGN(Shell);
};

}

#endif // SHELL_COMMON_ACE_SHELL_H_
