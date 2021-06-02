#pragma once

JSONAsset
ev_jsonloader_loadasset(
    AssetHandle handle);

void
ev_jsonloader_jsonasset_destr(
    JSONAsset json);

void
ev_jsonloader_setassettype(
    GenericHandle type);
