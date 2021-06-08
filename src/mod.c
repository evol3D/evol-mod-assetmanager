#define EV_MODULE_DEFINE
#include <evol/evolmod.h>
#include <assetsys/assetsys.h>
#include <evol/utils/aligned_alloc.h>

#define IMPORT_MODULE evmod_ecs
#include <evol/meta/module_import.h>

#include "loaders/LoaderCommon.h"
#include "loaders/TextLoader/TextLoader.h"
#include "loaders/JSONLoader/JSONLoader.h"
#include "loaders/MeshLoader/MeshLoader.h"
#include "loaders/ShaderLoader/ShaderLoader.h"

#define AssetSysCheck(errmsg_fmt, vars, ...) do { \
  assetsys_error_t res = __VA_ARGS__; \
  if(res != ASSETSYS_SUCCESS) { \
    ev_log_error(errmsg_fmt"`%s` failed. Error Message: %s", EV_EXPAND(vars), EV_STRINGIZE(__VA_ARGS__), assetsys_error_strings[res]); \
  } \
} while (0)

struct {
  assetsys_t *sys;

  evolmodule_t ecs_mod;
  ECSAssetWorldHandle world;
  AssetComponentID assetcomponent_id;

} AssetManagerData = {NULL};

void
onRemoveAssetComponent(
    ECSQuery query)
{
  Asset *assets = ECS->getQueryColumn(query, sizeof(Asset), 1);
  for(U32 i = 0; i < ECS->getQueryMatchCount(query); i++) {
    aligned_free(assets[i].data);
  }
}

void
onRemoveTextAsset(
    ECSQuery query)
{
  TextAsset *assets = ECS->getQueryColumn(query, sizeof(TextAsset), 1);
  for(U32 i = 0; i < ECS->getQueryMatchCount(query); i++) {
    ev_textloader_textasset_destr(assets[i]);
  }
}

void
onRemoveJSONAsset(
    ECSQuery query)
{
  JSONAsset *assets = ECS->getQueryColumn(query, sizeof(JSONAsset), 1);
  for(U32 i = 0; i < ECS->getQueryMatchCount(query); i++) {
    ev_jsonloader_jsonasset_destr(assets[i]);
  }
}


// void
// onRemoveMeshAsset(
//     ECSQuery query)
// {
//   MeshAsset *assets = ECS->getQueryColumn(query, sizeof(MeshAsset), 1);
//   for(int i = 0; i < ECS->getQueryMatchCount(query); i++) {
//     ev_meshloader_meshasset_destr(assets[i]);
//   }
// }

void
onRemoveShaderAsset(
    ECSQuery query)
{
  ShaderAsset *assets = ECS->getQueryColumn(query, sizeof(ShaderAsset), 1);
  for(U32 i = 0; i < ECS->getQueryMatchCount(query); i++) {
    ev_shaderloader_shaderasset_destr(assets[i]);
  }
}

EV_CONSTRUCTOR
{
  ev_log_trace("evmod_asset constructor");
  static_assert(sizeof(AssetEntityID) == sizeof(AssetHandle), "AssetEntityID not the same size of AssetHandle");

  AssetManagerData.sys = assetsys_create( 0 );

  AssetManagerData.ecs_mod = evol_loadmodule("ecs");
  if(AssetManagerData.ecs_mod) {
    imports(AssetManagerData.ecs_mod, (ECS, AssetECS));
    if(AssetECS) {
      AssetManagerData.world = AssetECS->newWorld();

      AssetManagerData.assetcomponent_id = AssetECS->registerComponent("Asset", sizeof(Asset), EV_ALIGNOF(Asset));
      AssetECS->setOnRemoveTrigger("AssetComponentOnRemove", "Asset", onRemoveAssetComponent);

      ev_textloader_setassettype(AssetECS->registerComponent("TextAsset", sizeof(TextAsset), EV_ALIGNOF(TextAsset)));
      AssetECS->setOnRemoveTrigger("TextAssetOnRemove", "TextAsset", onRemoveTextAsset);

      ev_jsonloader_setassettype(AssetECS->registerComponent("JSONAsset", sizeof(JSONAsset), EV_ALIGNOF(JSONAsset)));
      AssetECS->setOnRemoveTrigger("JSONAssetOnRemove", "JSONAsset", onRemoveJSONAsset);


      ev_jsonloader_setassettype(AssetECS->registerComponent("MeshAsset", sizeof(MeshAsset), EV_ALIGNOF(MeshAsset)));
      // AssetECS->setOnRemoveTrigger("MeshAssetOnRemove", "MeshAsset", onRemoveMeshAsset);

      ev_shaderloader_setassettype(AssetECS->registerComponent("ShaderAsset", sizeof(ShaderAsset), EV_ALIGNOF(ShaderAsset)));
      AssetECS->setOnRemoveTrigger("ShaderAssetOnRemove", "ShaderAsset", onRemoveShaderAsset);
    }
  }

  ev_shaderloader_init();

  return AssetManagerData.sys == NULL;
}

