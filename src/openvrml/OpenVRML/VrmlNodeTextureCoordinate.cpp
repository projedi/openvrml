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

#include "VrmlNodeTextureCoordinate.h"
#include "VrmlNodeType.h"
#include "VrmlNodeVisitor.h"

static VrmlNode *creator( VrmlScene *s ) {
  return new VrmlNodeTextureCoordinate(s);
}


// Define the built in VrmlNodeType:: "TextureCoordinate" fields

VrmlNodeType *VrmlNodeTextureCoordinate::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("TextureCoordinate", creator);
      t->reference();
    }

  VrmlNode::defineType(t);	// Parent class
  t->addExposedField("point", VrmlField::MFVEC2F);

  return t;
}

VrmlNodeType & VrmlNodeTextureCoordinate::nodeType() const 
{
    return *defineType(0);
}


VrmlNodeTextureCoordinate::VrmlNodeTextureCoordinate(VrmlScene *scene) :
  VrmlNode(scene)
{
}

VrmlNodeTextureCoordinate::~VrmlNodeTextureCoordinate()
{
}

bool VrmlNodeTextureCoordinate::accept(VrmlNodeVisitor & visitor) {
    if (!this->visited) {
        this->visited = true;
        visitor.visit(*this);
        return true;
    }
    
    return false;
}

VrmlNodeTextureCoordinate* VrmlNodeTextureCoordinate::toTextureCoordinate() const
{ return (VrmlNodeTextureCoordinate*) this; }


ostream& VrmlNodeTextureCoordinate::printFields(ostream& os, int indent)
{
  if (d_point.getLength() > 0) PRINT_FIELD(point);
  return os;
}

// Get the value of a field or eventOut.

const VrmlField *VrmlNodeTextureCoordinate::getField(const char *fieldName) const
{
  // exposedFields
  if ( strcmp( fieldName, "point" ) == 0 )
    return &d_point;
  return VrmlNode::getField(fieldName); // Parent class
}

// Set the value of one of the node fields.

void VrmlNodeTextureCoordinate::setField(const char *fieldName,
					 const VrmlField &fieldValue)
{
  if TRY_FIELD(point, MFVec2f)
  else
    VrmlNode::setField(fieldName, fieldValue);
}
