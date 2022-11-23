// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/public/flutter_glfw.h"

#include <GLFW/glfw3.h>
#include <assert.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/glfw/glfw_event_loop.h"
#include "flutter/shell/platform/glfw/keyboard_hook_handler.h"

#ifndef WINDOWS_PLATFORM
#define WINDOWS_PLATFORM
#endif

// GLFW_TRUE & GLFW_FALSE are introduced since libglfw-3.3,
// add definitions here to compile under the old versions.
#ifndef GLFW_TRUE
#define GLFW_TRUE 1
#endif
#ifndef GLFW_FALSE
#define GLFW_FALSE 0
#endif

using UniqueGLFWwindowPtr = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow*)>;

static constexpr double kDpPerInch = 160.0;

// Struct for storing state within an instance of the GLFW Window.
struct FlutterDesktopWindowControllerState {
  // The GLFW window that is bound to this state object.
  UniqueGLFWwindowPtr window = UniqueGLFWwindowPtr(nullptr, glfwDestroyWindow);

  // The invisible GLFW window used to upload resources in the background.
  UniqueGLFWwindowPtr resource_window =
      UniqueGLFWwindowPtr(nullptr, glfwDestroyWindow);

  // The handle to the Flutter engine instance.
  FLUTTER_API_SYMBOL(FlutterEngine) engine;

  // The window handle given to API clients.
  std::unique_ptr<FlutterDesktopWindow> window_wrapper;

  // Handlers for keyboard events from GLFW.
  std::vector<std::unique_ptr<flutter::KeyboardHookHandler>>
      keyboard_hook_handlers;

  // The event loop for the main thread that allows for delayed task execution.
  std::unique_ptr<flutter::GLFWEventLoop> event_loop;

  // Whether or not the pointer has been added (or if tracking is enabled,
  // has been added since it was last removed).
  bool pointer_currently_added = false;

  // The screen coordinates per inch on the primary monitor. Defaults to a sane
  // value based on pixel_ratio 1.0.
  double monitor_screen_coordinates_per_inch = kDpPerInch;
};

// Opaque reference for the GLFW window itself. This is separate from the
// controller so that it can be provided to plugins without giving them access
// to all of the controller-based functionality.
struct FlutterDesktopWindow {
  // The GLFW window that (indirectly) owns this state object.
  GLFWwindow* window;

  // Whether or not to track mouse movements to send kHover events.
  bool hover_tracking_enabled = true;

  // The ratio of pixels per screen coordinate for the window.
  double pixels_per_screen_coordinate = 1.0;

  // Resizing triggers a window refresh, but the resize already updates Flutter.
  // To avoid double messages, the refresh after each resize is skipped.
  bool skip_next_window_refresh = false;
};

// Struct for storing state of a Flutter engine instance.
struct FlutterDesktopEngineState {
  // The handle to the Flutter engine instance.
  FLUTTER_API_SYMBOL(FlutterEngine) engine;
};

// Retrieves state bag for the window in question from the GLFWWindow.
static FlutterDesktopWindowControllerState* GetSavedWindowState(
    GLFWwindow* window) {
  return reinterpret_cast<FlutterDesktopWindowControllerState*>(
      glfwGetWindowUserPointer(window));
}

// Creates and returns an invisible GLFW window that shares |window|'s resource
// context.
static UniqueGLFWwindowPtr CreateShareWindowForWindow(GLFWwindow* window) {
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  GLFWwindow* share_window = glfwCreateWindow(1, 1, "", NULL, window);
  glfwDefaultWindowHints();
  return UniqueGLFWwindowPtr(share_window, glfwDestroyWindow);
}

// Returns the number of screen coordinates per inch for the main monitor.
// If the information is unavailable, returns a default value that assumes
// that a screen coordinate is one dp.
static double GetScreenCoordinatesPerInch() {
  auto* primary_monitor = glfwGetPrimaryMonitor();
  auto* primary_monitor_mode = glfwGetVideoMode(primary_monitor);
  int primary_monitor_width_mm;
  glfwGetMonitorPhysicalSize(primary_monitor, &primary_monitor_width_mm,
                             nullptr);
  if (primary_monitor_width_mm == 0) {
    return kDpPerInch;
  }
  return primary_monitor_mode->width / (primary_monitor_width_mm / 25.4);
}

