// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_EMBEDDER_H_
#define FLUTTER_EMBEDDER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <memory>
#if defined(__cplusplus)
#include <functional>
extern "C" {
#endif

#ifndef FLUTTER_EXPORT
#define FLUTTER_EXPORT
#endif  // FLUTTER_EXPORT

#ifdef FLUTTER_API_SYMBOL_PREFIX
#define FLUTTER_EMBEDDING_CONCAT(a, b) a##b
#define FLUTTER_EMBEDDING_ADD_PREFIX(symbol, prefix) \
  FLUTTER_EMBEDDING_CONCAT(prefix, symbol)
#define FLUTTER_API_SYMBOL(symbol) \
  FLUTTER_EMBEDDING_ADD_PREFIX(symbol, FLUTTER_API_SYMBOL_PREFIX)
#else
#define FLUTTER_API_SYMBOL(symbol) symbol
#endif

#define FLUTTER_ENGINE_VERSION 1

typedef enum {
  kSuccess = 0,
  kInvalidLibraryVersion,
  kInvalidArguments,
  kInternalInconsistency,
} FlutterEngineResult;

typedef enum {
  kOpenGL,
  kSoftware,
} FlutterRendererType;

typedef enum {
  // Text has unknown text direction.
  kFlutterTextDirectionUnknown = 0,
  // Text is read from right to left.
  kFlutterTextDirectionRTL = 1,
  // Text is read from left to right.
  kFlutterTextDirectionLTR = 2,
} FlutterTextDirection;

typedef struct _FlutterEngine* FLUTTER_API_SYMBOL(FlutterEngine);

typedef struct {
  // horizontal scale factor
  double scaleX;
  // horizontal skew factor
  double skewX;
  // horizontal translation
  double transX;
  // vertical skew factor
  double skewY;
  // vertical scale factor
  double scaleY;
  // vertical translation
  double transY;
  // input x-axis perspective factor
  double pers0;
  // input y-axis perspective factor
  double pers1;
  // perspective scale factor
  double pers2;
} FlutterTransformation;

typedef void (*VoidCallback)(void* /* user data */);

typedef enum {
  // Specifies an OpenGL texture target type. Textures are specified using
  // the FlutterOpenGLTexture struct.
  kFlutterOpenGLTargetTypeTexture,
  // Specifies an OpenGL frame-buffer target type. Framebuffers are specified
  // using the FlutterOpenGLFramebuffer struct.
  kFlutterOpenGLTargetTypeFramebuffer,
} FlutterOpenGLTargetType;

typedef struct {
  // Target texture of the active texture unit (example GL_TEXTURE_2D).
  uint32_t target;
  // The name of the texture.
  uint32_t name;
  // The texture format (example GL_RGBA8).
  uint32_t format;
  // User data to be returned on the invocation of the destruction callback.
  void* user_data;
  // Callback invoked (on an engine managed thread) that asks the embedder to
  // collect the texture.
  VoidCallback destruction_callback;
} FlutterOpenGLTexture;

typedef struct {
  // The target of the color attachment of the frame-buffer. For example,
  // GL_TEXTURE_2D or GL_RENDERBUFFER. In case of ambiguity when dealing with
  // Window bound frame-buffers, 0 may be used.
  uint32_t target;

  // The name of the framebuffer.
  uint32_t name;

  // User data to be returned on the invocation of the destruction callback.
  void* user_data;

  // Callback invoked (on an engine managed thread) that asks the embedder to
  // collect the framebuffer.
  VoidCallback destruction_callback;
} FlutterOpenGLFramebuffer;

// ACE PC preivew
using IdleCallback = std::function<void(int64_t)>;
typedef bool (*UserBoolCallback)(const void*, const size_t, const int32_t, const int32_t);
#ifdef USE_GLFW_WINDOW
namespace flutter {
  class PointerDataPacket;
}

using HandleTouchEventCallback =
    std::function<bool(std::unique_ptr<flutter::PointerDataPacket>&)>;
#endif

typedef bool (*BoolCallback)(void* /* user data */);
typedef FlutterTransformation (*TransformationCallback)(void* /* user data */);
typedef uint32_t (*UIntCallback)(void* /* user data */);
typedef bool (*SoftwareSurfacePresentCallback)(void* /* user data */,
                                               const void* /* allocation */,
                                               size_t /* row bytes */,
                                               size_t /* height */);
typedef void* (*ProcResolver)(void* /* user data */, const char* /* name */);
typedef bool (*TextureFrameCallback)(void* /* user data */,
                                     int64_t /* texture identifier */,
                                     size_t /* width */,
                                     size_t /* height */,
                                     FlutterOpenGLTexture* /* texture out */);
typedef void (*VsyncCallback)(void* /* user data */, intptr_t /* baton */);

typedef struct {
  // The size of this struct. Must be sizeof(FlutterOpenGLRendererConfig).
  size_t struct_size;
  BoolCallback make_current;
  BoolCallback clear_current;
  BoolCallback present;
  UserBoolCallback send_current_surface;  // for ACE PC preivew
  UIntCallback fbo_callback;
  // This is an optional callback. Flutter will ask the emebdder to create a GL
  // context current on a background thread. If the embedder is able to do so,
  // Flutter will assume that this context is in the same sharegroup as the main
  // rendering context and use this context for asynchronous texture uploads.
  // Though optional, it is recommended that all embedders set this callback as
  // it will lead to better performance in texture handling.
  BoolCallback make_resource_current;
  // By default, the renderer config assumes that the FBO does not change for
  // the duration of the engine run. If this argument is true, the
  // engine will ask the embedder for an updated FBO target (via an fbo_callback
  // invocation) after a present call.
  bool fbo_reset_after_present;
  // The transformation to apply to the render target before any rendering
  // operations. This callback is optional.
  TransformationCallback surface_transformation;
  ProcResolver gl_proc_resolver;
  // When the embedder specifies that a texture has a frame available, the
  // engine will call this method (on an internal engine managed thread) so that
  // external texture details can be supplied to the engine for subsequent
  // composition.
  TextureFrameCallback gl_external_texture_frame_callback;
} FlutterOpenGLRendererConfig;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterSoftwareRendererConfig).
  size_t struct_size;
  // The callback presented to the embedder to present a fully populated buffer
  // to the user. The pixel format of the buffer is the native 32-bit RGBA
  // format. The buffer is owned by the Flutter engine and must be copied in
  // this callback if needed.
  SoftwareSurfacePresentCallback surface_present_callback;
} FlutterSoftwareRendererConfig;

