// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "flutter/fml/native_library.h"
#include "flutter/fml/paths.h"
#include "flutter/fml/size.h"
#include "flutter/shell/version/version.h"

// Include once for the default enum definition.
#include "flutter/shell/common/switches.h"

#undef SHELL_COMMON_SWITCHES_H_

struct SwitchDesc {
  flutter::Switch sw;
  const std::string_view flag;
  const char* help;
};

#undef DEF_SWITCHES_START
#undef DEF_SWITCH
#undef DEF_SWITCHES_END

// clang-format off
#define DEF_SWITCHES_START static const struct SwitchDesc gSwitchDescs[] = {
#define DEF_SWITCH(p_swtch, p_flag, p_help) \
  { flutter::Switch:: p_swtch, p_flag, p_help },
#define DEF_SWITCHES_END };
// clang-format on

#if FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE

// List of common and safe VM flags to allow to be passed directly to the VM.
// clang-format off
static const std::string gDartFlagsWhitelist[] = {
    "--max_profile_depth",
    "--profile_period",
    "--random_seed",
    "--enable_mirrors",
};
// clang-format on

#endif

// Include again for struct definition.
#include "flutter/shell/common/switches.h"

// Define symbols for the ICU data that is linked into the Flutter library on
// Android.  This is a workaround for crashes seen when doing dynamic lookups
// of the engine's own symbols on some older versions of Android.
#if OS_ANDROID
extern uint8_t _binary_icudtl_dat_start[];
extern uint8_t _binary_icudtl_dat_end[];

static std::unique_ptr<fml::Mapping> GetICUStaticMapping() {
  return std::make_unique<fml::NonOwnedMapping>(
      _binary_icudtl_dat_start,
      _binary_icudtl_dat_end - _binary_icudtl_dat_start);
}
#endif

