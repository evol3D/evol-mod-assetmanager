#define TYPE_MODULE evmod_assets
#include <evol/meta/type_import.h>
#include <evol/common/ev_log.h>

#include "../LoaderCommon.h"
#include "ShaderLoader.h"

#include <shaderc/shaderc.h>

struct {
  shaderc_compiler_t compiler;
  shaderc_compile_options_t compile_options;
} ShaderLoaderData = {NULL,NULL};

void
ev_shaderloader_init()
{
  ShaderLoaderData.compiler = shaderc_compiler_initialize();
  ShaderLoaderData.compile_options = shaderc_compile_options_initialize();
}

void
ev_shaderloader_deinit()
{
  shaderc_compile_options_release(ShaderLoaderData.compile_options);
  shaderc_compiler_release(ShaderLoaderData.compiler);
}

shaderc_shader_kind
shader_kind_from_ShaderAssetStage(
    ShaderAssetStage asset)
{
  switch(asset) {
    case EV_SHADERASSETSTAGE_VERTEX:
      return shaderc_glsl_vertex_shader;
    case EV_SHADERASSETSTAGE_FRAGMENT:
      return shaderc_glsl_fragment_shader;
    case EV_SHADERASSETSTAGE_COMPUTE:
      return shaderc_glsl_compute_shader;
    case EV_SHADERASSETSTAGE_GEOMETRY:
      return shaderc_glsl_geometry_shader;
    case EV_SHADERASSETSTAGE_DETECT:
      return shaderc_glsl_infer_from_source;
    default:
      ev_log_warn("Invalid ShaderAssetStage. Trying to infer stage from source.");
      return shaderc_glsl_infer_from_source;
  }
}
typedef shaderc_compilation_result_t (*ShaderCompilationFn)();
ShaderAsset
ev_shaderloader_loadasset(
    AssetHandle handle,
    ShaderAssetStage stage,
    CONST_STR shader_name,
    CONST_STR entrypoint,
    CompiledShaderType type)
{
  const Asset *asset = ev_asset_getfromhandle(handle);
  ShaderCompilationFn compilation_fn = 
    (type == EV_SHADER_BIN)
      ?(ShaderCompilationFn)shaderc_compile_into_spv:
      (type == EV_SHADER_ASM)
        ?(ShaderCompilationFn)shaderc_compile_into_spv_assembly
        :NULL;

  shaderc_compilation_result_t result = compilation_fn(
      ShaderLoaderData.compiler, 
      asset->data, asset->size, 
      shader_kind_from_ShaderAssetStage(stage), 
      shader_name, entrypoint?entrypoint:"main", 
      ShaderLoaderData.compile_options);

  if(shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) {
    if(shaderc_result_get_num_errors(result) > 0) {
      ev_log_error(shaderc_result_get_error_message(result));
    }
  }
  if(shaderc_result_get_num_warnings(result) > 0) {
    ev_log_warn(shaderc_result_get_error_message(result));
  }

  ShaderAsset inter = (ShaderAsset) {
    .binary = (PTR)shaderc_result_get_bytes(result),
    .len = shaderc_result_get_length(result),
    .internal_handle = (GenericHandle)result
  };

  ev_asset_markas(handle, LoaderData.assetType, &inter);

  return inter;
}

void
ev_shaderloader_shaderasset_destr(
    ShaderAsset shader)
{
  shaderc_result_release((shaderc_compilation_result_t)shader.internal_handle);
}

void
ev_shaderloader_setassettype(
    GenericHandle type)
{
  LoaderData.assetType = type;
}
