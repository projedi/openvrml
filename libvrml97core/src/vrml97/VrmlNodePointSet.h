//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//
//  VrmlNodePointSet.h

#ifndef VRMLNODEPOINTSET_H
#define VRMLNODEPOINTSET_H

#include "VrmlNodeGeometry.h"
#include "VrmlSFNode.h"


class VrmlNodePointSet : public VrmlNodeGeometry {

public:

  // Define the fields of pointSet nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType & nodeType() const;

  VrmlNodePointSet(VrmlScene *);
  virtual ~VrmlNodePointSet();

  virtual VrmlNode *cloneMe() const;
  virtual void cloneChildren(VrmlNamespace *);

  virtual bool isModified() const;

  virtual void clearFlags();

  virtual void addToScene( VrmlScene *s, const char *relUrl );

  virtual void copyRoutes(VrmlNamespace *ns) const;

  virtual ostream& printFields(ostream& os, int indent);

  virtual Viewer::Object insertGeometry(Viewer *);

  virtual const VrmlField *getField(const char *fieldName) const;
  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

protected:

  VrmlSFNode d_color;
  VrmlSFNode d_coord;

};

#endif
