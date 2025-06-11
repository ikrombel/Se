#pragma once

#include <Se/Value.h>

namespace Se
{

using JSONValue = Value;

/// JSON array type.
using JSONArray = Value::Array;
/// JSON object type.
using JSONObject = Value::Object;
/// JSON object iterator.
using JSONObjectIterator = Value::ObjectIterator;
/// Constant JSON object iterator.
using ConstJSONObjectIterator = Value::ConstObjectIterator;

}
