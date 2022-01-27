// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// 2021.2.10 Framework adapted to ACE.
//           Copyright (c) 2021 Huawei Device Co., Ltd. All rights reserved.

#include "flutter/shell/platform/android/platform_view_android.h"

#include <memory>
#include <utility>

#include "flutter/common/settings.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/shell/common/shell_io_manager.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "flutter/shell/platform/android/android_external_texture_gl.h"
#include "flutter/shell/platform/android/android_surface_gl.h"
#include "flutter/shell/platform/android/platform_message_response_android.h"
#include "flutter/shell/platform/android/platform_view_android_jni.h"
#include "flutter/shell/platform/android/vsync_waiter_android.h"
#if defined(OHOS_PLATFORM) && !defined(ENABLE_NATIVE_VIEW)
#include "flutter/shell/platform/ohos/vsync_waiter_embedder.h"
#endif

#include "flutter/fml/platform/android/jni_util.h"

namespace flutter {

namespace {

  bool CheckException(JNIEnv* env) {
    if (env->ExceptionCheck() == JNI_FALSE)
      return true;

    jthrowable exception = env->ExceptionOccurred();
    env->ExceptionClear();
    FML_LOG(ERROR) << fml::jni::GetJavaExceptionInfo(env, exception);
    env->DeleteLocalRef(exception);
    return false;
  }

}  // anonymous namespace


void FlutterViewHandlePlatformMessage(JNIEnv* env,
                                      jobject obj,
                                      jstring channel,
                                      jobject message,
                                      jint responseId) {}

static jmethodID g_on_first_frame_method = nullptr;
void FlutterViewOnFirstFrame(JNIEnv* env, jobject obj) {
  if (g_on_first_frame_method) {
    env->CallVoidMethod(obj, g_on_first_frame_method);
    FML_CHECK(CheckException(env));
  }
}

void FlutterViewOnPreEngineRestart(JNIEnv* env, jobject obj) {}

void FlutterViewHandlePlatformMessageResponse(JNIEnv* env,
                                              jobject obj,
                                              jint responseId,
                                              jobject response) {}

static fml::jni::ScopedJavaGlobalRef<jclass>* g_surface_texture_class = nullptr;

static jmethodID g_attach_to_gl_context_method = nullptr;
void SurfaceTextureAttachToGLContext(JNIEnv* env, jobject obj, jint textureId) {
  env->CallVoidMethod(obj, g_attach_to_gl_context_method, textureId);
  FML_CHECK(CheckException(env));
}

static jmethodID g_update_tex_image_method = nullptr;
void SurfaceTextureUpdateTexImage(JNIEnv* env, jobject obj) {
  env->CallVoidMethod(obj, g_update_tex_image_method);
  FML_CHECK(CheckException(env));
}

static jmethodID g_get_transform_matrix_method = nullptr;
void SurfaceTextureGetTransformMatrix(JNIEnv* env,
                                      jobject obj,
                                      jfloatArray result) {
  env->CallVoidMethod(obj, g_get_transform_matrix_method, result);
  FML_CHECK(CheckException(env));
}

static jmethodID g_detach_from_gl_context_method = nullptr;
void SurfaceTextureDetachFromGLContext(JNIEnv* env, jobject obj) {
  env->CallVoidMethod(obj, g_detach_from_gl_context_method);
  FML_CHECK(CheckException(env));
}



PlatformViewAndroid::PlatformViewAndroid(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    fml::jni::JavaObjectWeakGlobalRef java_object,
    bool use_software_rendering)
    : PlatformView(delegate, std::move(task_runners)),
      java_object_(java_object),
      android_surface_(AndroidSurface::Create(use_software_rendering)) {
  FML_CHECK(android_surface_)
      << "Could not create an OpenGL, Vulkan or Software surface to setup "
         "rendering.";
}

PlatformViewAndroid::PlatformViewAndroid(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    fml::jni::JavaObjectWeakGlobalRef java_object)
    : PlatformView(delegate, std::move(task_runners)),
      java_object_(java_object),
      android_surface_(nullptr) {}

PlatformViewAndroid::~PlatformViewAndroid() = default;

void PlatformViewAndroid::NotifyCreated(
    fml::RefPtr<AndroidNativeWindow> native_window) {
  if (android_surface_) {
    InstallFirstFrameCallback();

    fml::AutoResetWaitableEvent latch;
    fml::TaskRunner::RunNowOrPostTask(
        task_runners_.GetGPUTaskRunner(),
        [&latch, surface = android_surface_.get(),
         native_window = std::move(native_window)]() {
          surface->SetNativeWindow(native_window);
          latch.Signal();
        });
    latch.Wait();
  }

  PlatformView::NotifyCreated();
}

void PlatformViewAndroid::NotifyDestroyed() {
  PlatformView::NotifyDestroyed();

  if (android_surface_) {
    fml::AutoResetWaitableEvent latch;
    fml::TaskRunner::RunNowOrPostTask(
        task_runners_.GetGPUTaskRunner(),
        [&latch, surface = android_surface_.get()]() {
          surface->TeardownOnScreenContext();
          latch.Signal();
        });
    latch.Wait();
  }
}

void PlatformViewAndroid::NotifyChanged(const SkISize& size) {
  if (!android_surface_) {
    return;
  }
  fml::AutoResetWaitableEvent latch;
  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetGPUTaskRunner(),  //
      [&latch, surface = android_surface_.get(), size]() {
        surface->OnScreenSurfaceResize(size);
        latch.Signal();
      });
  latch.Wait();
}

