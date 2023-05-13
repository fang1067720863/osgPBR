#include "EnvLight.h"
#include <osgDB/ReadFile>
#include<osg/ShapeDrawable>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgEarth/FileUtils>

#include<osg/MatrixTransform>
#include"SnapImage.h"

using namespace osgEarth;
void IBLTechnique::startUp()
{
    createTextures();
    createHDRCubePass();
    createIrridiancePass();
    createPrefilterPass();
    createBrdfLUTPass();
}
void IBLTechnique::createIrridiancePass()
{

    // input: env_cubeMap output:irridianceCubeMap
    osg::ref_ptr<osg::Node> box = createHDRBox();
    VirtualProgram* vp = VirtualProgram::getOrCreate(box->getOrCreateStateSet());
    box->getOrCreateStateSet()->setTextureAttributeAndModes(0, envCubeMap.get(), osg::StateAttribute::ON);
    box->getOrCreateStateSet()->getOrCreateUniform("environmentMap", osg::Uniform::SAMPLER_CUBE)->set(0);
    const char* pick_preview = R"(


        #pragma vp_function vs, vertex_model
        out vec3 WorldPos;
        void vs(inout vec4 vertex) {
            WorldPos = normalize(vertex.xyz);
        }
        [break]

        #pragma vp_function fs, fragment_output
        in vec3 WorldPos;
        out vec4 FragColor;
        #define PI 3.14159265359f
        uniform samplerCube environmentMap;
        void fs(inout vec4 c) {
            vec3 N = normalize(WorldPos);

            vec3 irradiance = vec3(0.0);   
    
            // tangent space calculation from origin point
            vec3 up    = vec3(0.0, 1.0, 0.0);
            vec3 right = normalize(cross(up, N));
            up         = normalize(cross(N, right));
       
            float sampleDelta = 0.025;
            float nrSamples = 0.0f;
            for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
            {
                for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
                {
                    // spherical to cartesian (in tangent space)
                    vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
                    // tangent space to world
                    vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

                    irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
                    nrSamples++;
                }
            }
            irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
            FragColor = vec4(irradiance, 1.0);
        }
    )";

    ShaderLoader::load(vp, pick_preview);
    osg::Node* cameras = createSlaveCameras(box.get(), irridianceMap.get());
    root->addChild(cameras);
}


std::string read_raw_source(osgDB::Options *dbOptions,const std::string& filename)
{
    URIContext context(dbOptions);
    URI uri(filename, context);

    // searches OSG_FILE_PATH
    std::string path = osgDB::findDataFile(uri.full(), dbOptions);
    if (!path.empty())
    {
        std::string externalSource = URI(path, context).getString(dbOptions);
        return externalSource;
       /* if (!externalSource.empty())
        {
            OE_DEBUG << LC << "Loaded external shader " << filename << " from " << path << "\n";
            output = externalSource;
        }*/
    }
}
void IBLTechnique::createPrefilterPass()
{

    // input: env_cubeMap output:PrefilterCubeMap
    osg::ref_ptr<osg::Node> box = createHDRBox();
    VirtualProgram* vp = VirtualProgram::getOrCreate(box->getOrCreateStateSet());
    box->getOrCreateStateSet()->setTextureAttributeAndModes(0, envCubeMap.get(), osg::StateAttribute::ON);
    box->getOrCreateStateSet()->getOrCreateUniform("environmentMap", osg::Uniform::SAMPLER_CUBE)->set(0);

    auto shaderPath = "..//..//OE_PBR//Shader";
    auto shaderPath2 = "..//OE_PBR//Shader";

    osg::ref_ptr<osgDB::Options> dbo = new osgDB::Options();


    if (osgDB::fileExists(osgEarth::getAbsolutePath(shaderPath)))
    {
        dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath));
    }
    else {
        dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath2));
    }

    ShaderPackage shaders;
    shaders.load(vp, "IBLPrefilterMap.glsl", dbo);


    uint32_t maxMipLevels = 3;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {

        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        float roughness = (float)mip / (float)(maxMipLevels - 1);
        
        box->getOrCreateStateSet()->getOrCreateUniform("roughness", osg::Uniform::FLOAT)->set(roughness);
        osg::Node* cameras = createSlaveCameras(box.get(), prefilterMap.get(), mip,mipWidth,mipHeight, true);
        root->addChild(cameras);
    }
}
osg::ref_ptr<osg::Node> createQuad()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0, 0.0, 0.0), 512.0));

    geode->addDrawable(sd);
    return geode;
}
void IBLTechnique::createBrdfLUTPass()
{

    osg::ref_ptr<osg::Node> box = createQuad();
    VirtualProgram* vp = VirtualProgram::getOrCreate(box->getOrCreateStateSet());

    auto shaderPath = "..//..//OE_PBR//Shader";
    auto shaderPath2 = "..//OE_PBR//Shader";

    osg::ref_ptr<osgDB::Options> dbo = new osgDB::Options();


    if (osgDB::fileExists(osgEarth::getAbsolutePath(shaderPath)))
    {
        dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath));
    }
    else {
        dbo->setDatabasePath(osgEarth::getAbsolutePath(shaderPath2));
    }


    


    ShaderPackage shaders;
    shaders.load(vp, "IBLbrdfLUT.glsl", dbo);
    int w, h;
    w = h = 512;
    osg::ref_ptr<osg::Camera> cam = new osg::Camera();
    cam->addChild(box);
    cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
    cam->setClearColor(osg::Vec4(0, 0, 1, 1));
    cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    cam->setViewport(0, 0, w, h);
    cam->setRenderOrder(osg::Camera::PRE_RENDER);
    cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    cam->setImplicitBufferAttachmentMask(0, 0);

    //osg::Image* image = new osg::Image;
    //image->allocateImage(w, h, 1, GL_RGBA, GL_FLOAT);


    cam->attach(osg::Camera::COLOR_BUFFER, brdfLUTMap.get(), 0);
    cam->setProjectionMatrixAsOrtho2D(-1.0 * w / 2, (-1.0 * w / 2) + w * 1.0, -1.0 * h / 2, (-1.0 * h / 2) + h * 1.0);
    //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0), osg::Vec3(0.0, -1.0, 0.0));
    cam->setViewMatrix(
        osg::Matrixd::identity());

    

    {
        osg::Image* image = new osg::Image;
        //image->allocateImage(tex_width, tex_height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
        image->allocateImage(w, h, 1, GL_RGBA, GL_UNSIGNED_SHORT);

        // attach the image so its copied on each frame.
        cam->attach(osg::Camera::COLOR_BUFFER, image, 0);

        // Rather than attach the texture directly to illustrate the texture's ability to
        // detect an image update and to subload the image onto the texture.  You needn't
        // do this when using an Image for copying to, as a separate camera->attach(..)
        // would suffice as well, but we'll do it the long way round here just for demonstration
        // purposes (long way round meaning we'll need to copy image to main memory, then
        // copy it back to the graphics card to the texture in one frame).
        // The long way round allows us to manually modify the copied image via the callback
        // and then let this modified image by reloaded back.
        brdfLUTMap->setImage(0, image);
        auto snap = new SnapImage("brdfLUT.png", image, brdfLUTMap);
        cam->setPostDrawCallback(snap);
        viewer->addEventHandler(new SnapeImageHandler('p', snap));
    }
  

    root->addChild(cam);

}

