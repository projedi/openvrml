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

/**
 * @class Node
 *
 * @brief A node in the scene graph.
 *
 * Nodes are reference counted, optionally named objects.
 * The reference counting is manual (that is, each user of a
 * Node, such as the VrmlMFNode class, calls reference()
 * and dereference() explicitly). Should make it internal...
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "VrmlNode.h"
#include "Route.h"
#include "VrmlNamespace.h"
#include "nodetype.h"
#include "VrmlScene.h"
#include "MathUtils.h"
#include "VrmlBVolume.h"
#include "VrmlBSphere.h"
#include "VrmlRenderContext.h"

# ifndef NDEBUG
#   define VRML_NODE_DEBUG
# endif

using namespace OpenVRML;

/*
 * Given a NodeType, add in the fields, exposedFields, eventIns
 * and eventOuts defined by the particular node implementation.
 * There's a great big method in VrmlNamespace that just calls
 * defineType for every built in node. Nodes that inherit from other
 * nodes (eg NodeIFaceSet) must call their parent classe's
 * defineType, it doesn't happen automatically. The defineType for
 * Node doesn't actually do anything, since there (currently)
 * aren't any base events to be defined. It's just here for the sake
 * of symmetry (and makes a nice place to park a comment)
 *
 * @param type to get the predefined cached type, pass a null,
 *          otherwise the fields and events will be added to the
 *          passed in vrmlnodetype object
 *
 * @see VrmlNamespace::defineBuiltins()
 */
Node::Node(const NodeType & type, VrmlScene * scene):
        type(type), d_scene(scene), d_modified(false),
        visited(false), d_routes(0) {
  this->setBVolumeDirty(true);
}

Node::Node(const Node & node): type(node.type), id(node.id),
        d_scene(0), d_modified(true), d_routes(0) {
    this->setBVolumeDirty(true);
}


/**
 * @brief Destructor.
 *
 * Free name (if any) and route info.
 */
Node::~Node() 
{
  // Remove the node's name (if any) from the map...
  if (!this->id.empty())
  {
    if (d_scene && d_scene->scope())
      d_scene->scope()->removeNodeName(*this);
  }

  // Remove all routes from this node
  d_routes.resize(0);
}

/**
 * @fn void Node::accept(NodeVisitor & visitor);
 *
 * @brief Accept a visitor.
 *
 * If the node has not been visited, set the <var>visited</var> flag to
 * <code>true</code> and call <code>NodeVisitor::visit()</code> for this object.
 * Otherwise (if the <var>visited</var> flag is already
 * <code>true</code>), this method has no effect.
 *
 * <p>The fact that the <var>visited</var> flag is set <em>before</em> the
 * node is actually visited is an important detail. Even though scene
 * graphs should not have cycles, nodes can be self-referencing: a field
 * of a <code>Script</code> node can legally <code>USE</code> the
 * <code>Script</code> node. (This does not pose a problem for rendering
 * since nodes in a <code>Script</code> node's fields are not part of
 * the transformation hierarchy.)
 *
 * @param visitor
 * @return <code>true</code> if the visitor is accepted (the node
 *         <em>has not</em> been visited during this traversal),
 *         <code>false</code> otherwise (the node <em>has</em> been
 *         visited during this traversal.
 */

/**
 * @brief Recursively set the <var>visited</var> flag to
 *        <code>false</code> for this node and its children.
 *
 * Typically used by a visitor (a class that implements NodeVisitor)
 * after traversal is complete. The default implementation is only
 * appropriate for nodes with no child nodes.
 */
void Node::resetVisitedFlag() {
    this->visited = false;
}

/**
 * @brief Set the nodeId of the node.
 *
 * Some one else (the parser) needs to tell the scene about the name for
 * use in USE/ROUTEs.
 *
 * @param nodeName a string
 * @param ns a pointer to the VrmlNamespace to which this node should
 *           belong
 */
void Node::setId(const std::string & nodeId, VrmlNamespace * const ns) {
    this->id = nodeId;
    if (!nodeId.empty() && ns) {
        ns->addNodeName(*this);
    }
}

/**
 * @brief Retrieve the nodeId of this node.
 *
 * @return the nodeId
 */
const std::string & Node::getId() const {
    return this->id;
}

/**
 * @brief Add to scene.
 *
 * A node can belong to at most one scene for now. If it doesn't belong
 * to a scene, it can't be rendered.
 *
 * @param scene
 * @param relativeUrl
 */
