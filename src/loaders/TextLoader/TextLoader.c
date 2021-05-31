#define TYPE_MODULE evmod_assets
#include <evol/meta/type_import.h>

#include "../LoaderCommon.h"
#include "TextLoader.h"

TextAsset
ev_textloader_loadasset(
    AssetHandle handle)
{
  const Asset *asset = ev_asset_getfromhandle(handle);
  return (TextAsset) {
    .text = asset->data,
    .length = asset->size
  };
}
