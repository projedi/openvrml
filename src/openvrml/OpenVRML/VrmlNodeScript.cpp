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

#include <assert.h>

#include "VrmlNodeScript.h"
#include "VrmlNodeType.h"
#include "VrmlNodeVisitor.h"
#include "ScriptObject.h"
#include "VrmlScene.h"

#ifdef macintosh
extern char* strdup( const char* );
#endif

#if defined(WIN32)
// Disable warning messages about forcing value to bool 'true' or 'false'
#pragma warning (disable:4800)
#endif

// Script factory. Add each Script to the scene for fast access.

static VrmlNode *creator( VrmlScene *scene ) 
{
  return new VrmlNodeScript(scene);
}


// Define the built in VrmlNodeType:: "Script" fields

VrmlNodeType *VrmlNodeScript::defineType(VrmlNodeType *t)
{
  static VrmlNodeType *st = 0;

  if (! t)
    {
      if (st) return st;		// Only define the type once.
      t = st = new VrmlNodeType("Script", creator);
      t->reference();
    }

  VrmlNodeChild::defineType(t);	// Parent class
  t->addExposedField("url", VrmlField::MFSTRING);
  t->addField("directOutput", VrmlField::SFBOOL);
  t->addField("mustEvaluate", VrmlField::SFBOOL);

  return t;
}

// Should subclass NodeType and have each Script maintain its own type...

VrmlNodeType & VrmlNodeScript::nodeType() const
{
    return *defineType(0);
}


VrmlNodeScript::VrmlNodeScript( VrmlScene *scene ) :
  VrmlNodeChild(scene),
  d_directOutput(false),
  d_mustEvaluate(false),
  d_script(0),
  d_eventsReceived(0)
{
  if (d_scene) d_scene->addScript(this);
}

VrmlNodeScript::VrmlNodeScript(const VrmlNodeScript & node):
        VrmlNodeChild(node), d_directOutput(node.d_directOutput),
        d_mustEvaluate(node.d_mustEvaluate), d_url(node.d_url), d_script(0),
        d_eventIns(0), d_eventOuts(0), d_fields(0), d_eventsReceived(0) {
  // add eventIn/eventOut/fields from source Script
  FieldList::const_iterator i;

  for (i = node.d_eventIns.begin(); i != node.d_eventIns.end(); ++i)
    addEventIn( (*i)->name, (*i)->type );
  for (i = node.d_eventOuts.begin(); i != node.d_eventOuts.end(); ++i)
    addEventOut( (*i)->name, (*i)->type );
  for (i = node.d_fields.begin(); i != node.d_fields.end(); ++i)
    addField(  (*i)->name, (*i)->type, (*i)->value );
}


VrmlNodeScript::~VrmlNodeScript()
{
  shutdown( theSystem->time() );

  // removeScript ought to call shutdown...
  if (d_scene) d_scene->removeScript(this);

  delete d_script;

  // delete eventIn/eventOut/field ScriptField list contents
  FieldList::iterator i;
  for (i = d_eventIns.begin(); i != d_eventIns.end(); ++i)
    {
      ScriptField *r = *i;
      free(r->name);
      delete r->value;
      delete r;
    }
  for (i = d_eventOuts.begin(); i != d_eventOuts.end(); ++i)
    {
      ScriptField *r = *i;
      free(r->name);
      delete r->value;
      delete r;
    }
  for (i = d_fields.begin(); i != d_fields.end(); ++i)
    {
      ScriptField *r = *i;
      free(r->name);
      delete r->value;
      delete r;
    }
}

bool VrmlNodeScript::accept(VrmlNodeVisitor & visitor) {
    if (!this->visited) {
        this->visited = true;
        visitor.visit(*this);
        return true;
    }
    
    return false;
}

void VrmlNodeScript::resetVisitedFlag() {
    if (this->visited) {
        this->visited = false;
        for (FieldList::const_iterator itr = this->d_fields.begin();
                itr != this->d_fields.end(); ++itr) {
            assert((*itr)->value);
            if ((*itr)->type == VrmlField::SFNODE) {
                assert(dynamic_cast<VrmlSFNode *>((*itr)->value));
                static_cast<VrmlSFNode *>((*itr)->value)
                        ->get()->resetVisitedFlag();
            } else if ((*itr)->type == VrmlField::MFNODE) {
                assert(dynamic_cast<VrmlMFNode *>((*itr)->value));
                VrmlMFNode & mfnode = static_cast<VrmlMFNode &>(*(*itr)->value);
                for (size_t i = 0; i < mfnode.getLength(); ++i) {
                    mfnode.getElement(i)->resetVisitedFlag();
                }
            }
        }
    }
}

