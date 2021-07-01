#pragma once

#include <evol/common/ev_types.h>

typedef struct {
  PTR data;
  I64 size;

  U32 ref_count;
  U32 ticks_left;
} AssetStruct;

static struct {
  GenericHandle assetType;
} LoaderData;


const AssetStruct *
ev_asset_getfromhandle(
    AssetHandle handle);

void
ev_asset_markas(
    AssetHandle handle, 
    GenericHandle assetType, 
    PTR data);
