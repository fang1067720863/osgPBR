#include "ReflectionProbe.h"
#include<iostream>
#include<osg/ShapeDrawable>
#include<osg/MatrixTransform>
#include<osgShadow/ConvexPolyhedron>
#include <osgEarth/ShaderLoader>
#include <osgManipulator/TranslateAxisDragger>
using namespace osgManipulator;
osgManipulator::Dragger* createDragger()
{
	osgManipulator::Dragger* dragger = 0;

	osgManipulator::TranslateAxisDragger* d = new osgManipulator::TranslateAxisDragger();
	d->setupDefaultGeometry();
	d->setAxisLineWidth(5.0f);
	d->setPickCylinderRadius(1.0f);
	d->setConeHeight(0.2f);
	dragger = d;
	return dragger;

}

class ProbeCallback : public osgManipulator::DraggerCallback
{
public:
	ProbeCallback(osg::Group* camGp): osgManipulator::DraggerCallback()
	{
		for (int i = 0; i < 6; i++)
		{
			auto cam = dynamic_cast<osg::Camera*>(camGp->getChild(i));
			_cams.push_back(cam);
		}
	}

	std::vector<osg::observer_ptr<osg::Camera>> _cams;
	bool receive(const osgManipulator::TranslateInLineCommand& command) {

		switch (command.getStage())
		{
		case MotionCommand::START:
		{
			// Save the current matrix
			/*if (!_cam.valid())
			{
				return false;
			}*/
			/*osg::Vec3 eye, center, up, dir;
			_cam->getViewMatrixAsLookAt(eye, center, up);*/

			return true;
		}
		case MotionCommand::MOVE:
		{
			osg::Matrix localMotionMatrix = command.getMotionMatrix() * command.getLocalToWorld();
			auto t = localMotionMatrix.getTrans();

			osg::Vec3 eye, center, up, dir;
			for (auto& _cam : _cams)
			{
				_cam->getViewMatrixAsLookAt(eye, center, up);
				auto dir = center - eye;
				_cam->setViewMatrixAsLookAt(t, t + dir, up);
			}
			
			return true;
		}
		case MotionCommand::FINISH:
		{
			return true;
		}
		case MotionCommand::NONE:
		default:
			return false;
		}

	}
};

ReflectionProbe::ReflectionProbe(float n, float f, uint32_t resolution)
{

	_texture = new osg::TextureCubeMap();
	_texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	_texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	_texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
	_texture->setTextureSize(resolution, resolution);
	_texture->setInternalFormat(GL_RGBA);
	_texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	_texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	_texture->setNumMipmapLevels(5);

	createCubeCameras(n, f, resolution);
}

void ReflectionProbe::createCubeCameras(float n, float f, uint32_t resolution)
{
	auto clearColor = osg::Vec4(0, 0, 0, 0);
	unsigned int tex_width, tex_height;
	tex_width = tex_height = resolution;
	typedef std::pair<osg::Vec3, osg::Vec3> ImageData;
	const ImageData id[] =
	{
		ImageData(osg::Vec3(1, 0, 0), osg::Vec3(0, -1, 0)),  // +X
		ImageData(osg::Vec3(-1, 0, 0), osg::Vec3(0, -1, 0)), // -X
		ImageData(osg::Vec3(0, 1, 0), osg::Vec3(0, 0, 1)),   // +Y
		ImageData(osg::Vec3(0, -1, 0), osg::Vec3(0, 0, -1)), // -Y
		ImageData(osg::Vec3(0, 0, 1), osg::Vec3(0, -1, 0)),  // +Z
		ImageData(osg::Vec3(0, 0, -1), osg::Vec3(0, -1, 0))  // -Z
	};
	_camGp = new osg::Group();
	for (unsigned int i = 0; i < 6; ++i)
	{
		// create the camera
		osg::ref_ptr<osg::Camera> camera = new osg::Camera;

		camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		camera->setClearColor(clearColor);

		// set viewport
		camera->setViewport(0, 0, tex_width, tex_height);

		// set the camera to render before the main camera.
		camera->setRenderOrder(osg::Camera::PRE_RENDER);
		camera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
		camera->setCullingActive(true);
		camera->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);
		// tell the camera to use OpenGL frame buffer object where supported.
		camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		camera->setImplicitBufferAttachmentMask(0, 0);
		camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
		// attach the texture and use it as the color buffer.
		camera->attach(osg::Camera::COLOR_BUFFER, _texture, 0, i);
		camera->attach(osg::Camera::DEPTH_BUFFER, GL_DEPTH_COMPONENT);

		osg::Matrix viewMat;
		
		viewMat.makeLookAt(_pos,_pos + id[i].first, id[i].second);

		camera->setProjectionMatrixAsFrustum(-1.0, 1.0, -1.0, 1.0, n, f);
		camera->setViewMatrix(viewMat);

		_camGp->addChild(camera);


	}

}

