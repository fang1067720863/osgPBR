#pragma once

#include <osg/GLExtensions>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgManipulator/TranslateAxisDragger>
#include <osgShadow/ConvexPolyhedron>
#include <osgViewer/Viewer>

#include <iostream>

#include <osgUtil/Optimizer>

#include <osg/Texture2D>
#include <osg/Texture>
#include <osg/TextureCubeMap>

#include "AdvancedMaterial.h"
#include "CubeToQuad.h"
#include "EnvLight.h"
#include "GLTFV2Reader.h"
#include "IBLBaker.h"
#include "PbrLightEffect.h"
#include "PbrMaterial.h"
#include "RayPicker.h"
#include "ReflectionProbe.h"
#include "SkyBox.h"

struct DrawCallback : public osg::Drawable::DrawCallback
{

	DrawCallback()
	    : _firstTime(true)
	{
	}

	virtual void drawImplementation(osg::RenderInfo& renderInfo, const osg::Drawable* drawable) const
	{
		osg::State& state = *renderInfo.getState();

		// ZoneNamedN(___tracy_scoped_zone, "CustomDrawCallback", true);
		// OE_PROFILING_ZONE_NAMED("CustomDrawCallback");

		drawable->drawImplementation(renderInfo);
	}

	mutable bool _firstTime;
};

osg::ref_ptr<osg::Node> CreatePbrSphere()
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();

	osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));

	geode->addDrawable(sd);

	bool usePhong = false;
	bool usePBR = true;
	if (usePhong)
	{
		auto* phong = new PhongLightingEffect();
		phong->attach(geode->getOrCreateStateSet());

		osg::ref_ptr<osg::Material> m = new osgEarth::MaterialGL3();
		m->setAmbient(m->FRONT_AND_BACK, osg::Vec4(.5, .5, .5, 1));
		m->setDiffuse(m->FRONT_AND_BACK, osg::Vec4(1, 1, 1, 1));
		m->setSpecular(m->FRONT_AND_BACK, osg::Vec4(0.2, 0.2, 0.2, 1));
		m->setEmission(m->FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
		m->setShininess(m->FRONT_AND_BACK, 40.0);
		geode->getOrCreateStateSet()->setAttributeAndModes(m, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		MaterialCallback().operator()(m, 0L);
	}
	else if (usePBR)
	{
	}

	return geode;
}

osg::ref_ptr<osg::Node> CreateExtensionedMaterialSphere()
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();

	osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));

	geode->addDrawable(sd);
	osg::ref_ptr<osgEarth::ExtensionedMaterial> m = new osgEarth::ExtensionedMaterial();
	m->setName("PBR_MATERIAL");

	m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	m->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
	m->setMetallicFactor(1.00f);
	m->setRoughnessFactor(0.88f);
	m->setAoStrength(0.15f);

	std::string dir = "C:/Users/10677/source/repos/OE_PBR/OE_PBR/Asset/Material/metal/", format = ".png";
	m->extTextureAttribute("metalMap", dir + "metal" + format, "OE_ENABLE_Metal_MAP");
	m->extTextureAttribute("roughnessMap", dir + "rough" + format, "OE_ENABLE_Roughness_MAP");

	m->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::NormalMap, dir + "normal" + format);

	m->setMaterialFile("materials/metalroughness.glsl");

	geode->getOrCreateStateSet()->setAttributeAndModes(m, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	m->setReceiveEnvLight(false);

	ExtensionedMaterialCallback().operator()(m, 0L);

	auto* pbr = new PbrLightEffect();
	pbr->attach(geode->getOrCreateStateSet());

	auto* vp = osgEarth::VirtualProgram::get(geode->getOrCreateStateSet());
	vp->setShaderLogging(true);

	return geode;
}
osg::Node* CreateLight(osg::Light* myLight1, osg::Node* node)
{
	osg::Group* lightGroup = new osg::Group;

	float modelSize = 5.0f;

	myLight1->setLightNum(0);
	myLight1->setPosition(osg::Vec4(1.0f, 1.0f, 0.0f, 0.0f));
	myLight1->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	myLight1->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	myLight1->setSpotCutoff(20.0f);
	myLight1->setSpotExponent(0.5f);
	myLight1->setDirection(osg::Vec3(-0.382353f, -0.254902f, -0.382353f));

	osg::LightSource* lightS1 = new osg::LightSource;
	lightS1->setLight(myLight1);
	lightS1->setLocalStateSetModes(osg::StateAttribute::ON);

	/* lightS1->setStateSetModes(*rootStateSet, osg::StateAttribute::ON);*/
	lightGroup->addChild(lightS1);
#ifdef NDEBUG
	auto cb = new LightSourceGL3UniformGenerator();
	lightS1->addCullCallback(cb);
#endif

	/* EnvLightSource* els = new EnvLightSource;
	 els->addCullCallback(new EnvLightGL3UniformGenerator());
	 lightGroup->addChild(els);*/
	return lightGroup;
}

