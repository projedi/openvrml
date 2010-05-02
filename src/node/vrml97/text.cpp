// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 78 -*-
//
// OpenVRML
//
// Copyright 1998  Chris Morley
// Copyright 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010
//   Braden McDaniel
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

# include "text.h"
# include <private.h>
# include <openvrml/node_impl_util.h>
# include <openvrml/browser.h>
# include <openvrml/viewer.h>
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
#   include <ft2build.h>
#   include FT_FREETYPE_H
#   include FT_GLYPH_H
#   include FT_OUTLINE_H
#   ifdef _WIN32
#     include <windows.h>
#     include <strsafe.h>
#     include <shlobj.h>
#     undef interface
#   else
#     include <fontconfig/fontconfig.h>
extern "C" {
#     include <fontconfig/fcfreetype.h>
}
#   endif
# endif
# include <boost/array.hpp>
# include <boost/ptr_container/ptr_vector.hpp>
# include <boost/scope_exit.hpp>

# ifdef HAVE_CONFIG_H
#   include <config.h>
# endif

namespace {

# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
    typedef boost::mpl::if_c<sizeof (wchar_t) == 4,
                             wchar_t,
                             openvrml::int32>::type
        char32_t;
# endif

    class OPENVRML_LOCAL text_node :
        public openvrml::node_impl_util::abstract_node<text_node>,
        public openvrml::geometry_node {

        friend class openvrml_node_vrml97::text_metatype;

        class string_exposedfield : public exposedfield<openvrml::mfstring> {
        public:
            explicit string_exposedfield(text_node & node);
            string_exposedfield(const string_exposedfield & obj)
                OPENVRML_NOTHROW;
            virtual ~string_exposedfield() OPENVRML_NOTHROW;

        private:
            virtual std::auto_ptr<openvrml::field_value> do_clone() const
                OPENVRML_THROW1(std::bad_alloc);
            virtual void event_side_effect(const openvrml::mfstring & string,
                                           double timestamp)
                OPENVRML_THROW1(std::bad_alloc);
        };

        class font_style_exposedfield : public exposedfield<openvrml::sfnode> {
        public:
            explicit font_style_exposedfield(text_node & node);
            font_style_exposedfield(const font_style_exposedfield & obj)
                OPENVRML_NOTHROW;
            virtual ~font_style_exposedfield() OPENVRML_NOTHROW;

        private:
            virtual std::auto_ptr<openvrml::field_value> do_clone() const
                OPENVRML_THROW1(std::bad_alloc);
            virtual void event_side_effect(const openvrml::sfnode & font_style,
                                           double timestamp)
                OPENVRML_THROW1(std::bad_alloc);
        };

        class length_exposedfield : public exposedfield<openvrml::mffloat> {
        public:
            explicit length_exposedfield(text_node & node);
            length_exposedfield(const length_exposedfield & obj) OPENVRML_NOTHROW;
            virtual ~length_exposedfield() OPENVRML_NOTHROW;

        private:
            virtual std::auto_ptr<openvrml::field_value> do_clone() const
                OPENVRML_THROW1(std::bad_alloc);
            virtual void event_side_effect(const openvrml::mffloat & length,
                                           double timestamp)
                OPENVRML_THROW1(std::bad_alloc);
        };

        class max_extent_exposedfield : public exposedfield<openvrml::sffloat> {
        public:
            explicit max_extent_exposedfield(text_node & node);
            max_extent_exposedfield(const max_extent_exposedfield & obj)
                OPENVRML_NOTHROW;
            virtual ~max_extent_exposedfield() OPENVRML_NOTHROW;

        private:
            virtual std::auto_ptr<openvrml::field_value> do_clone() const
                OPENVRML_THROW1(std::bad_alloc);
            virtual void event_side_effect(const openvrml::sffloat & max_extent,
                                           double timestamp)
                OPENVRML_THROW1(std::bad_alloc);
        };

        string_exposedfield string_;
        font_style_exposedfield font_style_;
        length_exposedfield length_;
        max_extent_exposedfield max_extent_;
        openvrml::sfbool solid_;

        class glyph_geometry {
            std::vector<openvrml::vec2f> coord_;
            std::vector<openvrml::int32> coord_index_;
            float advance_width_;
            float advance_height_;

        public:
            glyph_geometry(FT_Face face, FT_UInt glyph_index, float size)
                OPENVRML_THROW1(std::bad_alloc);

            const std::vector<openvrml::vec2f> & coord() const;
            const std::vector<openvrml::int32> & coord_index() const;
            float advance_width() const;
            float advance_height() const;
        };

        class line_geometry {
            const bool horizontal_;
            const bool left_to_right_;
            const bool top_to_bottom_;

            std::vector<openvrml::vec2f> coord_;
            std::vector<openvrml::int32> coord_index_;
            float x_min_, x_max_, y_min_, y_max_;
            std::size_t polygons_;
            openvrml::vec2f pen_pos_;

        public:
            line_geometry(bool horizontal,
                          bool left_to_right,
                          bool top_to_bottom,
                          const openvrml::vec2f & pen_start);

            const std::vector<openvrml::vec2f> & coord() const;
            const std::vector<openvrml::int32> & coord_index() const;
            float x_min() const;
            float x_max() const;
            float y_min() const;
            float y_max() const;
            std::size_t polygons() const;

            void add(const glyph_geometry & glyph);
            void scale(float length);
        };

        class text_geometry {
            std::vector<openvrml::vec3f> coord_;
            std::vector<openvrml::int32> coord_index_;
            std::vector<openvrml::vec3f> normal_;
            std::vector<openvrml::vec2f> tex_coord_;
            float x_min_, x_max_, y_min_, y_max_;

        public:
            text_geometry(const boost::ptr_vector<line_geometry> & lines,
                          const std::string & major_alignment,
                          const std::string & minor_alignment,
                          bool horizontal,
                          float size,
                          float spacing,
                          float max_extent)
                OPENVRML_THROW1(std::bad_alloc);

            const std::vector<openvrml::vec3f> & coord() const OPENVRML_NOTHROW;
            const std::vector<openvrml::int32> & coord_index() const
                OPENVRML_NOTHROW;
            const std::vector<openvrml::vec3f> & normal() const
                OPENVRML_NOTHROW;
            const std::vector<openvrml::vec2f> & tex_coord() const
                OPENVRML_NOTHROW;

        private:
            void add(const line_geometry & line,
                     const std::string & major_alignment,
                     bool horizontal)
                OPENVRML_THROW1(std::bad_alloc);
            void scale(float max_extent) OPENVRML_NOTHROW;
            void minor_align(const std::string & align,
                             bool horizontal,
                             float size,
                             float spacing,
                             std::size_t lines)
                OPENVRML_NOTHROW;
            void generate_normals(std::size_t polygons)
                OPENVRML_THROW1(std::bad_alloc);
            void generate_tex_coords(float size)
                OPENVRML_THROW1(std::bad_alloc);
        };

        boost::scoped_ptr<text_geometry> text_geometry_;

# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
        typedef std::vector<std::vector<char32_t> > ucs4_string_t;
        typedef std::map<FT_UInt, glyph_geometry> glyph_geometry_map_t;

        ucs4_string_t ucs4_string;
        FT_Face face;
        glyph_geometry_map_t glyph_geometry_map;
# endif

    public:
        text_node(const openvrml::node_type & type,
                  const boost::shared_ptr<openvrml::scope> & scope);
        virtual ~text_node() OPENVRML_NOTHROW;

    private:
        virtual bool do_modified() const
            OPENVRML_THROW1(boost::thread_resource_error);

        virtual void do_render_geometry(openvrml::viewer & viewer,
                                        openvrml::rendering_context context);

        virtual void do_initialize(double timestamp)
            OPENVRML_THROW1(std::bad_alloc);
        virtual void do_shutdown(double timestamp) OPENVRML_NOTHROW;

        void update_ucs4() OPENVRML_THROW1(std::bad_alloc);
        void update_face() OPENVRML_THROW1(std::bad_alloc);
        void update_geometry() OPENVRML_THROW1(std::bad_alloc);
    };

    /**
     * @class text_node
     *
     * @brief Text node instances.
     */

    /**
     * @var class text_node::text_metatype
     *
     * @brief Class object for Text nodes.
     */

    /**
     * @internal
     *
     * @class text_node::string_exposedfield
     *
     * @brief string exposedField implementation.
     */

    /**
     * @brief Construct.
     *
     * @param node  text_node.
     */
    text_node::string_exposedfield::
    string_exposedfield(text_node & node):
        openvrml::node_event_listener(node),
        openvrml::event_emitter(static_cast<const openvrml::field_value &>(*this)),
        mfstring_listener(node),
        exposedfield<openvrml::mfstring>(node)
    {}

    /**
     * @brief Construct a copy.
     *
     * @param obj   instance to copy.
     */
    text_node::string_exposedfield::
    string_exposedfield(const string_exposedfield & obj) OPENVRML_NOTHROW:
        openvrml::event_listener(),
        openvrml::node_event_listener(obj.openvrml::node_event_listener::node()),
        openvrml::event_emitter(static_cast<const openvrml::field_value &>(*this)),
        mfstring_listener(obj.openvrml::node_event_listener::node()),
        exposedfield<openvrml::mfstring>(obj)
    {}

    /**
     * @brief Destroy.
     */
    text_node::string_exposedfield::
    ~string_exposedfield() OPENVRML_NOTHROW
    {}

    /**
     * @brief Polymorphically construct a copy.
     *
     * @return a copy of the instance.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    std::auto_ptr<openvrml::field_value>
    text_node::string_exposedfield::do_clone() const
        OPENVRML_THROW1(std::bad_alloc)
    {
        return std::auto_ptr<openvrml::field_value>(
            new string_exposedfield(*this));
    }

    /**
     * @brief Process event.
     *
     * @param string    text strings.
     * @param timestamp the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void
    text_node::string_exposedfield::
    event_side_effect(const openvrml::mfstring &, double)
        OPENVRML_THROW1(std::bad_alloc)
    {
        try {
            text_node & node =
                dynamic_cast<text_node &>(this->node_event_listener::node());
            node.update_ucs4();
            node.update_geometry();
        } catch (std::bad_cast & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
        }
    }

    /**
     * @internal
     *
     * @class text_node::font_style_exposedfield
     *
     * @brief fontStyle exposedField implementation.
     */

