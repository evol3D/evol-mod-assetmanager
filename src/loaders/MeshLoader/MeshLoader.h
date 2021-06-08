#pragma once

MeshAsset
ev_meshloader_loadasset(
    AssetHandle handle);

void
ev_meshloader_meshasset_destr(
    MeshAsset mesh);

void
ev_meshloader_setassettype(
    GenericHandle type);
