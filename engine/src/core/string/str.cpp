#include "./str.h"
#include "./cstring.h"

namespace ns {
Str::Str(cstr s) : Slice<char>(Slice<char>::from_parts(s, string_length(s))) {}
} // namespace ns