    /**
     * @brief Construct.
     *
     * @param node  text_node.
     */
    text_node::font_style_exposedfield::
    font_style_exposedfield(text_node & node):
        openvrml::node_event_listener(node),
        openvrml::event_emitter(static_cast<const openvrml::field_value &>(*this)),
        sfnode_listener(node),
        exposedfield<openvrml::sfnode>(node)
    {}

    /**
     * @brief Construct a copy.
     *
     * @param obj   instance to copy.
     */
    text_node::font_style_exposedfield::
    font_style_exposedfield(const font_style_exposedfield & obj) OPENVRML_NOTHROW:
        openvrml::event_listener(),
        openvrml::node_event_listener(obj.openvrml::node_event_listener::node()),
        openvrml::event_emitter(static_cast<const openvrml::field_value &>(*this)),
        sfnode_listener(obj.openvrml::node_event_listener::node()),
        exposedfield<openvrml::sfnode>(obj)
    {}

    /**
     * @brief Destroy.
     */
    text_node::font_style_exposedfield::~font_style_exposedfield() OPENVRML_NOTHROW
    {}

    /**
     * @brief Polymorphically construct a copy.
     *
     * @return a copy of the instance.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    std::auto_ptr<openvrml::field_value>
    text_node::font_style_exposedfield::do_clone() const
        OPENVRML_THROW1(std::bad_alloc)
    {
        return std::auto_ptr<openvrml::field_value>(
            new font_style_exposedfield(*this));
    }

    /**
     * @brief Process event.
     *
     * @param font_style    text strings.
     * @param timestamp     the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void
    text_node::font_style_exposedfield::
    event_side_effect(const openvrml::sfnode &, double)
        OPENVRML_THROW1(std::bad_alloc)
    {
        try {
            text_node & node =
                dynamic_cast<text_node &>(this->node_event_listener::node());
            node.update_ucs4();
            node.update_geometry();
        } catch (std::bad_cast & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
        }
    }

    /**
     * @internal
     *
     * @class text_node::length_exposedfield
     *
     * @brief length exposedField implementation.
     */

    /**
     * @brief Construct.
     *
     * @param node  text_node.
     */
    text_node::length_exposedfield::
    length_exposedfield(text_node & node):
        openvrml::node_event_listener(node),
        openvrml::event_emitter(static_cast<const openvrml::field_value &>(*this)),
        mffloat_listener(node),
        exposedfield<openvrml::mffloat>(node)
    {}

    /**
     * @brief Construct a copy.
     *
     * @param obj   instance to copy.
     */
    text_node::length_exposedfield::
    length_exposedfield(const length_exposedfield & obj) OPENVRML_NOTHROW:
        openvrml::event_listener(),
        openvrml::node_event_listener(obj.openvrml::node_event_listener::node()),
        openvrml::event_emitter(static_cast<const openvrml::field_value &>(*this)),
        mffloat_listener(obj.openvrml::node_event_listener::node()),
        exposedfield<openvrml::mffloat>(obj)
    {}

    /**
     * @brief Destroy.
     */
    text_node::length_exposedfield::~length_exposedfield()
        OPENVRML_NOTHROW
    {}

    /**
     * @brief Polymorphically construct a copy.
     *
     * @return a copy of the instance.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    std::auto_ptr<openvrml::field_value>
    text_node::length_exposedfield::do_clone() const
        OPENVRML_THROW1(std::bad_alloc)
    {
        return std::auto_ptr<openvrml::field_value>(
            new length_exposedfield(*this));
    }

    /**
     * @brief Process event.
     *
     * @param length    length of the text strings.
     * @param timestamp the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void
    text_node::length_exposedfield::
    event_side_effect(const openvrml::mffloat &, double)
        OPENVRML_THROW1(std::bad_alloc)
    {
        try {
            text_node & node =
                dynamic_cast<text_node &>(this->node_event_listener::node());
            node.update_geometry();
        } catch (std::bad_cast & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
        }
    }

    /**
     * @internal
     *
     * @class text_node::max_extent_exposedfield
     *
     * @brief maxExtent exposedField implementation.
     */

    /**
     * @brief Construct.
     *
     * @param node  text_node.
     */
    text_node::max_extent_exposedfield::
    max_extent_exposedfield(text_node & node):
        openvrml::node_event_listener(node),
        openvrml::event_emitter(static_cast<const openvrml::field_value &>(*this)),
        sffloat_listener(node),
        exposedfield<openvrml::sffloat>(node)
    {}

    /**
     * @brief Construct a copy.
     *
     * @param obj   instance to copy.
     */
    text_node::max_extent_exposedfield::
    max_extent_exposedfield(const max_extent_exposedfield & obj) OPENVRML_NOTHROW:
        openvrml::event_listener(),
        openvrml::node_event_listener(obj.openvrml::node_event_listener::node()),
        openvrml::event_emitter(static_cast<const openvrml::field_value &>(*this)),
        sffloat_listener(obj.openvrml::node_event_listener::node()),
        exposedfield<openvrml::sffloat>(obj)
    {}

    /**
     * @brief Destroy.
     */
    text_node::max_extent_exposedfield::
    ~max_extent_exposedfield() OPENVRML_NOTHROW
    {}

    /**
     * @brief Polymorphically construct a copy.
     *
     * @return a copy of the instance.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    std::auto_ptr<openvrml::field_value>
    text_node::max_extent_exposedfield::do_clone() const
        OPENVRML_THROW1(std::bad_alloc)
    {
        return std::auto_ptr<openvrml::field_value>(
            new max_extent_exposedfield(*this));
    }

    /**
     * @brief Process event.
     *
     * @param max_extent    maximum extent of the text strings.
     * @param timestamp     the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void
    text_node::max_extent_exposedfield::
    event_side_effect(const openvrml::sffloat &, double)
        OPENVRML_THROW1(std::bad_alloc)
    {
        try {
            text_node & node =
                dynamic_cast<text_node &>(this->node_event_listener::node());
            node.update_geometry();
        } catch (std::bad_cast & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
        }
    }

    /**
     * @var text_node::string_exposedfield text_node::string_
     *
     * @brief string exposedField.
     */

    /**
     * @var text_node::font_style_exposedfield text_node::font_style_
     *
     * @brief fontStyle exposedField.
     */

    /**
     * @var text_node::length_exposedfield text_node::length_
     *
     * @brief length exposedField.
     */

    /**
     * @var text_node::max_extent_exposedfield text_node::max_extent_
     *
     * @brief maxExtent exposedField.
     */

    /**
     * @internal
     *
     * @struct text_node::glyph_geometry
     *
     * @brief Used to hold the geometry of individual glyphs.
     */

    /**
     * @var std::vector<openvrml::vec2f> text_node::glyph_geometry::coord_
     *
     * @brief Glyph coordinates.
     */

    /**
     * @var std::vector<openvrml::int32> text_node::glyph_geometry::coord_index_
     *
     * @brief Glyph coordinate indices.
     */

    /**
     * @var float text_node::glyph_geometry::advance_width_
     *
     * @brief The distance the pen should advance horizontally after drawing
     *        the glyph.
     */

    /**
     * @var float text_node::glyph_geometry::advance_height_
     *
     * @brief The distance the pen should advance vertically after drawing the
     *      glyph.
     */

# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE

    /**
     * @internal
     *
     * @brief Determine whether three vertices are ordered counter-clockwise.
     *
     * Does not throw.
     *
     * @param p0 first vertex.
     * @param p1 second vertex.
     * @param p2 third vertex.
     *
     * @return 1 if the vertices are counter-clockwise, -1 if the vertices are
     *         clockwise, or 0 if the vertices are neither.
     */
    OPENVRML_LOCAL int ccw_(const openvrml::vec2f & p0,
                            const openvrml::vec2f & p1,
                            const openvrml::vec2f & p2)
        OPENVRML_NOTHROW
    {
        const float dx1 = p1.x() - p0.x();
        const float dy1 = p1.y() - p0.y();
        const float dx2 = p2.x() - p0.x();
        const float dy2 = p2.y() - p0.y();

        if (dx1 * dy2 > dy1 * dx2) { return 1; }
        if (dx1 * dy2 < dy1 * dx2) { return -1; }
        if ((dx1 * dx2 < 0) || (dy1 * dy2 < 0)) return -1;
        if ((dx1 * dx1 + dy1 * dy1) < (dx2 * dx2 + dy2 * dy2)) { return 1; }
        return 0;
    }

    /**
     * @internal
     *
     * @brief Determine whether two line segments intersect.
     *
     * Does not throw.
     *
     * @param l0p0 first point of the first line.
     * @param l0p1 second point of the first line.
     * @param l1p0 first point of the second line.
     * @param l1p1 second point of the second line.
     *
     * @return @c true if the line segments intersect; @c false otherwise.
     */
    OPENVRML_LOCAL bool intersect_(const openvrml::vec2f & l0p0,
                                   const openvrml::vec2f & l0p1,
                                   const openvrml::vec2f & l1p0,
                                   const openvrml::vec2f & l1p1)
        OPENVRML_NOTHROW
    {
        return ccw_(l0p0, l0p1, l1p0) * ccw_(l0p0, l0p1, l1p1) <= 0
            && ccw_(l1p0, l1p1, l0p0) * ccw_(l1p0, l1p1, l0p1) <= 0;
    }