void Node::addToScene(VrmlScene * scene, const std::string &) {
    this->d_scene = scene;
}


// Safe node downcasts. These avoid the dangerous casts of Node* (esp in
// presence of protos), but are ugly in that this class must know about all
// the subclasses. These return 0 if the typecast is invalid.
// Remember to also add new ones to NodeProto. Protos should
// return their first implementation node (except toProto()).

NodeAnchor * Node::toAnchor() const { return 0; }

NodeAppearance * Node::toAppearance() const { return 0; }

NodeAudioClip * Node::toAudioClip() const { return 0; }

NodeBackground * Node::toBackground() const { return 0; }

NodeBillboard * Node::toBillboard() const { return 0; }

NodeBox * Node::toBox() const { return 0; }

NodeChild * Node::toChild() const { return 0; }

NodeCollision * Node::toCollision() const { return 0; }

NodeColor * Node::toColor() const { return 0; }

NodeCone * Node::toCone() const { return 0; }

NodeCoordinate * Node::toCoordinate() const { return 0; }

NodeCylinder * Node::toCylinder() const { return 0; }

NodeDirLight * Node::toDirLight() const { return 0; }

NodeElevationGrid * Node::toElevationGrid() const { return 0; }

NodeExtrusion * Node::toExtrusion() const { return 0; }

NodeFog * Node::toFog() const { return 0; }

NodeFontStyle * Node::toFontStyle() const { return 0; }

NodeGeometry * Node::toGeometry() const { return 0; }

NodeGroup * Node::toGroup() const { return 0; }

NodeIFaceSet * Node::toIFaceSet() const { return 0; }

NodeImageTexture * Node::toImageTexture() const { return 0; }

NodeInline * Node::toInline() const { return 0; }

NodeLight * Node::toLight() const { return 0; }

NodeLOD * Node::toLOD() const { return 0; }

NodeMaterial * Node::toMaterial() const { return 0; }

NodeMovieTexture * Node::toMovieTexture() const { return 0; }

NodeNavigationInfo * Node::toNavigationInfo() const { return 0; }

NodeNormal * Node::toNormal() const { return 0; }

NodeOrientationInt * Node::toOrientationInt() const { return 0; }

NodePlaneSensor * Node::toPlaneSensor() const { return 0; }

NodePositionInt * Node::toPositionInt() const { return 0; }

NodeSphereSensor * Node::toSphereSensor() const { return 0; }

NodeCylinderSensor * Node::toCylinderSensor() const { return 0; }

NodePixelTexture* Node::toPixelTexture() const { return 0; }

NodePointLight * Node::toPointLight() const { return 0; }

NodePointSet * Node::toPointSet() const { return 0; }

NodeScalarInt * Node::toScalarInt() const { return 0; }

ScriptNode * Node::toScript() const { return 0; }

NodeShape * Node::toShape() const { return 0; }

NodeSound * Node::toSound() const { return 0; }

NodeSphere * Node::toSphere() const { return 0; }

NodeSpotLight * Node::toSpotLight() const { return 0; }

NodeSwitch * Node::toSwitch() const { return 0; }

NodeTexture * Node::toTexture() const { return 0; }

NodeTextureCoordinate * Node::toTextureCoordinate() const { return 0; }

NodeTextureTransform * Node::toTextureTransform() const { return 0; }

NodeTimeSensor * Node::toTimeSensor() const { return 0; }

NodeTouchSensor * Node::toTouchSensor() const { return 0; }

NodeTransform * Node::toTransform() const { return 0; }

NodeViewpoint * Node::toViewpoint() const { return 0; }


/**
 * @brief Add a route from an eventOut of this node to an eventIn of another
 *      node.
 */
void Node::addRoute(const std::string & fromEventOut,
                    const NodePtr & toNode,
                    const std::string & toEventIn) {
  // Check to make sure fromEventOut and toEventIn are valid names...
  
  // Is this route already here?
  RouteList::iterator i;

  for (i = d_routes.begin(); i != d_routes.end(); ++i)
  {
    if (toNode == (*i)->toNode
	&& fromEventOut == (*i)->fromEventOut
	&& toEventIn == (*i)->toEventIn)
      return;       // Ignore duplicate routes
  }

  // Add route
  Route* r = new Route(fromEventOut, toNode, toEventIn);

  d_routes.push_back(r);
}