void PlatformViewAndroid::DispatchPlatformMessage(JNIEnv* env,
                                                  std::string name,
                                                  jobject java_message_data,
                                                  jint java_message_position,
                                                  jint response_id) {
  uint8_t* message_data =
      static_cast<uint8_t*>(env->GetDirectBufferAddress(java_message_data));
  std::vector<uint8_t> message =
      std::vector<uint8_t>(message_data, message_data + java_message_position);

  fml::RefPtr<flutter::PlatformMessageResponse> response;
  if (response_id) {
    response = fml::MakeRefCounted<PlatformMessageResponseAndroid>(
        response_id, java_object_, task_runners_.GetPlatformTaskRunner());
  }

  PlatformView::DispatchPlatformMessage(
      fml::MakeRefCounted<flutter::PlatformMessage>(
          std::move(name), std::move(message), std::move(response)));
}

void PlatformViewAndroid::DispatchEmptyPlatformMessage(JNIEnv* env,
                                                       std::string name,
                                                       jint response_id) {
  fml::RefPtr<flutter::PlatformMessageResponse> response;
  if (response_id) {
    response = fml::MakeRefCounted<PlatformMessageResponseAndroid>(
        response_id, java_object_, task_runners_.GetPlatformTaskRunner());
  }

  PlatformView::DispatchPlatformMessage(
      fml::MakeRefCounted<flutter::PlatformMessage>(std::move(name),
                                                    std::move(response)));
}

void PlatformViewAndroid::InvokePlatformMessageResponseCallback(
    JNIEnv* env,
    jint response_id,
    jobject java_response_data,
    jint java_response_position) {
  if (!response_id)
    return;
  auto it = pending_responses_.find(response_id);
  if (it == pending_responses_.end())
    return;
  uint8_t* response_data =
      static_cast<uint8_t*>(env->GetDirectBufferAddress(java_response_data));
  std::vector<uint8_t> response = std::vector<uint8_t>(
      response_data, response_data + java_response_position);
  auto message_response = std::move(it->second);
  pending_responses_.erase(it);
  message_response->Complete(
      std::make_unique<fml::DataMapping>(std::move(response)));
}

void PlatformViewAndroid::InvokePlatformMessageEmptyResponseCallback(
    JNIEnv* env,
    jint response_id) {
  if (!response_id)
    return;
  auto it = pending_responses_.find(response_id);
  if (it == pending_responses_.end())
    return;
  auto message_response = std::move(it->second);
  pending_responses_.erase(it);
  message_response->CompleteEmpty();
}

// |PlatformView|
void PlatformViewAndroid::HandlePlatformMessage(
    fml::RefPtr<flutter::PlatformMessage> message) {
  JNIEnv* env = fml::jni::AttachCurrentThread();
  fml::jni::ScopedJavaLocalRef<jobject> view = java_object_.get(env);
  if (view.is_null())
    return;

  int response_id = 0;
  if (auto response = message->response()) {
    response_id = next_response_id_++;
    pending_responses_[response_id] = response;
  }
  auto java_channel = fml::jni::StringToJavaString(env, message->channel());
  if (message->hasData()) {
    fml::jni::ScopedJavaLocalRef<jbyteArray> message_array(
        env, env->NewByteArray(message->data().size()));
    env->SetByteArrayRegion(
        message_array.obj(), 0, message->data().size(),
        reinterpret_cast<const jbyte*>(message->data().data()));
    message = nullptr;

    // This call can re-enter in InvokePlatformMessageXxxResponseCallback.
    FlutterViewHandlePlatformMessage(env, view.obj(), java_channel.obj(),
                                     message_array.obj(), response_id);
  } else {
    message = nullptr;

    // This call can re-enter in InvokePlatformMessageXxxResponseCallback.
    FlutterViewHandlePlatformMessage(env, view.obj(), java_channel.obj(),
                                     nullptr, response_id);
  }
}

// |PlatformView|
void PlatformViewAndroid::OnPreEngineRestart() const {
  JNIEnv* env = fml::jni::AttachCurrentThread();
  fml::jni::ScopedJavaLocalRef<jobject> view = java_object_.get(env);
  if (view.is_null()) {
    // The Java object died.
    return;
  }
  FlutterViewOnPreEngineRestart(fml::jni::AttachCurrentThread(), view.obj());
}

void PlatformViewAndroid::RegisterExternalTexture(
    int64_t texture_id,
    const fml::jni::JavaObjectWeakGlobalRef& surface_texture) {
  RegisterTexture(
      std::make_shared<AndroidExternalTextureGL>(texture_id, surface_texture));
}