    /**
     * @brief Determine whether a line segment intersects any line segments
     *        in a contour.
     *
     * Does not throw.
     *
     * @param v0      initial vertex of the line segment.
     * @param v1      final vertex of the line segment.
     * @param contour a contour (a series of line segments).
     *
     * @return @c true if the line segment defined by (@p v0, @p v1)
     *         intersects any line segment in @p contour; @c false otherwise.
     */
    OPENVRML_LOCAL bool
    intersects_segment_in_contour(const openvrml::vec2f & v0,
                                  const openvrml::vec2f & v1,
                                  const std::vector<openvrml::vec2f> & contour)
        OPENVRML_NOTHROW
    {
        for (size_t j = 0; j < contour.size() - 1; ++j) {
            using openvrml::vec2f;
            //
            // Endpoints of the segment to test for intersection.
            //
            const vec2f & contour_v0 = contour[j];
            const vec2f & contour_v1 = contour[j + 1];
            //
            // We don't care if the endpoints match (and the intersection
            // test will treat this as an intersection).
            //
            if (contour_v0 == v0 || contour_v0 == v1
                || contour_v1 == v0 || contour_v1 == v1) { continue; }
            if (intersect_(v0, v1, contour_v0, contour_v1)) { return true; }
        }
        return false;
    }

    /**
     * @internal
     *
     * @brief Get the exterior vertext that should be used to connect to the
     *      interior contour.
     *
     * Finds the first vertex in @p exteriorContour such that a line segment
     * from the interior contour vertex at @p interiorIndex through the
     * exterior contour vertex does not cross @p interiorContour.
     *
     * Does not throw.
     *
     * @param exterior_contour the exterior contour.
     * @param interior_contour the interior contour.
     * @param interior_index   the index of a vertex in @p interiorContour to
     *                         be used as the interior connecting vertex.
     *
     * @return the index of a vertex in @p exteriorContour that is usable as
     *         the exterior connecting vertex, or -1 if no such vertex is
     *         found.
     */
    OPENVRML_LOCAL long get_exterior_connecting_vertex_index_(
        const std::vector<openvrml::vec2f> & exterior_contour,
        const std::vector<const std::vector<openvrml::vec2f> *> &
        interior_contours,
        const openvrml::vec2f & interior_vertex)
        OPENVRML_NOTHROW
    {
        assert(exterior_contour.size() > 1);
        assert(!interior_contours.empty());

        using openvrml::vec2f;

        typedef std::vector<const std::vector<vec2f> *> interior_contours_type;

        for (size_t i = 0; i < exterior_contour.size(); ++i) {
            const vec2f & exterior_vertex = exterior_contour[i];
            bool intersects_interior = false;
            for (interior_contours_type::const_iterator interior_contour =
                     interior_contours.begin();
                 interior_contour != interior_contours.end()
                     && !intersects_interior;
                 ++interior_contour) {
                assert(*interior_contour);
                if (intersects_segment_in_contour(interior_vertex,
                                                  exterior_vertex,
                                                  **interior_contour)) {
                    intersects_interior = true;
                }
            }
            if (!intersects_interior
                && !intersects_segment_in_contour(interior_vertex,
                                                  exterior_vertex,
                                                  exterior_contour)) {
                return long(i);
            }
        }
        return -1;
    }

    OPENVRML_LOCAL bool
    inside_contour_(const std::vector<openvrml::vec2f> & contour,
                    const openvrml::vec2f & point)
        OPENVRML_NOTHROW
    {
        using openvrml::vec2f;

        bool result = false;
        const size_t nvert = contour.size();
        for (size_t i = 0, j = nvert - 1; i < nvert; j = i++) {
            const vec2f & vi = contour[i];
            const vec2f & vj = contour[j];
            if ((((vi.y() <= point.y()) && (point.y() < vj.y()))
                 || ((vj.y() <= point.y()) && (point.y() < vi.y())))
                && (point.x() < (vj.x() - vi.x())
                    * (point.y() - vi.y()) / (vj.y() - vi.y()) + vi.x())) {
                result = !result;
            }
        }
        return result;
    }

    enum contour_type_ { exterior_, interior_ };

    OPENVRML_LOCAL contour_type_
    get_type_(const std::vector<openvrml::vec2f> & contour,
              const std::vector<std::vector<openvrml::vec2f> > & contours)
        OPENVRML_NOTHROW
    {
        using std::vector;

        assert(!contour.empty());
        const openvrml::vec2f & vertex = contour[0];

        bool is_interior = false;
        for (vector<vector<openvrml::vec2f> >::const_iterator test_contour =
                 contours.begin();
             test_contour != contours.end();
             ++test_contour) {
            if (&*test_contour == &contour) { continue; }
            if (inside_contour_(*test_contour, vertex)) {
                is_interior = !is_interior;
            }
        }
        return is_interior
            ? interior_
            : exterior_;
    }

    struct OPENVRML_LOCAL polygon_ {
        const std::vector<openvrml::vec2f> * exterior;
        std::vector<const std::vector<openvrml::vec2f> *> interiors;
    };

    struct OPENVRML_LOCAL inside_ :
        std::binary_function<const std::vector<openvrml::vec2f> *,
                             const std::vector<openvrml::vec2f> *,
                             bool> {
        bool operator()(const std::vector<openvrml::vec2f> * const lhs,
                        const std::vector<openvrml::vec2f> * const rhs) const
            {
                assert(lhs);
                assert(rhs);
                assert(!lhs->empty());
                //
                // Assume contours don't intersect. So if one point on lhs is
                // inside rhs, then assume all of lhs is inside rhs.
                //
                return inside_contour_(*rhs, lhs->front());
            }
    };

    OPENVRML_LOCAL const std::vector<polygon_>
    get_polygons_(const std::vector<std::vector<openvrml::vec2f> > & contours)
        OPENVRML_THROW1(std::bad_alloc)
    {
        using std::vector;
        using openvrml::vec2f;
        typedef std::multiset<const vector<vec2f> *, inside_>
            segregated_contours;

        //
        // First, divide the contours into interior and exterior contours.
        //
        segregated_contours interiors, exteriors;
        for (vector<vector<vec2f> >::const_iterator contour = contours.begin();
             contour != contours.end();
             ++contour) {
            switch (get_type_(*contour, contours)) {
            case interior_:
                interiors.insert(&*contour);
                break;
            case exterior_:
                exteriors.insert(&*contour);
                break;
            default:
                assert(false);
            }
        }

        //
        // For each exterior, find its associated interiors and group them in
        // a Polygon_.
        //
        vector<polygon_> polygons;
        while (!exteriors.empty()) {
            polygon_ polygon;
            polygon.exterior = *exteriors.begin();
            segregated_contours::iterator interior = interiors.begin();
            while (interior != interiors.end()) {
                assert(!(*interior)->empty());
                if (inside_contour_(*polygon.exterior, (*interior)->front())) {
                    polygon.interiors.push_back(*interior);
                    segregated_contours::iterator next = interior;
                    ++next;
                    interiors.erase(interior);
                    interior = next;
                } else {
                    ++interior;
                }
            }
            polygons.push_back(polygon);
            exteriors.erase(exteriors.begin());
        }
        return polygons;
    }

    OPENVRML_LOCAL long
    get_vertex_index_(const std::vector<openvrml::vec2f> & vertices,
                      const openvrml::vec2f & vertex)
        OPENVRML_NOTHROW
    {
        for (size_t i = 0; i < vertices.size(); ++i) {
            if (vertices[i] == vertex) { return long(i); }
        }
        return -1;
    }

    typedef std::multimap<const openvrml::vec2f *,
                          const std::vector<openvrml::vec2f> *>
        connection_map_t;

    //
    // Fill connection_map. For each interior contour, find the exterior
    // vertex that is closest to the first vertex in the interior contour, and
    // the put the pair in the map.
    //
    OPENVRML_LOCAL std::auto_ptr<connection_map_t>
    get_connection_map(const polygon_ & p)
    {
        using std::vector;
        using openvrml::vec2f;
        std::auto_ptr<connection_map_t> connection_map(new connection_map_t);
        for (vector<const vector<vec2f> *>::const_iterator interior =
                 p.interiors.begin();
             interior != p.interiors.end();
             ++interior) {
            assert(*interior);
            assert(!(*interior)->empty());
            long exterior_vertex_index =
                get_exterior_connecting_vertex_index_(*p.exterior,
                                                      p.interiors,
                                                      (*interior)->front());
            assert(exterior_vertex_index > -1);
            const vec2f * const exterior_vertex =
                &(*p.exterior)[exterior_vertex_index];
            assert(exterior_vertex);
            const connection_map_t::value_type value(exterior_vertex,
                                                     *interior);
            connection_map->insert(value);
        }
        return connection_map;
    }

    struct OPENVRML_LOCAL draw_glyph_polygon {
        draw_glyph_polygon(std::vector<openvrml::vec2f> & coord,
                           std::vector<openvrml::int32> & coord_index):
            coord(coord),
            coord_index(coord_index)
        {}

        void operator()(const polygon_ & p) const
        {
            using openvrml::vec2f;

            //
            // connectionMap is keyed on a pointer to a vertex on the exterior
            // contour, and maps to a pointer to the interior contour whose
            // first vertex is closest to the exterior vertex.
            //
            std::auto_ptr<connection_map_t> connection_map(get_connection_map(p));

            assert(!p.exterior->empty());
            for (size_t i = 0; i < p.exterior->size(); ++i) {
                const vec2f & exterior_vertex = (*p.exterior)[i];
                long exterior_index = get_vertex_index_(this->coord, exterior_vertex);
                if (exterior_index > -1) {
                    this->coord_index.push_back(exterior_index);
                } else {
                    this->coord.push_back(exterior_vertex);
                    assert(!this->coord.empty());
                    exterior_index = long(this->coord.size() - 1);
                    coord_index.push_back(exterior_index);
                }
                connection_map_t::iterator pos;
                while ((pos = connection_map->find(&exterior_vertex))
                       != connection_map->end()) {
                    for (int i = int(pos->second->size() - 1); i > -1; --i) {
                        const vec2f & interior_vertex = (*pos->second)[i];
                        const long interior_index =
                            get_vertex_index_(this->coord, interior_vertex);
                        if (interior_index > -1) {
                            this->coord_index.push_back(interior_index);
                        } else {
                            using openvrml::int32;
                            this->coord.push_back(interior_vertex);
                            assert(!this->coord.empty());
                            this->coord_index.push_back(int32(this->coord.size() - 1));
                        }
                    }
                    this->coord_index.push_back(exterior_index);
                    connection_map->erase(pos);
                }
            }
            assert(connection_map->empty());
            this->coord_index.push_back(-1);
        }
    private:
        std::vector<openvrml::vec2f> & coord;
        std::vector<openvrml::int32> & coord_index;
    };


