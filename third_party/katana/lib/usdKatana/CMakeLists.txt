set(PXR_PACKAGE usdKatana)

pxr_shared_library(${PXR_PACKAGE}
    LIBRARIES
        plug
        gf
        tf
        trace
        vt
        ar
        sdf
        usd
        usdHydra
        usdImaging
        usdImagingGL
        usdShade
        usdSkel
        usdGeom
        usdLux
        usdRi
        usdUI
        usdUtils
        cameraUtil
        katanaAttrfncApi
        ${Boost_SYSTEM_LIBRARY} 
        ${Boost_THREAD_LIBRARY} 

    PUBLIC_CLASSES
        attrMap
        baseMaterialHelpers
        blindDataObject
        cache
        debugCodes
        locks
        tokens
        lookAPI
        utils

        usdInArgs
        usdInPrivateData
        usdInPluginRegistry

        readBasisCurves
        readBlindData
        readCamera
        readConstraintTarget
        readGprim
        readGeomSubset
        readLight
        readLightFilter
        readMaterial
        readMesh
        readModel
        readNurbsPatch
        readPointInstancer
        readPoints
        readPrim
        readXformable

    PUBLIC_HEADERS
        api.h

    PYMODULE_CPPFILES
        wrapBlindDataObject.cpp
        wrapCache.cpp
        wrapLookAPI.cpp
        module.cpp

    PYMODULE_FILES
        __init__.py

    RESOURCE_FILES
        plugInfo.json
        generatedSchema.usda
        schema.usda:usdKatana/schema.usda
)