typedef struct {
  FlutterRendererType type;
  union {
    FlutterOpenGLRendererConfig open_gl;
    FlutterSoftwareRendererConfig software;
  };
} FlutterRendererConfig;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterWindowMetricsEvent).
  size_t struct_size;
  // Physical width of the window.
  size_t width;
  // Physical height of the window.
  size_t height;
  // Scale factor for the physical screen.
  double pixel_ratio;
} FlutterWindowMetricsEvent;

// The phase of the pointer event.
typedef enum {
  kCancel,
  // The pointer, which must have been down (see kDown), is now up.
  //
  // For touch, this means that the pointer is no longer in contact with the
  // screen. For a mouse, it means the last button was released. Note that if
  // any other buttons are still pressed when one button is released, that
  // should be sent as a kMove rather than a kUp.
  kUp,
  // The pointer, which must have been been up, is now down.
  //
  // For touch, this means that the pointer has come into contact with the
  // screen. For a mouse, it means a button is now pressed. Note that if any
  // other buttons are already pressed when a new button is pressed, that should
  // be sent as a kMove rather than a kDown.
  kDown,
  // The pointer moved while down.
  //
  // This is also used for changes in button state that don't cause a kDown or
  // kUp, such as releasing one of two pressed buttons.
  kMove,
  // The pointer is now sending input to Flutter. For instance, a mouse has
  // entered the area where the Flutter content is displayed.
  //
  // A pointer should always be added before sending any other events.
  kAdd,
  // The pointer is no longer sending input to Flutter. For instance, a mouse
  // has left the area where the Flutter content is displayed.
  //
  // A removed pointer should no longer send events until sending a new kAdd.
  kRemove,
  // The pointer moved while up.
  kHover,
} FlutterPointerPhase;