VrmlNodeScript* VrmlNodeScript::toScript() const
{ return (VrmlNodeScript*) this; }

void VrmlNodeScript::addToScene(VrmlScene * const scene,
                                const char * const relUrl) {
    theSystem->debug("VrmlNodeScript::%s 0x%x addToScene 0x%x\n",
		     name(), (unsigned)this, (unsigned)scene);

    this->d_relativeUrl.set(relUrl);
    if (this->d_scene == scene) {
        return;
    }
    if ((this->d_scene = scene)) {
        this->initialize(theSystem->time());
        this->d_scene->addScript(this);
    }
}


ostream& VrmlNodeScript::printFields(ostream& os, int indent)
{
  if (d_url.getLength() > 0) PRINT_FIELD(url);
  if (d_directOutput.get()) PRINT_FIELD(directOutput);
  if (d_mustEvaluate.get()) PRINT_FIELD(mustEvaluate);

  return os;
}


void VrmlNodeScript::initialize( double ts )
{
  theSystem->debug("Script.%s::initialize\n", name());
  assert(!this->d_script);

//  if (d_script) return;	       // How did I get here? Letting the days go by...
  d_eventsReceived = 0;
  if (d_url.getLength() > 0)
    {
      d_script = ScriptObject::create(*this, d_url);
      if (d_script)
	d_script->activate( ts, "initialize", 0, 0 );
    }
}

void VrmlNodeScript::shutdown( double ts )
{
  if (d_script)
    d_script->activate( ts, "shutdown", 0, 0 );
}

void VrmlNodeScript::update( VrmlSFTime &timeNow )
{
  if (d_eventsReceived > 0)
    {
      //theSystem->debug("Script.%s::update\n", name());
      if (d_script)
	d_script->activate( timeNow.get(), "eventsProcessed", 0, 0 );
      d_eventsReceived = 0;
    }
}


// 

void VrmlNodeScript::eventIn(double timeStamp,
			     const char *eventName,
			     const VrmlField & fieldValue)
{
  if (! d_script ) initialize( timeStamp );
  if (! d_script ) return;

  const char *origEventName = eventName;
  bool valid = hasEventIn( eventName );
  if (! valid && strncmp(eventName, "set_", 4) == 0 )
    {
      eventName += 4;
      valid = hasEventIn( eventName );
    }
#if 0
  cerr << "eventIn Script::" << name() << "." << origEventName
       << " " << fieldValue << ", valid " << valid
       << ", d_script " << (unsigned long)d_script
       << endl;
#endif
  if ( valid )
    {
      setEventIn( eventName, fieldValue );

      VrmlSFTime ts( timeStamp );
      const VrmlField *args[] = { &fieldValue, &ts };

      FieldList::const_iterator i;
      for (i = d_eventOuts.begin(); i != d_eventOuts.end(); ++i)
	(*i)->modified = false;

      d_script->activate( timeStamp, eventName, 2, args );

      // For each modified eventOut, send an event
      for (i = d_eventOuts.begin(); i != d_eventOuts.end(); ++i)
	if ((*i)->modified)
	  eventOut( timeStamp, (*i)->name, *((*i)->value) );

      ++d_eventsReceived;	// call eventsProcessed later
    }

  // Let the generic code handle the rest.
  else
    VrmlNode::eventIn( timeStamp, origEventName, fieldValue );

  // Scripts shouldn't generate redraws.
  clearModified();
}



// add events/fields