// Sends a window metrics update to the Flutter engine using the given
// framebuffer size and the current window information in |state|.
static void SendWindowMetrics(FlutterDesktopWindowControllerState* state,
                              int width,
                              int height) {
  double dpi = state->window_wrapper->pixels_per_screen_coordinate *
               state->monitor_screen_coordinates_per_inch;

  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  event.width = width;
  event.height = height;
  // The Flutter pixel_ratio is defined as DPI/dp. Limit the ratio to a minimum
  // of 1 to avoid rendering a smaller UI on standard resolution monitors.
  event.pixel_ratio = std::max(dpi / kDpPerInch, 1.0);
  FlutterEngineSendWindowMetricsEvent(state->engine, &event);
}

// When GLFW calls back to the window with a framebuffer size change, notify
// FlutterEngine about the new window metrics.
static void GLFWFramebufferSizeCallback(GLFWwindow* window,
                                        int width_px,
                                        int height_px) {
  int width;
  glfwGetWindowSize(window, &width, nullptr);
  auto* state = GetSavedWindowState(window);
  state->window_wrapper->pixels_per_screen_coordinate =
      width > 0 ? width_px / width : 1;

  SendWindowMetrics(state, width_px, height_px);
  state->window_wrapper->skip_next_window_refresh = true;
}

// Indicates that the window needs to be redrawn.
void GLFWWindowRefreshCallback(GLFWwindow* window) {
  auto* state = GetSavedWindowState(window);
  if (state->window_wrapper->skip_next_window_refresh) {
    state->window_wrapper->skip_next_window_refresh = false;
    return;
  }
  // There's no engine API to request a redraw explicitly, so instead send a
  // window metrics event with the current size to trigger it.
  int width_px, height_px;
  glfwGetFramebufferSize(window, &width_px, &height_px);
  if (width_px > 0 && height_px > 0) {
    SendWindowMetrics(state, width_px, height_px);
  }
}

// Sends a pointer event to the Flutter engine based on the given data.
//
// Any coordinate/distance values in |event_data| should be in screen
// coordinates; they will be adjusted to pixel values before being sent.
static void SendPointerEventWithData(GLFWwindow* window,
                                     const FlutterPointerEvent& event_data) {
  auto* state = GetSavedWindowState(window);
  // If sending anything other than an add, and the pointer isn't already added,
  // synthesize an add to satisfy Flutter's expectations about events.
  if (!state->pointer_currently_added &&
      event_data.phase != FlutterPointerPhase::kAdd) {
    FlutterPointerEvent event = {};
    event.phase = FlutterPointerPhase::kAdd;
    event.x = event_data.x;
    event.y = event_data.y;
    SendPointerEventWithData(window, event);
  }
  // Don't double-add (e.g., if events are delivered out of order, so an add has
  // already been synthesized).
  if (state->pointer_currently_added &&
      event_data.phase == FlutterPointerPhase::kAdd) {
    return;
  }

  FlutterPointerEvent event = event_data;
  // Set metadata that's always the same regardless of the event.
  event.struct_size = sizeof(event);
  event.timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  // Convert all screen coordinates to pixel coordinates.
  double pixels_per_coordinate =
      state->window_wrapper->pixels_per_screen_coordinate;
  event.x *= pixels_per_coordinate;
  event.y *= pixels_per_coordinate;
  event.scroll_delta_x *= pixels_per_coordinate;
  event.scroll_delta_y *= pixels_per_coordinate;

  FlutterEngineSendPointerEvent(state->engine, &event, 1);

  if (event_data.phase == FlutterPointerPhase::kAdd) {
    state->pointer_currently_added = true;
  } else if (event_data.phase == FlutterPointerPhase::kRemove) {
    state->pointer_currently_added = false;
  }
}

