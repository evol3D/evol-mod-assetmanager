#include <evjson.h>
#define TYPE_MODULE evmod_assets
#include <evol/meta/type_import.h>
#include <evol/common/ev_log.h>

#include "../LoaderCommon.h"
#include "JSONLoader.h"

JSONAsset
ev_jsonloader_loadasset(
    AssetHandle handle)
{
  const Asset *asset = ev_asset_getfromhandle(handle);
  JSONAsset inter = (JSONAsset) {
    .json_data = evjs_init(),
  };
  evjs_loadjson(inter.json_data, asset->data);

  ev_asset_markas(handle, LoaderData.assetType, &inter);

  return inter;
}

void
ev_jsonloader_jsonasset_destr(
    JSONAsset json)
{
  evjs_fini(json.json_data);
}

void
ev_jsonloader_setassettype(
    GenericHandle type)
{
  LoaderData.assetType = type;
}