void IBLTechnique::createHDRCubePass()
{

   
    osg::ref_ptr<osg::Node> box = createHDRBox();
    VirtualProgram* vp = VirtualProgram::getOrCreate(box->getOrCreateStateSet());
    box->getOrCreateStateSet()->setTextureAttributeAndModes(0, hdrMap.get(), osg::StateAttribute::ON);
    const char* pick_preview = R"(


        #pragma vp_function SampleSphericalMap, vertex_model
        const vec2 invAtan = vec2(0.1591, 0.3183);
        out vec2 spherical_uv;
        void SampleSphericalMap(inout vec4 vertex) {
            vec3 tmp = normalize(vertex.xyz);

            vec2 uv = vec2(atan(tmp.z, tmp.x), asin(tmp.y));
            uv *= invAtan;
            uv += 0.5;
            spherical_uv = uv;
        }

        [break]

         #pragma vp_function fs, fragment_output
        in vec2 spherical_uv;
        out vec4 frag;
        uniform sampler2D tex;
        void fs(inout vec4 c) {
            c = texture(tex, spherical_uv);
            frag = c;
        }
    )";

    ShaderLoader::load(vp, pick_preview);
    osg::Node* cameras = createSlaveCameras(box.get(),envCubeMap);
    root->addChild(cameras);
}

