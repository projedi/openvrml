//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//  See the file COPYING for license details.
//
//  VrmlNodePlaneSensor.h

#ifndef VRMLNODEPLANESENSOR_H
#define VRMLNODEPLANESENSOR_H

#include "VrmlNodeChild.h"
#include "VrmlSFBool.h"
#include "VrmlSFFloat.h"
#include "VrmlSFVec2f.h"
#include "VrmlSFVec3f.h"

class VrmlScene;


class VrmlNodePlaneSensor : public VrmlNodeChild {

public:

  // Define the fields of PlaneSensor nodes
  static VrmlNodeType *defineType(VrmlNodeType *t = 0);
  virtual VrmlNodeType & nodeType() const;

  VrmlNodePlaneSensor( VrmlScene *scene = 0);
  virtual ~VrmlNodePlaneSensor();

  virtual VrmlNode *cloneMe() const;

  virtual VrmlNodePlaneSensor* toPlaneSensor() const;

  virtual ostream& printFields(ostream& os, int indent);

  void activate( double timeStamp, bool isActive, double *p );

  virtual const VrmlField *getField(const char *fieldName) const;
  virtual void setField(const char *fieldName, const VrmlField &fieldValue);

  virtual void accumulateTransform( VrmlNode* );
  virtual VrmlNode* getParentTransform();

  bool isEnabled() { return d_enabled.get(); }

private:

  // Fields
  VrmlSFBool d_autoOffset;
  VrmlSFBool d_enabled;
  VrmlSFVec2f d_maxPosition;
  VrmlSFVec2f d_minPosition;
  VrmlSFVec3f d_offset;

  VrmlSFBool d_isActive;
  VrmlSFVec3f d_translation;
  VrmlSFVec3f d_trackPoint;

  VrmlSFVec3f d_activationPoint;

  VrmlNode *d_parentTransform;

};

#endif
