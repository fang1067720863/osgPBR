#pragma once


#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>

#include <osgGA/TrackballManipulator>

#include <osg/Camera>
#include <osg/RenderInfo>

#include <osgDB/WriteFile>
#include"ddsNew.h"
class SwitchOption
{
public:
    SwitchOption() :once(false) {}

    mutable bool once;
};

struct SnapImage : public SwitchOption, public osg::Camera::DrawCallback
{
    SnapImage(const std::string& filename, osg::Image* image) :
        _filename(filename),
        _image(image),
        _tex(nullptr)
    {
       // _image = new osg::Image;
    }
    SnapImage(const std::string& filename, osg::Image* image, osg::Texture2D * tex) :
        _filename(filename),
        _image(image),
        _tex(tex)
    {
        // _image = new osg::Image;
    }
    virtual void operator () (osg::RenderInfo& renderInfo) const
    {

        if (!once) return;
        osg::ref_ptr<osg::State> state = new osg::State;
        state->initializeExtensionProcs();
        osg::notify(osg::NOTICE) << "Camera callback" << std::endl;

        osg::Camera* camera = renderInfo.getCurrentCamera();
        osg::Viewport* viewport = camera ? camera->getViewport() : 0;

        GLboolean binding1D = GL_FALSE, binding2D = GL_FALSE, bindingRect = GL_FALSE, binding3D = GL_FALSE, binding2DArray = GL_FALSE, bindingCubeMap = GL_FALSE;

        glGetBooleanv(GL_TEXTURE_BINDING_1D, &binding1D);
        glGetBooleanv(GL_TEXTURE_BINDING_2D, &binding2D);
        glGetBooleanv(GL_TEXTURE_BINDING_RECTANGLE, &bindingRect);
        glGetBooleanv(GL_TEXTURE_BINDING_3D, &binding3D);
        glGetBooleanv(GL_TEXTURE_BINDING_CUBE_MAP, &bindingCubeMap);

        if (binding1D)
        {
            std::cout << "1D" << std::endl;
        }
        if (binding2D)
        {
            unsigned contextID = renderInfo.getContextID();
            std::cout<<"contextID"<<contextID<<std::endl;
            _tex->getImage()->readImageFromCurrentTexture(contextID, true);
            osgDB::writeImageFile(*_tex->getImage(), _filename);
            once = false;
        }
        if (bindingRect)
        {
            std::cout << "RecD" << std::endl;
        }
        if (binding3D)
        {
            std::cout << "3D" << std::endl;
        }
        if (bindingCubeMap)
        {
            std::cout << "CubeD" << std::endl;
        }

        //if (true)
        //{
        //    _tex->setUnRefImageDataAfterApply(false);

        // 
        //    unsigned contextID = renderInfo.getContextID();
        //    std::cout<<"contextID"<<contextID<<std::endl;
        //   _tex->getImage()->readImageFromCurrentTexture(contextID, true);
        //    osgDB::writeImageFile(*_tex->getImage(), _filename);
        //}
        //else {
        //   // _image->readImageFromCurrentTexture(0, true);
        //    osgDB::writeImageFile(*_image, _filename);
        //}
        

        //
       /* osg::Camera* cam = nullptr;*/

      
        //_image->readPixels(0, 0, _tex->getTextureWidth(), _tex->getTextureHeight(), _image->getPixelFormat(), _image->getDataType());
        //osgDB::writeImageFile(*_image, _filename);
        //float* data = (float*)_image->data(0, 0);
        //fprintf(stderr, "Float pixel data: r %e g %e b %e\n", data[0], data[1], data[2]);

        //osg::notify(osg::NOTICE) << "Taken screenshot, and written to '" << _filename << "'" << std::endl;
     
       /* float* data = (float*)_image->data(0, 0);
        fprintf(stderr, "Float pixel data: r %e g %e b %e\n", data[0], data[1], data[2]);*/

        

        //osg::notify(osg::NOTICE) << "Taken screenshot, and written to '" << _filename << "'" << std::endl;

      /*  if (viewport && _image.valid())
        {
            _image->readPixels(int(viewport->x()), int(viewport->y()), int(viewport->width()), int(viewport->height()),
                GL_RGBA,
                GL_UNSIGNED_BYTE);
          
        }*/

       
    }

    std::string                         _filename;
    osg::Image*    _image;
    osg::Texture2D* _tex;
};


struct SnapMipmap: public SwitchOption, public osg::Camera::DrawCallback
{

    SnapMipmap(const std::string& filename, std::vector<osg::Image*> images):
    _images(images),
    _filename(filename)
    {
        // _image = new osg::Image;
    }

