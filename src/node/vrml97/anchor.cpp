// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 78 -*-
//
// OpenVRML
//
// Copyright 1998  Chris Morley
// Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009  Braden McDaniel
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, see <http://www.gnu.org/licenses/>.
//

# include "anchor.h"
# include "grouping_node_base.h"
# include <openvrml/scene.h>
# include <boost/array.hpp>

# ifdef HAVE_CONFIG_H
#   include <config.h>
# endif

namespace {

    class OPENVRML_LOCAL anchor_node :
        public openvrml_node_vrml97::grouping_node_base<anchor_node>,
        public openvrml::pointing_device_sensor_node {

        friend class openvrml_node_vrml97::anchor_metatype;

        exposedfield<openvrml::sfstring> description_;
        exposedfield<openvrml::mfstring> parameter_;
        exposedfield<openvrml::mfstring> url_;

    public:
        anchor_node(const openvrml::node_type & type,
                    const boost::shared_ptr<openvrml::scope> & scope);
        virtual ~anchor_node() OPENVRML_NOTHROW;

    private:
        virtual void do_render_child(openvrml::viewer & viewer,
                                     openvrml::rendering_context context);
        virtual void do_activate(double timestamp, bool over, bool active,
                                 const double (&point)[3]);
    };

    /**
     * @class anchor_node
     *
     * @brief Represents Anchor node instances.
     */

    /**
     * @var class anchor_node::anchor_metatype
     *
     * @brief Class object for Anchor nodes.
     */

    /**
     * @var openvrml::node_impl_util::abstract_node<anchor_node>::exposedfield<openvrml::sfstring> anchor_node::description_
     *
     * @brief description exposedField
     */

    /**
     * @var openvrml::node_impl_util::abstract_node<anchor_node>::exposedfield<openvrml::mfstring> anchor_node::parameter_
     *
     * @brief parameter exposedField
     */

    /**
     * @var openvrml::node_impl_util::abstract_node<anchor_node>::exposedfield<openvrml::mfstring> anchor_node::url_
     *
     * @brief url exposedField
     */

    /**
     * @brief Construct.
     *
     * @param type  the node_type associated with this node.
     * @param scope     the scope to which the node belongs.
     */
    anchor_node::
    anchor_node(const openvrml::node_type & type,
                const boost::shared_ptr<openvrml::scope> & scope):
        node(type, scope),
        bounded_volume_node(type, scope),
        child_node(type, scope),
        grouping_node(type, scope),
        openvrml_node_vrml97::grouping_node_base<anchor_node>(type, scope),
        pointing_device_sensor_node(type, scope),
        description_(*this),
        parameter_(*this),
        url_(*this)
    {
        this->bounding_volume_dirty(true);
    }

    /**
     * @brief Destroy.
     */
    anchor_node::~anchor_node() OPENVRML_NOTHROW
    {}

    /**
     * @brief Render the node.
     *
     * @param viewer    a viewer.
     * @param context   a rendering context.
     */
    void
    anchor_node::
    do_render_child(openvrml::viewer & viewer,
                    const openvrml::rendering_context context)
    {
        viewer.set_sensitive(this);

        // Render children
        this->openvrml_node_vrml97::grouping_node_base<anchor_node>::
            do_render_child(viewer, context);

        viewer.set_sensitive(0);
    }

    /**
     * @brief Handle a click by loading the url.
     */
    void anchor_node::do_activate(double,
                                  const bool over,
                                  const bool active,
                                  const double (&)[3])
    {
        assert(this->scene());
        //
        // @todo This should really be (is_over && !is_active && n->was_active)
        // (ie, button up over the anchor after button down over the
        // anchor)
        //
        if (over && active) {
            this->scene()->load_url(this->url_.mfstring::value(),
                                    this->parameter_.mfstring::value());
        }
    }
}


/**
 * @brief @c node_metatype identifier.
 */
const char * const openvrml_node_vrml97::anchor_metatype::id =
    "urn:X-openvrml:node:Anchor";

/**
 * @brief Construct.
 *
 * @param browser the @c browser associated with this @c anchor_metatype.
 */
openvrml_node_vrml97::anchor_metatype::
anchor_metatype(openvrml::browser & browser):
    node_metatype(anchor_metatype::id, browser)
{}

/**
 * @brief Destroy.
 */
openvrml_node_vrml97::anchor_metatype::~anchor_metatype() OPENVRML_NOTHROW
{}

# define ANCHOR_INTERFACE_SEQ                                           \
    ((eventin,      mfnode,   "addChildren",    add_children_listener_)) \
    ((eventin,      mfnode,   "removeChildren", remove_children_listener_)) \
    ((exposedfield, mfnode,   "children",       children_))             \
    ((exposedfield, sfstring, "description",    description_))          \
    ((exposedfield, mfstring, "parameter",      parameter_))            \
    ((exposedfield, mfstring, "url",            url_))                  \
    ((field,        sfvec3f,  "bboxCenter",     bbox_center_))          \
    ((field,        sfvec3f,  "bboxSize",       bbox_size_))            \
    ((exposedfield, sfnode,   "metadata",       metadata))

OPENVRML_NODE_IMPL_UTIL_DEFINE_DO_CREATE_TYPE(openvrml_node_vrml97,
                                              anchor_metatype,
                                              anchor_node,
                                              ANCHOR_INTERFACE_SEQ)
