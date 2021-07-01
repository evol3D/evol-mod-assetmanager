#define TYPE_MODULE evmod_assets
#include <evol/meta/type_import.h>
#include <evol/common/ev_log.h>
#include <evjson.h>

#include "../LoaderCommon.h"
#include "ImageLoader.h"

EvImageFormat
strToFormat(
  evstr_ref str_ref);

ImageAsset
ev_imageloader_loadasset(
    AssetHandle handle)
{
  const AssetStruct *asset = ev_asset_getfromhandle(handle);

  uint32_t jsonLength = ((U32*)asset->data)[0];
  uint32_t blobLength = ((U32*)asset->data)[1];

  const char *json = (PTR)(&((U32*)asset->data)[2]);
  const void *data = json + jsonLength;

  evjson_t *evjs = evjs_init();

  evjs_loadjson(evjs, json);

  evstr_ref str_ref = evjs_get(evjs, "format")->as_str;

  ImageAsset inter = {
    .bufferSize = evjs_get(evjs, "buffer_size")->as_num,
    .height = evjs_get(evjs, "height")->as_num,
    .width = evjs_get(evjs, "width")->as_num,
    .format = strToFormat(str_ref),

    .data = data,
  };

  evjs_fini(evjs);

  ev_asset_markas(handle, LoaderData.assetType, &inter);

  return inter;
}

void
ev_imageloader_imageasset_destr(
    ImageAsset image)
{

}

void
ev_imageloader_setassettype(
    GenericHandle type)
{
  LoaderData.assetType = type;
}

EvImageFormat
strToFormat(
  evstr_ref str_ref)
{
  if(!strncmp(str_ref.data + str_ref.offset, "RGBA8", str_ref.len))
    return EV_IMAGEFORMAT_RGBA8;

  else
    return EV_IMAGEFORMAT_INVALID;
}