EV_DESTRUCTOR
{
  assetsys_destroy(AssetManagerData.sys);

  if(AssetManagerData.world) {
    AssetECS->destroyWorld(AssetManagerData.world);
  }
  if(AssetManagerData.ecs_mod) {
    evol_unloadmodule(AssetManagerData.ecs_mod);
  }

  ev_shaderloader_deinit();
  return 0;
}

AssetHandle
ev_asset_load(
    CONST_STR path)
{
  assetsys_file_t file;
  AssetSysCheck("Failed to find file %s. ", (path), assetsys_file(AssetManagerData.sys, path, &file));
  Asset a;
  a.size = (I64)assetsys_file_size(AssetManagerData.sys, file);
  a.data = aligned_malloc(a.size + 1, 16);
  int loaded_size = 0;
  AssetSysCheck("Failed to load file %s. ", (path), assetsys_file_load(AssetManagerData.sys, file, &loaded_size, a.data, a.size));
  a.size = (I64) loaded_size;
  ((char*)a.data)[a.size] = '\0';

  a.ref_count = 1;

  AssetEntityID entt = AssetECS->createEntity(AssetManagerData.world);
  AssetECS->setComponent(AssetManagerData.world, entt, AssetManagerData.assetcomponent_id, &a);

  return (AssetHandle)entt;
}

AssetHandle
ev_asset_clonehandle(
    AssetHandle handle)
{
  Asset *asset = AssetECS->getComponent(AssetManagerData.world, handle, AssetManagerData.assetcomponent_id);
  asset->ref_count++;

  return handle;
}

void
ev_asset_free(
    AssetHandle handle)
{
  Asset *asset = AssetECS->getComponent(AssetManagerData.world, handle, AssetManagerData.assetcomponent_id);
  asset->ref_count--;
  if(asset->ref_count == 0) {
    AssetECS->destroyEntity(AssetManagerData.world, handle);
  }
}

void
ev_assetmanager_mount(
  evstring *path,
  evstring *as)
{
  evstring_pushstr(as, ":/");
  AssetSysCheck("Failed to mount %s as %s. ", (*path, *as), assetsys_mount(AssetManagerData.sys, *path, *as));
}

const Asset *
ev_asset_getfromhandle(
    AssetHandle handle)
{
  return AssetECS->getComponent(AssetManagerData.world, handle, AssetManagerData.assetcomponent_id);
}

void
ev_asset_markas(
    AssetHandle handle, 
    GenericHandle assetType, 
    PTR data)
{
  AssetECS->setComponent(AssetManagerData.world, handle, assetType, data);
}

EV_BINDINGS
{
  ev_log_debug("Binding functions in evmod_asset");
  EV_NS_BIND_FN(AssetManager, mount, ev_assetmanager_mount);

  EV_NS_BIND_FN(Asset, load, ev_asset_load);
  EV_NS_BIND_FN(Asset, cloneHandle, ev_asset_clonehandle);
  EV_NS_BIND_FN(Asset, free, ev_asset_free);

  EV_NS_BIND_FN(TextLoader, loadAsset, ev_textloader_loadasset);

  EV_NS_BIND_FN(JSONLoader, loadAsset, ev_jsonloader_loadasset);

  EV_NS_BIND_FN(MeshLoader, loadAsset, ev_meshloader_loadasset);

  EV_NS_BIND_FN(ShaderLoader, loadAsset, ev_shaderloader_loadasset);

  return 0;
}
