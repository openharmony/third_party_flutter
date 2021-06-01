// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_ACE_ENGINE_H_
#define SHELL_COMMON_ACE_ENGINE_H_

#include <memory>
#include <string>

#include "flutter/assets/asset_manager.h"
#include "flutter/common/task_runners.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/lib/ui/semantics/semantics_node.h"
#include "flutter/lib/ui/text/font_collection.h"
#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/runtime/runtime_controller.h"
#include "flutter/runtime/runtime_delegate.h"
#include "flutter/shell/common/animator.h"
#include "flutter/shell/common/rasterizer.h"
#include "flutter/shell/common/run_configuration.h"
#include "flutter/shell/common/shell_io_manager.h"
#include "third_party/skia/include/core/SkPicture.h"

namespace flutter {

//------------------------------------------------------------------------------
/// The engine is a component owned by the shell that resides on the UI task
/// runner and is responsible for managing the needs of the root isolate and its
/// runtime. The engine can only be created, accessed and collected on the UI
/// task runner. Each shell owns exactly one instance of the engine.
///
/// The root isolate of Flutter application gets "window" bindings. Using these
/// bindings, the application can schedule frames, post layer-trees for
/// rendering, ask to decompress images and upload them to the GPU, etc..
/// Non-root isolates of the VM do not get any of these capabilities and are run
/// in a VM managed thread pool (so if they did have "window", the threading
/// guarantees needed for engine operation would be violated).
///
/// The engine is responsible for the entire life-cycle of the root isolate.
/// When the engine is collected, its owner assumes that the root isolate has
/// been shutdown and appropriate resources collected. While each engine
/// instance can only manage a single instance of a root isolate, it may restart
/// that isolate on request. This is how the cold-restart development scenario
/// is supported.
///
/// When the engine instance is initially created, the root isolate is created
/// but it is not in the |DartIsolate::Phase::Running| phase yet. It only moves
/// into that phase when a successful call to `Engine::Run` is made.
///
/// @see      `Shell`
///
/// @note     This name of this class is perhaps a bit unfortunate and has
///           sometimes been the cause of confusion. For a class named "Engine"
///           in the Flutter "Engine" repository, its responsibilities are
///           decidedly unremarkable. But, it does happen to be the primary
///           entry-point used by components higher up in the Flutter tech stack
///           (usually in Dart code) to peer into the lower level functionality.
///           Besides, the authors haven't been able to come up with a more apt
///           name and it does happen to be one of the older classes in the
///           repository.
///
class Engine final : public RuntimeDelegate {
 public:
  // ACE PC preivew
  using IdleCallback = std::function<void(int64_t)>;

  //----------------------------------------------------------------------------
  /// @brief      Indicates the result of the call to `Engine::Run`.
  ///
  enum class RunStatus {
    //--------------------------------------------------------------------------
    /// The call to |Engine::Run| was successful and the root isolate is in the
    /// `DartIsolate::Phase::Running` phase with its entry-point invocation
    /// already pending in the task queue.
    ///
    Success,

    //--------------------------------------------------------------------------
    /// The engine can only manage a single instance of a root isolate. If a
    /// previous call to run the root isolate was successful, subsequent calls
    /// to run the isolate (even if the new run configuration is different) will
    /// be rejected.
    ///
    /// It is up to the caller to decide to re-purpose the running isolate,
    /// terminate it, or use another shell to host the new isolate. This is
    /// mostly used by embedders which have a fire-and-forget strategy to root
    /// isolate launch. For example, the application may try to "launch" and
    /// isolate when the embedders launches or resumes from a paused state. That
    /// the isolate is running is not necessarily a failure condition for them.
    /// But from the engine's perspective, the run configuration was rejected.
    ///
    FailureAlreadyRunning,

