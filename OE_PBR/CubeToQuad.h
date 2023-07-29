#pragma once


#include<osg/Texture2D>
#include<osg/TextureCubeMap>
#include<osg/Geode>
#include<osg/Geometry>
#include<osgEarth/VirtualProgram>


#include <osgEarth/Common>
#include<osgEarth/Threading>
#include<osgEarth/URI>
#include <osgEarth/ShaderLoader>
#include<osgEarth/VirtualProgram>
#include <set>
#include <osg/Node>
#include<osg/PrimitiveSet>


#include"SnapImage.h"


class QuadTextureTransitor:public osg::Object
{
public:
    QuadTextureTransitor(){}
    QuadTextureTransitor(osg::Group* _root,
        osgViewer::Viewer* _viewer) :root(_root), viewer(_viewer) {

    }
    QuadTextureTransitor(const QuadTextureTransitor& teapot, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) {}
    META_Object(osgIBLApp, QuadTextureTransitor);


    osg::Geode* createQuadGeom(float width, float height)
    {
        auto geode = new osg::Geode();

        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

        osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();

        v->push_back(osg::Vec3(-0.5f * width, -0.5f * height, 0.0f));
        v->push_back(osg::Vec3(0.5f * width, -0.5f * height, 0.0f));
        v->push_back(osg::Vec3(0.5f * width, 0.5f * height, 0.0f));
        v->push_back(osg::Vec3(-0.5f * width, 0.5f * height, 0.0f));
        geom->setVertexArray(v.get());

        osg::ref_ptr<osg::Vec2Array> vt = new osg::Vec2Array();
        vt->push_back(osg::Vec2(0.0f, 0.0f));
        vt->push_back(osg::Vec2(1.0f, 0.0f));
        vt->push_back(osg::Vec2(1.0f, 1.0f));
        vt->push_back(osg::Vec2(0.0f, 1.0f));
        geom->setTexCoordArray(0, vt.get());

        osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array;
        c->push_back(osg::Vec4(0.f, 1.f, 1.f, 1.f));
        geom->setColorArray(c.get(), osg::Array::BIND_OVERALL);

        osg::ref_ptr<osg::Vec3Array> n = new osg::Vec3Array;
        n->push_back(osg::Vec3(0.f, -1.f, 0.0f));
        geom->setNormalArray(n.get(), osg::Array::BIND_OVERALL);


        //auto prim = new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4);

        osg::DrawElementsUByte* elems = new osg::DrawElementsUByte(osg::PrimitiveSet::TRIANGLES);
        elems->push_back(0);
        elems->push_back(1);
        elems->push_back(2);

        elems->push_back(2);
        elems->push_back(3);
        elems->push_back(0);

        geom->addPrimitiveSet(elems);
        geode->addDrawable(geom);
        return geode;
    }

    osg::Image* createMipmapImage(uint32_t mipLevel, int w, int h)
    {

        osg::Image* image = new osg::Image;
        osg::Image::MipmapDataType mipmapData;
        unsigned int s = w;
        unsigned int size = s;
        unsigned int totalSize = 0;
        unsigned i;
        for (i = 0; s > 0 && i < mipLevel; s >>= 1, ++i)
        {
            if (i > 0) mipmapData.push_back(totalSize);
            totalSize += s * s * 4;
        }

        unsigned char* ptr = new unsigned char[totalSize];
        image->setImage(size, size, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, ptr, osg::Image::USE_NEW_DELETE, 1);
        image->setMipmapLevels(mipmapData);

        return image;
    }


    static constexpr auto cube_to_quad = "cube_to_quad.glsl";
    //void testDDS()
    //{
    //    osg::ref_ptr<osg::Image> img = osgDB::readRefImageFile("C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\specular.dds");
    //    //"C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\specular.dds"
    // //"C:\\Users\\10677\\source\\repos\\OE_PBR\\OE_PBR\\Asset\\IBL\\Cerberus_NDiffuseHDR.dds"
    //    osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D();
    //    tex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    //    tex->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    //    tex->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
    //    tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
    //    tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

    //    tex->setImage(0, img.get());
    //    tex->setUseHardwareMipMapGeneration(false);

    //    osg::ref_ptr<osg::Node> node = createQuadGeom(10, 10);
    //    node->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex.get());
    //    root->addChild(node.get());
    //}


    osg::TextureCubeMap* createCubeMap(const std::vector<std::string>& imagePath, osgDB::Options* readOption)
    {
        osg::TextureCubeMap* tcm = new osg::TextureCubeMap;

        tcm->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
        tcm->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
        tcm->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP);
        tcm->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
        tcm->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        //tcm->setUseHardwareMipMapGeneration(true);
        
        for (unsigned int i = 0;i < 6; i++)
        {
            auto image = osgDB::readImageFile(imagePath[i], readOption);
           // image->flipHorizontal();
            tcm->setImage(i, image);
        }
       
