//
// OpenVRML
//
// Copyright (C) 1998  Chris Morley
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 

#ifndef VRMLNODE_H
#define VRMLNODE_H

#include <string.h>
#include <iostream.h>
#include <list>
#include <string>
#include <iostream.h>
#include "common.h"
#include "VrmlNodePtr.h"
#include "System.h"		// error

class Route;
class Viewer;

class VrmlNamespace;
class VrmlNodeType;
class VrmlField;
class VrmlScene;
class VrmlNodeVisitor;

#include "VrmlRenderContext.h"
class VrmlBVolume;

// For the safe downcasts
class VrmlNodeAnchor;
class VrmlNodeAppearance;
class VrmlNodeAudioClip;
class VrmlNodeBackground;
class VrmlNodeBillboard;
class VrmlNodeBox;   //LarryD Mar 08/99
class VrmlNodeChild;
class VrmlNodeCollision;
class VrmlNodeColor;
class VrmlNodeCone;   //LarryD Mar 08/99
class VrmlNodeCoordinate;
class VrmlNodeCylinder; //LarryD Mar 08/99
class VrmlNodeDirLight; //LarryD Mar 08/99
class VrmlNodeElevationGrid; //LarryD Mar 09/99
class VrmlNodeExtrusion; //LarryD Mar 09/99
class VrmlNodeFog;
class VrmlNodeFontStyle;
class VrmlNodeGeometry;
class VrmlNodeGroup;
class VrmlNodeIFaceSet;
class VrmlNodeInline;
class VrmlNodeLight;
class VrmlNodeMaterial;
class VrmlNodeMovieTexture;
class VrmlNodeNavigationInfo;
class VrmlNodeNormal;
class VrmlNodePlaneSensor;
class VrmlNodePointLight;
class VrmlNodeScript;
class VrmlNodeShape;
class VrmlNodeSphere; //LarryD Mar 08/99
class VrmlNodeSphereSensor;
class VrmlNodeCylinderSensor;
class VrmlNodeSound;
class VrmlNodeSpotLight;
class VrmlNodeSwitch;
class VrmlNodeTexture;
class VrmlNodeTextureCoordinate;
class VrmlNodeTextureTransform;
class VrmlNodeTimeSensor;
class VrmlNodeTouchSensor;
class VrmlNodeTransform;
class VrmlNodeViewpoint;
class VrmlNodeImageTexture;
class VrmlNodePixelTexture;
class VrmlNodeLOD;
class VrmlNodeScalarInt;
class VrmlNodeOrientationInt;
class VrmlNodePositionInt;

class VrmlNodeProto;

// Global output operator for nodes
//class VrmlNode;
//ostream& operator<<(ostream& os, const VrmlNode& f);

class OPENVRML_SCOPE VrmlNode {

    friend ostream & operator<<(ostream & os, const VrmlNode & f);

    std::string name;

public:
    static VrmlNodeType * defineType(VrmlNodeType * type);

    VrmlNode(VrmlScene * scene);
    VrmlNode(const VrmlNode &);
    virtual ~VrmlNode() = 0;

    const std::string & getName() const;
    void setName(const std::string & nodeName, VrmlNamespace * ns = 0);

    virtual VrmlNodeType & nodeType() const = 0;
    virtual bool accept(VrmlNodeVisitor & visitor) = 0;
    virtual void resetVisitedFlag();
    virtual const VrmlField * getField(const std::string & fieldId) const;
    virtual void setField(const std::string & fieldId,
                          const VrmlField & fieldValue);
    virtual void eventIn(double timeStamp,
		         const std::string & eventName,
		         const VrmlField & fieldValue);