namespace {
    void add(VrmlNodeScript::FieldList & recs, const char * ename,
             VrmlField::VrmlFieldType type) {
        VrmlNodeScript::ScriptField * const scriptField =
                new VrmlNodeScript::ScriptField;
        scriptField->name = strdup(ename);
        scriptField->type = type;
        switch (type) {
        case VrmlField::SFBOOL:
            scriptField->value = new VrmlSFBool();
            break;
        case VrmlField::SFCOLOR:
            scriptField->value = new VrmlSFColor();
            break;
        case VrmlField::SFFLOAT:
            scriptField->value = new VrmlSFFloat();
            break;
        case VrmlField::SFIMAGE:
            scriptField->value = new VrmlSFImage();
            break;
        case VrmlField::SFINT32:
            scriptField->value = new VrmlSFInt32();
            break;
        case VrmlField::SFNODE:
            scriptField->value = new VrmlSFNode();
            break;
        case VrmlField::SFROTATION:
            scriptField->value = new VrmlSFRotation();
            break;
        case VrmlField::SFSTRING:
            scriptField->value = new VrmlSFString();
            break;
        case VrmlField::SFTIME:
            scriptField->value = new VrmlSFTime();
            break;
        case VrmlField::SFVEC2F:
            scriptField->value = new VrmlSFVec2f();
            break;
        case VrmlField::SFVEC3F:
            scriptField->value = new VrmlSFVec3f();
            break;
        case VrmlField::MFCOLOR:
            scriptField->value = new VrmlMFColor();
            break;
        case VrmlField::MFFLOAT:
            scriptField->value = new VrmlMFFloat();
            break;
        case VrmlField::MFINT32:
            scriptField->value = new VrmlMFInt32();
            break;
        case VrmlField::MFNODE:
            scriptField->value = new VrmlMFNode();
            break;
        case VrmlField::MFROTATION:
            scriptField->value = new VrmlMFRotation();
            break;
        case VrmlField::MFSTRING:
            scriptField->value = new VrmlMFString();
            break;
        case VrmlField::MFTIME:
            scriptField->value = new VrmlMFTime();
            break;
        case VrmlField::MFVEC2F:
            scriptField->value = new VrmlMFVec2f();
            break;
        case VrmlField::MFVEC3F:
            scriptField->value = new VrmlMFVec3f();
            break;
        default:
            assert(false);
            break;
        }
        
        recs.push_front(scriptField);
    }
}

void VrmlNodeScript::addEventIn(const char *ename, VrmlField::VrmlFieldType t)
{
  add(d_eventIns, ename, t);
}

void VrmlNodeScript::addEventOut(const char *ename, VrmlField::VrmlFieldType t)
{
  add(d_eventOuts, ename, t);
}

void VrmlNodeScript::addField(const char *ename, VrmlField::VrmlFieldType t,
			      const VrmlField * val) {
    add(this->d_fields, ename, t);
    if (val) {
        this->set(this->d_fields, ename, *val);
    }
}

// get event/field values
#if 0
VrmlField*
VrmlNodeScript::getEventIn(const char *fname) const
{
  return get(d_eventIns, fname);
}

VrmlField*
VrmlNodeScript::getEventOut(const char *fname) const
{
  return get(d_eventOuts, fname);
}

VrmlField*
VrmlNodeScript::getField(const char *fname) const
{
  return get(d_fields, fname);
}
#endif

VrmlField*
VrmlNodeScript::get(const FieldList &recs, const char *fname) const
{
  FieldList::const_iterator i;
  for (i = recs.begin(); i != recs.end(); ++i) {
    if (strcmp((*i)->name, fname) == 0)
      return (*i)->value;
  }
  return 0;
}

// has

VrmlField::VrmlFieldType
VrmlNodeScript::hasEventIn(const char *ename) const
{
  return has(d_eventIns, ename);
}

VrmlField::VrmlFieldType
VrmlNodeScript::hasEventOut(const char *ename) const
{
  return has(d_eventOuts, ename);
}

VrmlField::VrmlFieldType
VrmlNodeScript::hasField(const char *ename) const
{
  return has(d_fields, ename);
}

VrmlField::VrmlFieldType VrmlNodeScript::hasInterface(const char * id) const
{
    VrmlField::VrmlFieldType fieldType = VrmlField::NO_FIELD;
    
    if ((fieldType = this->hasField(id)) != VrmlField::NO_FIELD) {
        return fieldType;
    }
    
    if ((fieldType = this->hasEventIn(id)) != VrmlField::NO_FIELD) {
        return fieldType;
    }
    
    if ((fieldType = this->hasEventOut(id)) != VrmlField::NO_FIELD) {
        return fieldType;
    }
    
    return fieldType;
}

VrmlField::VrmlFieldType
VrmlNodeScript::has(const FieldList &recs, const char *ename) const
{
  FieldList::const_iterator i;
  for (i = recs.begin(); i != recs.end(); ++i) {
    if (strcmp((*i)->name, ename) == 0)
      return (*i)->type;
  }
  return VrmlField::NO_FIELD;
}

// Get the value of a field or eventOut.