/**
 * @brief Remove a route from an eventOut of this node to an eventIn of another
 *      node.
 */
void Node::deleteRoute(const std::string & fromEventOut,
                       const NodePtr & toNode,
                       const std::string & toEventIn) {
  RouteList::iterator i;

  for (i = d_routes.begin(); i != d_routes.end(); ++i)
  {
    if (toNode == (*i)->toNode
	&& fromEventOut == (*i)->fromEventOut
	&& toEventIn == (*i)->toEventIn)
    {
      i = d_routes.erase(i);
    }
  }
}

/**
 * @brief Get the routes from this node.
 *
 * @return an std::vector of Routes from this node.
 */
std::list<Route*> Node::getRoutes() {
    return this->d_routes;
}

// Dirty bit - indicates node needs to be revisited for rendering.

void Node::setModified()
{
  d_modified = true;
  if (d_scene) d_scene->setModified(); 
}

bool Node::isModified() const
{
  return d_modified; 
}


/**
 * @brief Mark all the nodes in the path as (not) modified.
 *
 * Convenience function used by updateModified.
 *
 * @param path
 * @param mod
 * @param flags
 */
void
Node::markPathModified(NodePath& path, bool mod, int flags) {
  NodePath::iterator i;
  NodePath::iterator end = path.end();
  if (flags & 0x001) {
    for (i = path.begin(); i != end; ++i) {
      Node *c = *i;
      if (mod) {
	// do the proof that our invarient shows that this short
	// circuit is legal...
	c->setModified();
      } else
	c->clearModified();
    }
  }
  if (flags & 0x002) {
    for (i = path.begin(); i != end; ++i) {
      Node *c = *i;
      if (mod) {
	c->setBVolumeDirty(true);
      } else
	c->setBVolumeDirty(false);
    }
  }
}


void
Node::updateModified(NodePath& path, int flags)
{
  if (this->d_modified||this->d_bvol_dirty) markPathModified(path, true, flags);
}


/**
 * Propagate the bvolume dirty flag from children to parents. I
 * don't like this at all, but it's not worth making it pretty
 * because the need for it will go away when parent pointers are
 * implemented.
 *
 * @param path stack of ancestor nodes
 * @param mod set modified flag to this value
 * @param flags 1 indicates normal modified flag, 2 indicates the
 *              bvolume dirty flag, 3 indicates both
 */
// note not virtual
//
void
Node::updateModified(int flags)
{
  NodePath path;
  updateModified(path, flags);
}


void Node::clearFlags()
{
  d_flag = false;
}

// Render

//bool Node::cull(Viewer *v, RenderContext* c)
//{
//if (c && c->getCullFlag()) {
//VrmlBVolume* bv = this->getBVolume();
//int r = v->isectViewVolume(*bv); // better not be null...
//if (r == VrmlBVolume::BV_OUTSIDE) {
//cout << "Node::render():OUTSIDE:culled" << endl;
//return true;
//} else if (r == VrmlBVolume::BV_INSIDE) {
//cout << "Node::render():INSIDE:no more cull tests" << endl;
//c->setCullFlag(false);
//return false;
//} else {
//cout << "Node::render():PARTIAL:continue cull tests" << endl;
//return false;
//}
//}
//return false;
//}


/**
 * Get this node's bounding volume. Nodes that have no bounding
 * volume, or have a difficult to calculate bvolume (like, say,
 * Extrusion or Billboard) can just return an infinite bsphere. Note
 * that returning an infinite bvolume means that all the node's
 * ancestors will also end up with an infinite bvolume, and will
 * never be culled.
 *
 * @return this node's bounding volume
 */
const VrmlBVolume*
Node::getBVolume() const
{
  static VrmlBSphere* inf_bsphere = (VrmlBSphere*)0;
  if (!inf_bsphere) {
    inf_bsphere = new VrmlBSphere();
    inf_bsphere->setMAX();
  }
  ((Node*)this)->setBVolumeDirty(false);
  return inf_bsphere;
}


/**
 * Override a node's calculated bounding volume. Not implemented.
 *
 * @todo Implement me!
 */
void Node::setBVolume(const VrmlBVolume & v) {
    // XXX Implement me!
}