    struct OPENVRML_LOCAL GlyphContours_ {
        const float scale;
        std::vector<std::vector<openvrml::vec2f> > contours;

        explicit GlyphContours_(float scale);
    };

    GlyphContours_::GlyphContours_(const float scale):
        scale(scale)
    {}

    const float stepSize_ = 0.2f;

    extern "C" int
    moveTo_(const FT_Vector * const to,
            void * const user)
    {
        using std::vector;
        using openvrml::vec2f;
        using openvrml::make_vec2f;

        assert(user);
        GlyphContours_ & c = *static_cast<GlyphContours_ *>(user);
        try {
            c.contours.push_back(vector<vec2f>(1));
        } catch (std::bad_alloc & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
            return FT_Err_Out_Of_Memory;
        }
        const vec2f vertex = make_vec2f(to->x * c.scale, to->y * c.scale);
        c.contours.back().front() = vertex;
        return 0;
    }

    extern "C" int
    lineTo_(const FT_Vector * const to,
            void * const user)
    {
        using openvrml::make_vec2f;

        assert(user);
        GlyphContours_ & c = *static_cast<GlyphContours_ *>(user);
        const openvrml::vec2f vertex = make_vec2f(to->x * c.scale,
                                                  to->y * c.scale);
        try {
            c.contours.back().push_back(vertex);
        } catch (std::bad_alloc & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
            return FT_Err_Out_Of_Memory;
        }
        return 0;
    }

    /**
     * @brief de Casteljau's algorithm.
     *
     * This is a nice recursive algorithm defined by de-Casteljau which
     * calculates for a given control polygon the point that lies on the bezier
     * curve for any value of t, and can be used to evaluate and draw the
     * Bezier spline without using the Bernstein polynomials.
     *
     * The algorithm advances by creating in each step a polygons of degree one
     * less than the one created in the previous step until there is only one
     * point left, which is the point on the curve. The polygon vertices for
     * each step are defined by linear interpolation of two consecutive
     * vertices of the polygon from the previous step with a value of t (the
     * parameter):
     *
     * @param buffer    an array including the control points for the curve in
     *                  the first @p npoints elements. The total size of the
     *                  array must be @p npoints * @p npoints. The remaining
     *                  elements of the array will be used by the algorithm to
     *                  store temporary values.
     * @param npoints   the number of control points.
     * @param contour   the points on the curve are added to this array.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    OPENVRML_LOCAL void evaluateCurve_(openvrml::vec2f * const buffer,
                                       const size_t npoints,
                                       std::vector<openvrml::vec2f> & contour)
        OPENVRML_THROW1(std::bad_alloc)
    {
        for (size_t i = 1; i <= (1 / stepSize_); i++){
            const float t = i * stepSize_; // Parametric points 0 <= t <= 1
            for (size_t j = 1; j < npoints; j++) {
                for (size_t k = 0; k < (npoints - j); k++) {
                    openvrml::vec2f & element = buffer[j * npoints + k];
                    element.x((1 - t) * buffer[(j - 1) * npoints + k][0]
                              + t * buffer[(j - 1) * npoints + k + 1][0]);
                    element.y((1 - t) * buffer[(j - 1) * npoints + k][1]
                              + t * buffer[(j - 1) * npoints + k + 1][1]);
                }
            }
            //
            // Specify next vertex to be included on curve
            //
            contour.push_back(buffer[(npoints - 1) * npoints]); // throws std::bad_alloc
        }
    }

    extern "C" int
    conicTo_(const FT_Vector * const control,
             const FT_Vector * const to,
             void * const user)
    {
        using std::vector;
        using openvrml::vec2f;
        using openvrml::make_vec2f;

        assert(control);
        assert(to);
        assert(user);

        GlyphContours_ & c = *static_cast<GlyphContours_ *>(user);

        assert(!c.contours.empty());
        vector<vec2f> & contour = c.contours.back();
        const vec2f & lastVertex = contour[contour.size() - 1];

        assert(!contour.empty());
        const size_t npoints = 3;
        vec2f buffer[npoints * npoints] = {
            make_vec2f(lastVertex[0], lastVertex[1]),
            make_vec2f(control->x * c.scale, control->y * c.scale),
            make_vec2f(to->x * c.scale, to->y * c.scale)
        };

        try {
            evaluateCurve_(buffer, npoints, contour);
        } catch (std::bad_alloc & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
            return FT_Err_Out_Of_Memory;
        }
        return 0;
    }

    extern "C" int
    cubicTo_(const FT_Vector * const control1,
             const FT_Vector * const control2,
             const FT_Vector * const to,
             void * const user)
    {
        using std::vector;
        using openvrml::vec2f;
        using openvrml::make_vec2f;

        assert(control1);
        assert(control2);
        assert(to);
        assert(user);

        GlyphContours_ & c = *static_cast<GlyphContours_ *>(user);

        assert(!c.contours.empty());
        vector<vec2f> & contour = c.contours.back();
        assert(!contour.empty());
        const vec2f & lastVertex = contour.back();

        static const size_t npoints = 4;
        vec2f buffer[npoints * npoints] = {
            make_vec2f(lastVertex[0], lastVertex[1]),
            make_vec2f(control1->x * c.scale, control1->y * c.scale),
            make_vec2f(control2->x * c.scale, control2->y * c.scale),
            make_vec2f(to->x * c.scale, to->y * c.scale)
        };

        try {
            evaluateCurve_(buffer, npoints, contour);
        } catch (std::bad_alloc & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
            return FT_Err_Out_Of_Memory;
        }
        return 0;
    }
# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE

    /**
     * @brief Construct from a set of contours.
     *
     * @param[in,out] face      a FreeType font face.
     * @param[in] glyph_index   the glyph's index (from <a href="http://freetype.sourceforge.net/freetype2/docs/reference/ft2-base_interface.html#FT_Get_Char_Index">@c FT_Get_Char_Index</a>).
     * @param[in] size          the desired size for the glyph.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    text_node::glyph_geometry::
    glyph_geometry(const FT_Face face,
                   const FT_UInt glyph_index,
                   const float size)
        OPENVRML_THROW1(std::bad_alloc):
        advance_width_(0),
        advance_height_(0)
    {
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
        using std::vector;

        FT_Error error = FT_Err_Ok;
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE);
        assert(error == FT_Err_Ok);
        FT_Glyph glyph;
        error = FT_Get_Glyph(face->glyph, &glyph);
        assert(error == FT_Err_Ok);
        BOOST_SCOPE_EXIT((&glyph)) {
            FT_Done_Glyph(glyph);
        } BOOST_SCOPE_EXIT_END
        static FT_Outline_Funcs outlineFuncs = { moveTo_,
                                                 lineTo_,
                                                 conicTo_,
                                                 cubicTo_,
                                                 0,
                                                 0 };
        const float glyphScale = (face->bbox.yMax > 0.0)
                               ? size / face->bbox.yMax
                               : size;
        GlyphContours_ glyphContours(glyphScale);
        assert(glyph->format == FT_GLYPH_FORMAT_OUTLINE);
        const FT_OutlineGlyph outlineGlyph =
            static_cast<FT_OutlineGlyph>(static_cast<void *>(glyph));
        error = FT_Outline_Decompose(&outlineGlyph->outline,
                                     &outlineFuncs,
                                     &glyphContours);
        assert(error == FT_Err_Ok);

        assert(face->glyph);
        this->advance_width_ =
            FT_HAS_HORIZONTAL(face)
            ? face->glyph->metrics.horiAdvance * glyphScale
            : 0.0f;
        this->advance_height_ =
            FT_HAS_VERTICAL(face)
            ? face->glyph->metrics.vertAdvance * glyphScale
            : 0.0f;

        const vector<polygon_> & polygons =
            get_polygons_(glyphContours.contours);
        std::for_each(polygons.begin(), polygons.end(),
                      draw_glyph_polygon(this->coord_, this->coord_index_));
# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE
    }

    /**
     * @brief The glyph coordinates.
     *
     * @return the glyph coordinates.
     */
    const std::vector<openvrml::vec2f> &
    text_node::glyph_geometry::coord() const
    {
        return this->coord_;
    }

    /**
     * @brief The glyph coordinate indices.
     *
     * @return Coordinate indices describing polygons for the glyph.
     */
    const std::vector<openvrml::int32> &
    text_node::glyph_geometry::coord_index() const
    {
        return this->coord_index_;
    }

    /**
     * @brief The horizontal distance the cursor should advance in order to
     *        accommodate this glyph.
     *
     * @return the horizontal distance the cursor should advance in order to
     *         accommodate this glyph.
     */
    float text_node::glyph_geometry::advance_width() const
    {
        return this->advance_width_;
    }

    /**
     * @brief The vertical distance the cursor should advance in order to
     *        accommodate this glyph.
     *
     * @return the vertical distance the cursor should advance in order to
     *         accommodate this glyph.
     */
    float text_node::glyph_geometry::advance_height() const
    {
        return this->advance_height_;
    }


    /**
     * @internal
     *
     * @class text_node::line_geometry
     *
     * @brief Geometry data for a line of text.
     */

    /**
     * @var const bool text_node::line_geometry::horizontal_
     *
     * @brief @c true if text should be rendered horizontally; @c false if
     *        text should be rendered vertically.
     */

    /**
     * @var const bool text_node::line_geometry::left_to_right_
     *
     * @brief @c true if text should be rendered left-to-right; @c false if
     *        text should be rendered right-to-left.
     */

    /**
     * @var const bool text_node::line_geometry::top_to_bottom_
     *
     * @brief @c true if text should flow from top to bottom; @c false if text
     *        should flow from bottom to top.
     */