const VrmlField *VrmlNodeScript::getField(const char *fieldName) const
{
  // exposedFields
  if ( strcmp( fieldName, "url" ) == 0 )
    return &d_url;

  // look up 
  else if ( hasField(fieldName) )
    return get(d_fields, fieldName);

  // look up event outs? ....


  return VrmlNodeChild::getField( fieldName );
}


// Set the value of one of the node fields/events.
// setField is public so the parser can access it.

void VrmlNodeScript::setField(const char *fieldName,
			      const VrmlField &fieldValue)
{
  VrmlField::VrmlFieldType ft;

  if TRY_FIELD(url, MFString)	// need to re-initialize() if url changes...
  else if TRY_FIELD(directOutput, SFBool)
  else if TRY_FIELD(mustEvaluate, SFBool)
  else if ( (ft = hasField(fieldName)) != 0 )
    {
      if (ft == VrmlField::fieldType( fieldValue.fieldTypeName() ))
	set(d_fields, fieldName, fieldValue);
      else
	theSystem->error("Invalid type (%s) for %s field of Script node.\n",
		      fieldValue.fieldTypeName(), fieldName );
    }
  else
    VrmlNodeChild::setField(fieldName, fieldValue);
}

void
VrmlNodeScript::setEventIn(const char *fname, const VrmlField & value)
{
  set(d_eventIns, fname, value);
}

void
VrmlNodeScript::setEventOut(const char *fname, const VrmlField & value)
{
#if 0
  cerr << "Script::" << name() << " setEventOut(" << fname << ", "
       << value << endl;
#endif
  set(d_eventOuts, fname, value);
}

void VrmlNodeScript::set(const FieldList & recs, const char * fname,
                         const VrmlField & value) {
    for (FieldList::const_iterator itr = recs.begin(); itr != recs.end();
            ++itr) {
        if (strcmp((*itr)->name, fname) == 0) {
            //
            // Script nodes can be self referential! Check this condition,
            // and "undo" the refcounting: decrement the refcount on any
            // self-references we acquire ownership of, and increment the
            // refcount on any self-references for which we relinquish
            // ownership.
            //
            const VrmlField::VrmlFieldType fieldType(value.fieldType());
            if (fieldType == VrmlField::SFNODE) {
                const VrmlNodePtr & oldNode =
                        static_cast<VrmlSFNode *>((*itr)->value)->get();
                //
                // About to relinquish ownership of a SFNode value. If the
                // SFNode value is this Script node, then we need to
                // *increment* its refcount, since we previously
                // *decremented* it to accommodate creating a cycle between
                // refcounted objects.
                //
                if (oldNode && (oldNode.countPtr->first == this)) {
                    ++(oldNode.countPtr->second);
                }
                
	        delete (*itr)->value;
	        (*itr)->value = value.clone();
                
                //
                // Now, check to see if the new SFNode value is a self-
                // reference. If it is, we need to *decrement* the refcount.
                // A self-reference creates a cycle. If a Script node with
                // a self-reference were completely removed from the scene,
                // it still wouldn't be deleted (if we didn't do this)
                // because the reference it held to itself would prevent the
                // refcount from ever dropping to zero.
                //
                const VrmlNodePtr & newNode =
                        static_cast<VrmlSFNode *>((*itr)->value)->get();
                if (newNode && (newNode.countPtr->first == this)) {
                    --(newNode.countPtr->second);
                }
            } else if (fieldType == VrmlField::MFNODE) {
				size_t i;
                const VrmlMFNode & oldNodes =
                        static_cast<VrmlMFNode &>(*(*itr)->value);
                for (i = 0; i < oldNodes.getLength(); ++i) {
                    const VrmlNodePtr & node = oldNodes.getElement(i);
                    if (node && (node.countPtr->first == this)) {
                        ++(node.countPtr->second);
                    }
                }
                
                delete (*itr)->value;
                (*itr)->value = value.clone();
                
                const VrmlMFNode & newNodes =
                        static_cast<VrmlMFNode &>(*(*itr)->value);
                for (i = 0; i < newNodes.getLength(); ++i) {
                    const VrmlNodePtr & node = newNodes.getElement(i);
                    if (node && (node.countPtr->first == this)) {
                        --(node.countPtr->second);
                    }
                }
            } else {
	        delete (*itr)->value;
	        (*itr)->value = value.clone();
            }
            
	    (*itr)->modified = true;
	    return;
        }
    }
}