    // Safe node downcasts. These avoid the dangerous casts of VrmlNode* (esp in
    // presence of protos), but are ugly in that this class must know about all
    // the subclasses. These return 0 if the typecast is invalid.
    // Remember to also add new ones to VrmlNodeProto. Protos should
    // return their first implementation node (except toProto()).
    virtual VrmlNodeAnchor*	toAnchor() const;
    virtual VrmlNodeAppearance*	toAppearance() const;
    virtual VrmlNodeAudioClip*	toAudioClip() const;
    virtual VrmlNodeBackground*	toBackground() const;
    virtual const VrmlNodeBillboard * toBillboard() const;
    virtual VrmlNodeBillboard * toBillboard();
    virtual VrmlNodeBox*		toBox() const; //LarryD Mar 08/99
    virtual const VrmlNodeChild * toChild() const;
    virtual VrmlNodeChild * toChild();
    virtual const VrmlNodeCollision * toCollision() const;
    virtual VrmlNodeCollision * toCollision();
    virtual VrmlNodeColor*	toColor() const;
    virtual VrmlNodeCone* toCone() const; //LarryD Mar 08/99
    virtual VrmlNodeCoordinate*	toCoordinate() const;
    virtual VrmlNodeCylinder* toCylinder() const; //LarryD Mar 08/99
    virtual VrmlNodeDirLight* toDirLight() const; //LarryD Mar 08/99
    virtual VrmlNodeElevationGrid* toElevationGrid() const; //LarryD Mar 09/99
    virtual VrmlNodeExtrusion*    toExtrusion() const; //LarryD Mar 09/99
    virtual VrmlNodeFog*		toFog() const;
    virtual VrmlNodeFontStyle*	toFontStyle() const;
    virtual const VrmlNodeGeometry * toGeometry() const;
    virtual VrmlNodeGeometry * toGeometry();
    virtual VrmlNodeGroup*	toGroup() const;
    virtual VrmlNodeIFaceSet*	toIFaceSet() const;
    virtual VrmlNodeImageTexture* toImageTexture() const;
    virtual VrmlNodePixelTexture* toPixelTexture() const;
    virtual VrmlNodeInline*	toInline() const;
    virtual VrmlNodeLight*	toLight() const;
    virtual VrmlNodeMaterial*	toMaterial() const;
    virtual VrmlNodeMovieTexture*	toMovieTexture() const;
    virtual VrmlNodeNavigationInfo*	toNavigationInfo() const;
    virtual VrmlNodeNormal*	toNormal() const;
    virtual VrmlNodePlaneSensor*	toPlaneSensor() const;
    virtual VrmlNodeSphereSensor*	toSphereSensor() const;
    virtual VrmlNodeCylinderSensor* toCylinderSensor() const;
    virtual VrmlNodePointLight*	toPointLight() const;
    virtual VrmlNodeScript*	toScript() const;
    virtual VrmlNodeShape*	toShape() const;
    virtual VrmlNodeSphere* toSphere() const; //LarryD Mar 08/99
    virtual VrmlNodeSound*	toSound() const;
    virtual VrmlNodeSpotLight*	toSpotLight() const;
    virtual VrmlNodeSwitch* toSwitch() const; //LarryD Mar 08/99
    virtual VrmlNodeTexture*	toTexture() const;
    virtual VrmlNodeTextureCoordinate*	toTextureCoordinate() const;
    virtual VrmlNodeTextureTransform* toTextureTransform() const;
    virtual VrmlNodeTimeSensor*	toTimeSensor() const;
    virtual VrmlNodeTouchSensor*	toTouchSensor() const;
    virtual VrmlNodeTransform* toTransform() const;     //LarryD Feb 24/99
    virtual VrmlNodeViewpoint*	toViewpoint() const;

    virtual VrmlNodeLOD* toLOD() const;
    virtual VrmlNodeScalarInt* toScalarInt() const;
    virtual VrmlNodeOrientationInt* toOrientationInt() const;
    virtual VrmlNodePositionInt* toPositionInt() const;

    virtual VrmlNodeProto*	toProto() const;

    virtual void addToScene(VrmlScene *, const std::string & relativeUrl);

