#ifndef DEBUGDRAWER_H
#define DEBUGDRAWER_H

// CREDITS (Public Domain): Adapted from http://ogre3d.org/tikiwiki/Debug+Drawing+Utility+Class
#pragma once
 
#include "libsandboxcore_global.h"
#include <OgreVector3.h>
#include <OgreColourValue.h>
#include <OgreString.h>
#include <QObject>
#include <map>
#include <vector>
#include <stdint.h>

namespace Ogre
{
class SceneManager;
class ManualObject;
}
 
typedef std::pair<Ogre::Vector3, Ogre::ColourValue> VertexPair;

#define DEFAULT_ICOSPHERE_RECURSION_LEVEL	1

class DLL_EXPORT IcoSphere
{
public:
	struct TriangleIndices
	{
		int v1, v2, v3;
 
		TriangleIndices(int _v1, int _v2, int _v3) : v1(_v1), v2(_v2), v3(_v3) {}
 
		bool operator < (const TriangleIndices &o) const { return v1 < o.v1 && v2 < o.v2 && v3 < o.v3; }
	};
 
	struct LineIndices
	{
		int v1, v2;
 
		LineIndices(int _v1, int _v2) : v1(_v1), v2(_v2) {}
 
		bool operator == (const LineIndices &o) const
		{
			return (v1 == o.v1 && v2 == o.v2) || (v1 == o.v2 && v2 == o.v1);
		}
	};
 
	IcoSphere();
	~IcoSphere();
 
	void create(int recursionLevel);
    void addToLineIndices(int baseIndex, std::vector<int> *target);
    int addToVertices(std::vector<VertexPair> *target, const Ogre::Vector3 &position, const Ogre::ColourValue &colour, float scale);
    void addToTriangleIndices(int baseIndex, std::vector<int> *target);
 
private:
	int addVertex(const Ogre::Vector3 &vertex);
	void addLineIndices(int index0, int index1);
	void addTriangleLines(int index0, int index1, int index2);
	int getMiddlePoint(int index0, int index1);
	void addFace(int index0, int index1, int index2);
 
	void removeLineIndices(int index0, int index1);
 
	std::vector<Ogre::Vector3> vertices;
    std::vector<LineIndices> lineIndices;
    std::vector<int> triangleIndices;
    std::vector<TriangleIndices> faces;
    std::map<int64_t, int> middlePointIndexCache;
	int index;
};
 
class DLL_EXPORT DebugDrawer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ getEnabled WRITE setEnabled)
public: 
    Ogre::String getName() const;

	void build();
 
	void setIcoSphereRecursionLevel(int recursionLevel);
 
    Q_INVOKABLE void drawLine(const Ogre::Vector3 &start, const Ogre::Vector3 &end, const Ogre::ColourValue &colour);
    Q_INVOKABLE void drawCircle(const Ogre::Vector3 &centre, float radius, int segmentsCount, const Ogre::ColourValue& colour, bool isFilled = false);
    Q_INVOKABLE void drawCylinder(const Ogre::Vector3 &centre, float radius, int segmentsCount, float height, const Ogre::ColourValue& colour, bool isFilled = false);
    Q_INVOKABLE void drawQuad(const Ogre::Vector3 *vertices, const Ogre::ColourValue& colour, bool isFilled = false);
    Q_INVOKABLE void drawCuboid(const Ogre::Vector3 *vertices, const Ogre::ColourValue& colour, bool isFilled = false);
    Q_INVOKABLE void drawSphere(const Ogre::Vector3 &centre, float radius, const Ogre::ColourValue& colour, bool isFilled = false);
    Q_INVOKABLE void drawTetrahedron(const Ogre::Vector3 &centre, float scale, const Ogre::ColourValue& colour, bool isFilled = false);
    Q_INVOKABLE void drawArrow(const Ogre::Vector3 &start, const Ogre::Vector3 &end, const Ogre::ColourValue &colour, float headRadius = 0.02f);

    Q_INVOKABLE bool getEnabled() { return isEnabled; }
    Q_INVOKABLE void setEnabled(bool _isEnabled) { isEnabled = _isEnabled; }
	void switchEnabled() { isEnabled = !isEnabled; }
	
    Q_INVOKABLE void clear();
 
private:
    DebugDrawer(const Ogre::String& name, Ogre::SceneManager *_sceneManager, float _fillAlpha);
    ~DebugDrawer();

    // The scene acts as a factory class for DebugDrawers.
    friend class Scene;

    Ogre::String name;
	Ogre::SceneManager *sceneManager;
	Ogre::ManualObject *manualObject;
	float fillAlpha;
	IcoSphere icoSphere;
	
	bool isEnabled;
    // Added size tracking between build calls - for caching purposes.
    size_t lineIndicesSize, vertexIndicesSize;

    std::vector<VertexPair> lineVertices, triangleVertices;
    std::vector<int> lineIndices, triangleIndices;
 
	int linesIndex, trianglesIndex;

	void initialise();
	void shutdown();
 
	void buildLine(const Ogre::Vector3& start, const Ogre::Vector3& end, const Ogre::ColourValue& colour, float alpha = 1.0f);
	void buildQuad(const Ogre::Vector3 *vertices, const Ogre::ColourValue& colour, float alpha = 1.0f);
	void buildFilledQuad(const Ogre::Vector3 *vertices, const Ogre::ColourValue& colour, float alpha = 1.0f);
	void buildFilledTriangle(const Ogre::Vector3 *vertices, const Ogre::ColourValue& colour, float alpha = 1.0f);
	void buildCuboid(const Ogre::Vector3 *vertices, const Ogre::ColourValue& colour, float alpha = 1.0f);
	void buildFilledCuboid(const Ogre::Vector3 *vertices, const Ogre::ColourValue& colour, float alpha = 1.0f);
 
	void buildCircle(const Ogre::Vector3 &centre, float radius, int segmentsCount, const Ogre::ColourValue& colour, float alpha = 1.0f);
	void buildFilledCircle(const Ogre::Vector3 &centre, float radius, int segmentsCount, const Ogre::ColourValue& colour, float alpha = 1.0f);
 
	void buildCylinder(const Ogre::Vector3 &centre, float radius, int segmentsCount, float height, const Ogre::ColourValue& colour, float alpha = 1.0f);
	void buildFilledCylinder(const Ogre::Vector3 &centre, float radius, int segmentsCount, float height, const Ogre::ColourValue& colour, float alpha = 1.0f);

	void buildTetrahedron(const Ogre::Vector3 &centre, float scale, const Ogre::ColourValue &colour, float alpha = 1.0f);
	void buildFilledTetrahedron(const Ogre::Vector3 &centre, float scale, const Ogre::ColourValue &colour, float alpha = 1.0f);

	int addLineVertex(const Ogre::Vector3 &vertex, const Ogre::ColourValue &colour);
	void addLineIndices(int index1, int index2);
 
	int addTriangleVertex(const Ogre::Vector3 &vertex, const Ogre::ColourValue &colour);
	void addTriangleIndices(int index1, int index2, int index3);
 
	void addQuadIndices(int index1, int index2, int index3, int index4);
};

#endif // DEBUGDRAWER_H