/** 
 * Indicate that a node's bounding volume needs to be recalculated
 * (or not). If a node's bvolume is invalid, then the bvolumes of
 * all that node's ancestors are also invalid. Normally, the node
 * itself will determine when its bvolume needs updating.
 */
void
Node::setBVolumeDirty(bool f)
{
  this->d_bvol_dirty = f;
  if (f && this->d_scene) // only if dirtying, not clearing
    this->d_scene->d_flags_need_updating = true;
}

/**
 * Return true if the node's bounding volume needs to be
 * recalculated.
 */
bool
Node::isBVolumeDirty() const
{
  if (d_scene && d_scene->d_flags_need_updating) {
    d_scene->updateFlags();
    d_scene->d_flags_need_updating = false;
  }
  return this->d_bvol_dirty;
}

/**
 * Render this node. Actually, most of the rendering work is
 * delegated to the viewer, but this method is responsible for
 * traversal to the node's renderable children, including
 * culling. Each node class needs to implement this routine
 * appropriately. It's not abstract since it doesn't make sense to
 * call render on some nodes. Alternative would be to break render
 * out into a seperate mixins class, but that's probably overkill.
 *
 * @param v viewer implementation responsible for actually doing the
 *          drawing
 * @param rc generic context argument, holds things like the
 *          accumulated modelview transform.
 */
void Node::render(Viewer* v, VrmlRenderContext rc)
{
  //if (cull(v, c)) return;
  clearModified();
}

/**
 * @brief Accumulate transformations for proper rendering of bindable
 *        nodes.
 *
 * Cache a pointer to one of the parent transforms. The resulting
 * pointer is used by getParentTransform. Grouping nodes need to
 * redefine this, the default implementation does nothing.
 *
 * @param p parent node. can be null.
 *
 * @deprecated This routine will go away once parent pointers
 * are implemented.
 */
void Node::accumulateTransform(Node *)
{
  ;
}

/**
 * Return the nearest ancestor node that affects the modelview
 * transform. Doesn't work for nodes with more than one parent.
 */
Node* Node::getParentTransform() { return 0; }

/**
 * Compute the inverse of the transform above a viewpoint node. Just
 * like the version that takes a matrix, but instead calls
 * Viewer::setTransform at each level. The idea is to call this
 * routine right before the start of a render traversal.
 *
 * @see getParentTransform
 *
 * @deprecated This method is (gradually) being replaces by
 * inverseTranform(double[4][4]) and should no longer be used.
 */
void Node::inverseTransform(Viewer *v)
{
  Node *parentTransform = getParentTransform();
  if (parentTransform)
    parentTransform->inverseTransform(v);
}

/**
 * Compute the inverse of the transform stack above a Viewpoint
 * node. This is safe since the behavior of multi-parented
 * Viewpoint nodes is undefined. May be called at any time.
 *
 * @param M return the accumulated inverse
 *
 * @see accumulateTransform
 * @see getParentTransform
 */
void Node::inverseTransform(VrmlMatrix & M)
{
  Node *parentTransform = getParentTransform();
  if (parentTransform)
    parentTransform->inverseTransform(M);
  else
    M.makeIdentity();
}

/**
 * @fn void OpenVRML::Node::eventIn(double timestamp,
 *                                  const std::string & eventInId,
 *                                  const FieldValue & fieldValue)
 *
 * @brief Pass a named event to this node.
 *
 * This method needs to be overridden to support any node-specific eventIns
 * behaviors, but exposedFields (should be) handled here...
 */
void Node::eventIn(double timeStamp,
                   const std::string & eventName,
                   const FieldValue & fieldValue) {
    // Strip set_ prefix
    static const char * eventInPrefix = "set_";
    std::string basicEventName;
    if (std::equal(eventInPrefix, eventInPrefix + 4, eventName.begin())) {
        basicEventName = eventName.substr(4);
    } else {
        basicEventName = eventName;
    }

    // Handle exposedFields 
    if (this->type.hasExposedField(basicEventName)) {
        this->setField(basicEventName, fieldValue);
        std::string eventOutName = basicEventName + "_changed";
        this->eventOut(timeStamp, eventOutName, fieldValue);
        setModified();
    }
    
    // Handle set_field eventIn/field
    else if (this->type.hasEventIn(eventName)
            && this->type.hasField(basicEventName)) {
        this->setField(basicEventName, fieldValue);
        this->setModified();
    } else
        cerr << "Error: unhandled eventIn " << this->type.getId().c_str()
		<< "::" << this->id.c_str() << "." << eventName.c_str() << endl;
}

