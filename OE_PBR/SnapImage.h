#pragma once


#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>

#include <osgGA/TrackballManipulator>

#include <osg/Camera>
#include <osg/RenderInfo>

#include <osgDB/WriteFile>


struct SnapImage : public osg::Camera::DrawCallback
{
    SnapImage(const std::string& filename, osg::Image* image, osg::ref_ptr<osg::Texture2D> texture) :
        _filename(filename),
        _snapImage(false),
        _image(image),
        _tex(texture)
    {
       // _image = new osg::Image;
    }

    virtual void operator () (osg::RenderInfo& renderInfo) const
    {

        if (!_snapImage) return;

        osg::notify(osg::NOTICE) << "Camera callback" << std::endl;

        osg::Camera* camera = renderInfo.getCurrentCamera();
        osg::Viewport* viewport = camera ? camera->getViewport() : 0;

        osg::notify(osg::NOTICE) << "Camera callback " << camera << " " << viewport << std::endl;

        osgDB::writeImageFile(*_image, _filename);
        float* data = (float*)_image->data(0, 0);
        fprintf(stderr, "Float pixel data: r %e g %e b %e\n", data[0], data[1], data[2]);

        osg::notify(osg::NOTICE) << "Taken screenshot, and written to '" << _filename << "'" << std::endl;
     
        //float* data = (float*)_image->data(0, 0);
        //fprintf(stderr, "Float pixel data: r %e g %e b %e\n", data[0], data[1], data[2]);

        //osgDB::writeImageFile(*_image, _filename);

        //osg::notify(osg::NOTICE) << "Taken screenshot, and written to '" << _filename << "'" << std::endl;

      /*  if (viewport && _image.valid())
        {
            _image->readPixels(int(viewport->x()), int(viewport->y()), int(viewport->width()), int(viewport->height()),
                GL_RGBA,
                GL_UNSIGNED_BYTE);
          
        }*/

        _snapImage = false;
    }

    std::string                         _filename;
    mutable bool                        _snapImage;
    osg::Image*    _image;
    osg::ref_ptr<osg::Texture2D>       _tex;
};



struct SnapeImageHandler : public osgGA::GUIEventHandler
{

    SnapeImageHandler(int key, SnapImage* si) :
        _key(key),
        _snapImage(si) {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (ea.getHandled()) return false;

        switch (ea.getEventType())
        {
        case(osgGA::GUIEventAdapter::KEYUP):
        {
           /* if (ea.getKey() == 'o')
            {
                osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
                osg::Node* node = view ? view->getSceneData() : 0;
                if (node)
                {
                    osgDB::writeNodeFile(*node, "hud.osgt");
                    osgDB::writeNodeFile(*node, "hud.osgb");
                }
                return true;
            }*/

            if (ea.getKey() == _key)
            {
                osg::notify(osg::NOTICE) << "event handler" << std::endl;
                _snapImage->_snapImage = true;
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
    osg::ref_ptr<SnapImage> _snapImage;
};
