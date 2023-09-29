#include "ReflectionProbe.h"
#include<iostream>
#include<osg/ShapeDrawable>
#include<osg/MatrixTransform>
#include<osgShadow/ConvexPolyhedron>
#include <osgEarth/ShaderLoader>
#include <osgManipulator/TranslateAxisDragger>
#include"PbrMaterial.h"
#include"AdvancedMaterial.h"
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

void ReflectionProbe::setPosition(osg::Vec3d pos)
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

void ReflectionProbe::setCullMask(osg::Node::NodeMask mask)
{
	for (int i = 0; i < 6; i++)
	{
		osg::Camera* cam = dynamic_cast<osg::Camera*> (_camGp->getChild(i));
		cam->setCullMask(mask);
	}
	
}

void ReflectionProbe::installProbeCallback()
{

	auto probeCallback = [&](osg::StateAttribute* attr, osg::NodeVisitor* nv)
	{
		osgEarth::StandardPBRMaterial* material = static_cast<osgEarth::StandardPBRMaterial*>(attr);
		for (unsigned int i = 0; i < attr->getNumParents(); i++)
		{

			osg::StateSet* stateSet = attr->getParent(i);
			stateSet->getOrCreateUniform("ProbeData.pos", osg::Uniform::DOUBLE_VEC3)->set(_pos);
			stateSet->setTextureAttributeAndModes(material->texUnitCnt(), _texture, osg::StateAttribute::ON);
			stateSet->getOrCreateUniform("ProbeData.cubeMap", osg::Uniform::SAMPLER_CUBE)->set(_texture);
			material->incementTexUnit();

		}
	};
}

GenerateProbeUniforms::GenerateProbeUniforms() :osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
	setNodeMaskOverride(~0);

	
}


void GenerateProbeUniforms::apply(osg::Node& node)
{
	

	//if (node.getNodeMask() && TRANSLUCENT_MASK==0)
	//{
	//	//先只给半透物体加上
	//	return;
	//}
	osg::StateSet* ss = node.getStateSet();
	
	if (ss)
	{
		if (_statesets.find(ss) == _statesets.end())
		{
			const osg::StateSet::RefAttributePair* rap = ss->getAttributePair(osg::StateAttribute::MATERIAL);
			if (rap)
			{
				osgEarth::StandardPBRMaterial* material = dynamic_cast<osgEarth::StandardPBRMaterial*>(rap->first.get());
				
				if (material)
				{
					if (material->getUpdateCallback())
					{
						if (auto aMat = dynamic_cast<osgEarth::AdvancedMaterial*>(material))
						{
							
							aMat->addUpdateCallback("probe", [&](osg::StateAttribute* attr, osg::NodeVisitor* nv)
								{
									
									const auto probe = ProbeManager::instance()->getProbe(0);
									osgEarth::AdvancedMaterial* material = static_cast<osgEarth::AdvancedMaterial*>(attr);
									for (unsigned int i = 0; i < attr->getNumParents(); i++)
									{

										osg::StateSet* stateSet = attr->getParent(i);
									/*	int unit = material->getOrCreateTexUnit("probeMap");
										stateSet->getOrCreateUniform("oe_probe.pos", osg::Uniform::DOUBLE_VEC3)->set(probe->getPos());
										stateSet->setTextureAttributeAndModes(unit, probe->getCubeMap(), osg::StateAttribute::ON);
										stateSet->getOrCreateUniform("probeMap", osg::Uniform::SAMPLER_CUBE)->set(unit);*/
										auto map = ProbeManager::instance()->getTransmissionMap();
										if (map)
										{
											int unit = material->getOrCreateTexUnit("translucentMap");
											stateSet->setTextureAttributeAndModes(unit, map,osg::StateAttribute::ON);
											stateSet->getOrCreateUniform("translucentMap", osg::Uniform::SAMPLER_2D)->set(unit);
										}
										

									}
								});
						}
						else {

						}

					}
					

				}

				// mark this stateset as visited.
				_statesets.insert(ss);
			}
		}
	}
	traverse(node);
}

osg::ref_ptr<ProbeManager>& ProbeManager::instance()
{
	static osg::ref_ptr<ProbeManager> s_registry = new ProbeManager();
	/*  if (erase)
	  {
		  s_registry->destruct();
		  s_registry = 0;
	  }*/
	return s_registry; // will return NULL on erase
}

void ProbeManager::addProbe(osg::ref_ptr<ReflectionProbe> probe)
{
	_probes.push_back(probe);
}

osg::ref_ptr<ReflectionProbe> ProbeManager::createProbe()
{
	return osg::ref_ptr<ReflectionProbe>();
}

osg::ref_ptr<ReflectionProbe> ProbeManager::getProbe(int index) const
{
	if (index >= _probes.size())
	{
		return 0;
	}
	return _probes[index];
}

