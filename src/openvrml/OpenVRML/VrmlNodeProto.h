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

#ifndef VRMLNODEPROTO_H
#define VRMLNODEPROTO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "VrmlNode.h"
#include "VrmlNodeType.h"
#include "Viewer.h"

class VrmlMFNode;

class VrmlNodeProto : public VrmlNode {

public:

  virtual VrmlNodeType & nodeType() const;

  VrmlNodeProto(VrmlNodeType *nodeDef, VrmlScene *scene);
  VrmlNodeProto(const VrmlNodeProto&);
  virtual ~VrmlNodeProto();

  virtual VrmlNode *cloneMe() const;

  virtual void addToScene( VrmlScene *, const char *relUrl );
  virtual ostream& printFields(ostream& os, int indent);

  virtual VrmlNodeProto* toProto() const;

  // These are passed along to the first implementation node of the proto.
  virtual VrmlNodeAnchor*	toAnchor() const;
  virtual VrmlNodeAppearance*	toAppearance() const;
  virtual VrmlNodeAudioClip*	toAudioClip() const;
  virtual VrmlNodeBackground*	toBackground() const;
  virtual VrmlNodeChild*	toChild() const;
  virtual VrmlNodeColor*	toColor() const;
  virtual VrmlNodeCoordinate*	toCoordinate() const;
  virtual VrmlNodeFog*		toFog() const;
  virtual VrmlNodeFontStyle*	toFontStyle() const;
  virtual VrmlNodeGeometry*	toGeometry() const;
  virtual VrmlNodeGroup*	toGroup() const;
  virtual VrmlNodeInline*	toInline() const;
  virtual VrmlNodeLight*	toLight() const;
  virtual VrmlNodeMaterial*	toMaterial() const;
  virtual VrmlNodeMovieTexture*	toMovieTexture() const;
  virtual VrmlNodeNavigationInfo*	toNavigationInfo() const;
  virtual VrmlNodeNormal*	toNormal() const;
  virtual VrmlNodePlaneSensor*	toPlaneSensor() const;
  virtual VrmlNodeSphereSensor*	toSphereSensor() const;
  virtual VrmlNodeCylinderSensor*	toCylinderSensor() const;
  virtual VrmlNodePointLight*	toPointLight() const;
  virtual VrmlNodeScript*	toScript() const;
  virtual VrmlNodeSound*	toSound() const;
  virtual VrmlNodeSpotLight*	toSpotLight() const;
  virtual VrmlNodeTexture*	toTexture() const;
  virtual VrmlNodeTextureCoordinate*	toTextureCoordinate() const;
  virtual VrmlNodeTextureTransform* toTextureTransform() const;
  virtual VrmlNodeTimeSensor*	toTimeSensor() const;
  virtual VrmlNodeTouchSensor*	toTouchSensor() const;
  virtual VrmlNodeViewpoint*	toViewpoint() const;

  // Larry
  virtual VrmlNodeBox*toBox() const; 
  virtual VrmlNodeCone* toCone() const; 
  virtual VrmlNodeCylinder* toCylinder() const; 
  virtual VrmlNodeDirLight*toDirLight() const; 
  virtual VrmlNodeElevationGrid* toElevationGrid() const; 
  virtual VrmlNodeExtrusion* toExtrusion() const; 
  virtual VrmlNodeIFaceSet*toIFaceSet() const;
  virtual VrmlNodeShape*toShape() const;
  virtual VrmlNodeSphere* toSphere() const; 
  virtual VrmlNodeSwitch* toSwitch() const;  
  virtual VrmlNodeTransform* toTransform() const;
  virtual VrmlNodeImageTexture* toImageTexture() const;
  virtual VrmlNodePixelTexture* toPixelTexture() const;
  virtual VrmlNodeLOD* toLOD() const;
  virtual VrmlNodeScalarInt* toScalarInt() const;
  virtual VrmlNodeOrientationInt* toOrientationInt() const;
  virtual VrmlNodePositionInt* toPositionInt() const;

  virtual void render(Viewer *, VrmlRenderContext rc);

  virtual void eventIn(double timeStamp,
		       const char *eventName,
		       const VrmlField *fieldValue);

  virtual const VrmlField *getField(const char *fieldName) const;
  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual void updateModified(VrmlNodePath& path, int flags);

  virtual void accumulateTransform( VrmlNode* );

  // LarryD  Feb 11/99
  int size();
  // LarryD  Feb 11/99
  VrmlNode *child(int index);

  // LarryD Feb 11/99
  VrmlMFNode *getNodes()  { return d_nodes;}

  // Field name/value pairs specified in PROTO instantiation
  typedef struct {
    char *name;
    VrmlField *value;
  } NameValueRec;

  const VrmlBVolume* getBVolume() const;

private:

  VrmlNode *firstNode() const;

  // Instantiate the proto by cloning the node type implementation nodes.
  void instantiate();

  // Find a field by name
  NameValueRec *findField(const char *fieldName) const;

  VrmlNodeType *d_nodeType;	// Definition

  bool d_instantiated;
  VrmlNamespace *d_scope;	// Node type and name bindings

  VrmlMFNode *d_nodes;		// Local copy of implementation nodes.

  std::list<NameValueRec*> d_fields;	// Field values

  // Dispatch eventIns from outside the PROTO to internal eventIns
  typedef struct {
    char *name;
    VrmlNodeType::ISMap ismap;
  } EventDispatch;

  typedef std::list<EventDispatch*> EventDispatchList;

  EventDispatchList d_eventDispatch;
  
  Viewer::Object d_viewerObject; // move to VrmlNode.h ? ...

};

#endif _VRMLNODEPROTO_