    //--------------------------------------------------------------------------
    /// Used to indicate to the embedder that a root isolate was not already
    /// running but the run configuration was not valid and root isolate could
    /// not be moved into the `DartIsolate::Phase::Running` phase.
    ///
    /// The caller must attempt the run call again with a valid configuration.
    /// The set of all failure modes is massive and can originate from a variety
    /// of sub-components. The engine will attempt to log the same when
    /// possible. With the aid of logs, the common causes of failure are:
    ///
    /// * AOT assets give to JIT/DBC mode VM's and vice-versa.
    /// * The assets could not be found in the asset manager. Callers must make
    ///   sure their run configuration asset managers have been correctly setup.
    /// * The assets themselves were corrupt or invalid. Callers must make sure
    ///   their asset delivery mechanisms are sound.
    /// * The application entry-point or the root library of the entry-point
    ///   specified in the run configuration was invalid. Callers must make sure
    ///   that the entry-point is present in the application. If the name of the
    ///   entrypoint is not "main" in the root library, callers must also ensure
    ///   that the snapshotting process has not tree-shaken away this
    ///   entrypoint. This requires the decoration of the entrypoint with the
    ///   `@pragma('vm:entry-point')` directive. This problem will manifest in
    ///   AOT mode operation of the Dart VM.
    ///
    Failure,
  };

  //----------------------------------------------------------------------------
  /// @brief      While the engine operates entirely on the UI task runner, it
  ///             needs the capabilities of the other components to fulfill the
  ///             requirements of the root isolate. The shell is the only class
  ///             that implements this interface as no other component has
  ///             access to all components in a thread safe manner. The engine
  ///             delegates these tasks to the shell via this interface.
  ///
  class Delegate {
   public:
    //--------------------------------------------------------------------------
    /// @brief      When the Flutter application has a message to send to the
    ///             underlying platform, the message needs to be forwarded to
    ///             the platform on the the appropriate thread (via the platform
    ///             task runner). The engine delegates this task to the shell
    ///             via this method.
    ///
    /// @see        `PlatformView::HandlePlatformMessage`
    ///
    /// @param[in]  message  The message from the Flutter application to send to
    ///                      the underlying platform.
    ///
    virtual void OnEngineHandlePlatformMessage(
        fml::RefPtr<PlatformMessage> message) = 0;
    //--------------------------------------------------------------------------
    /// @brief      Notifies the delegate that the root isolate of the
    ///             application is about to be discarded and a new isolate with
    ///             the same runtime started in its place. This should only
    ///             happen in the Flutter "debug" runtime mode in the
    ///             cold-restart scenario. The embedder may need to reset native
    ///             resource in response to the restart.
    ///
    /// @see        `PlatformView::OnPreEngineRestart`
    ///
    virtual void OnPreEngineRestart() = 0;
    //--------------------------------------------------------------------------
    /// @brief      Notifies the shell that the application has an opinion about
    ///             whether its frame timings need to be reported backed to it.
    ///             Due to the asynchronous nature of rendering in Flutter, it
    ///             is not possible for the application to determine the total
    ///             time it took to render a specific frame. While the
    ///             layer-tree is constructed on the UI thread, it needs to be
    ///             rendering on the GPU thread. Dart code cannot execute on
    ///             this thread. So any instrumentation about the frame times
    ///             gathered on this thread needs to be aggregated and sent back
    ///             to the UI thread for processing in Dart.
    ///
    ///             When the application indicates that frame times need to be
    ///             reported, it collects this information till a specified
    ///             number of data points are gathered. Then this information is
    ///             sent back to Dart code via `Engine::ReportTimings`.
    ///
    ///             This option is engine counterpart of the
    ///             `Window._setNeedsReportTimings` in `window.dart`.
    ///
    /// @param[in]  needs_reporting  If reporting information should be
    ///                              collected and send back to Dart.
    ///
    virtual void SetNeedsReportTimings(bool needs_reporting) = 0;
  };