/**
 * @fn const OpenVRML::MFNode OpenVRML::Node::getChildren() const
 *
 * @brief Get this node's child nodes as an MFNode.
 *
 * This method is intended to provide generalized access to a node's child
 * nodes. The default implementation returns an empty MFNode. Node
 * implementations that include child nodes should override this method to
 * return an appropriate MFNode.
 *
 * <p>The returned MFNode should include <strong>all</strong> of the node's
 * child nodes, from all of the node's SFNode or MFNode fields. Since fields
 * do not have a defined order, no ordering is defined for the nodes that
 * occur in the returned MFNode. Therefore, traversals that depend on any
 * such ordering should not use this method.</p>
 *
 * @return an MFNode containing any children of this node.
 */
const MFNode Node::getChildren() const {
    return MFNode();
}

/**
 * @brief Send an event from this node.
 */
void Node::eventOut(double timeStamp, const std::string & id,
                    const FieldValue & fieldValue) {

  RouteList::const_iterator i;

  for (i = d_routes.begin(); i != d_routes.end(); ++i)
  {
    if (id == (*i)->fromEventOut)
    {
      FieldValue * eventValue = fieldValue.clone();
      assert(this->d_scene);
      this->d_scene->queueEvent(timeStamp, eventValue, (*i)->toNode,
				(*i)->toEventIn);
    }
  }
}

ostream& operator<<(ostream& os, const Node& f)
{ return f.print(os, 0); }


ostream& Node::print(ostream& os, int indent) const
{
  for (int i=0; i<indent; ++i)
    os << ' ';

  if (!this->id.empty()) {
    os << "DEF " << this->id.c_str() << " ";
  }

  os << this->type.getId().c_str() << " { ";

  // cast away const-ness for now...
  Node *n = (Node*)this;
  n->printFields(os, indent+INDENT_INCREMENT);

  os << " }";

  return os; 
}

// This should probably generate an error...
// Might be nice to make this non-virtual (each node would have
// to provide a getField(const char* name) method and specify
// default values in the addField(). The NodeType class would 
// have to make the fields list public.

ostream& Node::printFields(ostream& os, int /*indent*/)
{
  os << "# Error: " << this->type.getId().c_str()
     << "::printFields unimplemented.\n";
  return os; 
}


ostream& Node::printField(ostream & os, int indent,
                          const char *name, const FieldValue & f) {
  os << endl;
  for (int i=0; i<indent; ++i)
    os << ' ';
  os << name << ' ' << f;
  return os; 
}


/**
 * @brief Set a field by name (used by the parser, not for external
 *        consumption).
 *
 * Set the value of one of the node fields. No fields exist at the
 * top level, so reaching this indicates an error.
 *
 * @todo Make this method pure virtual.
 */
void Node::setField(const std::string & fieldId, const FieldValue &) {
    theSystem->error("%s::setField: no such field (%s)",
                     this->type.getId().c_str(), fieldId.c_str());
}

/**
 * @brief Get a field or eventOut by name.
 *
 * getField is used by Script nodes to access exposedFields. It does not
 * allow access to private fields (there tend to be specific access
 * functions for each field for programmatic access).
 *
 * @todo Make this method pure virtual.
 */
const FieldValue * Node::getField(const std::string & fieldId) const {
    theSystem->error("%s::getField: no such field (%s)\n",
                     this->type.getId().c_str(), fieldId.c_str());
    return 0;
}


/**
 * @brief Retrieve a named eventOut/exposedField value.
 *
 * Used by the script node to access the node fields. This just strips
 * the _changed suffix and tries to access the field using getField.
 */
const FieldValue * Node::getEventOut(const std::string & fieldName) const {
    static const char * eventOutSuffix = "_changed";
    std::string basicFieldName;
    if (fieldName.length() > 8
            && std::equal(fieldName.end() - 8, fieldName.end(),
                          eventOutSuffix)) {
        basicFieldName = fieldName.substr(0, fieldName.length() - 8);
    } else {
        basicFieldName = fieldName;
    }
    
    // Handle exposedFields 
    if (this->type.hasExposedField(basicFieldName)) {
        return getField(basicFieldName);
    } else if (this->type.hasEventOut(fieldName)) {
        return getField(fieldName);
    }
    return 0;
}
