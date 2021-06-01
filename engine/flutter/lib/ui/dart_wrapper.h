// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_DART_WRAPPER_H_
#define FLUTTER_LIB_UI_DART_WRAPPER_H_

#include <initializer_list>
#include <vector>

#include "flutter/fml/logging.h"
#include "flutter/fml/memory/ref_counted.h"

using Dart_Handle = const void*;
Dart_Handle Dart_Null();

class Dart_NativeArguments {
};

using Dart_NativeFunction = void (*)(Dart_NativeArguments);

#define DEFINE_WRAPPERTYPEINFO()
#define DART_BIND_ALL(a,b)
#define IMPLEMENT_WRAPPERTYPEINFO(a,b)

#define DartCallConstructor(a,b)

#define DART_NATIVE_CALLBACK(a,b)

#define DART_REGISTER_NATIVE(a,b)

#define Dart_NewTypedData(a,b) (b)
namespace tonic {

using Float32List = std::vector<float>;
using Float64List = std::vector<double>;
using Int32List = std::vector<int>;
using Uint16List = std::vector<unsigned short>;
using Uint8List = std::vector<unsigned char>;

class DartWrappable {
  public:
    virtual size_t GetAllocationSize();
    void ClearDartWrapper() {}
  private:
    virtual ~DartWrappable() {};
};

}

namespace flutter {

template <typename T>
class RefCountedDartWrappable : public fml::RefCountedThreadSafe<T> {
 public:
  virtual ~RefCountedDartWrappable() {}
  virtual size_t GetAllocationSize() { return 0;}
  void ClearDartWrapper() {}
};

}  // namespace flutter

namespace tonic {
class DartLibraryNatives {
 public:
  struct Entry {
    const char* symbol;
    Dart_NativeFunction func;
    int argument_count;
    bool auto_setup;
  };
  void Register(std::initializer_list<Entry> entries);
};

class DartPersistentValue {
};

template <typename T, typename Enable = void>
struct DartConverter {};

Dart_Handle ToDart(const char*msg);
Dart_Handle ToDart(int);

template <typename T>
struct DartListFactory {
};

}// namespace tonic

void Dart_ThrowException(Dart_Handle handle);

using Dart_CoreType_Id = enum {
  Dart_CoreType_Dynamic,
  Dart_CoreType_Int,
  Dart_CoreType_String
};

Dart_Handle Dart_NewListOf(Dart_CoreType_Id id, intptr_t len);

Dart_Handle Dart_ListSetAt(Dart_Handle list, intptr_t index, Dart_Handle value);

Dart_Handle Dart_NewListOfType(Dart_Handle type, intptr_t len);

bool LogIfError(Dart_Handle type);

#define FLUTTER_LIB_UI_ISOLATE_NAME_SERVER_H_

#endif  // FLUTTER_LIB_UI_DART_WRAPPER_H_