// Updates |event_data| with the current location of the mouse cursor.
static void SetEventLocationFromCursorPosition(
    GLFWwindow* window,
    FlutterPointerEvent* event_data) {
  glfwGetCursorPos(window, &event_data->x, &event_data->y);
}

// Set's |event_data|'s phase to either kMove or kHover depending on the current
// primary mouse button state.
static void SetEventPhaseFromCursorButtonState(
    GLFWwindow* window,
    FlutterPointerEvent* event_data) {
  event_data->phase =
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS
          ? FlutterPointerPhase::kMove
          : FlutterPointerPhase::kHover;
}

// Reports the mouse entering or leaving the Flutter view.
static void GLFWCursorEnterCallback(GLFWwindow* window, int entered) {
  FlutterPointerEvent event = {};
  event.phase =
      entered ? FlutterPointerPhase::kAdd : FlutterPointerPhase::kRemove;
  SetEventLocationFromCursorPosition(window, &event);
  SendPointerEventWithData(window, event);
}

// Reports mouse movement to the Flutter engine.
static void GLFWCursorPositionCallback(GLFWwindow* window, double x, double y) {
  FlutterPointerEvent event = {};
  event.x = x;
  event.y = y;
  SetEventPhaseFromCursorButtonState(window, &event);
  SendPointerEventWithData(window, event);
}

// Reports mouse button press to the Flutter engine.
static void GLFWMouseButtonCallback(GLFWwindow* window,
                                    int key,
                                    int action,
                                    int mods) {
  // Flutter currently doesn't understand other buttons, so ignore anything
  // other than left.
  if (key != GLFW_MOUSE_BUTTON_LEFT) {
    return;
  }
  FlutterPointerEvent event = {};
  event.phase = (action == GLFW_PRESS) ? FlutterPointerPhase::kDown
                                       : FlutterPointerPhase::kUp;
  SetEventLocationFromCursorPosition(window, &event);
  SendPointerEventWithData(window, event);

  // If mouse tracking isn't already enabled, turn it on for the duration of
  // the drag to generate kMove events.
  bool hover_enabled =
      GetSavedWindowState(window)->window_wrapper->hover_tracking_enabled;
  if (!hover_enabled) {
    glfwSetCursorPosCallback(
        window, (action == GLFW_PRESS) ? GLFWCursorPositionCallback : nullptr);
  }
  // Disable enter/exit events while the mouse button is down; GLFW will send
  // an exit event when the mouse button is released, and the pointer should
  // stay valid until then.
  if (hover_enabled) {
    glfwSetCursorEnterCallback(
        window, (action == GLFW_PRESS) ? nullptr : GLFWCursorEnterCallback);
  }
}

// Reports scroll wheel events to the Flutter engine.
static void GLFWScrollCallback(GLFWwindow* window,
                               double delta_x,
                               double delta_y) {
  FlutterPointerEvent event = {};
  SetEventLocationFromCursorPosition(window, &event);
  SetEventPhaseFromCursorButtonState(window, &event);
  event.signal_kind = FlutterPointerSignalKind::kFlutterPointerSignalKindScroll;
  // TODO: See if this can be queried from the OS; this value is chosen
  // arbitrarily to get something that feels reasonable.
  const int kScrollOffsetMultiplier = 20;
  event.scroll_delta_x = delta_x * kScrollOffsetMultiplier;
  event.scroll_delta_y = -delta_y * kScrollOffsetMultiplier;
  SendPointerEventWithData(window, event);
}

// Passes character input events to registered handlers.
static void GLFWCharCallback(GLFWwindow* window, unsigned int code_point) {
  for (const auto& handler :
       GetSavedWindowState(window)->keyboard_hook_handlers) {
    handler->CharHook(window, code_point);
  }
}

