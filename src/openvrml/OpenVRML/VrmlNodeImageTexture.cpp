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

#include "VrmlNodeImageTexture.h"
#include "Image.h"
#include "VrmlNodeType.h"
#include "VrmlNodeVisitor.h"
#include "VrmlScene.h"
#include "Doc.h"
#include "doc2.hpp"

static VrmlNode *creator( VrmlScene *s ) 
{ return new VrmlNodeImageTexture(s); }

const VrmlMFString& VrmlNodeImageTexture::getUrl() const 
{   return d_url; }

VrmlNodeImageTexture* VrmlNodeImageTexture::toImageTexture() const
{ return (VrmlNodeImageTexture*) this; }

// Define the built in VrmlNodeType:: "ImageTexture" fields

VrmlNodeType *VrmlNodeImageTexture::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;
      t = st = new VrmlNodeType("ImageTexture", creator);
      t->reference();
    }

  VrmlNodeTexture::defineType(t);	// Parent class

  t->addExposedField("url", VrmlField::MFSTRING);
  t->addField("repeatS", VrmlField::SFBOOL);
  t->addField("repeatT", VrmlField::SFBOOL);

  return t;
}

VrmlNodeImageTexture::VrmlNodeImageTexture(VrmlScene *scene) :
  VrmlNodeTexture(scene),
  d_repeatS(true),
  d_repeatT(true),
  d_image(0),
  d_texObject(0)
{
}

VrmlNodeType & VrmlNodeImageTexture::nodeType() const
{
    return *defineType(0);
}

VrmlNodeImageTexture::~VrmlNodeImageTexture()
{
  delete d_image;
  // delete d_texObject...
}

bool VrmlNodeImageTexture::accept(VrmlNodeVisitor & visitor) {
    if (!this->visited) {
        this->visited = true;
        visitor.visit(*this);
        return true;
    }
    
    return false;
}

ostream& VrmlNodeImageTexture::printFields(ostream& os, int indent)
{
  if (d_url.get()) PRINT_FIELD(url);
  if (! d_repeatS.get()) PRINT_FIELD(repeatS);
  if (! d_repeatT.get()) PRINT_FIELD(repeatT);
  return os;
}


void VrmlNodeImageTexture::render(Viewer *viewer, VrmlRenderContext rc)
{
  if ( isModified() )
    {
      if (d_image)
	{
	  delete d_image;		// URL is the only modifiable bit
	  d_image = 0;
	}
      if (d_texObject)
	{
	  viewer->removeTextureObject(d_texObject);
	  d_texObject = 0;
	}
    }

  // should probably read the image during addToScene...
  // should cache on url so multiple references to the same file are
  // loaded just once... of course world authors should just DEF/USE
  // them...
  if (! d_image && d_url.getLength() > 0)
    {
      const char *relUrl = d_relativeUrl.get() ? d_relativeUrl.get() :
	d_scene->urlDoc()->url();
      Doc relDoc(relUrl, static_cast<Doc const *>(0));
      d_image = new Image;
      if ( ! d_image->tryURLs( d_url.getLength(), d_url.get(), &relDoc ) )
	      theSystem->error("Couldn't read ImageTexture from URL %s\n", (char*)d_url.get(0));
    }

  // Check texture cache
  if (d_texObject && d_image)
    {
      viewer->insertTextureReference(d_texObject, d_image->nc());
    }
  else
    {
      unsigned char *pix;

      if (d_image && (pix = d_image->pixels()))
	{
	  // Ensure the image dimensions are powers of two
	  int sizes[] = { 2, 4, 8, 16, 32, 64, 128, 256 };
	  int nSizes = sizeof(sizes) / sizeof(int);
	  int w = d_image->w();
	  int h = d_image->h();
	  int i, j;
	  for (i=0; i<nSizes; ++i)
	    if (w < sizes[i]) break;
	  for (j=0; j<nSizes; ++j)
	    if (h < sizes[j]) break;

	  if (i > 0 && j > 0)
	    {
	      // Always scale images down in size and reuse the same pixel
	      // memory. This can cause some ugliness...
	      if (w != sizes[i-1] || h != sizes[j-1])
		{
		  viewer->scaleTexture( w, h, sizes[i-1], sizes[j-1],
					d_image->nc(), pix );
		  d_image->setSize( sizes[i-1], sizes[j-1] );
		}

	      d_texObject = viewer->insertTexture(d_image->w(),
						  d_image->h(),
						  d_image->nc(),
						  d_repeatS.get(),
						  d_repeatT.get(),
						  pix,
						  true);
	    }
	}
    }

  clearModified();
}


size_t VrmlNodeImageTexture::nComponents()
{
  return d_image ? d_image->nc() : 0;
}

size_t VrmlNodeImageTexture::width()
{
  return d_image ? d_image->w() : 0;
}

size_t VrmlNodeImageTexture::height()
{
  return d_image ? d_image->h() : 0;
}

size_t VrmlNodeImageTexture::nFrames()
{
  return 0;
}

const unsigned char * VrmlNodeImageTexture::pixels()
{
  return d_image ? d_image->pixels() : 0;
}


// Get the value of a field or eventOut.

const VrmlField *VrmlNodeImageTexture::getField(const char *fieldName) const
{
  // exposedFields
  if ( strcmp( fieldName, "url" ) == 0 )
    return &d_url;
  
  return VrmlNode::getField(fieldName); // Parent class
}

// Set the value of one of the node fields.

void VrmlNodeImageTexture::setField(const char *fieldName,
				    const VrmlField &fieldValue)
{
  if (strcmp(fieldName,"url") == 0)
    {
      delete d_image;
      d_image = 0;
    }

  if TRY_FIELD(url, MFString)
  else if TRY_FIELD(repeatS, SFBool)
  else if TRY_FIELD(repeatT, SFBool)
  else
    VrmlNode::setField(fieldName, fieldValue);
}

bool VrmlNodeImageTexture::getRepeatS() const
{
    return d_repeatS.get();
}

bool VrmlNodeImageTexture::getRepeatT() const
{
    return d_repeatT.get();
}