  //----------------------------------------------------------------------------
  /// @brief      Creates an instance of the engine. This is done by the Shell
  ///             on the the UI task runner.
  ///
  /// @param      delegate           The object used by the engine to perform
  ///                                tasks that require access to components
  ///                                that cannot be safely accessed by the
  ///                                engine. This is the shell.
  /// @param      vm                 An instance of the running Dart VM.
  /// @param[in]  isolate_snapshot   The snapshot used to create the root
  ///                                isolate. Even though the isolate is not
  ///                                `DartIsolate::Phase::Running` phase, it is
  ///                                created when the engine is created. This
  ///                                requires access to the isolate snapshot
  ///                                upfront.
  /// @param[in]  shared_snapshot    The portion of the isolate snapshot shared
  ///                                among multiple isolates.
  //  TODO(chinmaygarde): This is probably redundant now that the IO manager is
  //  it's own object.
  /// @param[in]  task_runners       The task runners used by the shell that
  ///                                hosts this engine.
  /// @param[in]  settings           The settings used to initialize the shell
  ///                                and the engine.
  /// @param[in]  animator           The animator used to schedule frames.
  //  TODO(chinmaygarde): Move this to `Engine::Delegate`
  /// @param[in]  snapshot_delegate  The delegate used to fulfill requests to
  ///                                snapshot a specified scene. The engine
  ///                                cannot snapshot a scene on the UI thread
  ///                                directly because the scene (described via
  ///                                an `SkPicture`) may reference resources on
  ///                                the GPU and there is no GPU context current
  ///                                on the UI thread. The delegate is a
  ///                                component that has access to all the
  ///                                requisite GPU resources.
  /// @param[in]  io_manager         The IO manager used by this root isolate to
  ///                                schedule tasks that manage resources on the
  ///                                GPU.
  ///
  Engine(Delegate& delegate,
         TaskRunners task_runners,
         Settings settings,
         std::unique_ptr<Animator> animator,
         fml::WeakPtr<IOManager> io_manager);

  //----------------------------------------------------------------------------
  /// @brief      Destroys the engine engine. Called by the shell on the UI task
  ///             runner. The running root isolate is terminated and will no
  ///             longer access the task runner after this call returns. This
  ///             allows the embedder to tear down the thread immediately if
  ///             needed.
  ///
  ~Engine() override;

  //----------------------------------------------------------------------------
  /// @brief      Gets the refresh rate in frames per second of the vsync waiter
  ///             used by the animator managed by this engine. This information
  ///             is purely advisory and is not used by any component. It is
  ///             only used by the tooling to visualize frame performance.
  ///
  /// @attention  The display refresh rate is useless for frame scheduling
  ///             because it can vary and more accurate frame specific
  ///             information is given to the engine by the vsync waiter
  ///             already. However, this call is used by the tooling to ask very
  ///             high level questions about display refresh rate. For example,
  ///             "Is the display 60 or 120Hz?". This information is quite
  ///             unreliable (not available immediately on launch on some
  ///             platforms), variable and advisory. It must not be used by any
  ///             component that claims to use it to perform accurate frame
  ///             scheduling.
  ///
  /// @return     The display refresh rate in frames per second. This may change
  ///             from frame to frame, throughout the lifecycle of the
  ///             application, and, may not be available immediately upon
  ///             application launch.
  ///
  float GetDisplayRefreshRate() const;

  //----------------------------------------------------------------------------
  /// @return     The pointer to this instance of the engine. The engine may
  ///             only be accessed safely on the UI task runner.
  ///
  fml::WeakPtr<Engine> GetWeakPtr() const;

