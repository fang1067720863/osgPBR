#pragma once


#include<osgGA/GUIEventHandler>
#include<osgViewer/Viewer>
#include<osgUtil/LineSegmentIntersector>
#include<iostream>
class RayPicker:public osgGA::GUIEventHandler
{

public:
	using FindCallback = std::function<bool(osg::MatrixTransform*)>;
	RayPicker(osgViewer::Viewer *viewer, FindCallback cb):_viewer(viewer),_cb(cb){}
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		switch (ea.getEventType())
		{
		case osgGA::GUIEventAdapter::PUSH:
			if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
			{
				std::cout << "111";
				Pick(ea.getX(), ea.getY());
			}
			return true;
		default:
			return false;
		}
		return false;
	}
protected:
	void Pick(float x, float y)
	{
		osgUtil::LineSegmentIntersector::Intersections intersections;
		if (_viewer->computeIntersections(x,y,intersections))
		{
			std::cout << "555";
			auto hiter = intersections.begin();
			for (; hiter != intersections.end(); hiter++)
			{
				if (!hiter->nodePath.empty())
				{
					// && !(hiter->nodePath.back()->getName().empty())
					const auto& nodePath = hiter->nodePath;
					for (const auto& node : nodePath)
					{
						
						std::cout << node->getCompoundClassName()<<" " << node->getName() << std::endl;
						auto maT = node->asTransform() ? node->asTransform()->asMatrixTransform() : NULL;
						if (maT)
						{
							_cb(maT);
						}
					}
					std::cout <<  std::endl;
				}
			}

		}
	}
private:
	
	FindCallback _cb;
	osgViewer::Viewer* _viewer;
	
};