#define TYPE_MODULE evmod_assets
#include <evol/meta/type_import.h>
#include <evol/common/ev_log.h>

#include "../LoaderCommon.h"
#include "TextLoader.h"

TextAsset
ev_textloader_loadasset(
    AssetHandle handle)
{
  const AssetStruct *asset = ev_asset_getfromhandle(handle);
  TextAsset inter = (TextAsset) {
    .text = evstring_new(asset->data),
  };

  ev_asset_markas(handle, LoaderData.assetType, &inter);

  return inter;
}

void
ev_textloader_textasset_destr(
    TextAsset txt)
{
  evstring_free(txt.text);
}

void
ev_textloader_setassettype(
    GenericHandle type)
{
  LoaderData.assetType = type;
}