        return tcm;
    }

    osg::TextureCubeMap* createCubeMap(const std::string& imagePath, osgDB::Options* readOption)
    {
        osg::TextureCubeMap* tcm = new osg::TextureCubeMap;
       // tcm->setNumMipmapLevels(9);
        tcm->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
        tcm->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
        tcm->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP);
        tcm->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST_MIPMAP_LINEAR);
        tcm->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
       // tcm->setUseHardwareMipMapGeneration(false);

        auto images = readCubeImages(imagePath, readOption);
        if (images.size() > 0)
        {
            std::cout << "input image miplevel is " << images[0]->getMipmapLevels().size() << std::endl;
        }
      
        for (uint32_t i = 0; i < 6; i++)
        {
            images[i]->flipHorizontal();
            tcm->setImage(i, images[i]);

            
        }
        return tcm;
      
    }
    
    void filpMipmap(osg::Image* image)
    {
        osg::Image::DataIterator imgData(image);
        unsigned int imgDataOffset = 0;
        //write main image: imageSize bytes
        for (osg::Image::DataIterator img_itr(image); img_itr.valid(); ++img_itr)
        {
            std::cout<< img_itr.size() << std::endl;
        }
    }
    void translate(const std::string& imageDir, const std::vector<std::string>& cubeImages, const std::string& outputPath, uint32_t outWidth, uint32_t outHeight, uint32_t mipmapLevel)
    {
        std::string path = imageDir;
        _dbo = new osgDB::Options("IBL");
        _dbo->setDatabasePath(path);
        auto inputMap = createCubeMap(cubeImages, _dbo.get());
        _setup(outputPath, outWidth, outHeight, mipmapLevel, inputMap);
    }
    void translate(const std::string& imageDir, const std::string& cubeImages, const std::string& outputPath, uint32_t outWidth, uint32_t outHeight, uint32_t mipmapLevel)
    {
        int width = outWidth;
        int height = outHeight;

        std::string path = imageDir;
        _dbo = new osgDB::Options("IBL");
        _dbo->setDatabasePath(path);
        auto inputMap = createCubeMap(cubeImages, _dbo.get());
        _setup(outputPath, outWidth, outHeight, mipmapLevel, inputMap);
    }
    void _setup(const std::string& outputPath, uint32_t outWidth, uint32_t outHeight, uint32_t mipmapLevel, osg::TextureCubeMap* inputMap)
    {
        auto shaderPath = "..//..//OE_PBR//Shader";
        auto shaderPath2 = "..//OE_PBR//Shader";

        osg::ref_ptr< osgDB::Options>  shaderDB = new osgDB::Options();
        if (osgDB::fileExists(osgEarth::getAbsolutePath(shaderPath)))
        {
            shaderDB->setDatabasePath(osgEarth::getAbsolutePath(shaderPath));
        }
        else {
            shaderDB->setDatabasePath(osgEarth::getAbsolutePath(shaderPath2));
        }
        shaderDB->setName("osgEarthShader");



        int w = outWidth;
        int h = outHeight;
        double offset = 2.0;
        osg::Camera* cam0 = nullptr;
        std::vector<osg::Image*> images;
        for (int i = 0; i < mipmapLevel; i++)
        {

            osg::Image* image = new osg::Image;
            image->allocateImage(w, h, 1, GL_RGBA, GL_FLOAT);

            images.push_back(image);

            auto node = createQuadGeom(1.0f * w, 1.0f * h);

            osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(node->getOrCreateStateSet());
            osgEarth::ShaderPackage shaders;
            shaders.load(vp, cube_to_quad, shaderDB.get());

            osg::ref_ptr<osg::MatrixTransform> matT = new osg::MatrixTransform();
            matT->setMatrix(osg::Matrixd::translate(0.0, 0.0, i*offset));
            matT->addChild(node);

            node->getOrCreateStateSet()->setTextureAttributeAndModes(0, inputMap, osg::StateAttribute::ON);
            node->getOrCreateStateSet()->getOrCreateUniform("IrradianceMap", osg::Uniform::SAMPLER_CUBE)->set(0);

            // debug模式下 设置uniform不生效？
            node->getOrCreateStateSet()->getOrCreateUniform("mipLevel", osg::Uniform::FLOAT)->set(i * 1.0f);

            osg::ref_ptr<osg::Camera> cam = new osg::Camera();
            if (i == 0)
            {
                cam0 = cam.get();
            }
            cam->addChild(node);
            cam->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
            cam->setClearColor(osg::Vec4(0, 0, 1, 1));
            cam->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cam->setViewport(0, 0, w, h);
            cam->setRenderOrder(osg::Camera::PRE_RENDER);
            cam->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
            cam->setImplicitBufferAttachmentMask(0, 0);


            cam->attach(osg::Camera::COLOR_BUFFER, image, 0, 0);
            cam->setProjectionMatrixAsOrtho2D(-1.0 * w / 2, (-1.0 * w / 2) + w * 1.0, -1.0 * h / 2, (-1.0 * h / 2) + h * 1.0);
            cam->setViewMatrix(
                osg::Matrixd::identity());



            root->addChild(cam);
            root->addChild(matT);

            w = w / 2;
            h = h / 2;
            offset += 0.5;
        }
        std::string fileName = outputPath;
        
        if (!_dbo->getDatabasePathList().empty())
        {
            fileName = osgDB::concatPaths(*_dbo->getDatabasePathList().begin(), fileName);
          /*  std::string path = osgDB::findDataFile(uri.full(), _dbo);*/
            std::cout << " fileName " << fileName << std::endl;

        }
        auto cb = new SnapMipmap(fileName, std::move(images));
        cam0->addPostDrawCallback(cb);
        viewer->addEventHandler(new SnapeImageHandler('p', cb));
    }
	
private:
    osg::Group* root;
    osgViewer::Viewer* viewer;

    osg::ref_ptr<osgDB::Options> _dbo;
    osg::ref_ptr<osg::Texture2D> target;
   
};