// The device type that created a pointer event.
typedef enum {
  kFlutterPointerDeviceKindMouse = 1,
  kFlutterPointerDeviceKindTouch,
} FlutterPointerDeviceKind;

// Flags for the |buttons| field of |FlutterPointerEvent| when |device_kind|
// is |kFlutterPointerDeviceKindMouse|.
typedef enum {
  kFlutterPointerButtonMousePrimary = 1 << 0,
  kFlutterPointerButtonMouseSecondary = 1 << 1,
  kFlutterPointerButtonMouseMiddle = 1 << 2,
  kFlutterPointerButtonMouseBack = 1 << 3,
  kFlutterPointerButtonMouseForward = 1 << 4,
  // If a mouse has more than five buttons, send higher bit shifted values
  // corresponding to the button number: 1 << 5 for the 6th, etc.
} FlutterPointerMouseButtons;

// The type of a pointer signal.
typedef enum {
  kFlutterPointerSignalKindNone,
  kFlutterPointerSignalKindScroll,
} FlutterPointerSignalKind;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterPointerEvent).
  size_t struct_size;
  FlutterPointerPhase phase;
  size_t timestamp;  // in microseconds.
  double x;
  double y;
  // An optional device identifier. If this is not specified, it is assumed that
  // the embedder has no multi-touch capability.
  int32_t device;
  FlutterPointerSignalKind signal_kind;
  double scroll_delta_x;
  double scroll_delta_y;
  // The type of the device generating this event.
  // Backwards compatibility note: If this is not set, the device will be
  // treated as a mouse, with the primary button set for |kDown| and |kMove|.
  // If set explicitly to |kFlutterPointerDeviceKindMouse|, you must set the
  // correct buttons.
  FlutterPointerDeviceKind device_kind;
  // The buttons currently pressed, if any.
  int64_t buttons;
} FlutterPointerEvent;

struct _FlutterPlatformMessageResponseHandle;
typedef struct _FlutterPlatformMessageResponseHandle
    FlutterPlatformMessageResponseHandle;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterPlatformMessage).
  size_t struct_size;
  const char* channel;
  const uint8_t* message;
  size_t message_size;
  // The response handle on which to invoke
  // |FlutterEngineSendPlatformMessageResponse| when the response is ready.
  // |FlutterEngineSendPlatformMessageResponse| must be called for all messages
  // received by the embedder. Failure to call
  // |FlutterEngineSendPlatformMessageResponse| will cause a memory leak. It is
  // not safe to send multiple responses on a single response object.
  const FlutterPlatformMessageResponseHandle* response_handle;
} FlutterPlatformMessage;

typedef void (*FlutterPlatformMessageCallback)(
    const FlutterPlatformMessage* /* message*/,
    void* /* user data */);

typedef void (*FlutterDataCallback)(const uint8_t* /* data */,
                                    size_t /* size */,
                                    void* /* user data */);

typedef struct {
  double left;
  double top;
  double right;
  double bottom;
} FlutterRect;

typedef struct _FlutterTaskRunner* FlutterTaskRunner;

typedef struct {
  FlutterTaskRunner runner;
  uint64_t task;
} FlutterTask;

typedef void (*FlutterTaskRunnerPostTaskCallback)(
    FlutterTask /* task */,
    uint64_t /* target time nanos */,
    void* /* user data */);

