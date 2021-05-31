#pragma once

#include <evol/common/ev_types.h>

typedef struct {
  PTR data;
  I64 size;

  U32 ref_count;
  U32 ticks_left;
} Asset;


const Asset *
ev_asset_getfromhandle(
    AssetHandle handle);
