/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#include "perfetto/base/logging.h"
#include "perfetto/ext/base/file_utils.h"
#include "perfetto/ext/base/scoped_file.h"
#include "perfetto/ext/base/string_splitter.h"
#include "perfetto/ext/base/utils.h"
#include "perfetto/profiling/deobfuscator.h"
#include "perfetto/trace_processor/trace_processor.h"
#include "tools/trace_to_text/deobfuscate_profile.h"
#include "tools/trace_to_text/utils.h"

namespace perfetto {
namespace trace_to_text {
namespace {

bool ParseFile(profiling::ProguardParser* p, FILE* f) {
  std::string contents;
  PERFETTO_CHECK(base::ReadFileStream(f, &contents));
  for (base::StringSplitter lines(std::move(contents), '\n'); lines.Next();) {
    if (!p->AddLine(lines.cur_token()))
      return false;
  }
  return true;
}

}  // namespace

int DeobfuscateProfile(std::istream* input, std::ostream* output) {
  base::ignore_result(input);
  base::ignore_result(output);
  auto maybe_map = GetPerfettoProguardMapPath();
  if (!maybe_map) {
    PERFETTO_ELOG("No PERFETTO_PROGUARD_MAP specified.");
    return 1;
  }
  for (const ProguardMap& map : *maybe_map) {
    const char* filename = map.filename.c_str();
    base::ScopedFstream f(fopen(filename, "r"));
    if (!f) {
      PERFETTO_ELOG("Failed to open %s", filename);
      return 1;
    }
    profiling::ProguardParser parser;
    if (!ParseFile(&parser, *f)) {
      PERFETTO_ELOG("Failed to parse %s", filename);
      return 1;
    }
    std::map<std::string, profiling::ObfuscatedClass> obfuscation_map =
        parser.ConsumeMapping();

    // TODO(fmayer): right now, we don't use the profile we are given. We can
    // filter the output to only contain the classes actually seen in the
    // profile.
    MakeDeobfuscationPackets(map.package, obfuscation_map,
                             [output](const std::string& packet_proto) {
                               WriteTracePacket(packet_proto, output);
                             });
  }
  return 0;
}

}  // namespace trace_to_text
}  // namespace perfetto