    /**
     * @var std::vector<openvrml::vec2f> text_node::line_geometry::coord_
     *
     * @brief Coordinate data for the line of text.
     */

    /**
     * @var std::vector<openvrml::int32> text_node::line_geometry::coord_index_
     *
     * @brief Coordinate indices describing polygons.
     */

    /**
     * @var float text_node::line_geometry::x_min_
     *
     * @brief Minimum <var>x</var> coordinate.
     */

    /**
     * @var float text_node::line_geometry::x_max_
     *
     * @brief Maximum <var>x</var> coordinate.
     */

    /**
     * @var float text_node::line_geometry::y_min_
     *
     * @brief Minimum <var>y</var> coordinate.
     */

    /**
     * @var float text_node::line_geometry::y_max_
     *
     * @brief Maximum <var>y</var> coordinate.
     */

    /**
     * @var std::size_t text_node::line_geometry::polygons_
     *
     * @brief The number of polygons in the line.
     */

    /**
     * @var float text_node::line_geometry::pen_x_
     *
     * @brief The &ldquo;pen&rdquo; position <var>x</var> coordinate.
     */

    /**
     * @var float text_node::line_geometry::pen_y_
     *
     * @brief The &ldquo;pen&rdquo; position <var>y</var> coordinate.
     */

    /**
     * @internal
     *
     * @brief Construct.
     *
     * @param[in] horizontal    @c true if text is being rendered horizontally;
     *                          @c false if text is being rendered vertically.
     * @param[in] left_to_right @c true if text is being rendered left-to-right;
     *                          @c false if text is being rendered right-to-
     *                          left.
     * @param[in] top_to_bottom @c true if text is being rendered top-to-bottom;
     *                          @c false if text is being rendered bottom-to-
     *                          top.
     * @param[in] pen_start     starting position for the "pen".
     */
    text_node::line_geometry::line_geometry(const bool horizontal,
                                            const bool left_to_right,
                                            const bool top_to_bottom,
                                            const openvrml::vec2f & pen_start):
        horizontal_(horizontal),
        left_to_right_(left_to_right),
        top_to_bottom_(top_to_bottom),
        x_min_(0), x_max_(0), y_min_(0), y_max_(0),
        polygons_(0),
        pen_pos_(pen_start)
    {}

    const std::vector<openvrml::vec2f> & text_node::line_geometry::coord() const
    {
        return this->coord_;
    }

    const std::vector<openvrml::int32> &
    text_node::line_geometry::coord_index() const
    {
        return this->coord_index_;
    }

    float text_node::line_geometry::x_min() const
    {
        return this->x_min_;
    }

    float text_node::line_geometry::x_max() const
    {
        return this->x_max_;
    }

    float text_node::line_geometry::y_min() const
    {
        return this->y_min_;
    }

    float text_node::line_geometry::y_max() const
    {
        return this->y_max_;
    }

    std::size_t text_node::line_geometry::polygons() const
    {
        return this->polygons_;
    }

    /**
     * @internal
     *
     * @brief Add geometry for a glyph to the line.
     *
     * @param[in] glyph geometry data for a glyph.
     */
    void text_node::line_geometry::add(const glyph_geometry & glyph)
    {
        using openvrml::vec2f;
        using std::min;
        using std::max;

        for (size_t i = 0; i < glyph.coord().size(); ++i) {
            const vec2f textVertex = glyph.coord()[i] + this->pen_pos_;
            this->coord_.push_back(textVertex);
            this->x_min_ = min(this->x_min_, textVertex[0]);
            this->x_max_ = max(this->x_max_, textVertex[0]);
            this->y_min_ = min(this->y_min_, textVertex[1]);
            this->y_max_ = max(this->y_max_, textVertex[1]);
        }

        for (size_t i = 0; i < glyph.coord_index().size(); ++i) {
            const long index = glyph.coord_index()[i];
            if (index > -1) {
                const size_t offset =
                    this->coord_.size() - glyph.coord().size();
                this->coord_index_.push_back(
                    static_cast<openvrml::int32>(offset + index));
            } else {
                this->coord_index_.push_back(-1);
                ++this->polygons_;
            }
        }

        if (this->horizontal_) {
            if (this->left_to_right_) {
                this->pen_pos_.vec[0] += glyph.advance_width();
            } else {
                this->pen_pos_.vec[0] -= glyph.advance_width();
            }
        } else {
            if (this->top_to_bottom_) {
                this->pen_pos_.vec[1] -= glyph.advance_height();
            } else {
                this->pen_pos_.vec[1] += glyph.advance_height();
            }
        }
    }

    void text_node::line_geometry::scale(const float length)
    {
        const float current_length = this->x_max_ - this->x_min_;
        const float scale_factor = current_length * length;
        for (size_t i = 0; i < this->coord_.size(); ++i) {
            this->coord_[i].vec[0] /= scale_factor;
        }
    }


    /**
     * @internal
     *
     * @struct text_node::text_geometry
     *
     * @brief Holds the text geometry.
     */

    /**
     * @var std::vector<openvrml::vec3f> text_node::text_geometry::coord
     *
     * @brief Text geometry coordinates.
     */

    /**
     * @var std::vector<openvrml::int32> text_node::text_geometry::coord_index
     *
     * @brief Text geometry coordinate indices.
     */

    /**
     * @var std::vector<openvrml::vec3f> text_node::text_geometry::normal
     *
     * @brief Text geometry normals.
     */

    /**
     * @var std::vector<openvrml::vec2f> text_node::text_geometry::tex_coord
     *
     * @brief Text geometry texture coordinates.
     */

    /**
     * @brief Construct.
     *
     * @param[in] lines             geometry data for the lines of text.
     * @param[in] major_alignment   one of @c "FIRST", @c "BEGIN", @c "MIDDLE",
     *                              or @c "END".
     * @param[in] minor_alignment   one of @c "FIRST", @c "BEGIN", @c "MIDDLE",
     *                              or @c "END".
     * @param[in] horizontal        @c true if text is being rendered
     *                              horizontally; @c false if text is being
     *                              rendered vertically.
     * @param[in] size              the size of the text.
     * @param[in] spacing           the line spacing.
     * @param[in] max_extent        the maximum allowed extent for the text.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    text_node::text_geometry::
    text_geometry(const boost::ptr_vector<line_geometry> & lines,
                  const std::string & major_alignment,
                  const std::string & minor_alignment,
                  const bool horizontal,
                  const float size,
                  const float spacing,
                  const float max_extent)
        OPENVRML_THROW1(std::bad_alloc)
    {
        std::size_t polygons = 0;
        for (boost::ptr_vector<line_geometry>::const_iterator line =
                 lines.begin();
             line != lines.end();
             ++line) {
            this->add(*line, major_alignment, horizontal);
            polygons += line->polygons();
        }
        if (max_extent > 0) { this->scale(max_extent); }
        this->minor_align(minor_alignment,
                          horizontal,
                          size,
                          spacing,
                          lines.size());
        this->generate_normals(polygons);
        this->generate_tex_coords(size);
    }

    /**
     * @brief Coordinates for the text geometry.
     *
     * @return coordinates for the text geometry.
     */
    const std::vector<openvrml::vec3f> & text_node::text_geometry::coord() const
        OPENVRML_NOTHROW
    {
        return this->coord_;
    }

    /**
     * @brief Coordinate indices for the text geometry.
     *
     * @return coordinate indices for the text geometry.
     */
    const std::vector<openvrml::int32> &
    text_node::text_geometry::coord_index() const OPENVRML_NOTHROW
    {
        return this->coord_index_;
    }

    /**
     * @brief Normals for the text geometry.
     *
     * @return normals for the text geometry.
     */
    const std::vector<openvrml::vec3f> &
    text_node::text_geometry::normal() const OPENVRML_NOTHROW
    {
        return this->normal_;
    }

    /**
     * @brief Texture coordinates for the text geometry.
     *
     * @return texture coordinates for the text geometry.
     */
    const std::vector<openvrml::vec2f> &
    text_node::text_geometry::tex_coord() const OPENVRML_NOTHROW
    {
        return this->tex_coord_;
    }

    /**
     * @brief Add a line of text.
     *
     * @param[in] line              geometry for a line of text.
     * @param[in] major_alignment   one of @c "FIRST", @c "BEGIN", @c "MIDDLE",
     *                              or @c "END".
     * @param[in] horizontal        @c true if text is being rendered
     *                              horizontally; @c false if text is being
     *                              rendered vertically.
     * @param[in] size              the size of the text.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void text_node::text_geometry::add(const line_geometry & line,
                                       const std::string & major_alignment,
                                       const bool horizontal)
        OPENVRML_THROW1(std::bad_alloc)
    {
        using std::min;
        using std::max;
        using openvrml::int32;
        using openvrml::vec2f;
        using openvrml::vec3f;
        using openvrml::make_vec3f;

        //
        // Add the line to the text geometry.  We need to adjust for the major
        // alignment.
        //
        float x_offset = 0.0f, y_offset = 0.0f;
        //
        // Offset is 0 for "BEGIN" or "FIRST" (or anything else, in our case).
        //
        if (major_alignment == "MIDDLE") {
            if (horizontal) {
                x_offset = -((line.x_max() - line.x_min()) / 2.0f);
            } else {
                y_offset = (line.y_max() - line.y_min()) / 2.0f;
            }
        } else if (major_alignment == "END") {
            if (horizontal) {
                x_offset = -(line.x_max() - line.x_min());
            } else {
                y_offset = line.y_max() - line.y_min();
            }
        }
        for (size_t i = 0; i < line.coord_index().size(); ++i) {
            const long index = line.coord_index()[i];
            if (index > -1) {
                const vec2f & line_vertex = line.coord()[index];
                const vec3f text_vertex = make_vec3f(line_vertex.x() + x_offset,
                                                     line_vertex.y() + y_offset,
                                                     0.0f);
                this->coord_.push_back(text_vertex);
                this->coord_index_.push_back(
                    static_cast<int32>(this->coord_.size() - 1));
                this->x_min_ = min(this->x_min_, text_vertex.x());
                this->x_max_ = max(this->x_max_, text_vertex.x());
                this->y_min_ = min(this->y_min_, text_vertex.y());
                this->y_max_ = max(this->y_max_, text_vertex.y());
            } else {
                this->coord_index_.push_back(-1);
            }
        }
    }

    /**
     * @brief Scale the text to the maximum allowed extent.
     *
     * @param[in] max_extent    the maximum allowed extent.
     */
    void text_node::text_geometry::scale(const float max_extent)
        OPENVRML_NOTHROW
    {
        assert(max_extent > 0);
        const float current_max_extent = this->x_max_ - this->x_min_;
        if (current_max_extent > max_extent) {
            for (size_t i = 0; i < this->coord_.size(); ++i) {
                this->coord_[i].vec[0] /= current_max_extent * max_extent;
            }
        }
    }