// An interface used by the Flutter engine to execute tasks at the target time
// on a specified thread. There should be a 1-1 relationship between a thread
// and a task runner. It is undefined behavior to run a task on a thread that is
// not associated with its task runner.
typedef struct {
  // The size of this struct. Must be sizeof(FlutterTaskRunnerDescription).
  size_t struct_size;
  void* user_data;
  // May be called from any thread. Should return true if tasks posted on the
  // calling thread will be run on that same thread.
  //
  // This field is required.
  BoolCallback runs_task_on_current_thread_callback;
  // May be called from any thread. The given task should be executed by the
  // embedder on the thread associated with that task runner by calling
  // |FlutterEngineRunTask| at the given target time. The system monotonic clock
  // should be used for the target time. The target time is the absolute time
  // from epoch (NOT a delta) at which the task must be returned back to the
  // engine on the correct thread. If the embedder needs to calculate a delta,
  // |FlutterEngineGetCurrentTime| may be called and the difference used as the
  // delta.
  //
  // This field is required.
  FlutterTaskRunnerPostTaskCallback post_task_callback;
  /// A unique identifier for the task runner. If multiple task runners service
  /// tasks on the same thread, their identifiers must match.
  size_t identifier;
} FlutterTaskRunnerDescription;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterCustomTaskRunners).
  size_t struct_size;
  // Specify the task runner for the thread on which the |FlutterEngineRun|
  /// call is made. The same task runner description can be specified for both
  /// the render and platform task runners. This makes the Flutter engine use
  /// the same thread for both task runners.
  const FlutterTaskRunnerDescription* platform_task_runner;
  /// Specify the task runner for the thread on which the render tasks will be
  /// run. The same task runner description can be specified for both the render
  /// and platform task runners. This makes the Flutter engine use the same
  /// thread for both task runners.
  const FlutterTaskRunnerDescription* render_task_runner;
} FlutterCustomTaskRunners;

typedef struct {
  // The type of the OpenGL backing store. Currently, it can either be a texture
  // or a framebuffer.
  FlutterOpenGLTargetType type;
  union {
    // A texture for Flutter to render into.
    FlutterOpenGLTexture texture;
    // A framebuffer for Flutter to render into. The embedder must ensure that
    // the framebuffer is complete.
    FlutterOpenGLFramebuffer framebuffer;
  };
} FlutterOpenGLBackingStore;

typedef struct {
  // A pointer to the raw bytes of the allocation described by this software
  // backing store.
  const void* allocation;
  // The number of bytes in a single row of the allocation.
  size_t row_bytes;
  // The number of rows in the allocation.
  size_t height;
  // A baton that is not interpreted by the engine in any way. It will be given
  // back to the embedder in the destruction callback below. Embedder resources
  // may be associated with this baton.
  void* user_data;
  // The callback invoked by the engine when it no longer needs this backing
  // store.
  VoidCallback destruction_callback;
} FlutterSoftwareBackingStore;

// The identifier of the platform view. This identifier is specified by the
// application when a platform view is added to the scene via the
// `SceneBuilder.addPlatformView` call.
typedef int64_t FlutterPlatformViewIdentifier;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterPlatformView).
  size_t struct_size;
  // The identifier of this platform view. This identifier is specified by the
  // application when a platform view is added to the scene via the
  // `SceneBuilder.addPlatformView` call.
  FlutterPlatformViewIdentifier identifier;
} FlutterPlatformView;

typedef enum {
  // Specifies an OpenGL backing store. Can either be an OpenGL texture or
  // framebuffer.
  kFlutterBackingStoreTypeOpenGL,
  // Specified an software allocation for Flutter to render into using the CPU.
  kFlutterBackingStoreTypeSoftware,
} FlutterBackingStoreType;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterBackingStore).
  size_t struct_size;
  // A baton that is not interpreted by the engine in any way. The embedder may
  // use this to associate resources that are tied to the lifecycle of the
  // |FlutterBackingStore|.
  void* user_data;
  // Specifies the type of backing store.
  FlutterBackingStoreType type;
  // Indicates if this backing store was updated since the last time it was
  // associated with a presented layer.
  bool did_update;
  union {
    // The description of the OpenGL backing store.
    FlutterOpenGLBackingStore open_gl;
    // The description of the software backing store.
    FlutterSoftwareBackingStore software;
  };
} FlutterBackingStore;