std::vector<osg::ref_ptr<StandardPBRMaterial> > createNoTexMaterials()
{
	std::vector<osg::ref_ptr<StandardPBRMaterial> > result;

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			osg::ref_ptr<osgEarth::StandardPBRMaterial> m = new osgEarth::StandardPBRMaterial();

			m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
			m->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
			m->setMetallicFactor(0.11f * i + 0.01);
			m->setRoughnessFactor(0.11f * j + 0.01);
			m->setReceiveEnvLight(true);
			result.push_back(m);
		}
	}

	return result;
}
std::vector<osg::ref_ptr<ExtensionedMaterial> > createMaterials()
{
	std::vector<osg::ref_ptr<ExtensionedMaterial> > result;

	auto absolutePath = osgEarth::Util::getAbsolutePath("Asset/Material");
	osg::ref_ptr<osgDB::Options> dbo = new osgDB::Options();
	if (osgDB::fileExists(absolutePath))
	{
		dbo->setDatabasePath(absolutePath);
	}

	osg::ref_ptr<osgEarth::ExtensionedMaterial> grass = new osgEarth::ExtensionedMaterial();
	grass->setName("grass");
	grass->setDataBaseOption(dbo);

	grass->setBaseColorFactor(osg::Vec4f(0.5f, 0.5f, 0.5f, 0.5f));
	grass->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
	grass->setMetallicFactor(1.0f);
	grass->setRoughnessFactor(1.0f);

	grass->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::MetalRoughenssMap, "grass/metalRoughness.png");
	grass->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::EmissiveMap, "grass/emissive.png");
	grass->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::NormalMap, "grass/normal.png");
	grass->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::OcclusionMap, "grass/ao.png");
	grass->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::BaseColorMap, "grass/albedo.png");
	grass->setReceiveEnvLight(true);

	grass->extTextureAttribute("metalMap", "grass/metallic.png", "OE_ENABLE_Metal_MAP");
	grass->extTextureAttribute("roughnessMap", "grass/roughness.png", "OE_ENABLE_Roughness_MAP");
	grass->setMaterialFile("materials/metalroughness.glsl");
	result.push_back(grass);

	osg::ref_ptr<osgEarth::ExtensionedMaterial> m = new osgEarth::ExtensionedMaterial();
	m->setName("metal");
	m->setDataBaseOption(dbo);

	m->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	m->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
	m->setMetallicFactor(1.00f);
	m->setRoughnessFactor(1.0f);
	m->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::BaseColorMap, "metal/albedo.png");
	m->setReceiveEnvLight(true);

	m->extTextureAttribute("metalMap", "metal/metal.png", "OE_ENABLE_Metal_MAP");
	m->extTextureAttribute("roughnessMap", "metal/rough.png", "OE_ENABLE_Roughness_MAP");
	m->setMaterialFile("materials/metalroughness.glsl");
	result.push_back(m);

	osg::ref_ptr<osgEarth::ExtensionedMaterial> marber = new osgEarth::ExtensionedMaterial();
	marber->setName("dalishi");
	marber->setDataBaseOption(dbo);
	marber->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	marber->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
	marber->setMetallicFactor(0.08f);
	marber->setRoughnessFactor(0.176f);

	marber->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::NormalMap, "gray/normal.png");
	marber->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::OcclusionMap, "gray/ao.png");
	marber->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::BaseColorMap, "gray/albedo.png");
	marber->setReceiveEnvLight(true);

	marber->extTextureAttribute("metalMap", "gray/metallic.png", "OE_ENABLE_Metal_MAP");
	marber->extTextureAttribute("roughnessMap", "gray/roughness.png", "OE_ENABLE_Roughness_MAP");
	marber->setMaterialFile("materials/metalroughness.glsl");
	result.push_back(marber);

	osg::ref_ptr<osgEarth::ExtensionedMaterial> rock = new osgEarth::ExtensionedMaterial();
	rock->setName("rock");
	rock->setDataBaseOption(dbo);

	rock->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	rock->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
	rock->setMetallicFactor(0.5f);
	rock->setRoughnessFactor(0.668f);
	rock->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::NormalMap, "rock/normal.png");
	rock->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::OcclusionMap, "rock/ao.png");
	rock->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::BaseColorMap, "rock/albedo.png");
	rock->setReceiveEnvLight(true);

	rock->extTextureAttribute("metalMap", "rock/metallic.png", "OE_ENABLE_Metal_MAP");
	rock->extTextureAttribute("roughnessMap", "rock/roughness.png", "OE_ENABLE_Roughness_MAP");
	rock->setMaterialFile("materials/metalroughness.glsl");
	result.push_back(rock);

	osg::ref_ptr<osgEarth::ExtensionedMaterial> water = new osgEarth::ExtensionedMaterial();
	water->setName("water");
	water->setDataBaseOption(dbo);

	water->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	water->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
	water->setMetallicFactor(0.04f);
	water->setRoughnessFactor(0.668f);
	water->setReceiveEnvLight(true);

	water->extTextureAttribute("water_M", "unreal/Textures/T_Water_M.tga", "water_M");
	water->extTextureAttribute("water_N", "unreal/Textures/T_Water_N.tga", "water_N");
	water->extTextureAttribute("perlin_Noise", "unreal/Textures/T_Perlin_Noise_M.tga", "perlin_Noise");
	water->setMaterialFile("materials/water_ocean.glsl");
	result.push_back(water);

	osg::ref_ptr<osgEarth::ExtensionedMaterial> gold = new osgEarth::ExtensionedMaterial();
	gold->setName("gold");
	gold->setDataBaseOption(dbo);

	gold->setBaseColorFactor(osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
	gold->setEmissiveFactor(osg::Vec3f(0.0f, 0.0f, 0.0f));
	gold->setMetallicFactor(0.8f);
	gold->setRoughnessFactor(0.168f);
	gold->setReceiveEnvLight(true);

	gold->extTextureAttribute("T_MacroVariation", "gold/T_MacroVariation.tga", "T_MacroVariation");
	gold->extTextureAttribute("T_Metal_Gold_D", "gold/T_Metal_Gold_D.tga", "T_Metal_Gold_D");
	gold->setMaterialImage(osgEarth::StandardPBRMaterial::TextureEnum::NormalMap, "gold/T_Metal_Gold_N.tga");
	gold->setMaterialFile("materials/mat_gold.glsl");
	result.push_back(gold);

	return result;
}