    /**
     * @brief Apply the minor alignment to the text.
     *
     * @param[in] align         one of @c "FIRST", @c "BEGIN", @c "MIDDLE", or
     *                          @c "END".
     * @param[in] horizontal    @c true if text is being rendered horizontally;
     *                          @c false if text is being rendered vertically.
     * @param[in] size          the size of the text.
     * @param[in] spacing       the line spacing.
     * @param[in] lines         the number of lines of text.
     */
    void text_node::text_geometry::minor_align(const std::string & align,
                                               const bool horizontal,
                                               const float size,
                                               const float spacing,
                                               const std::size_t lines)
        OPENVRML_NOTHROW
    {
        using openvrml::vec3f;
        using openvrml::make_vec3f;

        float x_offset = 0.0f, y_offset = 0.0f;
        if (align == "FIRST" || align == "") {
        } else if (align == "BEGIN") {
            if (horizontal) {
                y_offset = -(size * spacing);
            } else {
                x_offset = 0.0f;
            }
        } else if (align == "MIDDLE") {
            if (horizontal) {
                y_offset = ((size * spacing * lines) / 2.0f) - (size * spacing);
            } else {
                x_offset = ((size * spacing * lines) / 2.0f) - (size * spacing);
            }
        } else if (align == "END") {
            if (horizontal) {
                y_offset = size * spacing * (lines - 1);
            } else {
                x_offset = size * spacing * (lines - 1);
            }
        }
        for (size_t i = 0; i < this->coord_.size(); ++i) {
            const vec3f & vertex = this->coord_[i];
            this->coord_[i] = make_vec3f(vertex.x() + x_offset,
                                         vertex.y() + y_offset,
                                         vertex.z());
        }
    }

    /**
     * @brief Generate normals for the text geometry.
     *
     * @param[in] polygons  the number of polygons in the text geometry.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void text_node::text_geometry::generate_normals(const std::size_t polygons)
        OPENVRML_THROW1(std::bad_alloc)
    {
        this->normal_.resize(polygons);
        std::fill(this->normal_.begin(), this->normal_.end(),
                  openvrml::make_vec3f(0.0, 0.0, 1.0));
    }

    /**
     * @brief Generate texture coordinates for the text geometry.
     *
     * @param[in] size  the size of the text.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void text_node::text_geometry::generate_tex_coords(const float size)
        OPENVRML_THROW1(std::bad_alloc)
    {
        using openvrml::vec3f;
        using openvrml::make_vec2f;
        this->tex_coord_.resize(this->coord_.size());
        for (std::size_t i = 0; i < this->tex_coord_.size(); ++i) {
            const vec3f & vertex = this->coord_[i];
            this->tex_coord_[i] = make_vec2f(vertex.x() / size,
                                             vertex.y() / size);
        }
    }

    /**
     * @typedef text_node::ucs4_string_t
     *
     * @brief A vector of FcChar32 vectors.
     */

    /**
     * @typedef text_node::glyph_geometry_map_t
     *
     * @brief Maps FT_UInts to glyph_geometry.
     *
     * @see http://freetype.org/freetype2/docs/reference/ft2-basic_types.html#FT_UInt
     */

    /**
     * @var text_node::ucs4_string_t text_node::ucs4_string
     *
     * @brief UCS-4 equivalent of the (UTF-8) data in @a string.
     */

    /**
     * @var FT_Face text_node::face
     *
     * @brief Handle to the font face.
     *
     * @see http://freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_Face
     */

    /**
     * @var text_node::glyph_geometry_map_t text_node::glyph_geometry_map
     *
     * @brief Map of glyph indices to glyph_geometry.
     *
     * glyph_geometry instances are created as needed as new glyphs are
     * encountered. Once they are created, they are cached in the
     * glyph_geometry_map for rapid retrieval the next time the glyph is
     * encountered.
     */

    /**
     * @var text_node::text_geometry text_node::text_geometry_
     *
     * @brief The text geometry.
     */

    /**
     * @brief Construct.
     *
     * @param type  the node_type associated with the instance.
     * @param scope the scope that the new node will belong to.
     */
    text_node::
    text_node(const openvrml::node_type & type,
              const boost::shared_ptr<openvrml::scope> & scope):
        node(type, scope),
        bounded_volume_node(type, scope),
        openvrml::node_impl_util::abstract_node<text_node>(type, scope),
        geometry_node(type, scope),
        string_(*this),
        font_style_(*this),
        length_(*this),
        max_extent_(*this),
        solid_(true)
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
        ,face(0)
# endif
    {}

    /**
     * @brief Destroy.
     */
    text_node::~text_node() OPENVRML_NOTHROW
    {
        // shutdown sets this->face to 0.
        assert(this->face == 0);
    }

    /**
     * @brief Determine whether the node has been modified.
     *
     * @return @c true if the node or one of its children has been modified,
     *      @c false otherwise.
     */
    bool text_node::do_modified() const
        OPENVRML_THROW1(boost::thread_resource_error)
    {
        return this->font_style_.value()
            && this->font_style_.value()->modified();
    }

    /**
     * @brief Insert this geometry into @p v's display list.
     *
     * @param v         a viewer.
     * @param context   the rendering context.
     */
    void text_node::do_render_geometry(openvrml::viewer & v,
                                       openvrml::rendering_context)
    {
        using openvrml::int32;
        if (this->text_geometry_) {
            v.insert_shell(*this,
                           openvrml::viewer::mask_ccw,
                           this->text_geometry_->coord(),
                           this->text_geometry_->coord_index(),
                           std::vector<openvrml::color>(), // color
                           std::vector<int32>(), // colorIndex
                           this->text_geometry_->normal(),
                           std::vector<int32>(), // normalIndex
                           this->text_geometry_->tex_coord(),
                           std::vector<int32>()); // texCoordIndex
        }
        if (this->font_style_.value()) {
            this->font_style_.value()->modified(false);
        }
    }

    /**
     * @brief Initialize.
     *
     * @param timestamp the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void text_node::do_initialize(double) OPENVRML_THROW1(std::bad_alloc)
    {
        this->update_ucs4();
        this->update_face();
        this->update_geometry();
    }

    /**
     * @brief Shut down.
     *
     * @param timestamp the current time.
     */
    void text_node::do_shutdown(double) OPENVRML_NOTHROW
    {
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
        if (this->face) {
            FT_Error ftError = FT_Done_Face(this->face);
            assert(ftError == FT_Err_Ok); // Surely this can't fail.
            this->face = 0;
        }
# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE
    }

# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE

    OPENVRML_LOCAL ptrdiff_t utf8_to_ucs4_(const unsigned char * src_orig,
                                           char32_t & dst,
                                           size_t len)
    {
        const unsigned char *src = src_orig;

        if (len == 0) { return 0; }

        unsigned char s = *src++;
        len--;

        char32_t result;
        size_t extra;
        if (!(s & 0x80)) {
            result = s;
            extra = 0;
        } else if (!(s & 0x40)) {
            return -1;
        } else if (!(s & 0x20)) {
            result = s & 0x1f;
            extra = 1;
        } else if (!(s & 0x10)) {
            result = s & 0xf;
            extra = 2;
        } else if (!(s & 0x08)) {
            result = s & 0x07;
            extra = 3;
        } else if (!(s & 0x04)) {
            result = s & 0x03;
            extra = 4;
        } else if ( ! (s & 0x02)) {
            result = s & 0x01;
            extra = 5;
        } else {
            return -1;
        }
        if (extra > len) { return -1; }

        while (extra--) {
            result <<= 6;
            s = *src++;

            if ((s & 0xc0) != 0x80) { return -1; }

            result |= s & 0x3f;
        }
        dst = result;
        return src - src_orig;
    }

    OPENVRML_LOCAL bool utf8_len_(const unsigned char * utf8_str,
                                  size_t len,
                                  size_t & chars,
                                  size_t & max_char_width)
    {

        int n = 0;
        char32_t max = 0;
        while (len) {
            char32_t c;
            const ptrdiff_t clen = utf8_to_ucs4_(utf8_str, c, len);
            if (clen <= 0) {
                // Malformed UTF8 string.
                return false;
            }
            if (c > max) {
                max = c;
            }
            utf8_str += clen;
            len -= clen;
            n++;
        }
        chars = n;
        if (max >= 0x10000) {
            max_char_width = 4;
        } else if (max > 0x100) {
            max_char_width = 2;
        } else {
            max_char_width = 1;
        }
        return true;
    }
# endif // ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE

