# OSGEARTH PBR MATERIAL LIB

## V1.0
- 提供osgEarth::StandardPBRMaterial
- GLTFReaderV2读取gltf2.0模型
- PbrLightEffect实现Cook_BRDF光照

 ## V1.1
- 新增透明材质
- 新增IBL环境光照
- 扩展了dds插件对CubeMap的读取 和 GL_RGBA_ARB扩展压缩格式的支持
- 【Tool】 cubemap -> quadMap

 ## V1.2
- 提供osgEarth::ExtensionedMaterial 可自定义材质
- 实现了waterial example from ue
- 提供osg自己的材质资产格式 .omat
- 可以切换 HDR specularHDR 可设置mipmap
- OPAQUE

## V1.3
- 提供osgEarth::AdvancedMaterial 
- 支持sheen和clearcloat特性
- gtlf插件增加对khr_extenison的支持
- 完成relection probe cubeCamera以支持transmission

# TODO
- 直接通过mipmap Texture 写入到硬盘
- 透明材质
- 后处理
- 高级材质

- sheen tranmission ior anisotropy variants iridescence 
- exposure