void ReflectionProbe::setPosition(osg::Vec3 pos)
{
	for (int i = 0; i < 6; i++)
	{
		osg::Camera* cam =dynamic_cast<osg::Camera*> (_camGp->getChild(i));
		osg::Vec3 eye, center, up;
		cam->getViewMatrixAsLookAt(eye, center, up);
		auto dir = center - eye;
		cam->setViewMatrixAsLookAt(pos, pos+dir, up);
	}
	_pos = pos;
	
}

osg::Node* ReflectionProbe::getNode()
{
	auto gp = new osg::Group();

	// add cameras
	gp->addChild(_camGp);

	// add probe
	auto matrixT = new osg::MatrixTransform();
	matrixT->setMatrix(osg::Matrix::translate(_pos));
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 0.5f));
	geode->addDrawable(sd);
	matrixT->addChild(geode);

	osg::StateSet* stateset = geode->getOrCreateStateSet();
	stateset->setTextureAttribute(0, _texture, osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
	osgEarth::ShaderPackage shaders;
	const char* pick_preview = R"(
        #pragma vp_function fs, fragment_output
		out vec4 frag;
		in vec3 WorldPos;
		in vec3 vp_Normal;
		uniform samplerCube cubeMap;


		void fs(inout vec4 c) {
   
			vec3 N = normalize(vp_Normal.xyz);
			vec3 irradiance = texture(cubeMap, N).rgb;
			c = vec4(irradiance, 1.0f);
			frag = c;
		}

    )";

	osgEarth::ShaderLoader::load(vp, pick_preview);
	stateset->getOrCreateUniform("cubeMap", osg::Uniform::SAMPLER_CUBE)->set(0);
	gp->addChild(matrixT);

	// add drager
	auto dragger = createDragger();
	double scale = 20.0;
	dragger->setMatrix(osg::Matrix::scale(scale, scale, scale) *osg::Matrix::translate(_pos));
	dragger->addTransformUpdating(matrixT);
	dragger->setHandleEvents(true);
	dragger->setActivationModKeyMask(osgGA::GUIEventAdapter::MODKEY_CTRL);
	dragger->setActivationKeyEvent('a');
	dragger->addDraggerCallback(new ProbeCallback(_camGp));
	gp->addChild(dragger);

	// add camera frustum geom
	for (int i = 0; i < 6; i++)
	{
		//
		osg::Camera* camera = dynamic_cast<osg::Camera*> (_camGp->getChild(i));
		osg::Matrix debug_mvp = osg::Matrix::translate(_pos) * camera->getViewMatrix() * camera->getProjectionMatrix();
		osgShadow::ConvexPolyhedron frustum;
		frustum.setToUnitFrustum();
		frustum.transform(osg::Matrix::inverse(debug_mvp), debug_mvp);
		osg::Geometry* geom = frustum.buildGeometry(osg::Vec4(0.0, 1.0, 0.0, 1.0), osg::Vec4(0.0, 0.0, 1.0, 0.0));
		osg::Geode* geode = new osg::Geode();
		geode->addDrawable(geom);
		matrixT->addChild(geode);
	}
	
	
	return gp;
}

osg::Camera* ReflectionProbe::createDebugHUDCamera()
{
	return nullptr;
}

void ReflectionProbe::addReflectedGraph(osg::Node* graph)
{
	for (int i = 0; i < 6; i++)
	{
		osg::Camera* camera = dynamic_cast<osg::Camera*> (_camGp->getChild(i));
		camera->addChild(graph);
	}
}
