// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 78 -*-
//
// OpenVRML
//
// Copyright 1998  Chris Morley
// Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009  Braden McDaniel
// Copyright 2002  S. K. Bose
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

# ifdef HAVE_CONFIG_H
#   include <config.h>
# endif

# include <boost/array.hpp>
# include <private.h>
# include <openvrml/browser.h>

# include "grouping_node_base.h"
# include "billboard.h"

namespace {

    class OPENVRML_LOCAL billboard_node :
        public openvrml_node_vrml97::grouping_node_base<billboard_node> {
        friend class openvrml_node_vrml97::billboard_metatype;

        exposedfield<openvrml::sfvec3f> axis_of_rotation_;

    public:
        static
        const openvrml::mat4f
        billboard_to_matrix(const billboard_node & node,
                            const openvrml::mat4f & modelview);

        billboard_node(const openvrml::node_type & type,
                       const boost::shared_ptr<openvrml::scope> & scope);
        virtual ~billboard_node() OPENVRML_NOTHROW;

    private:
        virtual void do_render_child(openvrml::viewer & viewer,
                                     openvrml::rendering_context context);
    };


    /**
     * @class billboard_node
     *
     * @brief Billboard node instances.
     */

    /**
     * @var class billboard_node::billboard_metatype
     *
     * @brief Class object for Billboard nodes.
     */

    /**
     * @var openvrml::node_impl_util::abstract_node<billboard_node>::exposedfield<openvrml::sfvec3f> billboard_node::axis_of_rotation_
     *
     * @brief axisOfRotation exposedField.
     */

    /**
     * @brief Get the bounding box transformation matrix.
     *
     * @param node      a pointer to a billboard_node.
     * @param modelview input ModelView transformation matrix.
     *
     * @return the bounding box transformation matrix.
     */
    const openvrml::mat4f
    billboard_node::
    billboard_to_matrix(const billboard_node & node,
                        const openvrml::mat4f & modelview)
    {
        using namespace openvrml;

        const mat4f inverse_modelview = modelview.inverse();

        mat4f result;

        // Viewer position in local coordinate system
        const vec3f position = make_vec3f(inverse_modelview[3][0],
                                          inverse_modelview[3][1],
                                          inverse_modelview[3][2]).normalize();

        // Viewer-alignment
        if ((node.axis_of_rotation_.sfvec3f::value()[0] == 0)
            && (node.axis_of_rotation_.sfvec3f::value()[1] == 0)
            && (node.axis_of_rotation_.sfvec3f::value()[2] == 0)) {
            //
            // Viewer's up vector
            //
            const vec3f up = make_vec3f(inverse_modelview[1][0],
                                        inverse_modelview[1][1],
                                        inverse_modelview[1][2]).normalize();

            // get x-vector from the cross product of Viewer's
            // up vector and billboard-to-viewer vector.
            const vec3f X = up * position;

            result[0][0] = X[0];
            result[0][1] = X[1];
            result[0][2] = X[2];
            result[0][3] = 0.0;

            result[1][0] = up[0];
            result[1][1] = up[1];
            result[1][2] = up[2];
            result[1][3] = 0.0;

            result[2][0] = position[0];
            result[2][1] = position[1];
            result[2][2] = position[2];
            result[2][3] = 0.0;

            result[3][0] = 0.0;
            result[3][1] = 0.0;
            result[3][2] = 0.0;
            result[3][3] = 1.0;
        } else { // use axis of rotation
            // axis of rotation will be the y-axis vector
            const vec3f Y(node.axis_of_rotation_.sfvec3f::value());

            // Plane defined by the axisOfRotation and billboard-to-viewer
            // vector.
            const vec3f X = (Y * position).normalize();

            // Get Z axis vector from cross product of X and Y
            const vec3f Z = X * Y;

            // Transform Z axis vector of current coordinate system to new
            // coordinate system.
            float nz[3] = { X[2], Y[2], Z[2] };

            // Calculate the angle by which the Z axis vector of current
            // coordinate system has to be rotated around the Y axis to new
            // coordinate system.
            float angle = float(acos(nz[2]));
            if(nz[0] > 0) { angle = -angle; }
            result = make_rotation_mat4f(make_rotation(Y, angle));
        }
        return result;
    }

    /**
     * @brief Construct.
     *
     * @param type  the node_type associated with the node instance.
     * @param scope the scope to which the node belongs.
     */
    billboard_node::
    billboard_node(const openvrml::node_type & type,
                   const boost::shared_ptr<openvrml::scope> & scope):
        node(type, scope),
        bounded_volume_node(type, scope),
        child_node(type, scope),
        grouping_node(type, scope),
        openvrml_node_vrml97::grouping_node_base<billboard_node>(type, scope),
        axis_of_rotation_(*this, openvrml::make_vec3f(0.0, 1.0, 0.0))
    {}

    /**
     * @brief Destroy.
     */
    billboard_node::~billboard_node() OPENVRML_NOTHROW
    {}

    /**
     * @brief Render the node.
     *
     * @param viewer    a viewer.
     * @param context   the rendering context.
     */
    void
    billboard_node::
    do_render_child(openvrml::viewer & viewer,
                    openvrml::rendering_context context)
    {
        using openvrml::mat4f;

        mat4f new_LM = context.matrix();
        mat4f LM = billboard_to_matrix(*this, new_LM);
        new_LM = LM * new_LM;
        context.matrix(new_LM);

        if (this->modified()) {
            viewer.remove_object(*this);
        }

        if (this->children_.mfnode::value().size() > 0) {
            viewer.begin_object(this->id().c_str());

            viewer.transform(LM);

            // Render children
            this->openvrml_node_vrml97::grouping_node_base<billboard_node>::
                do_render_child(viewer, context);

            viewer.end_object();
        }

        this->node::modified(false);
    }
}

/**
 * @brief @c node_metatype identifier.
 */
const char * const openvrml_node_vrml97::billboard_metatype::id =
    "urn:X-openvrml:node:Billboard";

/**
 * @brief Construct.
 *
 * @param browser the @c browser associated with this class object.
 */
openvrml_node_vrml97::billboard_metatype::
billboard_metatype(openvrml::browser & browser):
    node_metatype(billboard_metatype::id, browser)
{}

/**
 * @brief Destroy.
 */
openvrml_node_vrml97::billboard_metatype::~billboard_metatype()
    OPENVRML_NOTHROW
{}

# define BILLBOARD_INTERFACE_SEQ                                        \
    ((eventin,      mfnode,  "addChildren",    add_children_listener_)) \
    ((eventin,      mfnode,  "removeChildren", remove_children_listener_)) \
    ((exposedfield, sfvec3f, "axisOfRotation", axis_of_rotation_))      \
    ((exposedfield, mfnode,  "children",       children_))              \
    ((field,        sfvec3f, "bboxCenter",     bbox_center_))           \
    ((field,        sfvec3f, "bboxSize",       bbox_size_))             \
    ((exposedfield, sfnode,  "metadata",       metadata))

OPENVRML_NODE_IMPL_UTIL_DEFINE_DO_CREATE_TYPE(openvrml_node_vrml97,
                                              billboard_metatype,
                                              billboard_node,
                                              BILLBOARD_INTERFACE_SEQ)