    /**
     * @brief Called when @a string changes to update the UCS-4 text.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void text_node::update_ucs4() OPENVRML_THROW1(std::bad_alloc)
    {
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
        this->ucs4_string.clear();
        this->ucs4_string.resize(this->string_.mfstring::value().size());

        for (size_t i = 0; i < this->string_.mfstring::value().size(); ++i) {
            using std::string;
            using std::vector;

            const string & element = this->string_.mfstring::value()[i];

            vector<char32_t> & ucs4Element = this->ucs4_string[i];

            //
            // First, we need to convert the characters from UTF-8 to UCS-4.
            //
            vector<unsigned char> utf8String(element.begin(), element.end());
            size_t chars = 0, max_char_width = 0;
            const bool well_formed = utf8_len_(&utf8String[0],
                                               utf8String.size(),
                                               chars,
                                               max_char_width);
            if (well_formed) {
                ucs4Element.resize(chars);
                vector<unsigned char>::iterator utf8interface =
                    utf8String.begin();
                vector<char32_t>::iterator ucs4interface = ucs4Element.begin();
                while (utf8interface != utf8String.end()) {
                    const ptrdiff_t utf8bytes =
                        utf8_to_ucs4_(&*utf8interface,
                                      *ucs4interface,
                                      utf8String.end() - utf8interface);
                    utf8interface += utf8bytes;
                    ucs4interface++;
                }
            }
        }
# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE
    }

# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE

    //
    // unsigned_char_traits is a model of the standard library Character Traits
    // concept.
    //
    struct OPENVRML_LOCAL unsigned_char_traits {
        typedef unsigned char char_type;
        typedef int int_type;
        typedef std::streampos pos_type;
        typedef std::streamoff off_type;
        typedef mbstate_t state_type;

        static void assign(char_type & c1, const char_type & c2);
        static bool eq(const char_type & c1, const char_type & c2);
        static bool lt(const char_type & c1, const char_type & c2);
        static int compare(const char_type * s1, const char_type * s2,
                           size_t n);
        static size_t length(const char_type * s);
        static const char_type * find(const char_type * s, size_t n,
                                      const char_type & a);
        static char_type * move(char_type * s1, const char_type * s2,
                                size_t n);
        static char_type * copy(char_type * s1, const char_type * s2,
                                size_t n);
        static char_type * assign(char_type * s, size_t n, char_type a);
        static int_type eof();
        static int_type not_eof(const int_type & c);
        static char_type to_char_type(const int_type & e);
        static int_type to_int_type(const char_type & c);
        static bool eq_int_type(const int_type & e1, const int_type & e2);
    };

    inline void unsigned_char_traits::assign(char_type & c1,
                                             const char_type & c2)
    {
        c1 = c2;
    }

    inline bool unsigned_char_traits::eq(const char_type & c1,
                                         const char_type & c2)
    {
        return c1 == c2;
    }

    inline bool unsigned_char_traits::lt(const char_type & c1,
                                         const char_type & c2)
    {
        return c1 < c2;
    }

    inline int unsigned_char_traits::compare(const char_type * s1,
                                             const char_type * s2,
                                             size_t n)
    {
        for (size_t i = 0; i < n; ++i) {
            if (!eq(s1[i], s2[i])) { return lt(s1[i], s2[i]) ? -1 : 1; }
        }
        return 0;
    }

    inline size_t unsigned_char_traits::length(const char_type * s)
    {
        const char_type * p = s;
        while (*p) { ++p; }
        return (p - s);
    }

    inline unsigned_char_traits::char_type *
    unsigned_char_traits::move(char_type * s1, const char_type * s2, size_t n)
    {
        return reinterpret_cast<char_type *>(
            memmove(s1, s2, n * sizeof(char_type)));
    }

    inline const unsigned_char_traits::char_type *
    unsigned_char_traits::find(const char_type * s,
                               size_t n,
                               const char_type & a)
    {
        for (const char_type * p = s; size_t(p - s) < n; ++p) {
            if (*p == a) { return p; }
        }
        return 0;
    }

    inline unsigned_char_traits::char_type *
    unsigned_char_traits::copy(char_type * s1, const char_type * s2, size_t n)
    {
        return reinterpret_cast<char_type *>(
            memcpy(s1, s2, n * sizeof(char_type)));
    }

    inline unsigned_char_traits::char_type *
    unsigned_char_traits::assign(char_type * s, size_t n, char_type a)
    {
        for (char_type * p = s; p < s + n; ++p) { assign(*p, a); }
        return s;
    }

    inline unsigned_char_traits::int_type unsigned_char_traits::eof()
    {
        return static_cast<int_type>(-1);
    }

    inline unsigned_char_traits::int_type
    unsigned_char_traits::not_eof(const int_type & c)
    {
        return eq_int_type(c, eof()) ? int_type(0) : c;
    }

    inline unsigned_char_traits::char_type
    unsigned_char_traits::to_char_type(const int_type & e)
    {
        return char_type(e);
    }

    inline unsigned_char_traits::int_type
    unsigned_char_traits::to_int_type(const char_type & c)
    {
        return int_type(c);
    }

    inline bool unsigned_char_traits::eq_int_type(const int_type & e1,
                                            const int_type & e2)
    {
        return e1 == e2;
    }

#   ifndef _WIN32
    const char * const fcResultMessage[] = { "match",
                                             "no match",
                                             "type mismatch",
                                             "no id" };

    //
    // FontconfigError and FreeTypeError are never thrown out of the library.
    //

    class OPENVRML_LOCAL FontconfigError : public std::runtime_error {
    public:
        explicit FontconfigError(const FcResult result):
            std::runtime_error(fcResultMessage[result])
        {}

        virtual ~FontconfigError() OPENVRML_NOTHROW
        {}
    };
#   endif

    class OPENVRML_LOCAL FreeTypeError : public std::runtime_error {
    public:
        //
        // The normal build of FreeType doesn't include a means of mapping
        // error codes to human-readable strings.  There's a means of
        // letting client apps do this by defining some macros, but that's
        // too much trouble for now.
        //
        explicit FreeTypeError(FT_Error):
            std::runtime_error("FreeType error")
        {}

        virtual ~FreeTypeError() OPENVRML_NOTHROW
        {}
    };

    typedef std::basic_string<unsigned char, unsigned_char_traits>
        unsigned_char_string;

    OPENVRML_LOCAL void
    get_font_filename(const std::vector<std::string> & family,
                      const std::string & style,
                      const unsigned_char_string & language,
                      std::vector<char> & filename,
                      FT_Long & face_index)
    {
        using std::vector;
# ifdef _WIN32
        LOGFONT lf;
        lf.lfHeight =         0;
        lf.lfWidth =          0;
        lf.lfEscapement =     0;
        lf.lfOrientation =    0;
        lf.lfWeight =         FW_MEDIUM;
        lf.lfItalic =         FALSE;
        lf.lfUnderline =      FALSE;
        lf.lfStrikeOut =      FALSE;
        lf.lfCharSet =        DEFAULT_CHARSET;
        lf.lfOutPrecision =   OUT_TT_ONLY_PRECIS;
        lf.lfClipPrecision =  CLIP_DEFAULT_PRECIS;
        lf.lfQuality =        DEFAULT_QUALITY;
        lf.lfPitchAndFamily = VARIABLE_PITCH | FF_ROMAN;

        HDC hdc = CreateCompatibleDC(0);
        BOOST_SCOPE_EXIT((hdc)) {
            DeleteDC(hdc);
        } BOOST_SCOPE_EXIT_END
        HFONT hfont = CreateFontIndirect(&lf);
        SelectObject(hdc, hfont);
        TCHAR faceName[256] = {};
        GetTextFace(hdc, sizeof faceName / sizeof (TCHAR), faceName);
        const int faceNameLen = lstrlen(faceName);

        //
        // Get the fonts folder.
        //
        TCHAR fontsPath[MAX_PATH];
        HRESULT status =
            SHGetFolderPath(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT,
                            fontsPath);
        if (FAILED(status)) { /* bail */ }

        //
        // Enumerate the fonts in the registry and pick one that matches.
        //
        HKEY fontsKey;
        LONG result =
            RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
                0,
                KEY_READ,
                &fontsKey);
        if (result != ERROR_SUCCESS) { /* bail */ }
        BOOST_SCOPE_EXIT((fontsKey)) {
            RegCloseKey(fontsKey);
        } BOOST_SCOPE_EXIT_END

        DWORD maxValueNameLen, maxValueLen;
        result = RegQueryInfoKey(fontsKey,
                                 NULL,  // lpClass
                                 NULL,  // lpcClass
                                 NULL,  // lpReserved
                                 NULL,  // lpcSubKeys
                                 NULL,  // lpcMaxSubKeyLen
                                 NULL,  // lpcMaxClassLen
                                 NULL,  // lpcValues
                                 &maxValueNameLen,
                                 &maxValueLen,
                                 NULL,  // lpcbSecurityDescriptor
                                 NULL); // lpftLastWriteTime

        DWORD index = 0;
        vector<TCHAR> valueName(maxValueNameLen + 1);
        DWORD type;
        vector<BYTE> data(maxValueLen);
        TCHAR fontPath[MAX_PATH] = {};
        result = ERROR_SUCCESS;
        while (result != ERROR_NO_MORE_ITEMS) {
            DWORD dataLength = DWORD(data.size());
            DWORD valueNameLength = DWORD(valueName.size());
            result = RegEnumValue(fontsKey,
                                  index,
                                  &valueName.front(),
                                  &valueNameLength,
                                  NULL,
                                  &type,
                                  &data.front(),
                                  &dataLength);
            if (result == ERROR_MORE_DATA) {
                data.resize(dataLength);
                continue;
            }
            if (result == ERROR_SUCCESS) {
                if (DWORD(faceNameLen + 1) <= valueNameLength
                    && std::equal(faceName, faceName + faceNameLen,
                                  valueName.begin())) {
                    HRESULT strcat_result = StringCchCat(fontPath,
                                                         MAX_PATH,
                                                         fontsPath);
                    assert(SUCCEEDED(strcat_result));
                    strcat_result = StringCchCat(fontPath, MAX_PATH, "\\");
                    assert(SUCCEEDED(strcat_result));
                    strcat_result =
                        StringCchCat(fontPath,
                                     MAX_PATH,
                                     reinterpret_cast<STRSAFE_LPCSTR>(
                                         &data.front()));
                    assert(SUCCEEDED(strcat_result));
                    break;
                }
                ++index;
            }
        }

        const size_t fontPathLen = lstrlen(fontPath);
        assert(fontPathLen != 0);
        filename.assign(fontPath, fontPath + fontPathLen + 1);
        face_index = 0;