std::vector<osg::ref_ptr<AdvancedMaterial> > createAdvancedMaterials()
{
	auto absolutePath = osgEarth::Util::getAbsolutePath("Asset/Material");
	osg::ref_ptr<osgDB::Options> dbo = new osgDB::Options();
	if (osgDB::fileExists(absolutePath))
	{
		dbo->setDatabasePath(absolutePath);
	}

	std::vector<osg::ref_ptr<AdvancedMaterial> > result;
	{
		osg::ref_ptr<osgEarth::AdvancedMaterial> m = new osgEarth::AdvancedMaterial();
		m->setName("sheen");
		m->setDataBaseOption(dbo);
		m->setUseSheen(true);
		m->setSheenColor(osg::Vec3f(1.0f, 0.0f, 1.0f));
		m->setSheenRoughness(0.5f);
		result.push_back(m);
	}
	{
		osg::ref_ptr<osgEarth::AdvancedMaterial> m1 = new osgEarth::AdvancedMaterial();
		m1->setDataBaseOption(dbo);
		m1->setName("Clearcoat");
		m1->setUseClearcoat(true);
		m1->setMetallicFactor(0.1);
		m1->setClearcoat(1.0f);
		m1->setClearcoatRoughness(0.1f);

		m1->setMaterialImage(StandardPBRMaterial::NormalMap, "brick/T_Brick_Clay_New_N.tga");
		m1->setMaterialImage(StandardPBRMaterial::BaseColorMap, "brick/T_Brick_Clay_New_D.tga");
		m1->extTextureAttribute("clearcoatRoughnessMap", "brick/T_Brick_Clay_New_D.tga", "USE_CLEARCOAT_ROUGHNESSMAP");
		result.push_back(m1);

		{
			osg::ref_ptr<osgEarth::AdvancedMaterial> m2 = new osgEarth::AdvancedMaterial();
			m2->setName("Transmission");
			m2->setDataBaseOption(dbo);
			m2->setUseTransmission(true);
			m2->setTransmission(1.0);
			m2->setThickness(0.1f);
			m2->setMetallicFactor(0.1);
			m2->setRoughnessFactor(0.1f);
			result.push_back(m2);
		}
	}
	return result;
}
osg::ref_ptr<osg::Group> createMaterialSpheres(int matType = 1)
{
	osg::ref_ptr<osg::Group> gp = new osg::Group();

	if (matType == 1)
	{
		std::vector<osg::ref_ptr<StandardPBRMaterial> > materials = std::move(createNoTexMaterials());
		int row, col;
		row = col = 9;

		for (size_t i = 0; i < row; i++)
		{
			for (size_t j = 0; j < col; j++)
			{
				osg::ref_ptr<osg::Geode> geode = new osg::Geode();
				osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));
				geode->addDrawable(sd);
				sd->setDrawCallback(new DrawCallback());
				geode->getOrCreateStateSet()->setAttributeAndModes(materials[i * col + j], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
				geode->getOrCreateStateSet()->setName("110");
				// geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
				osg::ref_ptr<osg::MatrixTransform> matrixT = new osg::MatrixTransform();

				matrixT->setMatrix(osg::Matrix::translate((double)i * 5.0, (double)j * 5.0, 0.0));
				matrixT->addChild(geode);
				matrixT->setName(materials[i * col + j]->getName());

				PBRMaterialCallback().operator()(materials[i * col + j], 0L);
				auto* pbr = new PbrLightEffect();
				pbr->attach(geode->getOrCreateStateSet());
				gp->addChild(matrixT);
			}
		}
	}
	else if (matType == 2)
	{
		auto materials = std::move(createMaterials());
		for (size_t i = 0; i < materials.size(); i++)
		{
			std::cout << " gsfdg " << i << std::endl;
			osg::ref_ptr<osg::Geode> geode = new osg::Geode();
			osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));
			geode->addDrawable(sd);
			geode->getOrCreateStateSet()->setAttributeAndModes(materials[i], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

			osg::ref_ptr<osg::MatrixTransform> matrixT = new osg::MatrixTransform();

			matrixT->setMatrix(osg::Matrix::translate((double)i * 5.0, 0.0, 0.0));
			matrixT->addChild(geode);
			matrixT->setName(materials[i]->getName());
			// materials[i]->setUpdateCallback(new ExtensionedMaterialCallback());
			ExtensionedMaterialCallback().operator()(materials[i], 0L);
			auto* pbr = new PbrLightEffect();
			pbr->attach(geode->getOrCreateStateSet());
			gp->addChild(matrixT);
		}
	}
	else
	{
		auto materials = std::move(createAdvancedMaterials());
		for (size_t i = 0; i < materials.size(); i++)
		{
			std::cout << " gsfdg " << i << std::endl;
			osg::ref_ptr<osg::Geode> geode = new osg::Geode();
			osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3f(0.0f, 0.0f, 0.0f), 2.0f));
			geode->addDrawable(sd);
			geode->getOrCreateStateSet()->setAttributeAndModes(materials[i], osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

			osg::ref_ptr<osg::MatrixTransform> matrixT = new osg::MatrixTransform();

			matrixT->setMatrix(osg::Matrix::translate((double)i * 5.0, 0.0, 0.0));
			matrixT->addChild(geode);
			matrixT->setName(materials[i]->getName());
			if (materials[i]->getUseTransmission())
			{
				geode->setNodeMask(TRANSLUCENT_MASK);
			}

			auto* pbr = new PbrLightEffect();
			pbr->attach(geode->getOrCreateStateSet());
			gp->addChild(matrixT);
		}
	}

	return gp;
}
osg::Camera* createHUDCamera(osg::Texture* _texture, osgDB::Options* shaderDB)
{
	osg::Camera* camera = new osg::Camera;

	// set the projection matrix
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0, 200, 0, 200));

	// set the view matrix
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());

	// only clear the depth buffer
	camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// draw subgraph after main camera view.
	camera->setRenderOrder(osg::Camera::POST_RENDER);

	// we don't want the camera to grab event focus from the viewers main camera(s).
	camera->setAllowEventFocus(false);

	{ // Add geode and drawable with BaseClass display
	  // create geode to contain hud drawables
		osg::Geode* geode = new osg::Geode;
		camera->addChild(geode);
		auto _hudOrigin = osg::Vec2(-100, -100);
		auto _hudSize = osg::Vec2(200, 200);
		// finally create and attach hud geometry
		osg::Geometry* geometry = osg::createTexturedQuadGeometry(osg::Vec3(_hudOrigin[0], _hudOrigin[1], 0),
		                                                          osg::Vec3(_hudSize[0], 0, 0),
		                                                          osg::Vec3(0, _hudSize[1], 0));

		osg::StateSet* stateset = geometry->getOrCreateStateSet();
		stateset->setTextureAttribute(0, _texture, osg::StateAttribute::ON);
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateset);
		osgEarth::ShaderPackage shaders;
		shaders.load(vp, "cube_to_quad.glsl", shaderDB);

		// ss->setTextureAttributeAndModes(0, inputMap, osg::StateAttribute::ON);
		stateset->getOrCreateUniform("IrradianceMap", osg::Uniform::SAMPLER_CUBE)->set(0);
		stateset->getOrCreateUniform("mipLevel", osg::Uniform::FLOAT)->set(0 * 1.0f);

		geode->addDrawable(geometry);
	}
	return camera;
}
osg::Group* setupCamera(osg::Node* reflectedSubgraph, osg::Texture* texture)
{
	auto clearColor = osg::Vec4(0, 0, 0, 0);

	unsigned int tex_width, tex_height;
	tex_width = tex_height = 256;
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
	osg::Group* cameras = new osg::Group();
	for (unsigned int i = 0; i < 6; ++i)
	{
		// create the camera
		osg::Camera* camera = new osg::Camera;

		camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		camera->setClearColor(clearColor);

		// set viewport
		camera->setViewport(0, 0, tex_width, tex_height);

		// set the camera to render before the main camera.
		camera->setRenderOrder(osg::Camera::PRE_RENDER);
		camera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);

		// tell the camera to use OpenGL frame buffer object where supported.
		camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		camera->setImplicitBufferAttachmentMask(0, 0);
		camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
		// attach the texture and use it as the color buffer.
		camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, i);

		osg::Matrix viewMat;
		osg::Vec3d position(1.0, 6.0, 0.0);
		viewMat.makeLookAt(position, position + id[i].first, id[i].second);

		// add subgraph to render
		camera->addChild(reflectedSubgraph);
		double size = 5.0;
		// camera->setProjectionMatrixAsOrtho2D(-size, size, -size, size);
		camera->setProjectionMatrixAsFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10.0);
		camera->setViewMatrix(viewMat);

		if (i == 0)
		{
			osg::Matrix debug_mvp = camera->getViewMatrix() * camera->getProjectionMatrix();
			osgShadow::ConvexPolyhedron frustum;
			frustum.setToUnitFrustum();
			frustum.transform(osg::Matrix::inverse(debug_mvp), debug_mvp);
			osg::Geometry* geom = frustum.buildGeometry(osg::Vec4(0.0, 1.0, 0.0, 1.0), osg::Vec4(0.0, 0.0, 1.0, 0.0));
			osg::Geode* geode = new osg::Geode();
			geode->addDrawable(geom);
			cameras->addChild(geode);
		}

		cameras->addChild(camera);
	}
	return cameras;
}