bool ProbeManager::empty() const
{
	if (_probes.size() == 0)
	{
		return true;
	}
	return false;
}

TransparentCamera::TransparentCamera():_rttSize(256)
{
}

void TransparentCamera::setView(osgViewer::View* view)
{
	if (view != _view.get())
	{
		_view = view;
		/*if (_graph.valid() == false)
		{
			_graph = view->getSceneData();
		}*/

		setupRTT(view);
	}
}

void TransparentCamera::setGraph(osg::Node* value)
{
	_graph = value;

	// remove old graph
	if (_rtt.valid())
		_rtt->removeChildren(0, _rtt->getNumChildren());

	// install new graph
	if (value)
		_rtt->addChild(value);
}

osg::Texture2D* TransparentCamera::getOrCreateTexture()
{
	osg::Texture2D* _debugTex = new osg::Texture2D(_pickImage.get());
	_debugTex->setTextureSize(_pickImage->s(), _pickImage->t());
	_debugTex->setUnRefImageDataAfterApply(false);
	_debugTex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST); // no filtering
	_debugTex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST); // no filtering
	_debugTex->setMaxAnisotropy(1.0f); // no filtering
	return _debugTex;
}

void TransparentCamera::traverse(osg::NodeVisitor& nv)
{
	if (nv.getVisitorType() == nv.CULL_VISITOR)
	{
		if (_rtt.valid() && _view.valid())
		{
			

		_rtt->setProjectionResizePolicy(_view->getCamera()->getProjectionResizePolicy());
		_rtt->setProjectionMatrix(_view->getCamera()->getProjectionMatrix());
		_rtt->setViewMatrix(_view->getCamera()->getViewMatrix());
		//_rtt->inheritCullSettings(*(_view->getCamera()), _view->getCamera()->getInheritanceMask());

		_rtt->accept(nv);

				
		}
	}
}

TransparentCamera::~TransparentCamera()
{
}

void TransparentCamera::setupRTT(osgViewer::View*view)
{
	_pickImage = new osg::Image();
	_pickImage->allocateImage(_rttSize, _rttSize, 1, GL_RGBA, GL_UNSIGNED_BYTE);
	memset(_pickImage->data(), 0, _pickImage->getTotalSizeInBytes());

	// Make an RTT camera and bind it to our image.
	_rtt = new osg::Camera();
	_rtt->setView(view); // so we have access to the 'real' viewport dimensions
	_rtt->setName("osgEarth.ObjectIDPicker");
	_rtt->addChild(_graph.get());
	_rtt->setClearColor(osg::Vec4(0, 0, 0, 0));
	_rtt->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_rtt->setViewport(0, 0, _rttSize, _rttSize);
	_rtt->setRenderOrder(osg::Camera::PRE_RENDER);
	_rtt->setReferenceFrame(osg::Transform::ABSOLUTE_RF_INHERIT_VIEWPOINT);
	_rtt->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	_rtt->attach(osg::Camera::COLOR_BUFFER0, _pickImage.get());
	_rtt->setSmallFeatureCullingPixelSize(-1.0f);

	_rtt->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
	_rtt->setCullingActive(true);
	_rtt->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);
	_rtt->setCullMask(~0x5);

	ProbeManager::instance()->setTransmissionMap(getOrCreateTexture());



	osg::StateSet* rttSS = _rtt->getOrCreateStateSet();

	// disable all the things that break ObjectID picking:
	osg::StateAttribute::GLModeValue disable =
		osg::StateAttribute::OFF |
		osg::StateAttribute::OVERRIDE |
		osg::StateAttribute::PROTECTED;

	rttSS->setMode(GL_LIGHTING, disable);
	rttSS->setMode(GL_CULL_FACE, disable);
	rttSS->setMode(GL_ALPHA_TEST, disable);



	// Disabling GL_BLEND is not enough, because osg::Text re-enables it
	// without regard for the OVERRIDE.
	/*rttSS->setAttributeAndModes(
		new osg::BlendFunc(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO),
		osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);*/

	// install the picking shaders:
	//VirtualProgram* vp = new VirtualProgram();
	//vp->setName(typeid(*this).name());
	//Shaders shaders;
	//shaders.load(vp, shaders.RTTPicker);
	//// Install shaders and bindings from the ObjectIndex:
	//Registry::objectIndex()->loadShaders(vp);
	//rttSS->setAttribute(vp);

	//// designate this as a pick camera
	//rttSS->setDefine("OE_LIGHTING", osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

	//// default value for the objectid override uniform:
	//rttSS->addUniform(new osg::Uniform(Registry::objectIndex()->getObjectIDUniformName().c_str(), 0u));
}