# else
        using std::string;

        string fontName;
        //
        // Set the family.
        //
        for (size_t i = 0; i < family.size(); ++i) {
            const string & element = family[i];
            if (element == "SERIF") {
                fontName += "serif";
            } else if (element == "SANS") {
                fontName += "sans";
            } else if (element == "TYPEWRITER") {
                fontName += "monospace";
            } else {
                fontName += element;
            }
            if (i + 1 < family.size()) { fontName += ", "; }
        }

        //
        // Set the weight.
        //
        if (style.find("BOLD") != string::npos) {
            fontName += ":bold";
        }

        //
        // Set the slant.
        //
        if (style.find("ITALIC") != string::npos) {
            fontName += ":italic";
        }

        //
        // For now, at least, we only want outline fonts.
        //
        fontName += ":outline=True";

        FcPattern * const initialPattern =
            FcNameParse(unsigned_char_string(fontName.begin(),
                                             fontName.end()).c_str());
        if (!initialPattern) { throw std::bad_alloc(); }
        BOOST_SCOPE_EXIT((initialPattern)) {
            FcPatternDestroy(initialPattern);
        } BOOST_SCOPE_EXIT_END

        //
        // Set the language.
        //
        if (!language.empty()) {
            FcPatternAddString(initialPattern,
                               FC_LANG,
                               language.c_str());
        }

        FcConfigSubstitute(0, initialPattern, FcMatchPattern);
        FcDefaultSubstitute(initialPattern);

        FcResult result = FcResultMatch;
        FcPattern * const matchedPattern =
            FcFontMatch(0, initialPattern, &result);
        if (result != FcResultMatch) { throw FontconfigError(result); }
        assert(matchedPattern);
        BOOST_SCOPE_EXIT((matchedPattern)) {
            FcPatternDestroy(matchedPattern);
        } BOOST_SCOPE_EXIT_END

        FcChar8 * filename_c_str = 0;
        result = FcPatternGetString(matchedPattern,
                                    FC_FILE,
                                    0,
                                    &filename_c_str);
        if (result != FcResultMatch) { throw FontconfigError(result); }

        size_t filenameLen = 0;
        for (; filename_c_str[filenameLen]; ++filenameLen) {}

        filename.assign(filename_c_str, filename_c_str + filenameLen + 1);

        int face_index_int = 0;
        result = FcPatternGetInteger(matchedPattern, FC_INDEX, 0,
                                     &face_index_int);
        if (result != FcResultMatch) { throw FontconfigError(result); }
        face_index = FT_Long(face_index_int);
# endif
    }

# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE

    /**
     * @brief Called when @a fontStyle changes to update the font face.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void text_node::update_face() OPENVRML_THROW1(std::bad_alloc)
    {
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
        using std::string;
        using openvrml::node_cast;
        using openvrml_node_vrml97::text_metatype;

        unsigned_char_string language;

        std::vector<string> family;
        family.push_back("SERIF");

        string style;

        openvrml::font_style_node * const fontStyle =
            node_cast<openvrml::font_style_node *>(
                this->font_style_.sfnode::value().get());
        if (fontStyle) {
            if (!fontStyle->family().empty()) {
                family = fontStyle->family();
                style = fontStyle->style();
                language.assign(fontStyle->language().begin(),
                                fontStyle->language().end());
            }
        }

        try {
            std::vector<char> ftFilename;
            FT_Long face_index;
            get_font_filename(family, style, language, ftFilename, face_index);

            text_metatype & nodeClass =
                const_cast<text_metatype &>(
                    static_cast<const text_metatype &>(
                        this->type().metatype()));

            FT_Face newFace = 0;
            FT_Error ftError = FT_Err_Ok;
            ftError = FT_New_Face(nodeClass.freeTypeLibrary,
                                  &ftFilename[0], FT_Long(face_index),
                                  &newFace);
            if (ftError) { throw FreeTypeError(ftError); }

            if (this->face) {
                ftError = FT_Done_Face(this->face);
                assert(ftError == FT_Err_Ok); // Surely this can't fail.
            }

            this->face = newFace;
            this->glyph_geometry_map.clear();
        } catch (std::runtime_error & ex) {
            OPENVRML_PRINT_EXCEPTION_(ex);
        }
# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE
    }

    const openvrml::vec2f get_pen_start_for_line(const std::size_t line_num,
                                                 const float size,
                                                 const float spacing,
                                                 const bool horizontal,
                                                 const bool left_to_right,
                                                 const bool top_to_bottom)
    {
        const float line_advance = size * spacing * line_num;

        openvrml::vec2f pen_pos = openvrml::make_vec2f();
        if (horizontal) {
            pen_pos.y(top_to_bottom ? -line_advance : line_advance);
        } else {
            pen_pos.x(left_to_right ? line_advance : -line_advance);
        }
        return pen_pos;
    }

    /**
     * @brief Called to update @a text_geometry.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void text_node::update_geometry() OPENVRML_THROW1(std::bad_alloc)
    {
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
        using std::auto_ptr;
        using std::max;
        using std::pair;
        using std::string;
        using std::vector;
        using boost::ptr_vector;
        using openvrml::node_cast;
        using openvrml::vec2f;
        using openvrml::make_vec2f;
        using openvrml::vec3f;
        using openvrml::make_vec3f;

        bool horizontal = true;
        string justify[2] = { "BEGIN", "FIRST" };
        bool leftToRight = true;
        bool topToBottom = true;
        float size = 1.0;
        float spacing = 1.0;
        openvrml::font_style_node * fontStyle =
            node_cast<openvrml::font_style_node *>(
                this->font_style_.value().get());
        if (fontStyle) {
            horizontal = fontStyle->horizontal();
            if (!fontStyle->justify().empty()) {
                justify[0] = fontStyle->justify()[0];
            }
            if (fontStyle->justify().size() > 1) {
                justify[1] = fontStyle->justify()[1];
            }
            leftToRight = fontStyle->left_to_right();
            topToBottom = fontStyle->top_to_bottom();
            size = fontStyle->size();
            spacing = fontStyle->spacing();
        }

        ptr_vector<line_geometry> lines(this->ucs4_string.size());
        const ucs4_string_t::const_iterator stringBegin =
            this->ucs4_string.begin();
        for (ucs4_string_t::const_iterator string = stringBegin;
             string != this->ucs4_string.end();
             ++string) {
            const size_t line = std::distance(stringBegin, string);
            const vec2f pen_start = get_pen_start_for_line(line,
                                                           size,
                                                           spacing,
                                                           horizontal,
                                                           leftToRight,
                                                           topToBottom);

            using openvrml::int32;

            auto_ptr<line_geometry> line_geom(new line_geometry(horizontal,
                                                                leftToRight,
                                                                topToBottom,
                                                                pen_start));
            for (vector<char32_t>::const_iterator character = string->begin();
                 character != string->end(); ++character) {
                assert(this->face);
                const FT_UInt glyphIndex =
#   ifdef _WIN32
                    FT_Get_Char_Index(this->face, *character);
#   else
                    FcFreeTypeCharIndex(this->face, *character);
#   endif

                const glyph_geometry * glyphGeometry = 0;
                const glyph_geometry_map_t::iterator pos =
                    this->glyph_geometry_map.find(glyphIndex);
                if (pos != this->glyph_geometry_map.end()) {
                    glyphGeometry = &pos->second;
                } else {
                    const glyph_geometry_map_t::value_type
                        value(glyphIndex,
                              glyph_geometry(this->face, glyphIndex, size));
                    const pair<glyph_geometry_map_t::iterator, bool> result =
                        this->glyph_geometry_map.insert(value);
                    assert(result.second);
                    glyphGeometry = &result.first->second;
                }
                assert(glyphGeometry);
                line_geom->add(*glyphGeometry);
            }

            //
            // Scale to length.
            //
            const float length = (line < this->length_.value().size())
                               ? this->length_.value()[line]
                               : 0.0f;
            if (length > 0.0f) { line_geom->scale(length); }

            lines.push_back(line_geom);
        }

        const float max_extent = max(this->max_extent_.value(), 0.0f);
        this->text_geometry_.reset(new text_geometry(lines,
                                                     justify[0],
                                                     justify[1],
                                                     horizontal,
                                                     size,
                                                     spacing,
                                                     max_extent));
# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE
    }
}


/**
 * @brief @c node_metatype identifier.
 */
const char * const openvrml_node_vrml97::text_metatype::id =
    "urn:X-openvrml:node:Text";

/**
 * @var FT_Library text_metatype::freeTypeLibrary
 *
 * @brief FreeType library handle.
 *
 * @see http://freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_Library
 */

/**
 * @brief Construct.
 *
 * @param browser the @c browser associated with this @c node_metatype.
 */
openvrml_node_vrml97::text_metatype::
text_metatype(openvrml::browser & browser):
    node_metatype(text_metatype::id, browser)
{
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
    FT_Error error = 0;
    error = FT_Init_FreeType(&this->freeTypeLibrary);
    if (error) {
        browser.err("error initializing FreeType library");
    }
# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE
}

/**
 * @brief Destroy.
 */
openvrml_node_vrml97::text_metatype::~text_metatype() OPENVRML_NOTHROW
{
# ifdef OPENVRML_ENABLE_RENDER_TEXT_NODE
    FT_Error error = 0;
    error = FT_Done_FreeType(this->freeTypeLibrary);
    if (error) {
        this->browser().err("error shutting down FreeType library");
    }
# endif // OPENVRML_ENABLE_RENDER_TEXT_NODE
}

# define TEXT_INTERFACE_SEQ                              \
    ((exposedfield, mfstring, "string", string_))        \
    ((exposedfield, sfnode,   "fontStyle", font_style_)) \
    ((exposedfield, mffloat,  "length",    length_))     \
    ((exposedfield, sffloat,  "maxExtent", max_extent_)) \
    ((exposedfield, sfnode,   "metadata",  metadata))    \
    ((field,        sfbool,   "solid",     solid_))

OPENVRML_NODE_IMPL_UTIL_DEFINE_DO_CREATE_TYPE(openvrml_node_vrml97,
                                              text_metatype,
                                              text_node,
                                              TEXT_INTERFACE_SEQ)