typedef struct {
  double x;
  double y;
} FlutterPoint;

typedef struct {
  double width;
  double height;
} FlutterSize;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterBackingStoreConfig).
  size_t struct_size;
  // The size of the render target the engine expects to render into.
  FlutterSize size;
} FlutterBackingStoreConfig;

typedef enum {
  // Indicates that the contents of this layer are rendered by Flutter into a
  // backing store.
  kFlutterLayerContentTypeBackingStore,
  // Indicates that the contents of this layer are determined by the embedder.
  kFlutterLayerContentTypePlatformView,
} FlutterLayerContentType;

typedef struct {
  // This size of this struct. Must be sizeof(FlutterLayer).
  size_t struct_size;
  // Each layer displays contents in one way or another. The type indicates
  // whether those contents are specified by Flutter or the embedder.
  FlutterLayerContentType type;
  union {
    // Indicates that the contents of this layer are rendered by Flutter into a
    // backing store.
    const FlutterBackingStore* backing_store;
    // Indicates that the contents of this layer are determined by the embedder.
    const FlutterPlatformView* platform_view;
  };
  // The offset of this layer (in physical pixels) relative to the top left of
  // the root surface used by the engine.
  FlutterPoint offset;
  // The size of the layer (in physical pixels).
  FlutterSize size;
} FlutterLayer;

typedef bool (*FlutterBackingStoreCreateCallback)(
    const FlutterBackingStoreConfig* config,
    FlutterBackingStore* backing_store_out,
    void* user_data);

typedef bool (*FlutterBackingStoreCollectCallback)(
    const FlutterBackingStore* renderer,
    void* user_data);

typedef bool (*FlutterLayersPresentCallback)(const FlutterLayer** layers,
                                             size_t layers_count,
                                             void* user_data);

typedef struct {
  // This size of this struct. Must be sizeof(FlutterCompositor).
  size_t struct_size;
  // A baton that in not interpreted by the engine in any way. If it passed back
  // to the embedder in  |FlutterCompositor.create_backing_store_callback|,
  // |FlutterCompositor.collect_backing_store_callback| and
  // |FlutterCompositor.present_layers_callback|
  void* user_data;
  // A callback invoked by the engine to obtain a backing store for a specific
  // |FlutterLayer|.
  //
  // On ABI stability: Callers must take care to restrict access within
  // |FlutterBackingStore::struct_size| when specifying a new backing store to
  // the engine. This only matters if the embedder expects to be used with
  // engines older than the version whose headers it used during compilation.
  FlutterBackingStoreCreateCallback create_backing_store_callback;
  // A callback invoked by the engine to release the backing store. The embedder
  // may collect any resources associated with the backing store.
  FlutterBackingStoreCollectCallback collect_backing_store_callback;
  // Callback invoked by the engine to composite the contents of each layer onto
  // the screen.
  FlutterLayersPresentCallback present_layers_callback;
} FlutterCompositor;