// Passes raw key events to registered handlers.
static void GLFWKeyCallback(GLFWwindow* window,
                            int key,
                            int scancode,
                            int action,
                            int mods) {
  for (const auto& handler :
       GetSavedWindowState(window)->keyboard_hook_handlers) {
    handler->KeyboardHook(window, key, scancode, action, mods);
  }
}

// Enables/disables the callbacks related to mouse tracking.
static void SetHoverCallbacksEnabled(GLFWwindow* window, bool enabled) {
  glfwSetCursorEnterCallback(window,
                             enabled ? GLFWCursorEnterCallback : nullptr);
  glfwSetCursorPosCallback(window,
                           enabled ? GLFWCursorPositionCallback : nullptr);
}

// Flushes event queue and then assigns default window callbacks.
static void GLFWAssignEventCallbacks(GLFWwindow* window) {
  glfwPollEvents();
  glfwSetKeyCallback(window, GLFWKeyCallback);
  glfwSetCharCallback(window, GLFWCharCallback);
  glfwSetMouseButtonCallback(window, GLFWMouseButtonCallback);
  glfwSetScrollCallback(window, GLFWScrollCallback);
  if (GetSavedWindowState(window)->window_wrapper->hover_tracking_enabled) {
    SetHoverCallbacksEnabled(window, true);
  }
}

static bool GLFWMakeContextCurrent(void* user_data) {
  GLFWwindow* window = reinterpret_cast<GLFWwindow*>(user_data);
  glfwMakeContextCurrent(window);
  return true;
}

static bool GLFWMakeResourceContextCurrent(void* user_data) {
  GLFWwindow* window = reinterpret_cast<GLFWwindow*>(user_data);
  glfwMakeContextCurrent(GetSavedWindowState(window)->resource_window.get());
  return true;
}

static bool GLFWClearContext(void* user_data) {
  glfwMakeContextCurrent(nullptr);
  return true;
}

static bool GLFWPresent(void* user_data) {
  GLFWwindow* window = reinterpret_cast<GLFWwindow*>(user_data);
  glfwSwapBuffers(window);
  return true;
}

static uint32_t GLFWGetActiveFbo(void* user_data) {
  return 0;
}

