// Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include "executeinfo.h"

namespace search::queryeval {

const ExecuteInfo ExecuteInfo::TRUE(true, 1.0);
const ExecuteInfo ExecuteInfo::FALSE(true, 1.0);

ExecuteInfo
ExecuteInfo::create(bool strict) {
    return ExecuteInfo(strict, 1.0);
}

}