  //----------------------------------------------------------------------------
  /// @brief      Moves the root isolate to the `DartIsolate::Phase::Running`
  ///             phase on a successful call to this method.
  ///
  ///             The isolate itself is created when the engine is created, but
  ///             it is not yet in the running phase. This is done to amortize
  ///             initial time taken to launch the root isolate. The isolate
  ///             snapshots used to run the isolate can be fetched on another
  ///             thread while the engine itself is launched on the UI task
  ///             runner.
  ///
  ///             Repeated calls to this method after a successful run will be
  ///             rejected even if the run configuration is valid (with the
  ///             appropriate error returned).
  ///
  /// @param[in]  configuration  The configuration used to run the root isolate.
  ///                            The configuration must be valid.
  ///
  /// @return     The result of the call to run the root isolate.
  ///
  FML_WARN_UNUSED_RESULT
  RunStatus Run(RunConfiguration configuration);

  //----------------------------------------------------------------------------
  /// @brief      Tears down an existing root isolate, reuses the components of
  ///             that isolate and attempts to launch a new isolate using the
  ///             given the run configuration. This is only used in the
  ///             "debug" Flutter runtime mode in the cold-restart scenario.
  ///
  /// @attention  This operation must be performed with care as even a
  ///             non-successful restart will still tear down any existing root
  ///             isolate. In such cases, the engine and its shell must be
  ///             discarded.
  ///
  /// @param[in]  configuration  The configuration used to launch the new
  ///                            isolate.
  ///
  /// @return     Whether the restart was successful. If not, the engine and its
  ///             shell must be discarded.
  ///
  FML_WARN_UNUSED_RESULT
  bool Restart(RunConfiguration configuration);

  //----------------------------------------------------------------------------
  /// @brief      Updates the asset manager referenced by the root isolate of a
  ///             Flutter application. This happens implicitly in the call to
  ///             `Engine::Run` and `Engine::Restart` as the asset manager is
  ///             referenced from the run configuration provided to those calls.
  ///             In addition to the the `Engine::Run` and `Engine::Restart`
  ///             calls, the tooling may need to update the assets available to
  ///             the application as the user adds them to their project. For
  ///             example, these assets may be referenced by code that is newly
  ///             patched in after a hot-reload. Neither the shell or the
  ///             isolate in relaunched in such cases. The tooling usually
  ///             patches in the new assets in a temporary location and updates
  ///             the asset manager to point to that location.
  ///
  /// @param[in]  asset_manager  The new asset manager to use for the running
  ///                            root isolate.
  ///
  /// @return     If the asset manager was successfully replaced. This may fail
  ///             if the new asset manager is invalid.
  ///
  bool UpdateAssetManager(std::shared_ptr<AssetManager> asset_manager);

  //----------------------------------------------------------------------------
  /// @brief      Notifies the engine that it is time to begin working on a new
  ///             frame previously scheduled via a call to
  ///             `Engine::ScheduleFrame`. This call originates in the animator.
  ///
  ///             The frame time given as the argument indicates the point at
  ///             which the current frame interval began. It is very slightly
  ///             (because of scheduling overhead) in the past. If a new layer
  ///             tree is not produced and given to the GPU task runner within
  ///             one frame interval from this point, the Flutter application
  ///             will jank.
  ///
  ///             If an root isolate is running, this method calls the
  ///             `::_beginFrame` method in `hooks.dart`. If a root isolate is
  ///             not running, this call does nothing.
  ///
  ///             This method encapsulates the entire UI thread frame workload.
  ///             The following (mis)behavior in the functioning of the method
  ///             will cause the jank in the Flutter application:
  ///             * The time taken by this method to create a layer-tree exceeds
  ///               on frame interval (for example, 16.66 ms on a 60Hz display).
  ///             * The time take by this method to generate a new layer-tree
  ///               causes the current layer-tree pipeline depth to change. To
  ///               illustrate this point, note that maximum pipeline depth used
  ///               by layer tree in the engine is 2. If both the UI and GPU
  ///               task runner tasks finish within one frame interval, the
  ///               pipeline depth is one. If the UI thread happens to be
  ///               working on a frame when the GPU thread is still not done
  ///               with the previous frame, the pipeline depth is 2. When the
  ///               pipeline depth changes from 1 to 2, animations and UI
  ///               interactions that cause the generation of the new layer tree
  ///               appropriate for (frame_time + one frame interval) will
  ///               actually end up at (frame_time + two frame intervals). This
  ///               is not what code running on the UI thread expected would
  ///               happen. This causes perceptible jank.
  ///
  /// @param[in]  frame_time  The point at which the current frame interval
  ///                         began. May be used by animation interpolators,
  ///                         physics simulations, etc..
  ///
  void BeginFrame(fml::TimePoint frame_time);

