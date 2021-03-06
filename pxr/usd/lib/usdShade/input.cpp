//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "pxr/pxr.h"
#include "pxr/usd/usdShade/input.h"

#include "pxr/usd/usdShade/connectableAPI.h"
#include "pxr/usd/usdShade/output.h"
#include "pxr/usd/usdShade/utils.h"

#include "pxr/usd/sdf/schema.h"
#include "pxr/usd/usd/relationship.h"

#include "pxr/usd/usdShade/connectableAPI.h"

#include <stdlib.h>
#include <algorithm>

#include "pxr/base/tf/envSetting.h"

PXR_NAMESPACE_OPEN_SCOPE


using std::vector;
using std::string;

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,
    (connectability)
    (renderType)
);

UsdShadeInput::UsdShadeInput(const UsdAttribute &attr)
    : _attr(attr)
{
}

TfToken 
UsdShadeInput::GetBaseName() const
{
    string name = GetFullName();
    if (TfStringStartsWith(name, UsdShadeTokens->inputs)) {
        return TfToken(name.substr(UsdShadeTokens->inputs.GetString().size()));
    } else if (UsdShadeUtils::ReadOldEncoding() && 
               TfStringStartsWith(name, UsdShadeTokens->interface_)) {
        return TfToken(name.substr(
            UsdShadeTokens->interface_.GetString().size()));
    }
    
    return GetFullName();
}

SdfValueTypeName 
UsdShadeInput::GetTypeName() const
{ 
    return _attr.GetTypeName();
}

static TfToken
_GetInputAttrName(const TfToken inputName) 
{
    return TfToken(UsdShadeTokens->inputs.GetString() + inputName.GetString());
}

UsdShadeInput::UsdShadeInput(
    UsdPrim prim,
    TfToken const &name,
    SdfValueTypeName const &typeName)
{
    // XXX what do we do if the type name doesn't match and it exists already?
    TfToken inputAttrName = _GetInputAttrName(name);
    if (prim.HasAttribute(inputAttrName)) {
        _attr = prim.GetAttribute(inputAttrName);
    }

    if (UsdShadeUtils::ReadOldEncoding()) {
        if (prim.HasAttribute(name)) {
            _attr = prim.GetAttribute(name);
        }
        else {
            TfToken interfaceAttrName(UsdShadeTokens->interface_.GetString() + 
                                      name.GetString());
            if (prim.HasAttribute(interfaceAttrName)) {
                _attr = prim.GetAttribute(interfaceAttrName);
            }
        }
    }

    if (!_attr) {
        if (UsdShadeUtils::WriteNewEncoding()) {
            _attr = prim.CreateAttribute(inputAttrName, typeName, 
                /* custom = */ false);
        } else {
            UsdShadeConnectableAPI connectable(prim);
            // If this is a node-graph and the name already contains "interface:" 
            // namespace in it, just create the attribute with the requested 
            // name.
            if (connectable.IsNodeGraph()) { 
                TfToken attrName = TfStringStartsWith(name.GetString(),
                    UsdShadeTokens->interface_) ? name : 
                    TfToken(UsdShadeTokens->interface_.GetString() + name.GetString());
                _attr = prim.CreateAttribute(attrName, typeName, /*custom*/ false);
            } else {
                // fallback to creating an old style UsdShadeParameter.
                _attr = prim.CreateAttribute(name, typeName, /*custom*/ false);
            }
        }
    }
}

bool
UsdShadeInput::Get(VtValue* value, UsdTimeCode time) const
{
    if (!_attr) {
        return false;
    }

    return _attr.Get(value, time);
}

bool
UsdShadeInput::Set(const VtValue& value, UsdTimeCode time) const
{
    return _attr.Set(value, time);
}

bool 
UsdShadeInput::SetRenderType(TfToken const& renderType) const
{
    return _attr.SetMetadata(_tokens->renderType, renderType);
}

TfToken 
UsdShadeInput::GetRenderType() const
{
    TfToken renderType;
    _attr.GetMetadata(_tokens->renderType, &renderType);
    return renderType;
}

bool 
UsdShadeInput::HasRenderType() const
{
    return _attr.HasMetadata(_tokens->renderType);
}


NdrTokenMap
UsdShadeInput::GetSdrMetadata() const
{
    NdrTokenMap result;

    VtDictionary sdrMetadata;
    if (GetAttr().GetMetadata(UsdShadeTokens->sdrMetadata, &sdrMetadata)){
        for (const auto &it : sdrMetadata) {
            result[TfToken(it.first)] = TfStringify(it.second);
        }
    }

    return result;
}

std::string 
UsdShadeInput::GetSdrMetadataByKey(const TfToken &key) const
{
    VtValue val;
    GetAttr().GetMetadataByDictKey(UsdShadeTokens->sdrMetadata, key, &val);
    return TfStringify(val);
}
    
void 
UsdShadeInput::SetSdrMetadata(const NdrTokenMap &sdrMetadata) const
{
    for (auto &i: sdrMetadata) {
        SetSdrMetadataByKey(i.first, i.second);
    }
}

void 
UsdShadeInput::SetSdrMetadataByKey(
    const TfToken &key, 
    const std::string &value) const
{
    GetAttr().SetMetadataByDictKey(UsdShadeTokens->sdrMetadata, key, value);
}

bool 
UsdShadeInput::HasSdrMetadata() const
{
    return GetAttr().HasMetadata(UsdShadeTokens->sdrMetadata);
}