osg::Node* IBLTechnique::createSlaveCameras(osg::Node* box, osg::Texture* tex, unsigned int level, unsigned int w, unsigned int h, bool generateMipmap)
{
    osg::Group* cameras = new osg::Group();
   
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(box);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, w, h);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, tex, level, osg::TextureCubeMap::POSITIVE_X, generateMipmap);
        cam->setProjectionMatrixAsOrtho2D(-1.0 * w / 2, (-1.0 * w / 2) + w * 1.0, -1.0 * h / 2, (-1.0 * h / 2) + h * 1.0);
        //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(1.0, 0.0, 0.0), osg::Vec3(0.0, -1.0, 0.0));
        cam->setViewMatrix(
            osg::Matrixd::rotate(osg::inDegrees(90.0f), 0.0, 1.0, 0.0) * osg::Matrixd::rotate(osg::inDegrees(90.0f), 0.0, 0.0, 1.0));

        cameras->addChild(cam);
    }


    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(box);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, w, h);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, tex, level, osg::TextureCubeMap::NEGATIVE_X, generateMipmap);
        cam->setProjectionMatrixAsOrtho2D(-1.0 * w / 2, (-1.0 * w / 2) + w * 1.0, -1.0 * h / 2, (-1.0 * h / 2) + h * 1.0);
        //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(-1.0, 0.0, 0.0), osg::Vec3(0.0, -1.0, 0.0));
        cam->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(-90.0f), 0.0, 1.0, 0.0) * osg::Matrixd::rotate(osg::inDegrees(-90.0f), 0.0, 0.0, 1.0));
       
        cameras->addChild(cam);
    }
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(box);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, w, h);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, tex, level, osg::TextureCubeMap::POSITIVE_Y, generateMipmap);
        cam->setProjectionMatrixAsOrtho2D(-1.0 * w / 2, (-1.0 * w / 2) + w * 1.0, -1.0 * h / 2, (-1.0 * h / 2) + h * 1.0);
        //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 1.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
        cam->setViewMatrix(osg::Matrixd());
        cameras->addChild(cam);
    }
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(box);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, w, h);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, tex, level, osg::TextureCubeMap::NEGATIVE_Y, generateMipmap);
        cam->setProjectionMatrixAsOrtho2D(-1.0 * w  / 2, (-1.0 * w / 2) + w * 1.0, -1.0 * h / 2, (-1.0 * h / 2) + h * 1.0);
        cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, -1.0, 0.0), osg::Vec3(0.0, -1.0, 0.0));
        cam->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(180.0f), 1.0, 0.0, 0.0));
        cameras->addChild(cam);
    }
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(box);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, w, h);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, tex, level, osg::TextureCubeMap::POSITIVE_Z, generateMipmap);
        cam->setProjectionMatrixAsOrtho2D(-1.0 * w / 2, (-1.0 * w / 2) + w * 1.0, -1.0 * h / 2, (-1.0 * h / 2) + h * 1.0);
        cam->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(-90.0f), 1.0, 0.0, 0.0));
        //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0), osg::Vec3(0.0, -1.0, 0.0));
        cameras->addChild(cam);
    }
    {
        osg::ref_ptr<osg::Camera> cam = new osg::Camera();
        cam->addChild(box);
        cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        cam->setClearColor(osg::Vec4(0, 0, 1, 1));
        cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        cam->setViewport(0, 0, w, h);
        cam->setRenderOrder(osg::Camera::PRE_RENDER);
        cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        cam->setImplicitBufferAttachmentMask(0, 0);
        cam->attach(osg::Camera::COLOR_BUFFER, tex, level, osg::TextureCubeMap::NEGATIVE_Z, generateMipmap);
        cam->setProjectionMatrixAsOrtho2D(-1.0 * w / 2, (-1.0 * w / 2) + w * 1.0, -1.0 * h / 2, (-1.0 * h / 2) + h * 1.0);
        //cam->setViewMatrixAsLookAt(osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, -1.0), osg::Vec3(0.0, -1.0, 0.0));
        cam->setViewMatrix(osg::Matrixd::rotate(osg::inDegrees(90.0f), 1.0, 0.0, 0.0) * osg::Matrixd::rotate(osg::inDegrees(180.0f), 0.0, 0.0, 1.0));
        cameras->addChild(cam);
    }
    return cameras;

}
osg::ref_ptr<osg::Node> IBLTechnique::createHDRBox()
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0, 0.0, 0.0), 16.0));

    geode->addDrawable(sd);
    osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform();
    trans->setMatrix(osg::Matrix::scale(16, 16, 16));
    trans->addChild(geode);
    return trans;
}





void IBLTechnique::createTextures()
{
    int w, h;
    w = h = 256;
    // hdr
    osg::ref_ptr <osg::Image> image = osgDB::readRefImageFile("C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\suburban_parking_area_4k.hdr");
    hdrMap = new osg::Texture2D(image.get());


    envCubeMap = new osg::TextureCubeMap();
    envCubeMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    envCubeMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    envCubeMap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    envCubeMap->setTextureSize(w, h);
    envCubeMap->setInternalFormat(GL_RGB);
    envCubeMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    envCubeMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    envCubeMap->setNumMipmapLevels(5);
    //envCubeMap->setUseHardwareMipMapGeneration(true);





    w = h = 32;
    irridianceMap = new osg::TextureCubeMap();
    irridianceMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    irridianceMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    irridianceMap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    irridianceMap->setTextureSize(w, h);
    irridianceMap->setInternalFormat(GL_RGB);

    irridianceMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    irridianceMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);


    w = h = 128;
    prefilterMap = new osg::TextureCubeMap();
    prefilterMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    prefilterMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    prefilterMap->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    prefilterMap->setTextureSize(w, h);
    prefilterMap->setInternalFormat(GL_RGB);

    prefilterMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    prefilterMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    //prefilterMap->setUseHardwareMipMapGeneration(true);
    prefilterMap->setNumMipmapLevels(5);

    w = h = 512;
    brdfLUTMap = new osg::Texture2D();
    brdfLUTMap->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    brdfLUTMap->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    brdfLUTMap->setTextureSize(w, h);
    brdfLUTMap->setInternalFormat(GL_RGBA);
    brdfLUTMap->setSourceType(GL_FLOAT);

    brdfLUTMap->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    brdfLUTMap->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    //prefilterMap->setUseHardwareMipMapGeneration(true);

}
    //bool noMipMap, hardwareMipmap;
   /* noMipMap = hardwareMipmap = false;*/
    //if (noMipMap)
    //{
    //   
    //else {
    //    irridianceMap->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    //    irridianceMap->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    //}