// |PlatformView|
std::unique_ptr<VsyncWaiter> PlatformViewAndroid::CreateVSyncWaiter(int32_t platform) {
  if (platform == static_cast<int32_t>(AcePlatform::ACE_PLATFORM_ANDROID)) {
    return VsyncWaiterAndroid::Create(task_runners_);
  }
#if defined(OHOS_PLATFORM) && !defined(ENABLE_NATIVE_VIEW)
  if (platform == static_cast<int32_t>(AcePlatform::ACE_PLATFORM_OHOS)) {
    return VsyncWaiterEmbedder::Create(task_runners_);
  }
#endif
  return nullptr;
}

// |PlatformView|
std::unique_ptr<Surface> PlatformViewAndroid::CreateRenderingSurface() {
  if (!android_surface_) {
    return nullptr;
  }
  return android_surface_->CreateGPUSurface();
}

// |PlatformView|
sk_sp<GrContext> PlatformViewAndroid::CreateResourceContext() const {
  if (!android_surface_) {
    return nullptr;
  }
  sk_sp<GrContext> resource_context;
  if (android_surface_->ResourceContextMakeCurrent()) {
    // TODO(chinmaygarde): Currently, this code depends on the fact that only
    // the OpenGL surface will be able to make a resource context current. If
    // this changes, this assumption breaks. Handle the same.
    resource_context = ShellIOManager::CreateCompatibleResourceLoadingContext(
        GrBackend::kOpenGL_GrBackend,
        GPUSurfaceGLDelegate::GetDefaultPlatformGLInterface());
  } else {
    FML_DLOG(ERROR) << "Could not make the resource context current.";
  }

  return resource_context;
}

// |PlatformView|
void PlatformViewAndroid::ReleaseResourceContext() const {
  if (android_surface_) {
    android_surface_->ResourceContextClearCurrent();
  }
}

void PlatformViewAndroid::InstallFirstFrameCallback() {
  // On Platform Task Runner.
  SetNextFrameCallback(
      [platform_view = GetWeakPtr(),
       platform_task_runner = task_runners_.GetPlatformTaskRunner()]() {
        // On GPU Task Runner.
        platform_task_runner->PostTask([platform_view]() {
          // Back on Platform Task Runner.
          if (platform_view) {
            reinterpret_cast<PlatformViewAndroid*>(platform_view.get())
                ->FireFirstFrameCallback();
          }
        });
      });
}

void PlatformViewAndroid::FireFirstFrameCallback() {
  JNIEnv* env = fml::jni::AttachCurrentThread();
  fml::jni::ScopedJavaLocalRef<jobject> view = java_object_.get(env);
  if (view.is_null()) {
    // The Java object died.
    return;
  }
  FlutterViewOnFirstFrame(fml::jni::AttachCurrentThread(), view.obj());
}

bool PlatformViewAndroid::RegisterOnFirstFrame(JNIEnv* env, jint platform) {
  jclass view_clazz = nullptr;
  if (platform == static_cast<jint>(AcePlatform::ACE_PLATFORM_ANDROID)) {
    view_clazz = env->FindClass("ohos/ace/adapter/AceViewAosp");
  } else if (platform == static_cast<jint>(AcePlatform::ACE_PLATFORM_OHOS)) {
    view_clazz = env->FindClass("ohos/ace/AceViewOhos");
  }
  if (view_clazz == nullptr) {
    FML_LOG(ERROR) << "Could not find AceView class";
    return false;
  }
  g_on_first_frame_method = env->GetMethodID(view_clazz, "onFirstFrame", "()V");
  if (g_on_first_frame_method == nullptr) {
    FML_LOG(ERROR) << "Could not locate onFirstFrame method";
    return false;
  }
  return true;
}

bool PlatformViewAndroid::Register(JNIEnv* env) {
  g_surface_texture_class = new fml::jni::ScopedJavaGlobalRef<jclass>(
      env, env->FindClass("android/graphics/SurfaceTexture"));
  if (g_surface_texture_class->is_null()) {
    FML_LOG(ERROR) << "Could not locate SurfaceTexture class";
    return false;
  }

  g_attach_to_gl_context_method = env->GetMethodID(
      g_surface_texture_class->obj(), "attachToGLContext", "(I)V");

  if (g_attach_to_gl_context_method == nullptr) {
    FML_LOG(ERROR) << "Could not locate attachToGlContext method";
    return false;
  }

  g_update_tex_image_method =
      env->GetMethodID(g_surface_texture_class->obj(), "updateTexImage", "()V");

  if (g_update_tex_image_method == nullptr) {
    FML_LOG(ERROR) << "Could not locate updateTexImage method";
    return false;
  }

  g_get_transform_matrix_method = env->GetMethodID(
      g_surface_texture_class->obj(), "getTransformMatrix", "([F)V");

  if (g_get_transform_matrix_method == nullptr) {
    FML_LOG(ERROR) << "Could not locate getTransformMatrix method";
    return false;
  }

  g_detach_from_gl_context_method = env->GetMethodID(
      g_surface_texture_class->obj(), "detachFromGLContext", "()V");

  if (g_detach_from_gl_context_method == nullptr) {
    FML_LOG(ERROR) << "Could not locate detachFromGlContext method";
    return false;
  }

  return true;
}

}  // namespace flutter