bool 
UsdShadeInput::HasSdrMetadataByKey(const TfToken &key) const
{
    return GetAttr().HasMetadataDictKey(UsdShadeTokens->sdrMetadata, key);
}

void 
UsdShadeInput::ClearSdrMetadata() const
{
    GetAttr().ClearMetadata(UsdShadeTokens->sdrMetadata);
}

void
UsdShadeInput::ClearSdrMetadataByKey(const TfToken &key) const
{
    GetAttr().ClearMetadataByDictKey(UsdShadeTokens->sdrMetadata, key);
}

/* static */
bool 
UsdShadeInput::IsInput(const UsdAttribute &attr)
{
    return attr && attr.IsDefined() && 
            // If reading of old encoding is supported, then assume it's      
            // an input as long as it's not in the "outputs:" namespace.
            // If support for reading the old encoding is disabled, then only
            // identify as an input if the attr is in the "inputs:" namespace.
            (UsdShadeUtils::ReadOldEncoding() ? 
             !TfStringStartsWith(attr.GetName().GetString(), 
                                UsdShadeTokens->outputs) :
             TfStringStartsWith(attr.GetName().GetString(), 
                                UsdShadeTokens->inputs));
}

/* static */
bool
UsdShadeInput::IsInterfaceInputName(const std::string & name)
{
    if (TfStringStartsWith(name, UsdShadeTokens->inputs)) {
        return true;
    }

    if (UsdShadeUtils::ReadOldEncoding() &&
        TfStringStartsWith(name, UsdShadeTokens->interface_)) {
        return true;
    }

    return false;
}

bool 
UsdShadeInput::SetDocumentation(const std::string& docs) const
{
    if (!_attr) {
        return false;
    }

    return _attr.SetDocumentation(docs);
}

std::string 
UsdShadeInput::GetDocumentation() const
{
    if (!_attr) {
        return "";
    }

    return _attr.GetDocumentation();
}

bool 
UsdShadeInput::SetDisplayGroup(const std::string& docs) const
{
    if (!_attr) {
        return false;
    }

    return _attr.SetDisplayGroup(docs);
}

std::string 
UsdShadeInput::GetDisplayGroup() const
{
    if (!_attr) {
        return "";
    }

    return _attr.GetDisplayGroup();
}

bool 
UsdShadeInput::CanConnect(const UsdAttribute &source) const 
{
    return UsdShadeConnectableAPI::CanConnect(*this, source);
}

bool 
UsdShadeInput::CanConnect(const UsdShadeInput &sourceInput) const 
{
    return CanConnect(sourceInput.GetAttr());
}

bool 
UsdShadeInput::CanConnect(const UsdShadeOutput &sourceOutput) const
{
    return CanConnect(sourceOutput.GetAttr());
}

bool 
UsdShadeInput::ConnectToSource(
    UsdShadeConnectableAPI const &source, 
    TfToken const &sourceName, 
    UsdShadeAttributeType const sourceType,
    SdfValueTypeName typeName) const 
{
    return UsdShadeConnectableAPI::ConnectToSource(*this, source, 
        sourceName, sourceType, typeName);   
}

bool 
UsdShadeInput::ConnectToSource(SdfPath const &sourcePath) const 
{
    return UsdShadeConnectableAPI::ConnectToSource(*this, sourcePath);
}

bool 
UsdShadeInput::ConnectToSource(UsdShadeInput const &sourceInput) const 
{
    return UsdShadeConnectableAPI::ConnectToSource(*this, sourceInput);
}

bool 
UsdShadeInput::ConnectToSource(UsdShadeOutput const &sourceOutput) const 
{
    return UsdShadeConnectableAPI::ConnectToSource(*this, sourceOutput);
}

bool 
UsdShadeInput::GetConnectedSource(UsdShadeConnectableAPI *source, 
                                  TfToken *sourceName,
                                  UsdShadeAttributeType *sourceType) const 
{
    return UsdShadeConnectableAPI::GetConnectedSource(*this, source, 
        sourceName, sourceType);
}

bool 
UsdShadeInput::GetRawConnectedSourcePaths(SdfPathVector *sourcePaths) const 
{
    return UsdShadeConnectableAPI::GetRawConnectedSourcePaths(*this, 
        sourcePaths);
}

bool 
UsdShadeInput::HasConnectedSource() const 
{
    return UsdShadeConnectableAPI::HasConnectedSource(*this);
}

bool 
UsdShadeInput::IsSourceConnectionFromBaseMaterial() const 
{
    return UsdShadeConnectableAPI::IsSourceConnectionFromBaseMaterial(*this);
}

bool 
UsdShadeInput::DisconnectSource() const 
{
    return UsdShadeConnectableAPI::DisconnectSource(*this);
}

bool 
UsdShadeInput::ClearSource() const 
{
    return UsdShadeConnectableAPI::ClearSource(*this);
}

bool 
UsdShadeInput::SetConnectability(const TfToken &connectability) const
{
    return _attr.SetMetadata(_tokens->connectability, connectability);
}

TfToken 
UsdShadeInput::GetConnectability() const
{
    TfToken connectability; 
    _attr.GetMetadata(_tokens->connectability, &connectability);

    // If there's an authored non-empty connectability value, then return it. 
    // If not, return "full".
    if (!connectability.IsEmpty()) {
        return connectability;
    }

    return UsdShadeTokens->full;
}

bool 
UsdShadeInput::ClearConnectability() const
{
    return _attr.ClearMetadata(_tokens->connectability);
}

PXR_NAMESPACE_CLOSE_SCOPE