typedef struct {
  // The size of this struct. Must be sizeof(FlutterProjectArgs).
  size_t struct_size;

  // A callback that gets invoked by the engine when it attempts to wait for a
  // platform vsync event. The engine will give the platform a baton that needs
  // to be returned back to the engine via |FlutterEngineOnVsync|. All batons
  // must be retured to the engine before initializing a
  // |FlutterEngineShutdown|. Not doing the same will result in a memory leak.
  // While the call to |FlutterEngineOnVsync| must occur on the thread that made
  // the call to |FlutterEngineRun|, the engine will make this callback on an
  // internal engine-managed thread. If the components accessed on the embedder
  // are not thread safe, the appropriate re-threading must be done.
  VsyncCallback vsync_callback;

  // Typically the Flutter engine create and manages its internal threads. This
  // optional argument allows for the specification of task runner interfaces to
  // event loops managed by the embedder on threads it creates.
  const FlutterCustomTaskRunners* custom_task_runners;

  // Typically, Flutter renders the layer hierarchy into a single root surface.
  // However, when embedders need to interleave their own contents within the
  // Flutter layer hierarchy, their applications can push platform views within
  // the Flutter scene. This is done using the `SceneBuilder.addPlatformView`
  // call. When this happens, the Flutter rasterizer divides the effective view
  // hierarchy into multiple layers. Each layer gets its own backing store and
  // Flutter renders into the same. Once the layers contents have been
  // fulfilled, the embedder is asked to composite these layers on-screen. At
  // this point, it can interleave its own contents within the effective
  // hierarchy. The interface for the specification of these layer backing
  // stores and the hooks to listen for the composition of layers on-screen can
  // be controlled using this field. This field is completely optional. In its
  // absence, platforms views in the scene are ignored and Flutter renders to
  // the root surface as normal.
  const FlutterCompositor* compositor;
} FlutterProjectArgs;

//------------------------------------------------------------------------------
/// @brief      Initialize and run a Flutter engine instance and return a handle
///             to it. This is a convenience method for the the pair of calls to
///             `FlutterEngineInitialize` and `FlutterEngineRunInitialized`.
///
/// @note       This method of running a Flutter engine works well except in
///             cases where the embedder specifies custom task runners via
///             `FlutterProjectArgs::custom_task_runners`. In such cases, the
///             engine may need the embedder to post tasks back to it before
///             `FlutterEngineRun` has returned. Embedders can only post tasks
///             to the engine if they have a handle to the engine. In such
///             cases, embedders are advised to get the engine handle via the
///             `FlutterInitializeCall`. Then they can call
///             `FlutterEngineRunInitialized` knowing that they will be able to
///             service custom tasks on other threads with the engine handle.
///
/// @param[in]  version    The Flutter embedder API version. Must be
///                        FLUTTER_ENGINE_VERSION.
/// @param[in]  config     The renderer configuration.
/// @param[in]  args       The Flutter project arguments.
/// @param      user_data  A user data baton passed back to embedders in
///                        callbacks.
/// @param[out] engine_out The engine handle on successful engine creation.
///
/// @return     The result of the call to run the Flutter engine.
///
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineRun(const FlutterRendererConfig* config,
                                     const FlutterProjectArgs* args,
                                     void* user_data,
                                     FLUTTER_API_SYMBOL(FlutterEngine) *
                                         engine_out);

//------------------------------------------------------------------------------
/// @brief      Shuts down a Flutter engine instance. The engine handle is no
///             longer valid for any calls in the embedder API after this point.
///             Making additional calls with this handle is undefined behavior.
///
/// @note       This de-initializes the Flutter engine instance (via an implicit
///             call to `FlutterEngineDeinitialize`) if necessary.
///
/// @param[in]  engine  The Flutter engine instance to collect.
///
/// @return     The result of the call to shutdown the Flutter engine instance.
///
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineShutdown(FLUTTER_API_SYMBOL(FlutterEngine)
                                              engine);
//------------------------------------------------------------------------------
/// @brief      Initialize a Flutter engine instance. This does not run the
///             Flutter application code till the `FlutterEngineRunInitialized`
///             call is made. Besides Flutter application code, no tasks are
///             scheduled on embedder managed task runners either. This allows
///             embedders providing custom task runners to the Flutter engine to
///             obtain a handle to the Flutter engine before the engine can post
///             tasks on these task runners.
///
/// @param[in]  version    The Flutter embedder API version. Must be
///                        FLUTTER_ENGINE_VERSION.
/// @param[in]  config     The renderer configuration.
/// @param[in]  args       The Flutter project arguments.
/// @param      user_data  A user data baton passed back to embedders in
///                        callbacks.
/// @param[out] engine_out The engine handle on successful engine creation.
///
/// @return     The result of the call to initialize the Flutter engine.
///
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineInitialize(const FlutterRendererConfig* config,
                                            const FlutterProjectArgs* args,
                                            void* user_data,
                                            FLUTTER_API_SYMBOL(FlutterEngine) *
                                                engine_out);