namespace flutter {

void PrintUsage(const std::string& executable_name) {
  std::cerr << std::endl << "  " << executable_name << std::endl << std::endl;

  std::cerr << "Versions: " << std::endl << std::endl;

  std::cerr << "Flutter Engine Version: " << GetFlutterEngineVersion()
            << std::endl;
  std::cerr << "Skia Version: " << GetSkiaVersion() << std::endl;

  std::cerr << "Dart Version: " << GetDartVersion() << std::endl << std::endl;

  std::cerr << "Available Flags:" << std::endl;

  const uint32_t column_width = 80;

  const uint32_t flags_count = static_cast<uint32_t>(Switch::Sentinel);

  uint32_t max_width = 2;
  for (uint32_t i = 0; i < flags_count; i++) {
    auto desc = gSwitchDescs[i];
    max_width = std::max<uint32_t>(desc.flag.size() + 2, max_width);
  }

  const uint32_t help_width = column_width - max_width - 3;

  std::cerr << std::string(column_width, '-') << std::endl;
  for (uint32_t i = 0; i < flags_count; i++) {
    auto desc = gSwitchDescs[i];

    std::cerr << std::setw(max_width)
              << std::string("--") +
                     std::string{desc.flag.data(), desc.flag.size()}
              << " : ";

    std::istringstream stream(desc.help);
    int32_t remaining = help_width;

    std::string word;
    while (stream >> word && remaining > 0) {
      remaining -= (word.size() + 1);
      if (remaining <= 0) {
        std::cerr << std::endl
                  << std::string(max_width, ' ') << "   " << word << " ";
        remaining = help_width;
      } else {
        std::cerr << word << " ";
      }
    }

    std::cerr << std::endl;
  }
  std::cerr << std::string(column_width, '-') << std::endl;
}

const std::string_view FlagForSwitch(Switch swtch) {
  for (uint32_t i = 0; i < static_cast<uint32_t>(Switch::Sentinel); i++) {
    if (gSwitchDescs[i].sw == swtch) {
      return gSwitchDescs[i].flag;
    }
  }
  return std::string_view();
}

#if FLUTTER_RUNTIME_MODE != FLUTTER_RUNTIME_MODE_RELEASE

static bool IsWhitelistedDartVMFlag(const std::string& flag) {
  for (uint32_t i = 0; i < fml::size(gDartFlagsWhitelist); ++i) {
    const std::string& allowed = gDartFlagsWhitelist[i];
    // Check that the prefix of the flag matches one of the whitelisted flags.
    // We don't need to worry about cases like "--safe --sneaky_dangerous" as
    // the VM will discard these as a single unrecognized flag.
    if (std::equal(allowed.begin(), allowed.end(), flag.begin())) {
      return true;
    }
  }
  return false;
}

#endif

template <typename T>
static bool GetSwitchValue(const fml::CommandLine& command_line,
                           Switch sw,
                           T* result) {
  std::string switch_string;

  if (!command_line.GetOptionValue(FlagForSwitch(sw), &switch_string)) {
    return false;
  }

  std::stringstream stream(switch_string);
  T value = 0;
  if (stream >> value) {
    *result = value;
    return true;
  }

  return false;
}

std::unique_ptr<fml::Mapping> GetSymbolMapping(std::string symbol_prefix,
                                               std::string native_lib_path) {
  const uint8_t* mapping;
  intptr_t size;

  auto lookup_symbol = [&mapping, &size, symbol_prefix](
                           const fml::RefPtr<fml::NativeLibrary>& library) {
    mapping = library->ResolveSymbol((symbol_prefix + "_start").c_str());
    size = reinterpret_cast<intptr_t>(
        library->ResolveSymbol((symbol_prefix + "_size").c_str()));
  };

  fml::RefPtr<fml::NativeLibrary> library =
      fml::NativeLibrary::CreateForCurrentProcess();
  lookup_symbol(library);

  if (!(mapping && size)) {
    // Symbol lookup for the current process fails on some devices.  As a
    // fallback, try doing the lookup based on the path to the Flutter library.
    library = fml::NativeLibrary::Create(native_lib_path.c_str());
    lookup_symbol(library);
  }

  FML_CHECK(mapping && size) << "Unable to resolve symbols: " << symbol_prefix;
  return std::make_unique<fml::NonOwnedMapping>(mapping, size);
}

Settings SettingsFromCommandLine(const fml::CommandLine& command_line) {
  Settings settings = {};

  settings.enable_software_rendering =
      command_line.HasOption(FlagForSwitch(Switch::EnableSoftwareRendering));

  settings.skia_deterministic_rendering_on_cpu =
      command_line.HasOption(FlagForSwitch(Switch::SkiaDeterministicRendering));

  settings.verbose_logging =
      command_line.HasOption(FlagForSwitch(Switch::VerboseLogging));

  command_line.GetOptionValue(FlagForSwitch(Switch::FlutterAssetsDir),
                              &settings.assets_path);

  std::vector<std::string_view> aot_shared_library_name =
      command_line.GetOptionValues(FlagForSwitch(Switch::AotSharedLibraryName));

  std::string snapshot_asset_path;
  command_line.GetOptionValue(FlagForSwitch(Switch::SnapshotAssetPath),
                              &snapshot_asset_path);

  std::string vm_snapshot_data_filename;
  command_line.GetOptionValue(FlagForSwitch(Switch::VmSnapshotData),
                              &vm_snapshot_data_filename);

  std::string vm_snapshot_instr_filename;
  command_line.GetOptionValue(FlagForSwitch(Switch::VmSnapshotInstructions),
                              &vm_snapshot_instr_filename);

  std::string isolate_snapshot_data_filename;
  command_line.GetOptionValue(FlagForSwitch(Switch::IsolateSnapshotData),
                              &isolate_snapshot_data_filename);

  std::string isolate_snapshot_instr_filename;
  command_line.GetOptionValue(
      FlagForSwitch(Switch::IsolateSnapshotInstructions),
      &isolate_snapshot_instr_filename);

  if (settings.icu_initialization_required) {
    command_line.GetOptionValue(FlagForSwitch(Switch::ICUDataFilePath),
                                &settings.icu_data_path);
    if (command_line.HasOption(FlagForSwitch(Switch::ICUSymbolPrefix))) {
      std::string icu_symbol_prefix, native_lib_path;
      command_line.GetOptionValue(FlagForSwitch(Switch::ICUSymbolPrefix),
                                  &icu_symbol_prefix);
      command_line.GetOptionValue(FlagForSwitch(Switch::ICUNativeLibPath),
                                  &native_lib_path);

#if OS_ANDROID
      settings.icu_mapper = GetICUStaticMapping;
#else
      settings.icu_mapper = [icu_symbol_prefix, native_lib_path] {
        return GetSymbolMapping(icu_symbol_prefix, native_lib_path);
      };
#endif
    }
  }

  settings.use_test_fonts =
      command_line.HasOption(FlagForSwitch(Switch::UseTestFonts));

  return settings;
}

}  // namespace flutter
