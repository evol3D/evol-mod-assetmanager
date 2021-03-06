#define EV_MODULE_DEFINE
#include <evol/evolmod.h>
#include <assetsys/assetsys.h>
#include <cute_filewatch.h>

#include <evol/utils/aligned_alloc.h>

#define IMPORT_MODULE evmod_ecs
#include <evol/meta/module_import.h>

#include "loaders/LoaderCommon.h"
#include "loaders/TextLoader/TextLoader.h"
#include "loaders/JSONLoader/JSONLoader.h"
#include "loaders/MeshLoader/MeshLoader.h"
#include "loaders/ShaderLoader/ShaderLoader.h"
#include "loaders/ImageLoader/ImageLoader.h"

#define AssetSysCheck(errmsg_fmt, vars, ...) do { \
  assetsys_error_t res = __VA_ARGS__; \
  if(res != ASSETSYS_SUCCESS) { \
    ev_log_error(errmsg_fmt"`%s` failed. Error Message: %s", EV_EXPAND(vars), EV_STRINGIZE(__VA_ARGS__), assetsys_error_strings[res]); \
  } \
} while (0)

struct {
  assetsys_t *sys;
  filewatch_t *fwatch;

  evolmodule_t ecs_mod;
  ECSAssetWorldHandle world;
  AssetComponentID assetcomponent_id;

} AssetManagerData = {
  NULL, NULL, NULL, 
  0, 0};

void
onRemoveAssetComponent(
    ECSQuery query)
{
  AssetStruct *assets = ECS->getQueryColumn(query, sizeof(AssetStruct), 1);
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
// onRemoveImageAsset(
//     ECSQuery query)
// {
//   ImageAsset *assets = ECS->getQueryColumn(query, sizeof(ImageAsset), 1);
//   for(int i = 0; i < ECS->getQueryMatchCount(query); i++) {
//     ev_imageloader_imageasset_destr(assets[i]);
//   }
// }

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
  AssetManagerData.fwatch = filewatch_create(AssetManagerData.sys, NULL);

  AssetManagerData.ecs_mod = evol_loadmodule("ecs");
  if(AssetManagerData.ecs_mod) {
    imports(AssetManagerData.ecs_mod, (ECS, AssetECS));
    if(AssetECS) {
      AssetManagerData.world = AssetECS->newWorld();

      AssetManagerData.assetcomponent_id = AssetECS->registerComponent("Asset", sizeof(AssetStruct), EV_ALIGNOF(AssetStruct));
      AssetECS->setOnRemoveTrigger("AssetComponentOnRemove", "Asset", onRemoveAssetComponent);

      ev_textloader_setassettype(AssetECS->registerComponent("TextAsset", sizeof(TextAsset), EV_ALIGNOF(TextAsset)));
      AssetECS->setOnRemoveTrigger("TextAssetOnRemove", "TextAsset", onRemoveTextAsset);

      ev_jsonloader_setassettype(AssetECS->registerComponent("JSONAsset", sizeof(JSONAsset), EV_ALIGNOF(JSONAsset)));
      AssetECS->setOnRemoveTrigger("JSONAssetOnRemove", "JSONAsset", onRemoveJSONAsset);

      ev_jsonloader_setassettype(AssetECS->registerComponent("ImageAsset", sizeof(ImageAsset), EV_ALIGNOF(ImageAsset)));
      // AssetECS->setOnRemoveTrigger("ImageAssetOnRemove", "ImageAsset", onRemoveImageAsset);

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
  filewatch_free(AssetManagerData.fwatch);
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
  AssetStruct a;
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
  AssetStruct *asset = AssetECS->getComponent(AssetManagerData.world, handle, AssetManagerData.assetcomponent_id);
  asset->ref_count++;

  return handle;
}

void
ev_asset_free(
    AssetHandle handle)
{
  AssetStruct *asset = AssetECS->getComponent(AssetManagerData.world, handle, AssetManagerData.assetcomponent_id);
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
  /* AssetSysCheck("Failed to mount %s as %s. ", (*path, *as), assetsys_mount(AssetManagerData.sys, *path, *as)); */
  filewatch_mount(AssetManagerData.fwatch, *path, *as);
}

void
ev_assetmanager_watch(
  const char *path,
  FN_PTR callback)
{
  filewatch_start_watching(AssetManagerData.fwatch, path, callback, NULL, 0);
}

void
ev_assetmanager_watchrecursively(
  const char *path,
  FN_PTR callback)
{
  filewatch_start_watching(AssetManagerData.fwatch, path, callback, NULL, 1);
}

void
ev_assetmanager_stopwatching(
  const char *path)
{
  filewatch_stop_watching(AssetManagerData.fwatch, path);
}

const AssetStruct *
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

void
ev_assetmanager_update()
{
  filewatch_update(AssetManagerData.fwatch);
  filewatch_notify(AssetManagerData.fwatch);
}

EV_BINDINGS
{
  ev_log_debug("Binding functions in evmod_asset");
  EV_NS_BIND_FN(AssetManager, mount, ev_assetmanager_mount);
  EV_NS_BIND_FN(AssetManager, watch, ev_assetmanager_watch);
  EV_NS_BIND_FN(AssetManager, watchRecursively, ev_assetmanager_watchrecursively);
  EV_NS_BIND_FN(AssetManager, stopWatching, ev_assetmanager_stopwatching);
  EV_NS_BIND_FN(AssetManager, update, ev_assetmanager_update);

  EV_NS_BIND_FN(Asset, load, ev_asset_load);
  EV_NS_BIND_FN(Asset, cloneHandle, ev_asset_clonehandle);
  EV_NS_BIND_FN(Asset, free, ev_asset_free);

  EV_NS_BIND_FN(TextLoader, loadAsset, ev_textloader_loadasset);

  EV_NS_BIND_FN(JSONLoader, loadAsset, ev_jsonloader_loadasset);

  EV_NS_BIND_FN(MeshLoader, loadAsset, ev_meshloader_loadasset);

  EV_NS_BIND_FN(ShaderLoader, loadAsset, ev_shaderloader_loadasset);

  EV_NS_BIND_FN(ImageLoader, loadAsset, ev_imageloader_loadasset);

  return 0;
}
