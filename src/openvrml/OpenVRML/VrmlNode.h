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
#include <vector>
#include <iostream.h>
#include "common.h"
#include "field.h"
#include "VrmlNodePtr.h"
#include "System.h"		// error
#include "VrmlRenderContext.h"

namespace OpenVRML {
    class Node;
}

ostream & operator<<(ostream & os, const OpenVRML::Node & f);

namespace OpenVRML {

    class Route;
    class Viewer;
    class VrmlNamespace;
    class NodeType;
    class FieldValue;
    class VrmlScene;
    class NodeVisitor;
    class BVolume;
    class ScriptNode;
    class ProtoNode;
    class NodeAnchor;
    class NodeAppearance;
    class NodeAudioClip;
    class NodeBackground;
    class NodeBillboard;
    class NodeBox;
    class NodeChild;
    class NodeCollision;
    class NodeColor;
    class NodeCone;
    class NodeCoordinate;
    class NodeCylinder;
    class NodeDirLight;
    class NodeElevationGrid;
    class NodeExtrusion;
    class NodeFog;
    class NodeFontStyle;
    class NodeGeometry;
    class NodeGroup;
    class NodeIFaceSet;
    class NodeInline;
    class NodeLight;
    class NodeMaterial;
    class NodeMovieTexture;
    class NodeNavigationInfo;
    class NodeNormal;
    class NodePlaneSensor;
    class NodePointLight;
    class NodePointSet;
    class NodeShape;
    class NodeSphere;
    class NodeSphereSensor;
    class NodeCylinderSensor;
    class NodeSound;
    class NodeSpotLight;
    class NodeSwitch;
    class NodeTexture;
    class NodeTextureCoordinate;
    class NodeTextureTransform;
    class NodeTimeSensor;
    class NodeTouchSensor;
    class NodeTransform;
    class NodeViewpoint;
    class NodeImageTexture;
    class NodePixelTexture;
    class NodeLOD;
    class NodeScalarInt;
    class NodeOrientationInt;
    class NodePositionInt;
    class VrmlMatrix;

    class OPENVRML_SCOPE Node {
        friend ostream & ::operator<<(ostream & os, const Node & f);

        std::string id;

    public:
        const NodeType & type;
        typedef std::list<Route *> RouteList;

        virtual ~Node() = 0;

        const std::string & getId() const;
        void setId(const std::string & nodeId, VrmlNamespace * ns = 0);

        virtual bool accept(NodeVisitor & visitor) = 0;
        virtual void resetVisitedFlag();
        virtual const FieldValue * getField(const std::string & fieldId) const;
        virtual void setField(const std::string & fieldId,
                              const FieldValue & fieldValue);
        virtual void eventIn(double timeStamp,
		             const std::string & eventName,
		             const FieldValue & fieldValue);
        virtual const MFNode getChildren() const;
        virtual void addToScene(VrmlScene *, const std::string & relativeUrl);

        virtual ScriptNode * toScript() const;
        virtual NodeAnchor * toAnchor() const;
        virtual NodeAppearance * toAppearance() const;
        virtual NodeAudioClip * toAudioClip() const;
        virtual NodeBackground * toBackground() const;
        virtual NodeBillboard * toBillboard() const;
        virtual NodeBox * toBox() const;
        virtual NodeChild * toChild() const;
        virtual NodeCollision * toCollision() const;
        virtual NodeColor * toColor() const;
        virtual NodeCone * toCone() const;
        virtual NodeCoordinate * toCoordinate() const;
        virtual NodeCylinder * toCylinder() const;
        virtual NodeCylinderSensor * toCylinderSensor() const;
        virtual NodeDirLight * toDirLight() const;
        virtual NodeElevationGrid * toElevationGrid() const;
        virtual NodeExtrusion * toExtrusion() const;
        virtual NodeFog * toFog() const;
        virtual NodeFontStyle * toFontStyle() const;
        virtual NodeGeometry * toGeometry() const;
        virtual NodeGroup * toGroup() const;
        virtual NodeIFaceSet * toIFaceSet() const;
        virtual NodeImageTexture * toImageTexture() const;
        virtual NodeInline * toInline() const;
        virtual NodeLOD * toLOD() const;
        virtual NodeLight * toLight() const;
        virtual NodeMaterial * toMaterial() const;
        virtual NodeMovieTexture * toMovieTexture() const;
        virtual NodeNavigationInfo * toNavigationInfo() const;
        virtual NodeNormal * toNormal() const;
        virtual NodeOrientationInt * toOrientationInt() const;
        virtual NodePixelTexture * toPixelTexture() const;
        virtual NodePlaneSensor * toPlaneSensor() const;
        virtual NodePointLight * toPointLight() const;
        virtual NodePointSet * toPointSet() const;
        virtual NodePositionInt * toPositionInt() const;
        virtual NodeScalarInt * toScalarInt() const;
        virtual NodeShape * toShape() const;
        virtual NodeSound * toSound() const;
        virtual NodeSphere * toSphere() const;
        virtual NodeSphereSensor * toSphereSensor() const;
        virtual NodeSpotLight * toSpotLight() const;
        virtual NodeSwitch * toSwitch() const;
        virtual NodeTexture * toTexture() const;
        virtual NodeTextureCoordinate * toTextureCoordinate() const;
        virtual NodeTextureTransform * toTextureTransform() const;
        virtual NodeTimeSensor * toTimeSensor() const;
        virtual NodeTouchSensor * toTouchSensor() const;
        virtual NodeTransform * toTransform() const;
        virtual NodeViewpoint * toViewpoint() const;