    // Write self
    ostream& print(ostream& os, int indent) const;
    virtual ostream& printFields(ostream& os, int indent);
    static ostream& printField(ostream&, int, const char*, const VrmlField&);

    // Indicate that the node state has changed, need to re-render
    void setModified();
    void clearModified() { d_modified = false; }
    virtual bool isModified() const;
    typedef std::list< VrmlNode* > VrmlNodePath; // duplicate from VrmlScene. argh.


    static void markPathModified(VrmlNodePath& path, bool mod, int flags = 0x003);

    // do the work of updatemodified. move this to be protected
    //
    virtual void updateModified(VrmlNodePath& path, int flags = 0x003);

    void updateModified(int flags = 0x003);

    virtual const VrmlBVolume* getBVolume() const;

    virtual void setBVolume(const VrmlBVolume& v);

    virtual void setBVolumeDirty(bool f);

    virtual bool isBVolumeDirty() const;


    // A generic flag (typically used to find USEd nodes).
    void setFlag() { d_flag = true; }
    virtual void clearFlags();	// Clear childrens flags too.
    bool isFlagSet() { return d_flag; }

    // Add a ROUTE from a field in this node
    void addRoute(const std::string & fromEventOut, const VrmlNodePtr & toNode, const std::string & toEventIn);

    // Delete a ROUTE from a field in this node
    void deleteRoute(const std::string & fromEventOut, const VrmlNodePtr & toNode, const std::string & toEventIn);

    Route * getRoutes();


    const VrmlField * getEventOut(const std::string & fieldName) const;


    virtual void render(Viewer *, VrmlRenderContext rc);


    virtual void accumulateTransform(VrmlNode*);

    virtual VrmlNode* getParentTransform();

    virtual void inverseTransform(Viewer *);

    virtual void inverseTransform(double [4][4]);

    VrmlScene *scene() { return d_scene; }

protected:
    enum { INDENT_INCREMENT = 4 };

    // Send a named event from this node.
    void eventOut(double timeStamp, const std::string & eventName,
		  const VrmlField & fieldValue);

    // Scene this node belongs to
    VrmlScene *d_scene;

    // True if a field changed since last render
    bool d_modified;
    bool d_flag;
    bool d_bvol_dirty;
    bool visited;

    // Routes from this node (clean this up, add RouteList ...)
    Route *d_routes;
};


// Ugly macro used in printFields
#define PRINT_FIELD(_f) printField(os,indent+INDENT_INCREMENT,#_f,d_##_f)

// Ugly macros used in setField

#define TRY_FIELD(_f,_t) \
(fieldId == #_f) {\
    if ( fieldValue.to##_t() )\
      d_##_f = (Vrml##_t &)fieldValue;\
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s).\n",\
	    fieldValue.fieldTypeName(), #_f, nodeType().getName().c_str(), #_t);\
  }

// For SFNode fields. Allow un-fetched EXTERNPROTOs to succeed...
#define TRY_SFNODE_FIELD(_f,_n) \
(fieldId == #_f) { \
    VrmlSFNode *x=(VrmlSFNode*)&fieldValue; \
    if (fieldValue.toSFNode() && \
	( (!x->get()) || x->get()->to##_n() || x->get()->toProto() )) \
      d_##_f = (VrmlSFNode &)fieldValue; \
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s).\n",\
	    fieldValue.fieldTypeName(), #_f, nodeType().getName().c_str(), #_n);\
  }

#define TRY_SFNODE_FIELD2(_f,_n1,_n2) \
(fieldId == #_f) { \
    VrmlSFNode *x=(VrmlSFNode*)&fieldValue; \
    if (fieldValue.toSFNode() && \
	((!x->get()) || x->get()->to##_n1() || x->get()->to##_n2() || \
	 x->get()->toProto() )) \
      d_##_f = (VrmlSFNode &)fieldValue; \
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s or %s).\n",\
	    fieldValue.fieldTypeName(), #_f, nodeType().getName().c_str(), #_n1, #_n2);\
  }


#endif