  void ReportTimings(std::vector<int64_t> timings);
  //----------------------------------------------------------------------------
  /// @brief      Notifies the engine that the UI task runner is not expected to
  ///             undertake a new frame workload till a specified timepoint. The
  ///             timepoint is measured in microseconds against the system's
  ///             monotonic clock. It is recommended that the clock be accessed
  ///             via `Dart_TimelineGetMicros` from `dart_api.h` for
  ///             consistency. In reality, the clocks used by Dart, FML and
  ///             std::steady_clock are all the same and the timepoints can be
  ///             converted from on clock type to another.
  ///
  ///             The Dart VM uses this notification to schedule book-keeping
  ///             tasks that may include a garbage collection. In this way, it
  ///             is less likely for the VM to perform such (potentially long
  ///             running) tasks in the middle of a frame workload.
  ///
  ///             This notification is advisory. That is, not providing this
  ///             notification does not mean garbage collection is postponed
  ///             till this call is made. If this notification is not provided,
  ///             garbage collection will happen based on the usual heuristics
  ///             used by the Dart VM.
  ///
  ///             Currently, this idle notification is delivered to the engine
  ///             at two points. Once, the deadline is calculated based on how
  ///             much time in the current frame interval is left on the UI task
  ///             runner. Since the next frame workload cannot begin till at
  ///             least the next callback from the vsync waiter, this period may
  ///             be used to used as a "small" idle notification. On the other
  ///             hand, if no more frames are scheduled, a large (but arbitrary)
  ///             idle notification deadline is chosen for a "big" idle
  ///             notification. Again, this notification does not guarantee
  ///             collection, just gives the Dart VM more hints about opportune
  ///             moments to perform collections.
  ///
  //  TODO(chinmaygarde): This should just use fml::TimePoint instead of having
  //  to remember that the unit is microseconds (which is no used anywhere else
  //  in the engine).
  ///
  /// @param[in]  deadline  The deadline as a timepoint in microseconds measured
  ///                       against the system monotonic clock. Use
  ///                       `Dart_TimelineGetMicros()`, for consistency.
  ///
  void NotifyIdle(int64_t deadline);

  //----------------------------------------------------------------------------
  /// @brief      Indicates to the Flutter application that it has obtained a
  ///             rendering surface. This is a good opportunity for the engine
  ///             to start servicing any outstanding frame requests from the
  ///             Flutter applications. Flutter application that have no
  ///             rendering concerns may never get a rendering surface. In such
  ///             cases, while their root isolate can perform as normal, any
  ///             frame requests made by them will never be serviced and layer
  ///             trees produced outside of frame workloads will be dropped.
  ///
  ///             Very close to when this call is made, the application can
  ///             expect the updated viewport metrics. Rendering only begins
  ///             when the Flutter application gets an output surface and a
  ///             valid set of viewport metrics.
  ///
  /// @see        `OnOutputSurfaceDestroyed`
  ///
  void OnOutputSurfaceCreated();

  //----------------------------------------------------------------------------
  /// @brief      Indicates to the Flutter application that a previously
  ///             acquired rendering surface has been lost. Further frame
  ///             requests will no longer be serviced and any layer tree
  ///             submitted for rendering will be dropped. If/when a new surface
  ///             is acquired, a new layer tree must be generated.
  ///
  /// @see        `OnOutputSurfaceCreated`
  ///
  void OnOutputSurfaceDestroyed();