    virtual void operator () (osg::RenderInfo& renderInfo) const
    {
        if (!once) return;
        once = false;
        size_t miplevel = _images.size();

        osg::ref_ptr<osg::Image> image = new osg::Image;
        osg::Image::MipmapDataType mipmapData;
        unsigned int width = _images[0]->s();
        unsigned int height = _images[0]->t();
        
        unsigned int totalSize = 0;
        unsigned i;
        unsigned int pixelSize = 16; //rgba * GL_FLOAT
        for (i = 0; width > 0 && i < miplevel; width >>= 1, height>>=1, ++i)
        {
            if (i > 0) mipmapData.push_back(totalSize);
            totalSize += width * height * pixelSize;
        }

        unsigned char* ptr = new unsigned char[totalSize];
        image->setImage(_images[0]->s(), _images[0]->t(), 1, GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT, ptr, osg::Image::USE_NEW_DELETE, 1);
        image->setMipmapLevels(mipmapData);


        unsigned int size = width * height;
        for (i = 0; size > 0,i < miplevel ; size >>= 2,  ++i)
        {
            for (osg::Image::DataIterator itr(_images[i]); itr.valid(); ++itr)
            {
                
                memcpy(ptr, itr.data(), itr.size());
                ptr += itr.size();

               /* auto top = itr.data();
                width >>= 2;*/
            }
        }
        image->flipVertical();
        writeDDSNew(*image, _filename);
       

    }

    std::vector<osg::Image*> _images;
    std::string _filename;
};

void flipImageVertical(unsigned char* top, unsigned char* bottom, unsigned int rowSize, unsigned int rowStep)
{
    while (top < bottom)
    {
        unsigned char* t = top;
        unsigned char* b = bottom;
        for (unsigned int i = 0; i < rowSize; ++i, ++t, ++b)
        {
            unsigned char temp = *t;
            *t = *b;
            *b = temp;
        }
        top += rowStep;
        bottom -= rowStep;
    }
}

class SnapImageV2 : public SwitchOption,public osg::Drawable::DrawCallback
{
public:
    SnapImageV2(){}

    virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const
    {
        unsigned int contextId = renderInfo.getContextID();
        drawable->drawImplementation(renderInfo);
        if (!once)
        {
            return;
        }
        GLboolean binding1D = GL_FALSE, binding2D = GL_FALSE, bindingRect = GL_FALSE, binding3D = GL_FALSE, binding2DArray = GL_FALSE, bindingCubeMap = GL_FALSE;

        glGetBooleanv(GL_TEXTURE_BINDING_1D, &binding1D);
        glGetBooleanv(GL_TEXTURE_BINDING_2D, &binding2D);
        glGetBooleanv(GL_TEXTURE_BINDING_RECTANGLE, &bindingRect);
        glGetBooleanv(GL_TEXTURE_BINDING_3D, &binding3D);
        glGetBooleanv(GL_TEXTURE_BINDING_CUBE_MAP, &bindingCubeMap);

        if (binding1D)
        {
            std::cout << "1D" << std::endl;
        }
        if (binding2D)
        {
            std::cout << "2D" << std::endl;
        }
        if (bindingRect)
        {
            std::cout << "RecD" << std::endl;
        }
        if (binding3D)
        {
            std::cout << "3D" << std::endl;
        }
        if (bindingCubeMap)
        {
            std::cout << "CubeD" << std::endl;
        }
       /* osg::GLBufferObject* glbo = _dyn->getBufferObject()->getOrCreateGLBufferObject(contextId);
        glbo->bindBuffer();
        GLfloat* data = (GLfloat*)glbo->_persistentDMA + glbo->getOffset(_dyn->getBufferIndex());
        if (data)
        {
            _rate += 0.01;
            float value = sinf(_rate) * _scale + _offset;
            for (int i = 0; i < 4; i++) {
                data[i * 4 + 0] = float(i) * 0.25 * value;
            }
            glbo->commitDMA(_dyn->getBufferIndex());
        }*/
       
    }

};


struct SnapeImageHandler : public osgGA::GUIEventHandler
{

    SnapeImageHandler(int key, SwitchOption* si) :
        _key(key),
        _switch(si) {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (ea.getHandled()) return false;

        switch (ea.getEventType())
        {
        case(osgGA::GUIEventAdapter::KEYUP):
        {

            if (ea.getKey() == _key)
            {
                osg::notify(osg::NOTICE) << "event handler" << std::endl;
                _switch->once = true;
                return true;
            }

            break;
        }
        default:
            break;
        }

        return false;
    }

    int                     _key;
    SwitchOption* _switch;
  /*  osg::ref_ptr<SnapImage> _snapImage;*/
};