// Clears the GLFW window to Material Blue-Grey.
//
// This function is primarily to fix an issue when the Flutter Engine is
// spinning up, wherein artifacts of existing windows are rendered onto the
// canvas for a few moments.
//
// This function isn't necessary, but makes starting the window much easier on
// the eyes.
static void GLFWClearCanvas(GLFWwindow* window) {
  glfwMakeContextCurrent(window);
  // This color is Material Blue Grey.
  glClearColor(236.0f / 255.0f, 239.0f / 255.0f, 241.0f / 255.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glFlush();
  glfwSwapBuffers(window);
  glfwMakeContextCurrent(nullptr);
}

// Resolves the address of the specified OpenGL or OpenGL ES
// core or extension function, if it is supported by the current context.
static void* GLFWProcResolver(void* user_data, const char* name) {
  return reinterpret_cast<void*>(glfwGetProcAddress(name));
}

static void GLFWErrorCallback(int error_code, const char* description) {
  std::cerr << "GLFW error " << error_code << ": " << description << std::endl;
}

// Spins up an instance of the Flutter Engine.
//
// This function launches the Flutter Engine in a background thread, supplying
// the necessary callbacks for rendering within a GLFWwindow (if one is
// provided).
//
// Returns a caller-owned pointer to the engine.
static FLUTTER_API_SYMBOL(FlutterEngine)
    RunFlutterEngine(GLFWwindow* window,
                     const SurfacePresentCallback& sendSurface,
                     const FlutterCustomTaskRunners* custom_task_runners) {
  // FlutterProjectArgs is expecting a full argv, so when processing it for
  // flags the first item is treated as the executable and ignored. Add a dummy
  // value so that all provided arguments are used.
  std::vector<const char*> argv = {"placeholder"};

  FlutterRendererConfig config = {};
  if (window == nullptr) {
    config.type = kOpenGL;
    config.open_gl.struct_size = sizeof(config.open_gl);
    config.open_gl.make_current = [](void* data) -> bool { return false; };
    config.open_gl.clear_current = [](void* data) -> bool { return false; };
    config.open_gl.present = [](void* data) -> bool { return false; };
    config.open_gl.fbo_callback = [](void* data) -> uint32_t { return 0; };
  } else {
    // Provide the necessary callbacks for rendering within a GLFWwindow.
    config.type = kOpenGL;
    config.open_gl.struct_size = sizeof(config.open_gl);
    config.open_gl.make_current = GLFWMakeContextCurrent;
    config.open_gl.clear_current = GLFWClearContext;
    config.open_gl.present = GLFWPresent;
    config.open_gl.send_current_surface = sendSurface;
    config.open_gl.fbo_callback = GLFWGetActiveFbo;
    config.open_gl.make_resource_current = GLFWMakeResourceContextCurrent;
    config.open_gl.gl_proc_resolver = GLFWProcResolver;
  }
  FlutterProjectArgs args = {};
  args.struct_size = sizeof(FlutterProjectArgs);
  args.custom_task_runners = custom_task_runners;
  FLUTTER_API_SYMBOL(FlutterEngine) engine = nullptr;
  auto result =
      FlutterEngineRun(&config, &args, window, &engine);
  if (result != kSuccess || engine == nullptr) {
    std::cerr << "Failed to start Flutter engine: error " << result
              << std::endl;
    return nullptr;
  }
  return engine;
}

bool FlutterDesktopInit() {
  // Before making any GLFW calls, set up a logging error handler.
  glfwSetErrorCallback(GLFWErrorCallback);
  return glfwInit();
}

void FlutterDesktopTerminate() {
  glfwTerminate();
}

FlutterDesktopWindowControllerRef FlutterDesktopCreateWindow(
    int &initial_width,
    int &initial_height,
    const char* title,
    const SurfacePresentCallback& sendSurface) {
  auto state = std::make_unique<FlutterDesktopWindowControllerState>();

#ifndef USE_GLFW_WINDOW
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#endif

  // Create the window, and set the state as its user data.
  state->window = UniqueGLFWwindowPtr(
      glfwCreateWindow(initial_width, initial_height, title, NULL, NULL),
      glfwDestroyWindow);
  GLFWwindow* window = state->window.get();
  if (window == nullptr) {
    return nullptr;
  }
  GLFWClearCanvas(window);
  glfwSetWindowUserPointer(window, state.get());

  // Create the share window before starting the engine, since it may call
  // GLFWMakeResourceContextCurrent immediately.
  state->resource_window = CreateShareWindowForWindow(window);

  // Create an event loop for the window. It is not running yet.
  state->event_loop = std::make_unique<flutter::GLFWEventLoop>(
      std::this_thread::get_id(),  // main GLFW thread
      [state = state.get()](const auto* task) {
        if (FlutterEngineRunTask(state->engine, task) != kSuccess) {
          std::cerr << "Could not post an engine task." << std::endl;
        }
      });

  // Configure task runner interop.
  FlutterTaskRunnerDescription platform_task_runner = {};
  platform_task_runner.struct_size = sizeof(FlutterTaskRunnerDescription);
  platform_task_runner.user_data = state.get();
  platform_task_runner.runs_task_on_current_thread_callback =
      [](void* state) -> bool {
    return reinterpret_cast<FlutterDesktopWindowControllerState*>(state)
        ->event_loop->RunsTasksOnCurrentThread();
  };
  platform_task_runner.post_task_callback =
      [](FlutterTask task, uint64_t target_time_nanos, void* state) -> void {
    reinterpret_cast<FlutterDesktopWindowControllerState*>(state)
        ->event_loop->PostTask(task, target_time_nanos);
  };

  FlutterCustomTaskRunners custom_task_runners = {};
  custom_task_runners.struct_size = sizeof(FlutterCustomTaskRunners);
  custom_task_runners.platform_task_runner = &platform_task_runner;
  custom_task_runners.render_task_runner = &platform_task_runner;

  // Start the engine.
  state->engine =
      RunFlutterEngine(window, sendSurface, &custom_task_runners);
  if (state->engine == nullptr) {
    return nullptr;
  }

  state->window_wrapper = std::make_unique<FlutterDesktopWindow>();
  state->window_wrapper->window = window;

  // Trigger an initial size callback to send size information to Flutter.
  state->monitor_screen_coordinates_per_inch = GetScreenCoordinatesPerInch();
  int width_px, height_px;
#ifdef USE_GLFW_WINDOW
  glfwGetFramebufferSize(window, &width_px, &height_px);
  initial_width = width_px;
  initial_height = height_px;
#else
  glfwGetWindowSize(window, &width_px, &height_px);
#endif
  GLFWFramebufferSizeCallback(window, width_px, height_px);

  // Set up GLFW callbacks for the window.
  glfwSetFramebufferSizeCallback(window, GLFWFramebufferSizeCallback);
  glfwSetWindowRefreshCallback(window, GLFWWindowRefreshCallback);
  GLFWAssignEventCallbacks(window);

  return state.release();
}

void FlutterDesktopDestroyWindow(FlutterDesktopWindowControllerRef controller) {
  FlutterEngineShutdown(controller->engine);
  delete controller;
}

void FlutterDesktopSetIdleCallback(FlutterDesktopWindowControllerRef controller,
                                   const IdleCallback& idleCallback) {
  FlutterEngineSetIdleNotificationCallback(controller->engine, idleCallback);
}

void FlutterDesktopAddKeyboardHookHandler(FlutterDesktopWindowControllerRef controller,
                                          std::unique_ptr<flutter::KeyboardHookHandler> keyboardHookHandler) {
  controller->keyboard_hook_handlers.push_back(std::move(keyboardHookHandler));
}

FLUTTER_EXPORT void FlutterDesktopSetClipboardData(
    FlutterDesktopWindowControllerRef controller, const char* data) {
  GLFWwindow* window = FlutterDesktopGetWindow(controller)->window;
  glfwSetClipboardString(window, data);
}

FLUTTER_EXPORT const char* FlutterDesktopGetClipboardData(
    FlutterDesktopWindowControllerRef controller) {
  GLFWwindow* window = FlutterDesktopGetWindow(controller)->window;
  auto result = glfwGetClipboardString(window);
  return result == NULL ? "" : result;
}

void FlutterDesktopGetFramebufferSize(FlutterDesktopWindowRef flutter_window,
                                  int* width,
                                  int* height) {
  glfwGetFramebufferSize(flutter_window->window, width, height);
}

void FlutterDesktopWindowSetHoverEnabled(FlutterDesktopWindowRef flutter_window,
                                         bool enabled) {
  flutter_window->hover_tracking_enabled = enabled;
  SetHoverCallbacksEnabled(flutter_window->window, enabled);
}

void FlutterDesktopWindowSetTitle(FlutterDesktopWindowRef flutter_window,
                                  const char* title) {
  GLFWwindow* window = flutter_window->window;
  glfwSetWindowTitle(window, title);
}

void FlutterDesktopWindowSetIcon(FlutterDesktopWindowRef flutter_window,
                                 uint8_t* pixel_data,
                                 int width,
                                 int height) {
  GLFWimage image = {width, height, static_cast<unsigned char*>(pixel_data)};
  glfwSetWindowIcon(flutter_window->window, pixel_data ? 1 : 0, &image);
}

void FlutterDesktopWindowGetFrame(FlutterDesktopWindowRef flutter_window,
                                  int* x,
                                  int* y,
                                  int* width,
                                  int* height) {
  glfwGetWindowPos(flutter_window->window, x, y);
  glfwGetWindowSize(flutter_window->window, width, height);
  // The above gives content area size and position; adjust for the window
  // decoration to give actual window frame.
  int frame_left, frame_top, frame_right, frame_bottom;
  glfwGetWindowFrameSize(flutter_window->window, &frame_left, &frame_top,
                         &frame_right, &frame_bottom);
  if (x) {
    *x -= frame_left;
  }
  if (y) {
    *y -= frame_top;
  }
  if (width) {
    *width += frame_left + frame_right;
  }
  if (height) {
    *height += frame_top + frame_bottom;
  }
}

void FlutterDesktopWindowSetFrame(FlutterDesktopWindowRef flutter_window,
                                  int x,
                                  int y,
                                  int width,
                                  int height) {
  // Get the window decoration sizes to adjust, since the GLFW setters take
  // content position and size.
  int frame_left, frame_top, frame_right, frame_bottom;
  glfwGetWindowFrameSize(flutter_window->window, &frame_left, &frame_top,
                         &frame_right, &frame_bottom);
  glfwSetWindowPos(flutter_window->window, x + frame_left, y + frame_top);
  glfwSetWindowSize(flutter_window->window, width - frame_left - frame_right,
                    height - frame_top - frame_bottom);
}

void FlutterDesktopGetWindowSize(FlutterDesktopWindowRef flutter_window,
                                 int* width,
                                 int* height) {
  glfwGetWindowSize(flutter_window->window, width, height);
}

void FlutterDesktopSetWindowSize(FlutterDesktopWindowRef flutter_window,
                                 int& width,
                                 int& height) {
  glfwSetWindowSize(flutter_window->window, width, height);
#ifdef USE_GLFW_WINDOW
  int width_px, height_px;
  glfwGetFramebufferSize(flutter_window->window, &width_px, &height_px);
  width = width_px;
  height = height_px;
#endif
  GLFWFramebufferSizeCallback(flutter_window->window, width, height);

  // Set up GLFW callbacks for the window.
  glfwSetFramebufferSizeCallback(flutter_window->window, GLFWFramebufferSizeCallback);
  glfwSetWindowRefreshCallback(flutter_window->window, GLFWWindowRefreshCallback);
  GLFWAssignEventCallbacks(flutter_window->window);
}

double FlutterDesktopWindowGetScaleFactor(
    FlutterDesktopWindowRef flutter_window) {
  return flutter_window->pixels_per_screen_coordinate;
}

void FlutterDesktopRunWindowLoop(FlutterDesktopWindowControllerRef controller) {
  GLFWwindow* window = controller->window.get();
  while (!glfwWindowShouldClose(window)) {
    auto wait_duration = std::chrono::milliseconds::max();
    controller->event_loop->WaitForEvents(wait_duration);
  }
  FlutterDesktopDestroyWindow(controller);
}

void FlutterDesktopWaitForEvents(FlutterDesktopWindowControllerRef controller) {
  auto wait_duration = std::chrono::milliseconds::max();
  controller->event_loop->WaitForEvents(wait_duration);
}

bool FlutterDesktopWindowShouldClose(FlutterDesktopWindowControllerRef controller) {
  GLFWwindow* window = controller->window.get();
  return glfwWindowShouldClose(window);
}

FlutterDesktopWindowRef FlutterDesktopGetWindow(
    FlutterDesktopWindowControllerRef controller) {
  // Currently, one registrar acts as the registrar for all plugins, so the
  // name is ignored. It is part of the API to reduce churn in the future when
  // aligning more closely with the Flutter registrar system.
  return controller->window_wrapper.get();
}

FlutterDesktopEngineRef FlutterDesktopRunEngine() {
  auto engine =
      RunFlutterEngine(nullptr, nullptr, nullptr /* custom task runners */);
  if (engine == nullptr) {
    return nullptr;
  }
  auto engine_state = new FlutterDesktopEngineState();
  engine_state->engine = engine;
  return engine_state;
}

bool FlutterDesktopShutDownEngine(FlutterDesktopEngineRef engine_ref) {
  std::cout << "Shutting down flutter engine process." << std::endl;
  auto result = FlutterEngineShutdown(engine_ref->engine);
  delete engine_ref;
  return (result == kSuccess);
}
