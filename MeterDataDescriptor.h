#pragma once
#include "IHandleValue.h"



struct MeterDataDescriptor
{
  const char *Name;
  const char *Code;
  IHandleValue *HandleValue;
  unsigned Length;
  long Value;
  long DecimalValue;
  bool Send;
  char *ValueString;
};