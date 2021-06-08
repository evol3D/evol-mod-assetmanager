#define TYPE_MODULE evmod_assets
#include <evol/meta/type_import.h>
#include <evol/common/ev_log.h>
#include <evjson.h>

#include "../LoaderCommon.h"
#include "MeshLoader.h"

typedef struct {
  Vec4 pos;
  Vec4 nor;
  Vec4 col;
  Vec4 uv;
} Vertex;

MeshAsset
ev_meshloader_loadasset(
    AssetHandle handle)
{
  const Asset *asset = ev_asset_getfromhandle(handle);

  uint32_t jsonLength = ((U32*)asset->data)[0];
  uint32_t blobLength = ((U32*)asset->data)[1];

  const char *json = (PTR)(&((U32*)asset->data)[2]);
  const void *data = json + jsonLength;

  evjson_t *evjs = evjs_init();

  evjs_loadjson(evjs, json);

  MeshAsset inter = {
    .vertexBuferSize = evjs_get(evjs, "vertex_buffer_size")->as_num,
    .vertexCount = evjs_get(evjs, "vertex_count")->as_num,
    .vertexData = data,

    .indexBuferSize = evjs_get(evjs, "index_buffer_size")->as_num,
    .indexCount = evjs_get(evjs, "index_count")->as_num,
    .indexData = (PTR)(&((char*)data)[inter.vertexBuferSize]),
  };

  evjs_fini(evjs);

  ev_asset_markas(handle, LoaderData.assetType, &inter);

  return inter;
}

void
ev_meshloader_textasset_destr(
    MeshAsset mesh)
{

}

void
ev_meshloader_setassettype(
    GenericHandle type)
{
  LoaderData.assetType = type;
}
