//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  VrmlNodeGeometry.h

#ifndef VRMLNODEGEOMETRY_H
#define VRMLNODEGEOMETRY_H

#include "VrmlNode.h"
#include "Viewer.h"

class VrmlNodeColor;

class VrmlNodeGeometry : public VrmlNode {

public:

  // Define the fields of all built in geometry nodes
  static VrmlNodeType *defineType(VrmlNodeType *t);

  VrmlNodeGeometry(VrmlScene *);
  ~VrmlNodeGeometry();

  virtual VrmlNodeGeometry* toGeometry() const;

  // Specific geometry nodes need only define insertGeometry(), not render().
  virtual void render(Viewer *);

  virtual Viewer::Object insertGeometry(Viewer *) = 0;

  virtual VrmlNodeColor *color();

protected:

  Viewer::Object d_viewerObject; // move to VrmlNode? ...

};

#endif