        // Write self
        ostream& print(ostream& os, int indent) const;
        virtual ostream& printFields(ostream& os, int indent);
        static ostream& printField(ostream&, int, const char*, const FieldValue &);

        // Indicate that the node state has changed, need to re-render
        void setModified();
        void clearModified() { d_modified = false; }
        virtual bool isModified() const;
        typedef std::list< Node* > NodePath; // duplicate from VrmlScene. argh.


        static void markPathModified(NodePath& path, bool mod, int flags = 0x003);

        // do the work of updatemodified. move this to be protected
        //
        virtual void updateModified(NodePath& path, int flags = 0x003);

        void updateModified(int flags = 0x003);

        virtual const BVolume * getBVolume() const;

        virtual void setBVolume(const BVolume & v);

        virtual void setBVolumeDirty(bool f);

        virtual bool isBVolumeDirty() const;


        // A generic flag (typically used to find USEd nodes).
        void setFlag() { d_flag = true; }
        virtual void clearFlags();	// Clear childrens flags too.
        bool isFlagSet() { return d_flag; }

        // Add a ROUTE from a field in this node
        void addRoute(const std::string & fromEventOut, const NodePtr & toNode, const std::string & toEventIn);

        // Delete a ROUTE from a field in this node
        void deleteRoute(const std::string & fromEventOut, const NodePtr & toNode, const std::string & toEventIn);

        RouteList getRoutes();

        const FieldValue * getEventOut(const std::string & fieldName) const;


        virtual void render(Viewer *, VrmlRenderContext rc);


        virtual void accumulateTransform(Node*);

        virtual Node* getParentTransform();

        virtual void inverseTransform(Viewer *);

        virtual void inverseTransform(VrmlMatrix &);

        VrmlScene *scene() { return d_scene; }

    protected:
        Node(const NodeType & nodeType, VrmlScene * scene);
        Node(const Node &);

        enum { INDENT_INCREMENT = 4 };

        // Send a named event from this node.
        void eventOut(double timeStamp, const std::string & eventName,
		      const FieldValue & fieldValue);

        // Scene this node belongs to
        VrmlScene *d_scene;

        // True if a field changed since last render
        bool d_modified;
        bool d_flag;
        bool d_bvol_dirty;
        bool visited;

        // Routes from this node (clean this up, add RouteList ...)
        RouteList d_routes;
    };
}

// Ugly macro used in printFields
#define PRINT_FIELD(_f) printField(os,indent+INDENT_INCREMENT,#_f,d_##_f)

// Ugly macros used in setField

#define TRY_FIELD(_f,_t) \
(fieldId == #_f) {\
    if ( fieldValue.to##_t() )\
      d_##_f = (_t &)fieldValue;\
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s).\n",\
	    fieldValue.fieldTypeName(), #_f, this->type.getId().c_str(), #_t);\
  }

// For SFNode fields. Allow un-fetched EXTERNPROTOs to succeed...
#define TRY_SFNODE_FIELD(_f,_n) \
(fieldId == #_f) { \
    SFNode *x=(SFNode*)&fieldValue; \
    if (fieldValue.toSFNode() \
            && (!x->get() || x->get()->to##_n() \
                || dynamic_cast<ProtoNode *>(x->get().get()))) \
        d_##_f = (SFNode &)fieldValue; \
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s).\n",\
	    fieldValue.fieldTypeName(), #_f, this->type.getId().c_str(), #_n);\
  }

#define TRY_SFNODE_FIELD2(_f,_n1,_n2) \
(fieldId == #_f) { \
    SFNode *x=(SFNode*)&fieldValue; \
    if (fieldValue.toSFNode() && \
	((!x->get()) || x->get()->to##_n1() || x->get()->to##_n2() || \
	 dynamic_cast<ProtoNode *>(x->get().get()))) \
      d_##_f = (SFNode &)fieldValue; \
    else \
      theSystem->error("Invalid type (%s) for %s field of %s node (expected %s or %s).\n",\
	    fieldValue.fieldTypeName(), #_f, this->type.getId().c_str(), #_n1, #_n2);\
  }


#endif

