// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cassert>

namespace core {

// TODO: replace assert with some better way

#ifdef DEBUG
#define dcheck(exp) assert(exp);
#else
#define dcheck(exp)
#endif

}  // namespace core