//------------------------------------------------------------------------------
/// @brief      Stops running the Flutter engine instance. After this call, the
///             embedder is also guaranteed that no more calls to post tasks
///             onto custom task runners specified by the embedder are made. The
///             Flutter engine handle still needs to be collected via a call to
///             `FlutterEngineShutdown`.
///
/// @param[in]  engine    The running engine instance to de-initialize.
///
/// @return     The result of the call to de-initialize the Flutter engine.
///
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineDeinitialize(FLUTTER_API_SYMBOL(FlutterEngine)
                                                  engine);

//------------------------------------------------------------------------------
/// @brief      Runs an initialized engine instance. An engine can be
///             initialized via `FlutterEngineInitialize`. An initialized
///             instance can only be run once. During and after this call,
///             custom task runners supplied by the embedder are expected to
///             start servicing tasks.
///
/// @param[in]  engine  An initialized engine instance that has not previously
///                     been run.
///
/// @return     The result of the call to run the initialized Flutter
///             engine instance.
///
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineRunInitialized(
    FLUTTER_API_SYMBOL(FlutterEngine) engine);

FLUTTER_EXPORT
FlutterEngineResult FlutterEngineSetIdleNotificationCallback(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const IdleCallback& idle_notification_callback);

FLUTTER_EXPORT
FlutterEngineResult FlutterEngineSendWindowMetricsEvent(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const FlutterWindowMetricsEvent* event);

FLUTTER_EXPORT
FlutterEngineResult FlutterEngineSendPointerEvent(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const FlutterPointerEvent* events,
    size_t events_count);

FLUTTER_EXPORT
FlutterEngineResult FlutterEngineSendPlatformMessage(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const FlutterPlatformMessage* message);

// Creates a platform message response handle that allows the embedder to set a
// native callback for a response to a message. This handle may be set on the
// |response_handle| field of any |FlutterPlatformMessage| sent to the engine.
//
// The handle must be collected via a call to
// |FlutterPlatformMessageReleaseResponseHandle|. This may be done immediately
// after a call to |FlutterEngineSendPlatformMessage| with a platform message
// whose response handle contains the handle created using this call. In case a
// handle is created but never sent in a message, the release call must still be
// made. Not calling release on the handle results in a small memory leak.
//
// The user data baton passed to the data callback is the one specified in this
// call as the third argument.
FLUTTER_EXPORT
FlutterEngineResult FlutterPlatformMessageCreateResponseHandle(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    FlutterDataCallback data_callback,
    void* user_data,
    FlutterPlatformMessageResponseHandle** response_out);

// Collects the handle created using
// |FlutterPlatformMessageCreateResponseHandle|.
FLUTTER_EXPORT
FlutterEngineResult FlutterPlatformMessageReleaseResponseHandle(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    FlutterPlatformMessageResponseHandle* response);

FLUTTER_EXPORT
FlutterEngineResult FlutterEngineSendPlatformMessageResponse(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const FlutterPlatformMessageResponseHandle* handle,
    const uint8_t* data,
    size_t data_length);

// This API is only meant to be used by platforms that need to flush tasks on a
// message loop not controlled by the Flutter engine. This API will be
// deprecated soon.
FLUTTER_EXPORT
FlutterEngineResult __FlutterEngineFlushPendingTasksNow();

