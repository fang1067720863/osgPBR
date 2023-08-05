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
- 提供osg自己的材质资产格式 .omat
- 可以切换 HDR specularHDR 可设置mipmap
- OPAQUE

# TODO
- 直接通过mipmap Texture 写入到硬盘
- IBLBaker 生成的diffuse和specular还有问题
- waterial example from ue
- trackball 重写
- 透明材质
- 两个插件写好
- 后处理
- 高级材质
- gltf reader uv解析

- sheen tranmission ior anisotropy variants iridescence 
- exposure