  //----------------------------------------------------------------------------
  /// @brief      Ace PC preivew
  ///
  void SetIdleNotificationCallback(const IdleCallback& idleCallback);

  //----------------------------------------------------------------------------
  /// @brief      Updates the viewport metrics for the currently running Flutter
  ///             application. The viewport metrics detail the size of the
  ///             rendering viewport in texels as well as edge insets if
  ///             present.
  ///
  /// @see        `ViewportMetrics`
  ///
  /// @param[in]  metrics  The metrics
  ///
  void SetViewportMetrics(const ViewportMetrics& metrics);

  //----------------------------------------------------------------------------
  /// @brief      Notifies the engine that the embedder has sent it a message.
  ///             This call originates in the platform view and has been
  ///             forwarded to the engine on the UI task runner here.
  ///
  /// @param[in]  message  The message sent from the embedder to the Dart
  ///                      application.
  ///
  void DispatchPlatformMessage(fml::RefPtr<PlatformMessage> message);

  //----------------------------------------------------------------------------
  /// @brief      Notifies the engine that the embedder has sent it a pointer
  ///             data packet. A pointer data packet may contain multiple
  ///             input events. This call originates in the platform view and
  ///             the shell has forwarded the same to the engine on the UI task
  ///             runner here.
  ///
  /// @param[in]  packet         The pointer data packet containing multiple
  ///                            input events.
  /// @param[in]  trace_flow_id  The trace flow identifier associated with the
  ///                            pointer data packet. The engine uses this trace
  ///                            identifier to connect trace flows in the
  ///                            timeline from the input event event to the
  ///                            frames generated due to those input events.
  ///                            These flows are tagged as "PointerEvent" in the
  ///                            timeline and allow grouping frames and input
  ///                            events into logical chunks.
  ///
  void DispatchPointerDataPacket(const PointerDataPacket& packet,
                                 uint64_t trace_flow_id);
  // |RuntimeDelegate|
  void ScheduleFrame(bool regenerate_layer_tree = true) override;

  // |RuntimeDelegate|
  FontCollection& GetFontCollection() override;

 private:
  Engine::Delegate& delegate_;
  const Settings settings_;
  std::unique_ptr<Animator> animator_;
  std::unique_ptr<RuntimeController> runtime_controller_;
  std::string initial_route_;
  ViewportMetrics viewport_metrics_;
  std::shared_ptr<AssetManager> asset_manager_;
  bool activity_running_;
  bool have_surface_;
  std::once_flag font_flag_;
  std::unique_ptr<FontCollection> font_collection_;
  fml::WeakPtrFactory<Engine> weak_factory_;

  // |RuntimeDelegate|
  std::string DefaultRouteName() override;

  // |RuntimeDelegate|
  void Render(std::unique_ptr<flutter::LayerTree> layer_tree) override;

  void HandlePlatformMessage(fml::RefPtr<PlatformMessage> message) override;

  void SetNeedsReportTimings(bool value) override;

  void StopAnimator();

  void StartAnimatorIfPossible();

  bool HandleLifecyclePlatformMessage(PlatformMessage* message);

  bool HandleNavigationPlatformMessage(fml::RefPtr<PlatformMessage> message);

  bool HandleLocalizationPlatformMessage(PlatformMessage* message);

  void HandleSettingsPlatformMessage(PlatformMessage* message);

  void HandleAssetPlatformMessage(fml::RefPtr<PlatformMessage> message);

  bool GetAssetAsBuffer(const std::string& name, std::vector<uint8_t>* data);

  RunStatus PrepareAndLaunchIsolate(RunConfiguration configuration);

  friend class testing::ShellTest;

  FML_DISALLOW_COPY_AND_ASSIGN(Engine);
};

}  // namespace flutter

#endif  // SHELL_COMMON_ENGINE_H_
