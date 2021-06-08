#pragma once

void
ev_shaderloader_init();

void
ev_shaderloader_deinit();

ShaderAsset
ev_shaderloader_loadasset(
    AssetHandle handle,
    ShaderAssetStage stage,
    CONST_STR shader_name,
    CONST_STR entrypoint,
    CompiledShaderType type);

void
ev_shaderloader_shaderasset_destr(
    ShaderAsset shader);

void
ev_shaderloader_setassettype(
    GenericHandle type);
