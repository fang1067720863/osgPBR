//#pragma once
//
////author:1067720863@qq.com
////create date:2023/04
////decription: Cook_BRDF Light
//
//#include<Windows.h>
//
//#include <osg/StateSet>
//#include<osg/Texture2D>
//#include <osg/Uniform>
//#include <osg/observer_ptr>
//
//
//#include <osgEarth/Common>
//#include<osgEarth/Threading>
//#include<osgEarth/URI>
//#include <osgEarth/ShaderLoader>
//#include<osgEarth/VirtualProgram>
//#include <set>
//#include <osg/Node>
//#include <osg/NodeVisitor>
//#include <osg/LightSource>
//#include <osg/Light>
//#include<osg/Texture>
//#include<osg/Vec4>
//#include<osg/Camera>
//#include<osg/State>
//#include<osg/StateAttribute>
//#include <osg/TextureCubeMap>
//#include <osgEarth/Common>
//#include <osgEarth/Threading>
//#include"Export.h"
//#include <osgViewer/Viewer>
//
//class IBLTechnique : public osg::Object {
//
//public:
//    IBLTechnique() {}
//    IBLTechnique(osg::Group* _root, osgViewer::Viewer* _viewer) :root(_root), viewer(_viewer) {}
//    virtual ~IBLTechnique() {}
//
//    virtual osg::Object* cloneType() const override { return new IBLTechnique(); }
//    virtual osg::Object* clone(const osg::CopyOp& copyop) const override { return new IBLTechnique(*this, copyop); }
//
//    IBLTechnique(const IBLTechnique& es, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) {}
//    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const IBLTechnique*>(obj) != NULL; }
//    virtual const char* libraryName() const { return "osgShadow"; }
//    virtual const char* className() const { return "ShadowTechnique"; }
//    struct Config
//    {
//        bool baked{ false };
//    };
//    void startUp();
//
//    void createIrridiancePass();
//
//    void createPrefilterPass();
//
//    void createBrdfLUTPass();
//
//    void createHDRCubePass();
//
//    void createTextures();
//
//    osg::Node* createSlaveCameras(osg::Node* box, osg::Texture* tex, unsigned int level = 0, unsigned int w = 256, unsigned int h = 256, bool generateMipmap = false);
//
//    osg::ref_ptr<osg::Node> createHDRBox();
//
//private:
//    osg::ref_ptr<osg::TextureCubeMap> envCubeMap;
//    osg::ref_ptr<osg::TextureCubeMap> irridianceMap;
//    osg::ref_ptr<osg::TextureCubeMap> prefilterMap;
//    osg::ref_ptr<osg::Texture2D> brdfLUTMap;
//    osg::ref_ptr<osg::Texture2D> hdrMap;
//
//    osg::Group* root;
//    osgViewer::Viewer* viewer;
//};
