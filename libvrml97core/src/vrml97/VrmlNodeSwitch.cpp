//
//  Vrml 97 library
//  Copyright (C) 1998 Chris Morley
//  See the file COPYING for license details.
//
//  VrmlNodeSwitch.cpp

#include "VrmlNodeSwitch.h"
#include "VrmlNodeType.h"
#include "Viewer.h"
#include "VrmlBSphere.h"


// Return a new VrmlNodeSwitch
static VrmlNode *creator( VrmlScene *s ) { return new VrmlNodeSwitch(s); }


// Define the built in VrmlNodeType:: "Switch" fields

VrmlNodeType *VrmlNodeSwitch::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("Switch", creator);
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("choice", VrmlField::MFNODE);
  t->addExposedField("whichChoice", VrmlField::SFINT32);

  return t;
}

VrmlNodeType & VrmlNodeSwitch::nodeType() const
{
    return *defineType(0);
}


VrmlNodeSwitch::VrmlNodeSwitch(VrmlScene *scene) :
  VrmlNodeChild(scene),
  d_whichChoice(-1)
{
  this->setBVolumeDirty(true);
}

VrmlNodeSwitch::~VrmlNodeSwitch()
{
}


VrmlNode *VrmlNodeSwitch::cloneMe() const
{
  return new VrmlNodeSwitch(*this);
}

void VrmlNodeSwitch::cloneChildren(VrmlNamespace *ns)
{
  int n = d_choice.size();
  VrmlNode **kids = d_choice.get();
  for (int i = 0; i<n; ++i)
    {
      if (! kids[i]) continue;
      VrmlNode *newKid = kids[i]->clone(ns)->reference();
      kids[i]->dereference();
      kids[i] = newKid;
    }
}


bool VrmlNodeSwitch::isModified() const
{
  if (d_modified) return true;

  int w = d_whichChoice.get();

  return (w >= 0 && w < (int) d_choice.size() && d_choice[w]->isModified());
}


// ok: again we get this issue of whether to check _all_ the children
// or just the current choice (ref LOD). again, chooise to test them
// all. note that the original isModified() just tested the current
// one. keep that in mind, and change it back when confirmed safe.
//
void VrmlNodeSwitch::updateModified(VrmlNodePath& path)
{
  if (this->isModified()) markPathModified(path, true);
  path.push_front(this);
  int n = d_choice.size();
  for (int i=0; i<n; ++i)
    d_choice[i]->updateModified(path);
  path.pop_front();
}


void VrmlNodeSwitch::clearFlags()
{
  VrmlNode::clearFlags();
  
  int n = d_choice.size();
  for (int i = 0; i<n; ++i)
    d_choice[i]->clearFlags();
}

void VrmlNodeSwitch::addToScene( VrmlScene *s, const char *rel )
{
  d_scene = s;
  
  int n = d_choice.size();

  for (int i = 0; i<n; ++i)
    d_choice[i]->addToScene(s, rel);
}

void VrmlNodeSwitch::copyRoutes( VrmlNamespace *ns ) const
{
  VrmlNode::copyRoutes(ns);
  
  int n = d_choice.size();
  for (int i = 0; i<n; ++i)
    d_choice[i]->copyRoutes(ns);
}


ostream& VrmlNodeSwitch::printFields(ostream& os, int indent)
{
  if (d_choice.size() > 0) PRINT_FIELD(choice);
  if (d_whichChoice.get() != -1) PRINT_FIELD(whichChoice);
  return os;
}


// Render the selected child

void VrmlNodeSwitch::render(Viewer *viewer, VrmlRenderContext rc)
{
  int w = d_whichChoice.get();

  if (w >= 0 && w < (int) d_choice.size())
    d_choice[w]->render(viewer, rc);

  clearModified();
}


// Get the value of one of the node fields.
const VrmlField *VrmlNodeSwitch::getField(const char *fieldName) const
{
  if ( strcmp( fieldName, "choice" ) == 0 )
    return &d_choice;
  else if ( strcmp( fieldName, "whichChoice" ) == 0 )
    return &d_whichChoice;

  return VrmlNodeChild::getField( fieldName );
}

// Set the value of one of the node fields.
void VrmlNodeSwitch::setField(const char *fieldName,
			      const VrmlField &fieldValue)
{
  if TRY_FIELD(choice, MFNode)
  else if TRY_FIELD(whichChoice, SFInt)
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
  this->setBVolumeDirty(true);
}

VrmlNodeSwitch* VrmlNodeSwitch::toSwitch() const //LarryD
{ return (VrmlNodeSwitch*) this; }


const VrmlBVolume* VrmlNodeSwitch::getBVolume() const
{
  //cout << "VrmlNodeGroup[" << this << "]::getBVolume()" << endl;
  if (this->isBVolumeDirty())
    ((VrmlNodeSwitch*)this)->recalcBSphere();
  return &d_bsphere;
}

void
VrmlNodeSwitch::recalcBSphere()
{
  //cout << "VrmlNodeSwitch[" << this << "]::recalcBSphere()" << endl;
  d_bsphere.reset();
  int w = d_whichChoice.get();
  if (w >= 0 && w < (int) d_choice.size()) {
    const VrmlBVolume* ci_bv = d_choice[w]->getBVolume();
    if (ci_bv)
      d_bsphere.extend(*ci_bv);
  }
  this->setBVolumeDirty(false);
}