// Register an external texture with a unique (per engine) identifier. Only
// rendering backends that support external textures accept external texture
// registrations. After the external texture is registered, the application can
// mark that a frame is available by calling
// |FlutterEngineMarkExternalTextureFrameAvailable|.
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineRegisterExternalTexture(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    int64_t texture_identifier);

// Unregister a previous texture registration.
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineUnregisterExternalTexture(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    int64_t texture_identifier);

// Mark that a new texture frame is available for a given texture identifier.
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineMarkExternalTextureFrameAvailable(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    int64_t texture_identifier);

// Notify the engine that a vsync event occurred. A baton passed to the
// platform via the vsync callback must be returned. This call must be made on
// the thread on which the call to |FlutterEngineRun| was made.
//
// |frame_start_time_nanos| is the point at which the vsync event occurred or
// will occur. If the time point is in the future, the engine will wait till
// that point to begin its frame workload. The system monotonic clock is used as
// the timebase.
//
// |frame_target_time_nanos| is the point at which the embedder anticipates the
// next vsync to occur. This is a hint the engine uses to schedule Dart VM
// garbage collection in periods in which the various threads are most likely to
// be idle. For example, for a 60Hz display, embedders should add 16.6 * 1e6 to
// the frame time field. The system monotonic clock is used as the timebase.
//
// That frame timepoints are in nanoseconds.
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineOnVsync(FLUTTER_API_SYMBOL(FlutterEngine)
                                             engine,
                                         intptr_t baton,
                                         uint64_t frame_start_time_nanos,
                                         uint64_t frame_target_time_nanos);

// A profiling utility. Logs a trace duration begin event to the timeline. If
// the timeline is unavailable or disabled, this has no effect. Must be
// balanced with an duration end event (via
// |FlutterEngineTraceEventDurationEnd|) with the same name on the same thread.
// Can be called on any thread. Strings passed into the function will NOT be
// copied when added to the timeline. Only string literals may be passed in.
FLUTTER_EXPORT
void FlutterEngineTraceEventDurationBegin(const char* name);

// A profiling utility. Logs a trace duration end event to the timeline. If the
// timeline is unavailable or disabled, this has no effect. This call must be
// preceded by a trace duration begin call (via
// |FlutterEngineTraceEventDurationBegin|) with the same name on the same
// thread. Can be called on any thread. Strings passed into the function will
// NOT be copied when added to the timeline. Only string literals may be passed
// in.
FLUTTER_EXPORT
void FlutterEngineTraceEventDurationEnd(const char* name);

// A profiling utility. Logs a trace duration instant event to the timeline. If
// the timeline is unavailable or disabled, this has no effect. Can be called
// on any thread. Strings passed into the function will NOT be copied when added
// to the timeline. Only string literals may be passed in.
FLUTTER_EXPORT
void FlutterEngineTraceEventInstant(const char* name);

// Posts a task onto the Flutter render thread. Typically, this may be called
// from any thread as long as a |FlutterEngineShutdown| on the specific engine
// has not already been initiated.
FLUTTER_EXPORT
FlutterEngineResult FlutterEnginePostRenderThreadTask(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    VoidCallback callback,
    void* callback_data);

// Get the current time in nanoseconds from the clock used by the flutter
// engine. This is the system monotonic clock.
FLUTTER_EXPORT
uint64_t FlutterEngineGetCurrentTime();

// Inform the engine to run the specified task. This task has been given to
// the engine via the |FlutterTaskRunnerDescription.post_task_callback|. This
// call must only be made at the target time specified in that callback. Running
// the task before that time is undefined behavior.
FLUTTER_EXPORT
FlutterEngineResult FlutterEngineRunTask(FLUTTER_API_SYMBOL(FlutterEngine)
                                             engine,
                                         const FlutterTask* task);

#ifdef USE_GLFW_WINDOW
FLUTTER_EXPORT
void FlutterEngineRegisterHandleTouchEventCallback(HandleTouchEventCallback&& callback);
#endif

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_EMBEDDER_H_
