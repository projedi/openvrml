// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 79 -*-
//
// OpenVRML
//
// Copyright 1998  Chris Morley
// Copyright 2001, 2002, 2003, 2004, 2005  Braden McDaniel
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

# ifdef HAVE_CONFIG_H
#   include <config.h>
# endif

# include <cstdlib>
# include <cctype>
# include <cstring>
# include <algorithm>
# include <fstream>
# include <functional>
# include <sstream>
# include <stack>
# ifdef _WIN32
#   include <sys/timeb.h>
#   include <time.h>
# else
#   include <sys/time.h>
# endif
# include <boost/bind.hpp>
# include <boost/cast.hpp>
# include <boost/shared_ptr.hpp>
# include <boost/spirit.hpp>
# include <boost/spirit/phoenix.hpp>
# include <boost/thread/thread.hpp>
# include <boost/utility.hpp>
# ifdef OPENVRML_ENABLE_GZIP
#   include <zlib.h>
# endif
# include <private.h>
# include "browser.h"
# include "viewer.h"
# include "scope.h"
# include "script.h"
# include "system.h"
# include "vrml97node.h"

namespace openvrml {

    class vrml97_root_scope : public scope {
    public:
        vrml97_root_scope(const browser & browser,
                        const std::string & uri = std::string())
            throw (std::bad_alloc);
        virtual ~vrml97_root_scope() throw ();
    };


    class null_node_class : public node_class {
    public:
        explicit null_node_class(openvrml::browser & browser) throw ();
        virtual ~null_node_class() throw ();

    private:
        virtual const node_type_ptr
        do_create_type(const std::string & id,
                       const node_interface_set & interfaces) const
            throw ();
    };


    class null_node_type : public node_type {
    public:
        explicit null_node_type(null_node_class & nodeClass) throw ();
        virtual ~null_node_type() throw ();

    private:
        virtual const node_interface_set & do_interfaces() const throw ();
        virtual const node_ptr
        do_create_node(const boost::shared_ptr<openvrml::scope> & scope,
                       const initial_value_map & initial_values) const
            throw ();
    };
}

namespace {

    using namespace openvrml;

    class proto_node;
    class proto_impl_cloner;

    /**
     * @internal
     *
     * @brief <code>node_class</code> for <code>PROTO</code>s.
     *
     * The proto_node_class is OpenVRML's in-memory representation of a
     * <code>PROTO</code> (as opposed to a <code>PROTO</code> instance).
     * Through the <code>proto_node::proto_node_type</code> intermediary, it
     * facilitates spawning any number of <code>proto_node</code> instances.
     */
    class proto_node_class : public node_class {
        friend class proto_node;
        friend class proto_impl_cloner;

    public:
        class is_target {
        public:
            node * impl_node;
            std::string impl_node_interface;

            is_target(node & impl_node,
                      const std::string & impl_node_interface);
        };

        typedef std::multimap<std::string, is_target> is_map_t;

        class route {
        public:
            node * from;
            std::string eventout;
            node * to;
            std::string eventin;

            route(node & from, const std::string & eventout,
                  node & to, const std::string & eventin);
        };

        typedef std::vector<route> routes_t;

        typedef std::map<std::string, boost::shared_ptr<field_value> >
            default_value_map_t;

    private:
        node_interface_set interfaces;
        default_value_map_t default_value_map;
        std::vector<node_ptr> impl_nodes;
        routes_t routes;
        is_map_t is_map;

        class proto_node_type : public node_type {
            node_interface_set interfaces_;

        public:
            proto_node_type(const proto_node_class & node_class,
                            const std::string & id,
                            const node_interface_set & interfaces)
                throw (unsupported_interface, std::bad_alloc);
            virtual ~proto_node_type() throw ();

        private:
            virtual const node_interface_set & do_interfaces() const throw ();
            virtual const node_ptr
            do_create_node(const boost::shared_ptr<openvrml::scope> & scope,
                           const initial_value_map & initial_values) const
                throw (std::bad_alloc);
        };


    public:
        proto_node_class(
            openvrml::browser & browser,
            const node_interface_set & interfaces,
            const default_value_map_t & default_value_map,
            const std::vector<node_ptr> & impl_nodes,
            const is_map_t & is_map,
            const routes_t & routes);
        virtual ~proto_node_class() throw ();

        virtual const node_type_ptr
        do_create_type(const std::string & id,
                       const node_interface_set & interfaces) const
            throw (unsupported_interface, std::bad_alloc);
    };


    /**
     * @internal
     *
     * @brief A <code>PROTO</code> instance node.
     *
     * Like a typical node implementation, <code>proto_node</code>s have a
     * many-to-one relationship with the
     * <code>proto_node::proto_node_type</code> instance that creates them. And
     * <code>proto_node::proto_node_type</code> has, in turn, a many-to-one
     * relationship with the <code>proto_node_class</code> instance that
     * creates them. Unlike a typical node implementation, there will very
     * likely be more than one <code>proto_node_class</code> instance known to
     * the <code>browser</code> instance; there will be one for each
     * <code>PROTO</code> known to the <code>browser</code>.
     *
     * As the <code>proto_node_class</code> encodes the data in a
     * <code>PROTO</code>, the <code>proto_node::proto_node_type</code> can
     * be seen as modeling <code>EXTERNPROTO</code>. Each
     * <code>EXTERNPROTO</code> will spawn a new
     * <code>proto_node::proto_node_type</code> from the
     * <code>proto_node_class</code> that corresponds to the <code>PROTO</code>
     * to which the <code>EXTERNPROTO</code> refers. Recall that an
     * <code>EXTERNPROTO</code> provides a subset of the interfaces defined
     * for a <code>PROTO</code>; thus, for a <code>PROTO</code> with
     * <var>n</var> interfaces, there are <var>n</var>! possible unique
     * <code>EXTERNPROTO</code>s (and thus unique
     * <code>proto_node::proto_node_type</code> instances).
     *
     * Structurally, the implementation of <code>proto_node</code> is very
     * similar to that of <code>proto_node_class</code>. The difference is that
     * event pathways for <code>ROUTE</code>s and <code>IS</code> mappings are
     * actually created in the <code>proto_node</code>. The
     * <code>proto_node_class</code>, on the other hand, includes metadata
     * about how these event pathways @e should be created.
     */
    class proto_node : public node {
        template <typename FieldValue>
        class proto_eventin : public field_value_listener<FieldValue> {
            typedef std::set<field_value_listener<FieldValue> *> listeners;
            listeners listeners_;

        public:
            typedef FieldValue field_value_type;
            typedef field_value_listener<FieldValue> event_listener_type;

            explicit proto_eventin(proto_node & node);
            virtual ~proto_eventin() throw ();

            bool is(event_listener_type & listener) throw (std::bad_alloc);

        protected:
            virtual void do_process_event(const FieldValue & value,
                                          double timestamp)
                throw (std::bad_alloc);
        };

        static boost::shared_ptr<openvrml::event_listener>
        create_eventin(field_value::type_id, proto_node & node)
            throw (std::bad_alloc);

        static bool
        eventin_is(field_value::type_id field_type,
                   openvrml::event_listener & impl_eventin,
                   openvrml::event_listener & interface_eventin)
            throw (std::bad_alloc);

        template <typename FieldValue>
        class proto_eventout : public field_value_emitter<FieldValue> {
        protected:
            class listener_t : public field_value_listener<FieldValue> {
                proto_eventout & emitter;
                proto_node & node;

            public:
                FieldValue value;

                listener_t(proto_eventout & emitter,
                           proto_node & node,
                           const FieldValue & initial_value);
                virtual ~listener_t() throw ();

            private:
                virtual void do_process_event(const FieldValue & value,
                                              double timestamp)
                    throw (std::bad_alloc);
            } listener;

        public:
            typedef FieldValue field_value_type;
            typedef field_value_emitter<FieldValue> event_emitter_type;
            typedef field_value_listener<FieldValue> event_listener_type;

            proto_eventout(
                proto_node & node,
                const FieldValue & initial_value = FieldValue());
            virtual ~proto_eventout() throw ();

            bool is(event_emitter_type & emitter) throw (std::bad_alloc);
        };

        static boost::shared_ptr<openvrml::event_emitter>
        create_eventout(field_value::type_id, proto_node & node)
            throw (std::bad_alloc);

        static bool
        eventout_is(field_value::type_id field_type,
                    openvrml::event_emitter & impl_eventout,
                    openvrml::event_emitter & interface_eventout)
            throw (std::bad_alloc);

        template <typename FieldValue>
        class proto_exposedfield : public proto_eventin<FieldValue>,
                                   public proto_eventout<FieldValue> {
        public:
            proto_exposedfield(proto_node & node,
                                const FieldValue & initial_value);
            virtual ~proto_exposedfield() throw ();

        private:
            virtual void do_process_event(const FieldValue & value,
                                          double timestamp)
                throw (std::bad_alloc);
        };

        static boost::shared_ptr<openvrml::event_listener>
        create_exposedfield(const field_value & initial_value,
                            proto_node & node)
            throw (std::bad_alloc);

        boost::shared_ptr<openvrml::scope> proto_scope;
        std::vector<node_ptr> impl_nodes;

        typedef boost::shared_ptr<openvrml::event_listener> eventin_ptr;
        typedef std::map<std::string, eventin_ptr> eventin_map_t;
        eventin_map_t eventin_map;

        typedef boost::shared_ptr<openvrml::event_emitter> eventout_ptr;
        typedef std::map<std::string, eventout_ptr> eventout_map_t;
        eventout_map_t eventout_map;

    public:
        proto_node(const node_type & type,
                   const boost::shared_ptr<openvrml::scope> & scope,
                   const initial_value_map & initial_values)
            throw (std::bad_alloc);
        virtual ~proto_node() throw ();

    private:
        virtual void do_initialize(double timestamp)
            throw (std::bad_alloc);

        virtual const field_value & do_field(const std::string & id) const
            throw (unsupported_interface);
        virtual openvrml::event_listener &
        do_event_listener(const std::string & id)
            throw (unsupported_interface);
        virtual openvrml::event_emitter &
        do_event_emitter(const std::string & id)
            throw (unsupported_interface);

        virtual void do_shutdown(double timestamp) throw ();

        virtual script_node * to_script() throw ();
        virtual appearance_node * to_appearance() throw ();
        virtual child_node * to_child() throw ();
        virtual color_node * to_color() throw ();
        virtual coordinate_node * to_coordinate() throw ();
        virtual font_style_node * to_font_style() throw () ;
        virtual geometry_node * to_geometry() throw ();
        virtual grouping_node * to_grouping() throw ();
        virtual material_node * to_material() throw ();
        virtual normal_node * to_normal() throw ();
        virtual sound_source_node * to_sound_source() throw ();
        virtual texture_node * to_texture() throw ();
        virtual texture_coordinate_node * to_texture_coordinate() throw ();
        virtual texture_transform_node * to_texture_transform() throw ();
        virtual transform_node * to_transform() throw ();
        virtual viewpoint_node * to_viewpoint() throw ();
    };


    class node_path_element {
    public:
        std::vector<node_ptr>::size_type index;
        std::string field_id;

        node_path_element();
    };

    node_path_element::node_path_element():
        index(0)
    {}

    typedef std::list<node_path_element> node_path_t;

    class path_getter : boost::noncopyable {
        const node & objective;
        node_path_t & node_path;
        bool found;

    public:
        path_getter(const node & objective, node_path_t & node_path) throw ();

        void get_path_from(const node_ptr & node) throw (std::bad_alloc);
        void get_path_from(const std::vector<node_ptr> & nodes)
            throw (std::bad_alloc);

    private:
        void traverse_children(node & n) throw (std::bad_alloc);
    };

    path_getter::path_getter(const node & objective, node_path_t & node_path)
        throw ():
        objective(objective),
        node_path(node_path),
        found(false)
    {}

    void path_getter::get_path_from(const node_ptr & node)
        throw (std::bad_alloc)
    {
        if (node) {
            this->node_path.push_back(node_path_element());
            if (node.get() == &objective) {
                this->found = true;
                return;
            }
            this->traverse_children(*node);
            if (!this->found) { this->node_path.pop_back(); }
        }
    }

    void path_getter::get_path_from(const std::vector<node_ptr> & nodes)
        throw (std::bad_alloc)
    {
        this->node_path.push_back(node_path_element());
        node_path_element & back = this->node_path.back();
        while (back.index < nodes.size()) {
            assert(&back == &this->node_path.back());
            if (nodes[back.index].get() == &this->objective) {
                this->found = true;
                return;
            }
            if (nodes[back.index].get()) {
                this->traverse_children(*nodes[back.index]);
            }
            //
            // We need to bail early to avoid incrementing the counter.
            //
            if (this->found) { return; }
            ++back.index;
        }
        if (!this->found) { this->node_path.pop_back(); }
    }

    void path_getter::traverse_children(node & n) throw (std::bad_alloc)
    {
        const node_interface_set & interfaces = n.type().interfaces();
        node_path_element & back = this->node_path.back();
        for (node_interface_set::const_iterator interface = interfaces.begin();
             !this->found && interface != interfaces.end();
             ++interface) {
            assert(&back == &this->node_path.back());
            if (interface->type == node_interface::field_id
                || interface->type == node_interface::exposedfield_id) {
                if (interface->field_type == field_value::sfnode_id) {
                    back.field_id = interface->id;
                    try {
                        const sfnode & value =
                            static_cast<const sfnode &>(
                                n.field(interface->id));
                        this->get_path_from(value.value);
                    } catch (unsupported_interface & ex) {
                        OPENVRML_PRINT_EXCEPTION_(ex);
                    }
                } else if (interface->field_type == field_value::mfnode_id) {
                    back.field_id = interface->id;
                    try {
                        const mfnode & value =
                            static_cast<const mfnode &>(
                                n.field(interface->id));
                        this->get_path_from(value.value);
                    } catch (unsupported_interface & ex) {
                        OPENVRML_PRINT_EXCEPTION_(ex);
                    }
                }
            }
        }
    }

    const node_path_t get_path(const std::vector<node_ptr> & root,
                               const node & objective)
        throw (std::bad_alloc)
    {
        node_path_t path;
        path_getter(objective, path).get_path_from(root);
        return path;
    }

    node * resolve_node_path(const node_path_t & path,
                             const std::vector<node_ptr> & root)
    {
        using boost::next;
        using boost::prior;
        assert(!path.empty());
        node * result = root[path.front().index].get();
        const node_path_t::const_iterator before_end = prior(path.end());
        for (node_path_t::const_iterator path_element = path.begin();
             path_element != before_end;
             ++path_element) {
            assert(result);
            try {
                const field_value & field_val =
                    result->field(path_element->field_id);
                const field_value::type_id type = field_val.type();
                if (type == field_value::sfnode_id) {
                    result =
                        static_cast<const sfnode &>(field_val).value.get();
                } else if (type == field_value::mfnode_id) {
                    result = static_cast<const mfnode &>(field_val)
                        .value[next(path_element)->index].get();
                }
            } catch (unsupported_interface & ex) {
                OPENVRML_PRINT_EXCEPTION_(ex);
            }
        }
        return result;
    }

    class field_value_cloner {
    protected:
        const boost::shared_ptr<openvrml::scope> & target_scope;
        std::set<node *> traversed_nodes;

    public:
        explicit field_value_cloner(const boost::shared_ptr<openvrml::scope> & target_scope):
            target_scope(target_scope)
        {
            assert(target_scope);
        }

        virtual ~field_value_cloner()
        {}

        void clone_field_value(const node_ptr & src_node,
                               const field_value & src,
                               field_value & dest)
            throw (std::bad_alloc)
        {
            assert(src.type() == dest.type());
            const field_value::type_id type = src.type();
            if (type == field_value::sfnode_id) {
                this->clone_sfnode(src_node,
                                   static_cast<const sfnode &>(src),
                                   static_cast<sfnode &>(dest));
            } else if (type == field_value::mfnode_id) {
                this->clone_mfnode(src_node,
                                   static_cast<const mfnode &>(src),
                                   static_cast<mfnode &>(dest));
            } else {
                //
                // Do a shallow copy for other types.
                //
                dest.assign(src);
            }
        }

    private:
        virtual const node_ptr clone_node(const node_ptr & n)
            throw (std::bad_alloc)
        {
            using std::set;

            assert(this->target_scope);

            node_ptr result;

            if (!n) { return result; }

            const bool already_traversed =
                (this->traversed_nodes.find(n.get())
                 != this->traversed_nodes.end());

            if (already_traversed) {
                result.reset(this->target_scope->find_node(n->id()));
                assert(result);
            } else {
                initial_value_map initial_values;
                const node_interface_set & interfaces =
                    n->type().interfaces();
                for (node_interface_set::const_iterator interface =
                         interfaces.begin();
                     interface != interfaces.end();
                     ++interface) {
                    using std::string;
                    const node_interface::type_id type = interface->type;
                    const string & id = interface->id;
                    if (type == node_interface::exposedfield_id
                        || type == node_interface::field_id) {
                        using std::auto_ptr;
                        using boost::shared_ptr;
                        auto_ptr<field_value> val =
                            field_value::create(interface->field_type);
                        assert(val->type() == n->field(id).type());
                        this->clone_field_value(n, n->field(id), *val);
                        bool succeeded =
                            initial_values.insert(
                                make_pair(id, shared_ptr<field_value>(val)))
                            .second;
                        assert(succeeded);
                    }
                }
                result = n->type().create_node(this->target_scope,
                                               initial_values);
                if (!n->id().empty()) { result->id(n->id()); }
            }
            return result;
        }

        void clone_sfnode(const node_ptr & src_node,
                          const sfnode & src,
                          sfnode & dest)
            throw (std::bad_alloc)
        {
            dest.value = (src.value == src_node)
                       ? node_ptr::self
                       : this->clone_node(src.value);
        }

        void clone_mfnode(const node_ptr & src_node,
                          const mfnode & src,
                          mfnode & dest)
            throw (std::bad_alloc)
        {
            using std::swap;
            using std::vector;
            mfnode result(src.value.size());
            for (vector<node_ptr>::size_type i = 0;
                 i < src.value.size();
                 ++i) {
                result.value[i] = (src.value[i] == src_node)
                                ? node_ptr::self
                                : this->clone_node(src.value[i]);
            }
            swap(dest, result);
        }
    };

    //
    // Clone the implementation nodes.
    //
    class proto_impl_cloner : public field_value_cloner {
        const proto_node_class & node_class;
        const initial_value_map & initial_values_;

    public:
        proto_impl_cloner(const proto_node_class & node_class,
                          const initial_value_map & initial_values,
                          const boost::shared_ptr<openvrml::scope> & target_scope):
            field_value_cloner(target_scope),
            node_class(node_class),
            initial_values_(initial_values)
        {}

        const std::vector<node_ptr> clone() throw (std::bad_alloc)
        {
            using std::vector;

            vector<node_ptr> result(this->node_class.impl_nodes.size());

            for (vector<node_ptr>::size_type i = 0;
                 i < this->node_class.impl_nodes.size();
                 ++i) {
                result[i] = this->clone_node(this->node_class.impl_nodes[i]);
                assert(result[i]);
            }
            return result;
        }

    private:
        struct matches_is_target :
            std::unary_function<proto_node_class::is_map_t::value_type, bool> {

            explicit matches_is_target(
                const proto_node_class::is_target & is_target):
                is_target(&is_target)
            {}

            result_type operator()(const argument_type & is_map_value) const
            {
                return (is_map_value.second.impl_node
                        == this->is_target->impl_node)
                    && (is_map_value.second.impl_node_interface
                        == this->is_target->impl_node_interface);
            }

        private:
            const proto_node_class::is_target * is_target;
        };

        virtual const node_ptr clone_node(const node_ptr & n)
            throw (std::bad_alloc)
        {
            using std::set;

            assert(this->target_scope);

            node_ptr result;

            if (!n) { return result; }

            const bool already_traversed =
                (this->traversed_nodes.find(n.get())
                 != this->traversed_nodes.end());

            if (already_traversed) {
                result.reset(this->target_scope->find_node(n->id()));
                assert(result);
            } else {
                initial_value_map initial_values;
                const node_interface_set & interfaces =
                    n->type().interfaces();
                for (node_interface_set::const_iterator interface =
                         interfaces.begin();
                     interface != interfaces.end();
                     ++interface) {
                    using std::string;
                    const node_interface::type_id type = interface->type;
                    const string & id = interface->id;
                    if (type == node_interface::exposedfield_id
                        || type == node_interface::field_id) {
                        using std::auto_ptr;
                        using std::find_if;
                        using boost::shared_ptr;
                        const field_value * src_val = 0;
                        auto_ptr<field_value> dest_val =
                            field_value::create(interface->field_type);
                        assert(dest_val->type() == n->field(id).type());

                        //
                        // If the field/exposedField is IS'd, get the value
                        // from the initial_values_, or alternatively the
                        // default_values_.
                        //
                        typedef proto_node_class::is_target is_target;
                        typedef proto_node_class::is_map_t is_map;
                        typedef proto_node_class::default_value_map_t
                            default_value_map;

                        is_map::const_iterator is_mapping =
                            find_if(this->node_class.is_map.begin(),
                                    this->node_class.is_map.end(),
                                    matches_is_target(
                                        is_target(*n, interface->id)));
                        if (is_mapping != this->node_class.is_map.end()) {
                            using boost::bind;
                            using std::logical_or;
                            //
                            // If an exposedField in the implementation is IS'd
                            // to an eventIn or an eventOut in the interface,
                            // we'll still get here.  So if the implementation
                            // node interface is an exposedField, we need to
                            // check to see if the PROTO interface is an
                            // eventIn or an eventOut.
                            //
                            node_interface_set::const_iterator
                                proto_interface =
                                find_if(this->node_class.interfaces.begin(),
                                        this->node_class.interfaces.end(),
                                        bind(logical_or<bool>(),
                                             bind(node_interface_matches_exposedfield(),
                                                  _1,
                                                  is_mapping->first),
                                             bind(node_interface_matches_field(),
                                                  _1,
                                                  is_mapping->first)));

                            if (proto_interface
                                != this->node_class.interfaces.end()) {
                                initial_value_map::const_iterator
                                    initial_value =
                                    this->initial_values_.find(
                                        is_mapping->first);
                                if (initial_value
                                    != this->initial_values_.end()) {
                                    src_val = initial_value->second.get();
                                } else {
                                    default_value_map::const_iterator
                                        default_value =
                                        this->node_class.default_value_map
                                        .find(is_mapping->first);
                                    assert(default_value
                                           != this->node_class
                                           .default_value_map.end());
                                    src_val = default_value->second.get();
                                }
                            } else {
                                src_val = &n->field(id);
                            }
                        } else {
                            src_val = &n->field(id);
                        }

                        assert(src_val);
                        this->clone_field_value(n, *src_val, *dest_val);

                        bool succeeded =
                            initial_values.insert(
                                make_pair(id,
                                          shared_ptr<field_value>(dest_val)))
                            .second;
                        assert(succeeded);
                    }
                }
                result = n->type().create_node(this->target_scope,
                                               initial_values);
                if (!n->id().empty()) { result->id(n->id()); }
            }
            return result;
        }

        void clone_sfnode(const sfnode & src, sfnode & dest)
            throw (std::bad_alloc)
        {
            dest.value = this->clone_node(src.value);
        }

        void clone_mfnode(const mfnode & src, mfnode & dest)
            throw (std::bad_alloc)
        {
            using std::swap;
            using std::vector;
            mfnode result(src.value.size());
            for (vector<node_ptr>::size_type i = 0;
                 i < src.value.size();
                 ++i) {
                result.value[i] = this->clone_node(src.value[i]);
            }
            swap(dest, result);
        }
    };

    /**
     * @brief Construct.
     *
     * @param impl_node             a node in the PROTO implementation.
     * @param impl_node_interface   an interface of @p impl_node.
     */
    proto_node_class::
    is_target::is_target(node & impl_node,
                         const std::string & impl_node_interface):
        impl_node(&impl_node),
        impl_node_interface(impl_node_interface)
    {}

    /**
     * @brief Construct.
     *
     * @param from      event source node.
     * @param eventout  eventOut of @p from.
     * @param to        event destination node.
     * @param eventin   eventIn of @p to.
     */
    proto_node_class::route::route(node & from,
                                   const std::string & eventout,
                                   node & to,
                                   const std::string & eventin):
        from(&from),
        eventout(eventout),
        to(&to),
        eventin(eventin)
    {}

    /**
     * @brief Construct.
     *
     * @param node_class    the proto_node_class that spawned the
     *                      proto_node_type.
     * @param id            node_type identifier.
     * @param interfaces    a subset of the interfaces supported by the
     *                      @p node_class.
     *
     * @exception unsupported_interface if an interface in @p interfaces is not
     *                                  supported by the @p node_class.
     * @exception std::bad_alloc        if memory allocation fails.
     */
    proto_node_class::
    proto_node_type::proto_node_type(const proto_node_class & node_class,
                                     const std::string & id,
                                     const node_interface_set & interfaces)
        throw (unsupported_interface, std::bad_alloc):
        node_type(node_class, id)
    {
        using std::find;
        using std::invalid_argument;
        for (node_interface_set::const_iterator interface = interfaces.begin();
             interface != interfaces.end();
             ++interface) {
            node_interface_set::const_iterator pos =
                find(node_class.interfaces.begin(),
                     node_class.interfaces.end(),
                     *interface);
            if (pos == node_class.interfaces.end()) {
                throw unsupported_interface(*interface);
            }
            const bool succeeded = this->interfaces_.insert(*interface).second;
            assert(succeeded);
        }
    }

    /**
     * @brief Destroy.
     */
    proto_node_class::proto_node_type::~proto_node_type() throw ()
    {}

    /**
     * @brief Interfaces.
     *
     * @return the interfaces.
     */
    const node_interface_set &
    proto_node_class::proto_node_type::do_interfaces() const throw ()
    {
        return this->interfaces_;
    }

    const node_ptr
    proto_node_class::proto_node_type::
    do_create_node(const boost::shared_ptr<openvrml::scope> & scope,
                   const initial_value_map & initial_values) const
        throw (std::bad_alloc)
    {
        return node_ptr(new proto_node(*this, scope, initial_values));
    }

    /**
     * @internal
     *
     * @class proto_node::proto_eventin
     *
     * @brief PROTO eventIn handler class template.
     */

    /**
     * @typedef proto_node::proto_eventin::listeners
     *
     * @brief Set of event listeners.
     */

    /**
     * @var proto_node::proto_eventin::listeners proto_node::proto_eventin::listeners_
     *
     * @brief Set of event listeners to which events are delegated for
     *        processing.
     */

    /**
     * @typedef proto_node::proto_eventin::field_value_type
     *
     * @brief Field value type.
     */

    /**
     * @typedef proto_node::proto_eventin::event_listener_type
     *
     * @brief Type of event listeners to which the instance delegates.
     */

    /**
     * @brief Construct.
     *
     * @param node  proto_node.
     */
    template <typename FieldValue>
    proto_node::proto_eventin<FieldValue>::proto_eventin(proto_node & node):
        field_value_listener<FieldValue>(node)
    {}

    /**
     * @brief Destroy.
     */
    template <typename FieldValue>
    proto_node::proto_eventin<FieldValue>::~proto_eventin()
        throw ()
    {}

    /**
     * @brief Process event.
     *
     * @param value     field value.
     * @param timestamp the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    template <typename FieldValue>
    void
    proto_node::proto_eventin<FieldValue>::
    do_process_event(const FieldValue & value, const double timestamp)
        throw (std::bad_alloc)
    {
        for (typename listeners::const_iterator listener =
                 this->listeners_.begin();
             listener != this->listeners_.end();
             ++listener) {
            (*listener)->process_event(value, timestamp);
        }
    }

    /**
     * @brief Add a listener to delegate to. Corresponds to an IS statement.
     *
     * @param listener  an event_listener to delegate to.
     *
     * @return @c true if @p listener is added successfully; @c false
     *         otherwise (if it already exists in the list of delegates).
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    template <typename FieldValue>
    bool
    proto_node::proto_eventin<FieldValue>::is(event_listener_type & listener)
        throw (std::bad_alloc)
    {
        return this->listeners_.insert(&listener).second;
    }

    /**
     * @brief Factory function for proto_eventin<FieldValue> instances.
     *
     * @param type  field_value::type_id.
     * @param node  proto_node.
     *
     * @return a boost::shared_ptr to a proto_eventout<FieldValue> instance.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    boost::shared_ptr<event_listener>
    proto_node::create_eventin(const field_value::type_id type,
                               proto_node & node)
        throw (std::bad_alloc)
    {
        boost::shared_ptr<openvrml::event_listener> result;
        switch (type) {
        case field_value::sfbool_id:
            result.reset(new proto_eventin<sfbool>(node));
            break;
        case field_value::sfcolor_id:
            result.reset(new proto_eventin<sfcolor>(node));
            break;
        case field_value::sffloat_id:
            result.reset(new proto_eventin<sffloat>(node));
            break;
        case field_value::sfimage_id:
            result.reset(new proto_eventin<sfimage>(node));
            break;
        case field_value::sfint32_id:
            result.reset(new proto_eventin<sfint32>(node));
            break;
        case field_value::sfnode_id:
            result.reset(new proto_eventin<sfnode>(node));
            break;
        case field_value::sfrotation_id:
            result.reset(new proto_eventin<sfrotation>(node));
            break;
        case field_value::sfstring_id:
            result.reset(new proto_eventin<sfstring>(node));
            break;
        case field_value::sftime_id:
            result.reset(new proto_eventin<sftime>(node));
            break;
        case field_value::sfvec2f_id:
            result.reset(new proto_eventin<sfvec2f>(node));
            break;
        case field_value::sfvec3f_id:
            result.reset(new proto_eventin<sfvec3f>(node));
            break;
        case field_value::mfcolor_id:
            result.reset(new proto_eventin<mfcolor>(node));
            break;
        case field_value::mffloat_id:
            result.reset(new proto_eventin<mffloat>(node));
            break;
        case field_value::mfint32_id:
            result.reset(new proto_eventin<mfint32>(node));
            break;
        case field_value::mfnode_id:
            result.reset(new proto_eventin<mfnode>(node));
            break;
        case field_value::mfrotation_id:
            result.reset(new proto_eventin<mfrotation>(node));
            break;
        case field_value::mfstring_id:
            result.reset(new proto_eventin<mfstring>(node));
            break;
        case field_value::mftime_id:
            result.reset(new proto_eventin<mftime>(node));
            break;
        case field_value::mfvec2f_id:
            result.reset(new proto_eventin<mfvec2f>(node));
            break;
        case field_value::mfvec3f_id:
            result.reset(new proto_eventin<mfvec3f>(node));
            break;
        default:
            assert(false);
            break;
        }
        assert(result.get());
        return result;
    }

    /**
     * @brief Create an IS mapping between an event_listener in the PROTO
     *        implementation and an event_listener in the PROTO interface.
     *
     * @param field_type        field value type of the concrete listeners.
     * @param impl_eventin      event_listener of a node in the PROTO
     *                          implementation.
     * @param interface_eventin event_listener from the PROTO interface.
     *
     * @return @c true if the IS mapping is established successfully; @c false
     *         otherwise (i.e., if the mapping already exists).
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    bool
    proto_node::eventin_is(const field_value::type_id field_type,
                           openvrml::event_listener & impl_eventin,
                           openvrml::event_listener & interface_eventin)
        throw (std::bad_alloc)
    {
        using boost::polymorphic_downcast;
        bool succeeded;
        switch (field_type) {
        case field_value::sfbool_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfbool> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfbool_listener *>(&impl_eventin));
            break;
        case field_value::sfcolor_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfcolor> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfcolor_listener *>(&impl_eventin));
            break;
        case field_value::sffloat_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sffloat> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sffloat_listener *>(&impl_eventin));
            break;
        case field_value::sfimage_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfimage> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfimage_listener *>(&impl_eventin));
            break;
        case field_value::sfint32_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfint32> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfint32_listener *>(&impl_eventin));
            break;
        case field_value::sfnode_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfnode> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfnode_listener *>(&impl_eventin));
            break;
        case field_value::sfrotation_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfrotation> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfrotation_listener *>(
                         &impl_eventin));
            break;
        case field_value::sfstring_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfstring> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfstring_listener *>(
                         &impl_eventin));
            break;
        case field_value::sftime_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sftime> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sftime_listener *>(&impl_eventin));
            break;
        case field_value::sfvec2f_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfvec2f> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfvec2f_listener *>(&impl_eventin));
            break;
        case field_value::sfvec3f_id:
            succeeded =
                polymorphic_downcast<proto_eventin<sfvec3f> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<sfvec3f_listener *>(&impl_eventin));
            break;
        case field_value::mfcolor_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mfcolor> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mfcolor_listener *>(&impl_eventin));
            break;
        case field_value::mffloat_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mffloat> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mffloat_listener *>(
                         &impl_eventin));
            break;
        case field_value::mfint32_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mfint32> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mfint32_listener *>(&impl_eventin));
            break;
        case field_value::mfnode_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mfnode> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mfnode_listener *>(&impl_eventin));
            break;
        case field_value::mfrotation_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mfrotation> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mfrotation_listener *>(
                         &impl_eventin));
            break;
        case field_value::mfstring_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mfstring> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mfstring_listener *>(
                         &impl_eventin));
            break;
        case field_value::mftime_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mftime> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mftime_listener *>(&impl_eventin));
            break;
        case field_value::mfvec2f_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mfvec2f> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mfvec2f_listener *>(&impl_eventin));
            break;
        case field_value::mfvec3f_id:
            succeeded =
                polymorphic_downcast<proto_eventin<mfvec3f> *>(
                    &interface_eventin)
                ->is(*polymorphic_downcast<mfvec3f_listener *>(&impl_eventin));
            break;
        default:
            assert(false);
            break;
        }
        return succeeded;
    }

    /**
     * @internal
     *
     * @class proto_node::proto_eventout
     *
     * @brief PROTO eventOut handler class template.
     */

    /**
     * @internal
     *
     * @class proto_node::proto_eventout::listener_t
     *
     * @brief Listens for events emitted from nodes in the PROTO implementation
     *        in order to propagate them out of the PROTO instance.
     */

    /**
     * @var proto_node::proto_eventout & proto_node::proto_eventout::listener_t::emitter
     *
     * @brief Reference to the outer proto_eventout class.
     *
     * @todo It's annoying that we need to carry this around.  Should
     *       investigate the possibility of promoting all this stuff to
     *       proto_eventout and have proto_eventout privately inherit
     *       field_value_listener<FieldValue>.
     */

    /**
     * @var proto_node & proto_node::proto_eventout::listener_t::node
     *
     * @brief Reference to the proto_node instance.
     */

    /**
     * @var FieldValue proto_node::proto_eventout::listener_t::value
     *
     * @brief The value of the most recently emitted event.
     */

    /**
     * @brief Construct.
     *
     * @param emitter       proto_eventout.
     * @param node          proto_node.
     * @param initial_value initial value (used for exposedFields).
     */
    template <typename FieldValue>
    proto_node::proto_eventout<FieldValue>::listener_t::
    listener_t(proto_eventout & emitter,
               proto_node & node,
               const FieldValue & initial_value):
        field_value_listener<FieldValue>(node),
        emitter(emitter),
        node(node),
        value(initial_value)
    {}

    /**
     * @brief Destroy.
     */
    template <typename FieldValue>
    proto_node::proto_eventout<FieldValue>::listener_t::~listener_t() throw ()
    {}

    /**
     * @brief Process event.
     *
     * @param value     new value.
     * @param timestamp the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    template<typename FieldValue>
    void
    proto_node::proto_eventout<FieldValue>::listener_t::
    do_process_event(const FieldValue & value, const double timestamp)
        throw (std::bad_alloc)
    {
        if (timestamp > this->emitter.last_time()) {
            this->value = value;
            node::emit_event(this->emitter, timestamp);
        }
    }

    /**
     * @var proto_node::proto_eventout::listener_t proto_node::proto_eventout::listener
     *
     * @brief Listens for events emitted from nodes in the PROTO implementation
     *        in order to propagate them out of the PROTO instance.
     */

    /**
     * @typedef proto_node::proto_eventout<FieldValue>::field_value_type
     *
     * @brief Field value type.
     */

    /**
     * @typedef proto_node::proto_eventout<FieldValue>::event_emitter_type
     *
     * @brief Event emitter type.
     */

    /**
     * @typedef proto_node::proto_eventout<FieldValue>::event_listener_type
     *
     * @brief Event listener type.
     */

    /**
     * @brief Construct.
     *
     * @param node          proto_node.
     * @param initial_value initial value.  This is used by
     *                      proto_exposedfield<FieldValue>
     */
    template <typename FieldValue>
    proto_node::proto_eventout<FieldValue>::
    proto_eventout(proto_node & node, const FieldValue & initial_value):
        field_value_emitter<FieldValue>(this->listener.value),
        listener(*this, node, initial_value)
    {}

    /**
     * @brief Destroy.
     */
    template <typename FieldValue>
    proto_node::proto_eventout<FieldValue>::~proto_eventout() throw ()
    {}

    /**
     * @brief Create an IS mapping.
     *
     * @param emitter   the event_emitter from a node in the PROTO
     *                  implementation.
     *
     * @return @c true if the IS mapping is created successfully; @c false
     *         otherwise (i.e., if it already exists).
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    template <typename FieldValue>
    bool
    proto_node::proto_eventout<FieldValue>::
    is(event_emitter_type & emitter) throw (std::bad_alloc)
    {
        return emitter.add(this->listener);
    }

    /**
     * @brief Factory function for proto_eventout<FieldValue> instances.
     *
     * @param type  field_value::type_id.
     * @param node  proto_node.
     *
     * @return a boost::shared_ptr to a proto_eventout<FieldValue> instance.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    boost::shared_ptr<event_emitter>
    proto_node::create_eventout(const field_value::type_id type,
                                proto_node & node)
        throw (std::bad_alloc)
    {
        boost::shared_ptr<openvrml::event_emitter> result;
        switch (type) {
        case field_value::sfbool_id:
            result.reset(new proto_eventout<sfbool>(node));
            break;
        case field_value::sfcolor_id:
            result.reset(new proto_eventout<sfcolor>(node));
            break;
        case field_value::sffloat_id:
            result.reset(new proto_eventout<sffloat>(node));
            break;
        case field_value::sfimage_id:
            result.reset(new proto_eventout<sfimage>(node));
            break;
        case field_value::sfint32_id:
            result.reset(new proto_eventout<sfint32>(node));
            break;
        case field_value::sfnode_id:
            result.reset(new proto_eventout<sfnode>(node));
            break;
        case field_value::sfrotation_id:
            result.reset(new proto_eventout<sfrotation>(node));
            break;
        case field_value::sfstring_id:
            result.reset(new proto_eventout<sfstring>(node));
            break;
        case field_value::sftime_id:
            result.reset(new proto_eventout<sftime>(node));
            break;
        case field_value::sfvec2f_id:
            result.reset(new proto_eventout<sfvec2f>(node));
            break;
        case field_value::sfvec3f_id:
            result.reset(new proto_eventout<sfvec3f>(node));
            break;
        case field_value::mfcolor_id:
            result.reset(new proto_eventout<mfcolor>(node));
            break;
        case field_value::mffloat_id:
            result.reset(new proto_eventout<mffloat>(node));
            break;
        case field_value::mfint32_id:
            result.reset(new proto_eventout<mfint32>(node));
            break;
        case field_value::mfnode_id:
            result.reset(new proto_eventout<mfnode>(node));
            break;
        case field_value::mfrotation_id:
            result.reset(new proto_eventout<mfrotation>(node));
            break;
        case field_value::mfstring_id:
            result.reset(new proto_eventout<mfstring>(node));
            break;
        case field_value::mftime_id:
            result.reset(new proto_eventout<mftime>(node));
            break;
        case field_value::mfvec2f_id:
            result.reset(new proto_eventout<mfvec2f>(node));
            break;
        case field_value::mfvec3f_id:
            result.reset(new proto_eventout<mfvec3f>(node));
            break;
        default:
            assert(false);
            break;
        }
        assert(result.get());
        return result;
    }

    /**
     * @brief Create an IS mapping between an event_emitter in the PROTO
     *        implementation and an event_emitter in the PROTO interface.
     *
     * @param field_type            field value type of the concrete emitters.
     * @param impl_eventout         event_emitter of a node in the PROTO
     *                              implementation.
     * @param interface_eventout    event_emitter from the PROTO interface.
     *
     * @return @c true if the IS mapping is established successfully; @c false
     *         otherwise (i.e., if the mapping already exists).
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    bool
    proto_node::eventout_is(const field_value::type_id field_type,
                            openvrml::event_emitter & impl_eventout,
                            openvrml::event_emitter & interface_eventout)
        throw (std::bad_alloc)
    {
        using boost::polymorphic_downcast;
        bool succeeded;
        switch (field_type) {
        case field_value::sfbool_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfbool> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfbool_emitter *>(&impl_eventout));
            break;
        case field_value::sfcolor_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfcolor> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfcolor_emitter *>(&impl_eventout));
            break;
        case field_value::sffloat_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sffloat> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sffloat_emitter *>(&impl_eventout));
            break;
        case field_value::sfimage_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfimage> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfimage_emitter *>(&impl_eventout));
            break;
        case field_value::sfint32_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfint32> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfint32_emitter *>(&impl_eventout));
            break;
        case field_value::sfnode_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfnode> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfnode_emitter *>(&impl_eventout));
            break;
        case field_value::sfrotation_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfrotation> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfrotation_emitter *>(
                         &impl_eventout));
            break;
        case field_value::sfstring_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfstring> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfstring_emitter *>(
                         &impl_eventout));
            break;
        case field_value::sftime_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sftime> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sftime_emitter *>(
                         &impl_eventout));
            break;
        case field_value::sfvec2f_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfvec2f> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfvec2f_emitter *>(&impl_eventout));
            break;
        case field_value::sfvec3f_id:
            succeeded =
                polymorphic_downcast<proto_eventout<sfvec3f> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<sfvec3f_emitter *>(&impl_eventout));
            break;
        case field_value::mfcolor_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mfcolor> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mfcolor_emitter *>(&impl_eventout));
            break;
        case field_value::mffloat_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mffloat> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mffloat_emitter *>(&impl_eventout));
            break;
        case field_value::mfint32_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mfint32> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mfint32_emitter *>(&impl_eventout));
            break;
        case field_value::mfnode_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mfnode> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mfnode_emitter *>(&impl_eventout));
            break;
        case field_value::mfrotation_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mfrotation> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mfrotation_emitter *>(
                         &impl_eventout));
            break;
        case field_value::mfstring_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mfstring> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mfstring_emitter *>(
                         &impl_eventout));
            break;
        case field_value::mftime_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mftime> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mftime_emitter *>(&impl_eventout));
            break;
        case field_value::mfvec2f_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mfvec2f> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mfvec2f_emitter *>(&impl_eventout));
            break;
        case field_value::mfvec3f_id:
            succeeded =
                polymorphic_downcast<proto_eventout<mfvec3f> *>(
                    &interface_eventout)
                ->is(*polymorphic_downcast<mfvec3f_emitter *>(&impl_eventout));
            break;
        default:
            assert(false);
            break;
        }
        return succeeded;
    }

    /**
     * @internal
     *
     * @class proto_node::proto_exposedfield
     *
     * @brief PROTO exposedField handler class template.
     */

    /**
     * @brief Construct.
     *
     * @param node          proto_node.
     * @param initial_value initial value.
     */
    template <typename FieldValue>
    proto_node::proto_exposedfield<FieldValue>::
    proto_exposedfield(proto_node & node, const FieldValue & initial_value):
        proto_eventin<FieldValue>(node),
        proto_eventout<FieldValue>(node, initial_value)
    {}

    /**
     * @brief Destroy.
     */
    template <typename FieldValue>
    proto_node::proto_exposedfield<FieldValue>::~proto_exposedfield() throw ()
    {}

    /**
     * @brief Process an event.
     *
     * @param value     event value.
     * @param timestamp the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    template <typename FieldValue>
    void
    proto_node::proto_exposedfield<FieldValue>::
    do_process_event(const FieldValue & value, const double timestamp)
        throw (std::bad_alloc)
    {
        this->proto_eventin<FieldValue>::do_process_event(value, timestamp);
        this->listener.value = value;
        node::emit_event(*this, timestamp);
    }

    /**
     * @brief Factory function for proto_exposedfield<FieldValue> instances.
     *
     * @param type  field_value::type_id.
     * @param node  proto_node.
     *
     * @return a boost::shared_ptr to a proto_exposedfield<FieldValue>
     *         instance.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    boost::shared_ptr<event_listener>
    proto_node::create_exposedfield(const field_value & initial_value,
                                    proto_node & node)
        throw (std::bad_alloc)
    {
        using boost::polymorphic_downcast;
        boost::shared_ptr<openvrml::event_listener> result;
        switch (initial_value.type()) {
        case field_value::sfbool_id:
            result.reset(
                new proto_exposedfield<sfbool>(
                    node,
                    *polymorphic_downcast<const sfbool *>(&initial_value)));
            break;
        case field_value::sfcolor_id:
            result.reset(
                new proto_exposedfield<sfcolor>(
                    node,
                    *polymorphic_downcast<const sfcolor *>(&initial_value)));
            break;
        case field_value::sffloat_id:
            result.reset(
                new proto_exposedfield<sffloat>(
                    node,
                    *polymorphic_downcast<const sffloat *>(&initial_value)));
            break;
        case field_value::sfimage_id:
            result.reset(
                new proto_exposedfield<sfimage>(
                    node,
                    *polymorphic_downcast<const sfimage *>(&initial_value)));
            break;
        case field_value::sfint32_id:
            result.reset(
                new proto_exposedfield<sfint32>(
                    node,
                    *polymorphic_downcast<const sfint32 *>(&initial_value)));
            break;
        case field_value::sfnode_id:
            result.reset(
                new proto_exposedfield<sfnode>(
                    node,
                    *polymorphic_downcast<const sfnode *>(&initial_value)));
            break;
        case field_value::sfrotation_id:
            result.reset(
                new proto_exposedfield<sfrotation>(
                    node,
                    *polymorphic_downcast<const sfrotation *>(
                        &initial_value)));
            break;
        case field_value::sfstring_id:
            result.reset(
                new proto_exposedfield<sfstring>(
                    node,
                    *polymorphic_downcast<const sfstring *>(&initial_value)));
            break;
        case field_value::sftime_id:
            result.reset(
                new proto_exposedfield<sftime>(
                    node,
                    *polymorphic_downcast<const sftime *>(&initial_value)));
            break;
        case field_value::sfvec2f_id:
            result.reset(
                new proto_exposedfield<sfvec2f>(
                    node,
                    *polymorphic_downcast<const sfvec2f *>(&initial_value)));
            break;
        case field_value::sfvec3f_id:
            result.reset(
                new proto_exposedfield<sfvec3f>(
                    node,
                    *polymorphic_downcast<const sfvec3f *>(&initial_value)));
            break;
        case field_value::mfcolor_id:
            result.reset(
                new proto_exposedfield<mfcolor>(
                    node,
                    *polymorphic_downcast<const mfcolor *>(&initial_value)));
            break;
        case field_value::mffloat_id:
            result.reset(
                new proto_exposedfield<mffloat>(
                    node,
                    *polymorphic_downcast<const mffloat *>(&initial_value)));
            break;
        case field_value::mfint32_id:
            result.reset(
                new proto_exposedfield<mfint32>(
                    node,
                    *polymorphic_downcast<const mfint32 *>(&initial_value)));
            break;
        case field_value::mfnode_id:
            result.reset(
                new proto_exposedfield<mfnode>(
                    node,
                    *polymorphic_downcast<const mfnode *>(&initial_value)));
            break;
        case field_value::mfrotation_id:
            result.reset(
                new proto_exposedfield<mfrotation>(
                    node,
                    *polymorphic_downcast<const mfrotation *>(
                        &initial_value)));
            break;
        case field_value::mfstring_id:
            result.reset(
                new proto_exposedfield<mfstring>(
                    node,
                    *polymorphic_downcast<const mfstring *>(&initial_value)));
            break;
        case field_value::mftime_id:
            result.reset(
                new proto_exposedfield<mftime>(
                    node,
                    *polymorphic_downcast<const mftime *>(&initial_value)));
            break;
        case field_value::mfvec2f_id:
            result.reset(
                new proto_exposedfield<mfvec2f>(
                    node,
                    *polymorphic_downcast<const mfvec2f *>(&initial_value)));
            break;
        case field_value::mfvec3f_id:
            result.reset(
                new proto_exposedfield<mfvec3f>(
                    node,
                    *polymorphic_downcast<const mfvec3f *>(&initial_value)));
            break;
        default:
            assert(false);
        }
        assert(result.get());
        return result;
    }

    /**
     * @brief Construct.
     *
     * @param type  node_type.
     * @param scope scope.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    proto_node::proto_node(const node_type & type,
                           const boost::shared_ptr<openvrml::scope> & scope,
                           const initial_value_map & initial_values)
        throw (std::bad_alloc):
        node(type, scope),
        proto_scope(scope)
    {
        const proto_node_class & node_class =
            static_cast<const proto_node_class &>(type.node_class());

        this->impl_nodes = proto_impl_cloner(node_class,
                                             initial_values,
                                             this->proto_scope).clone();

        //
        // Establish routes.
        //
        typedef proto_node_class::routes_t routes_t;
        for (routes_t::const_iterator route = node_class.routes.begin();
             route != node_class.routes.end();
             ++route) {
            // XXX
            // XXX It would be better to store the node_paths along with the
            // XXX route instead of rebuilding them every time we instantiate
            // XXX the PROTO.
            // XXX
            node_path_t path_to_from;
            assert(!node_class.impl_nodes.empty());
            path_getter(*route->from, path_to_from)
                .get_path_from(node_class.impl_nodes);
            assert(!path_to_from.empty());
            node * const from_node = resolve_node_path(path_to_from,
                                                       this->impl_nodes);
            assert(from_node);

            node_path_t path_to_to;
            path_getter(*route->to, path_to_to)
                .get_path_from(node_class.impl_nodes);
            node * const to_node = resolve_node_path(path_to_to,
                                                     this->impl_nodes);
            assert(to_node);

            try {
                add_route(*from_node, route->eventout,
                          *to_node, route->eventin);
            } catch (unsupported_interface & ex) {
                OPENVRML_PRINT_EXCEPTION_(ex);
            } catch (field_value_type_mismatch & ex) {
                OPENVRML_PRINT_EXCEPTION_(ex);
            }
        }

        //
        // Add eventIns, eventOuts, exposedFields.
        //
        for (node_interface_set::const_iterator interface =
                 node_class.interfaces.begin();
             interface != node_class.interfaces.end();
             ++interface) {
            using boost::shared_ptr;
            using boost::dynamic_pointer_cast;
            using std::pair;
            bool succeeded;
            shared_ptr<openvrml::event_listener> interface_eventin;
            shared_ptr<openvrml::event_emitter> interface_eventout;
            typedef proto_node_class::is_map_t is_map_t;
            pair<is_map_t::const_iterator, is_map_t::const_iterator> is_range;
            initial_value_map::const_iterator initial_value;
            switch (interface->type) {
            case node_interface::eventin_id:
                interface_eventin = create_eventin(interface->field_type,
                                                   *this);
                is_range = node_class.is_map.equal_range(interface->id);
                for (is_map_t::const_iterator is_mapping = is_range.first;
                     is_mapping != is_range.second;
                     ++is_mapping) {
                    assert(is_mapping->second.impl_node);
                    node_path_t path_to_impl_node;
                    path_getter(*is_mapping->second.impl_node,
                                path_to_impl_node)
                        .get_path_from(node_class.impl_nodes);
                    node * impl_node = resolve_node_path(path_to_impl_node,
                                                         this->impl_nodes);
                    assert(impl_node);
                    const std::string & impl_node_interface =
                        is_mapping->second.impl_node_interface;
                    try {
                        openvrml::event_listener & impl_eventin =
                            impl_node->event_listener(impl_node_interface);
                        succeeded = eventin_is(interface->field_type,
                                               impl_eventin,
                                               *interface_eventin);
                        assert(succeeded);
                    } catch (unsupported_interface & ex) {
                        OPENVRML_PRINT_EXCEPTION_(ex);
                    }
                }
                succeeded = this->eventin_map
                    .insert(make_pair(interface->id, interface_eventin))
                    .second;
                assert(succeeded);
                break;
            case node_interface::eventout_id:
                interface_eventout = create_eventout(interface->field_type,
                                                     *this);
                is_range = node_class.is_map.equal_range(interface->id);
                for (is_map_t::const_iterator is_mapping = is_range.first;
                     is_mapping != is_range.second;
                     ++is_mapping) {
                    assert(is_mapping->second.impl_node);
                    node_path_t path_to_impl_node;
                    path_getter(*is_mapping->second.impl_node,
                                path_to_impl_node)
                        .get_path_from(node_class.impl_nodes);
                    node * impl_node = resolve_node_path(path_to_impl_node,
                                                         this->impl_nodes);
                    assert(impl_node);
                    const std::string & impl_node_interface =
                        is_mapping->second.impl_node_interface;
                    try {
                        openvrml::event_emitter & impl_eventout =
                            impl_node->event_emitter(impl_node_interface);
                        succeeded = eventout_is(interface->field_type,
                                                impl_eventout,
                                                *interface_eventout);
                        assert(succeeded);
                    } catch (unsupported_interface & ex) {
                        OPENVRML_PRINT_EXCEPTION_(ex);
                    }
                }
                succeeded = this->eventout_map
                    .insert(make_pair(interface->id, interface_eventout))
                    .second;
                assert(succeeded);
                break;
            case node_interface::exposedfield_id:
                initial_value = initial_values.find(interface->id);
                if (initial_value == initial_values.end()) {
                    initial_value =
                        node_class.default_value_map.find(interface->id);
                    assert(initial_value
                           != node_class.default_value_map.end());
                }
                interface_eventin = create_exposedfield(*initial_value->second,
                                                        *this);
                interface_eventout =
                    dynamic_pointer_cast<openvrml::event_emitter>(
                        interface_eventin);
                is_range = node_class.is_map.equal_range(interface->id);
                for (is_map_t::const_iterator is_mapping = is_range.first;
                     is_mapping != is_range.second;
                     ++is_mapping) {
                    assert(is_mapping->second.impl_node);
                    node_path_t path_to_impl_node;
                    path_getter(*is_mapping->second.impl_node,
                                path_to_impl_node)
                        .get_path_from(node_class.impl_nodes);
                    node * impl_node = resolve_node_path(path_to_impl_node,
                                                         this->impl_nodes);
                    assert(impl_node);
                    const std::string & impl_node_interface =
                        is_mapping->second.impl_node_interface;
                    try {
                        openvrml::event_listener & impl_eventin =
                            impl_node->event_listener(impl_node_interface);
                        succeeded = eventin_is(interface->field_type,
                                               impl_eventin,
                                               *interface_eventin);
                        assert(succeeded);
                        openvrml::event_emitter & impl_eventout =
                            impl_node->event_emitter(impl_node_interface);
                        succeeded = eventout_is(interface->field_type,
                                                impl_eventout,
                                                *interface_eventout);
                        assert(succeeded);
                    } catch (unsupported_interface & ex) {
                        OPENVRML_PRINT_EXCEPTION_(ex);
                    }
                }
                succeeded = this->eventin_map
                    .insert(make_pair("set_" + interface->id,
                                      interface_eventin)).second;
                assert(succeeded);
                succeeded = this->eventout_map
                    .insert(make_pair(interface->id + "_changed",
                                      interface_eventout)).second;
                assert(succeeded);
                break;
            case node_interface::field_id:
                break;
            default:
                assert(false);
            }
        }
    }

    /**
     * @brief Destroy.
     */
    proto_node::~proto_node() throw ()
    {}

    /**
     * @brief Initialize.
     *
     * @param timestamp the current time.
     *
     * @exception std::bad_alloc    if memory allocation fails.
     */
    void proto_node::do_initialize(const double timestamp)
        throw (std::bad_alloc)
    {
        using std::vector;
        for (vector<node_ptr>::const_iterator node = this->impl_nodes.begin();
             node != impl_nodes.end();
             ++node) {
            (*node)->initialize(*this->scene(), timestamp);
        }
    }

    /**
     * @brief Field accessor implementation.
     *
     * @param id    field identifier.
     *
     * @exception unsupported_interface if the node has no field @p id.
     *
     * @todo Make this function handle exposedFields.
     */
    const field_value & proto_node::do_field(const std::string & id) const
        throw (unsupported_interface)
    {
        //
        // First, we need to find the implementation node that the field is
        // IS'd to.  For the accessor, we don't care if there's more than one.
        //
        const proto_node_class & node_class =
            static_cast<const proto_node_class &>(this->type().node_class());
        proto_node_class::is_map_t::const_iterator is_mapping =
            node_class.is_map.find(id);
        if (is_mapping != node_class.is_map.end()) {
            //
            // Get the path to the implementation node.
            //
            assert(is_mapping->second.impl_node);
            assert(!is_mapping->second.impl_node_interface.empty());
            node_path_t path;
            path_getter(*is_mapping->second.impl_node, path)
                .get_path_from(node_class.impl_nodes);

            //
            // Resolve the path against this instance's implementation nodes.
            //
            node * const impl_node = resolve_node_path(path, this->impl_nodes);

            //
            // Set the field value for the implementation node.
            //
            return impl_node->field(is_mapping->second.impl_node_interface);
        } else {
            //
            // If there are no IS mappings for the field, then return the
            // default value.
            //
            proto_node_class::default_value_map_t::const_iterator
                default_value = node_class.default_value_map.find(id);
            if (default_value == node_class.default_value_map.end()) {
                throw unsupported_interface(this->type(), id);
            }
            return *default_value->second;
        }
    }

    /**
     * @brief event_listener accessor implementation.
     *
     * @param id    eventIn identifier.
     *
     * @return the event_listener for the eventIn @p id.
     *
     * @exception unsupported_interface if the node has no eventIn @p id.
     */
    event_listener &
    proto_node::do_event_listener(const std::string & id)
        throw (unsupported_interface)
    {
        eventin_map_t::iterator pos = this->eventin_map.find(id);
        if (pos == this->eventin_map.end()) {
            throw unsupported_interface(this->type(),
                                        node_interface::eventin_id,
                                        id);
        }
        return *pos->second;
    }

    /**
     * @brief event_emitter accessor implementation.
     *
     * @param id    eventOut identifier.
     *
     * @return the event_emitter for the eventOut @p id.
     *
     * @exception unsupported_interface if the node has no eventOut @p id.
     */
    event_emitter & proto_node::do_event_emitter(const std::string & id)
        throw (unsupported_interface)
    {
        eventout_map_t::iterator pos = this->eventout_map.find(id);
        if (pos == this->eventout_map.end()) {
            throw unsupported_interface(this->type(),
                                        node_interface::eventout_id,
                                        id);
        }
        return *pos->second;
    }

    /**
     * @brief Initialize.
     *
     * @param timestamp the current time.
     */
    void proto_node::do_shutdown(const double timestamp) throw ()
    {
        using std::vector;
        for (vector<node_ptr>::const_iterator node = this->impl_nodes.begin();
             node != impl_nodes.end();
             ++node) {
            (*node)->shutdown(timestamp);
        }
    }

    /**
     * @brief Cast to a script_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a script_node, or 0 otherwise.
     */
    script_node * proto_node::to_script() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<script_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to an appearance_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is an appearance_node, or 0 otherwise.
     */
    appearance_node * proto_node::to_appearance() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<appearance_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a child_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a child_node, or 0 otherwise.
     */
    child_node * proto_node::to_child() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<child_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a color_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a color_node, or 0 otherwise.
     */
    color_node * proto_node::to_color() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<color_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a coordinate_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a coordinate_node, or 0 otherwise.
     */
    coordinate_node * proto_node::to_coordinate() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<coordinate_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a font_style_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a font_style_node, or 0 otherwise.
     */
    font_style_node * proto_node::to_font_style() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<font_style_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a geometry_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a geometry_node, or 0 otherwise.
     */
    geometry_node * proto_node::to_geometry() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<geometry_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a grouping_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a geometry_node, or 0 otherwise.
     */
    grouping_node * proto_node::to_grouping() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<grouping_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a material_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a material_node, or 0 otherwise.
     */
    material_node * proto_node::to_material() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<material_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a normal_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a normal_node, or 0 otherwise.
     */
    normal_node * proto_node::to_normal() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<normal_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a sound_source_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a sound_source_node, or 0 otherwise.
     */
    sound_source_node * proto_node::to_sound_source() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<sound_source_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a texture_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a texture_node, or 0 otherwise.
     */
    texture_node * proto_node::to_texture() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<texture_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a texture_coordinate_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a texture_coordinate_node, or 0 otherwise.
     */
    texture_coordinate_node * proto_node::to_texture_coordinate() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<texture_coordinate_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a texture_transform_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a texture_transform_node, or 0 otherwise.
     */
    texture_transform_node * proto_node::to_texture_transform() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<texture_transform_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a transform_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a transform_node, or 0 otherwise.
     */
    transform_node * proto_node::to_transform() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<transform_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Cast to a viewpoint_node.
     *
     * @return a pointer to the first node in the implementation if that node
     *         is a viewpoint_node, or 0 otherwise.
     */
    viewpoint_node * proto_node::to_viewpoint() throw ()
    {
        assert(!this->impl_nodes.empty());
        assert(this->impl_nodes[0]);
        return node_cast<viewpoint_node *>(this->impl_nodes[0].get());
    }

    /**
     * @brief Construct.
     *
     * @param browser
     * @param interfaces
     * @param default_value_map
     * @param impl_nodes
     * @param is_map
     * @param routes
     */
    proto_node_class::
    proto_node_class(openvrml::browser & browser,
                     const node_interface_set & interfaces,
                     const default_value_map_t & default_value_map,
                     const std::vector<node_ptr> & impl_nodes,
                     const is_map_t & is_map,
                     const routes_t & routes):
        node_class(browser),
        interfaces(interfaces),
        default_value_map(default_value_map),
        impl_nodes(impl_nodes),
        routes(routes),
        is_map(is_map)
    {}

    proto_node_class::~proto_node_class() throw ()
    {}

    const node_type_ptr
    proto_node_class::
    do_create_type(const std::string & id,
                   const node_interface_set & interfaces) const
        throw (unsupported_interface, std::bad_alloc)
    {
        return node_type_ptr(new proto_node_type(*this, id, interfaces));
    }


    class default_navigation_info : public navigation_info_node {
    public:
        explicit default_navigation_info(const null_node_type & type) throw ();
        virtual ~default_navigation_info() throw ();

        virtual const std::vector<float> & avatar_size() const throw ();
        virtual bool headlight() const throw ();
        virtual float speed() const throw ();
        virtual const std::vector<std::string> & type() const throw ();
        virtual float visibility_limit() const throw ();

    private:
        virtual void do_field(const std::string & id,
                              const field_value & value)
            throw ();
        virtual const field_value & do_field(const std::string & id) const
            throw ();
        virtual void do_process_event(const std::string & id,
                                      const field_value & value,
                                      double timestamp)
            throw ();
        virtual const field_value & do_eventout(const std::string & id) const
            throw ();

        virtual openvrml::event_listener &
        do_event_listener(const std::string & id)
            throw (unsupported_interface);
        virtual openvrml::event_emitter &
        do_event_emitter(const std::string & id)
            throw (unsupported_interface);
    };

    const boost::shared_ptr<openvrml::scope> null_scope_ptr;

    /**
     * @brief Construct.
     *
     * @param t node type.
     */
    default_navigation_info::default_navigation_info(const null_node_type & t)
        throw ():
        node(t, null_scope_ptr),
        child_node(t, null_scope_ptr),
        openvrml::navigation_info_node(t, null_scope_ptr)
    {}

    /**
     * @brief Destroy
     */
    default_navigation_info::~default_navigation_info() throw ()
    {}

    /**
     * @brief The avatar dimensions.
     *
     * @return [0.25, 1.6, 0.75]
     */
    const std::vector<float> & default_navigation_info::avatar_size() const
        throw ()
    {
        static const float array[] = { 0.25, 1.6, 0.75 };
        static const std::vector<float> vec(array, array + 3);
        return vec;
    }

    /**
     * @brief The headlight state.
     *
     * @return @c true
     */
    bool default_navigation_info::headlight() const throw ()
    {
        return true;
    }

    /**
     * @brief The speed of the user view.
     *
     * @return 1.0
     */
    float default_navigation_info::speed() const throw ()
    {
        return 1.0;
    }

    /**
     * @brief The navigation type.
     *
     * @return ["WALK", "ANY"]
     */
    const std::vector<std::string> & default_navigation_info::type() const
        throw ()
    {
        static const char * array[] = { "WALK", "ANY" };
        static const std::vector<std::string> vec(array, array + 2);
        return vec;
    }

    /**
     * @brief The visibility limit.
     *
     * @return 0.0
     */
    float default_navigation_info::visibility_limit() const throw ()
    {
        return 0.0;
    }

    void default_navigation_info::do_field(const std::string & id,
                                           const field_value & value)
        throw ()
    {
        assert(false);
    }

    const field_value &
    default_navigation_info::do_field(const std::string & id) const
        throw ()
    {
        assert(false);
        static const sfbool value;
        return value;
    }

    void default_navigation_info::do_process_event(const std::string & id,
                                                   const field_value & value,
                                                   double timestamp)
        throw ()
    {
        assert(false);
    }

    const field_value &
    default_navigation_info::do_eventout(const std::string & id) const throw ()
    {
        assert(false);
        static const sfbool value;
        return value;
    }

    event_listener &
    default_navigation_info::do_event_listener(const std::string & id)
        throw (unsupported_interface)
    {
        assert(false);
        throw unsupported_interface(this->node::type(), id);
        return *static_cast<openvrml::event_listener *>(0);
    }

    event_emitter &
    default_navigation_info::do_event_emitter(const std::string & id)
        throw (unsupported_interface)
    {
        assert(false);
        throw unsupported_interface(this->node::type(), id);
        return *static_cast<openvrml::event_emitter *>(0);
    }


    class default_viewpoint : public viewpoint_node {
        mat4f userViewTransform;

    public:
        explicit default_viewpoint(const null_node_type & type) throw ();
        virtual ~default_viewpoint() throw ();

        virtual const mat4f & transformation() const throw ();
        virtual const mat4f & user_view_transform() const throw ();
        virtual void user_view_transform(const mat4f & transform) throw ();
        virtual const std::string & description() const throw ();
        virtual float field_of_view() const throw ();

    private:
        virtual void do_field(const std::string & id,
                              const field_value & value)
            throw ();
        virtual const field_value & do_field(const std::string & id) const
            throw ();
        virtual void do_process_event(const std::string & id,
                                      const field_value & value,
                                      double timestamp)
            throw ();
        virtual const field_value & do_eventout(const std::string & id) const
            throw ();

        virtual openvrml::event_listener &
        do_event_listener(const std::string & id)
            throw (unsupported_interface);
        virtual openvrml::event_emitter &
        do_event_emitter(const std::string & id)
            throw (unsupported_interface);
    };

    /**
     * @brief Construct.
     *
     * @param type  the browser's null_node_type instance.
     */
    default_viewpoint::default_viewpoint(const null_node_type & type)
        throw ():
        node(type, null_scope_ptr),
        child_node(type, null_scope_ptr),
        viewpoint_node(type, null_scope_ptr)
    {}

    /**
     * @brief Destroy.
     */
    default_viewpoint::~default_viewpoint() throw ()
    {}

    const mat4f & default_viewpoint::transformation() const throw ()
    {
        static const vec3f position(0.0, 0.0, 10.0);
        static const rotation orientation;
        static const vec3f scale(1.0, 1.0, 1.0);
        static const rotation scaleOrientation;
        static const vec3f center;
        static const mat4f t(mat4f::transformation(position,
                                                   orientation,
                                                   scale,
                                                   scaleOrientation,
                                                   center));
        return t;
    }

    const mat4f & default_viewpoint::user_view_transform() const throw ()
    {
        return this->userViewTransform;
    }

    void default_viewpoint::user_view_transform(const mat4f & transform)
        throw ()
    {
        this->userViewTransform = transform;
    }

    const std::string & default_viewpoint::description() const throw ()
    {
        static const std::string desc;
        return desc;
    }

    float default_viewpoint::field_of_view() const throw ()
    {
        static const float fieldOfView = 0.785398f;
        return fieldOfView;
    }

    void default_viewpoint::do_field(const std::string & id,
                                     const field_value & value)
        throw ()
    {
        assert(false);
    }

    const field_value &
    default_viewpoint::do_field(const std::string & id) const
        throw ()
    {
        assert(false);
        static const sfbool value;
        return value;
    }

    void default_viewpoint::do_process_event(const std::string & id,
                                             const field_value & value,
                                             double timestamp)
        throw ()
    {
        assert(false);
    }

    const field_value &
    default_viewpoint::do_eventout(const std::string & id) const throw ()
    {
        assert(false);
        static const sfbool value;
        return value;
    }

    event_listener &
    default_viewpoint::do_event_listener(const std::string & id)
        throw (unsupported_interface)
    {
        assert(false);
        throw unsupported_interface(this->type(), id);
        return *static_cast<openvrml::event_listener *>(0);
    }

    event_emitter &
    default_viewpoint::do_event_emitter(const std::string & id)
        throw (unsupported_interface)
    {
        assert(false);
        throw unsupported_interface(this->type(), id);
        return *static_cast<openvrml::event_emitter *>(0);
    }


    class uri {
        struct grammar : public boost::spirit::grammar<grammar> {
            struct absolute_uri_closure :
                boost::spirit::closure<absolute_uri_closure,
                                       std::string::const_iterator,
                                       std::string::const_iterator> {
                member1 scheme_begin;
                member2 scheme_end;
            };

            struct server_closure :
                boost::spirit::closure<server_closure,
                                       std::string::const_iterator,
                                       std::string::const_iterator> {
                member1 userinfo_begin;
                member2 userinfo_end;
            };

            template <typename ScannerT>
            struct definition {
                typedef boost::spirit::rule<ScannerT> rule_type;
                typedef boost::spirit::rule<ScannerT,
                                            absolute_uri_closure::context_t>
                    absolute_uri_rule_type;
                typedef boost::spirit::rule<ScannerT,
                                            server_closure::context_t>
                    server_rule_type;

                rule_type uri_reference;
                absolute_uri_rule_type absolute_uri;
                rule_type relative_uri;
                rule_type hier_part;
                rule_type opaque_part;
                rule_type uric_no_slash;
                rule_type net_path;
                rule_type abs_path;
                rule_type rel_path;
                rule_type rel_segment;
                rule_type scheme;
                rule_type authority;
                rule_type reg_name;
                server_rule_type server;
                rule_type userinfo;
                rule_type hostport;
                rule_type host;
                rule_type hostname;
                rule_type domainlabel;
                rule_type toplabel;
                rule_type ipv4address;
                rule_type port;
                rule_type path_segments;
                rule_type segment;
                rule_type param;
                rule_type pchar;
                rule_type query;
                rule_type fragment;
                rule_type uric;
                rule_type reserved;
                rule_type unreserved;
                rule_type mark;
                rule_type escaped;

                explicit definition(const grammar & self);

                const boost::spirit::rule<ScannerT> & start() const;
            };

            mutable uri & uri_ref;

            explicit grammar(uri & uri_ref) throw ();
        };

        std::string str_;
        std::string::const_iterator scheme_begin, scheme_end;
        std::string::const_iterator scheme_specific_part_begin,
                                    scheme_specific_part_end;
        std::string::const_iterator authority_begin, authority_end;
        std::string::const_iterator userinfo_begin, userinfo_end;
        std::string::const_iterator host_begin, host_end;
        std::string::const_iterator port_begin, port_end;
        std::string::const_iterator path_begin, path_end;
        std::string::const_iterator query_begin, query_end;
        std::string::const_iterator fragment_begin, fragment_end;

    public:
        uri() throw (std::bad_alloc);
        explicit uri(const std::string & str)
            throw (openvrml::invalid_url, std::bad_alloc);

        operator std::string() const throw (std::bad_alloc);

        const std::string scheme() const throw (std::bad_alloc);
        const std::string scheme_specific_part() const throw (std::bad_alloc);
        const std::string authority() const throw (std::bad_alloc);
        const std::string userinfo() const throw (std::bad_alloc);
        const std::string host() const throw (std::bad_alloc);
        const std::string port() const throw (std::bad_alloc);
        const std::string path() const throw (std::bad_alloc);
        const std::string query() const throw (std::bad_alloc);
        const std::string fragment() const throw (std::bad_alloc);

        const uri resolve_against(const uri & absolute_uri) const
            throw (std::bad_alloc);
    };

    uri::grammar::grammar(uri & uri_ref) throw ():
        uri_ref(uri_ref)
    {}

    template <typename ScannerT>
    uri::grammar::definition<ScannerT>::definition(const grammar & self)
    {
        using namespace boost::spirit;
        using namespace phoenix;

        BOOST_SPIRIT_DEBUG_NODE(uri_reference);
        BOOST_SPIRIT_DEBUG_NODE(absolute_uri);
        BOOST_SPIRIT_DEBUG_NODE(relative_uri);
        BOOST_SPIRIT_DEBUG_NODE(hier_part);
        BOOST_SPIRIT_DEBUG_NODE(opaque_part);
        BOOST_SPIRIT_DEBUG_NODE(uric_no_slash);
        BOOST_SPIRIT_DEBUG_NODE(net_path);
        BOOST_SPIRIT_DEBUG_NODE(abs_path);
        BOOST_SPIRIT_DEBUG_NODE(rel_path);
        BOOST_SPIRIT_DEBUG_NODE(rel_segment);
        BOOST_SPIRIT_DEBUG_NODE(scheme);
        BOOST_SPIRIT_DEBUG_NODE(authority);
        BOOST_SPIRIT_DEBUG_NODE(reg_name);
        BOOST_SPIRIT_DEBUG_NODE(server);
        BOOST_SPIRIT_DEBUG_NODE(userinfo);
        BOOST_SPIRIT_DEBUG_NODE(hostport);
        BOOST_SPIRIT_DEBUG_NODE(host);
        BOOST_SPIRIT_DEBUG_NODE(hostname);
        BOOST_SPIRIT_DEBUG_NODE(domainlabel);
        BOOST_SPIRIT_DEBUG_NODE(toplabel);
        BOOST_SPIRIT_DEBUG_NODE(ipv4address);
        BOOST_SPIRIT_DEBUG_NODE(port);
        BOOST_SPIRIT_DEBUG_NODE(path_segments);
        BOOST_SPIRIT_DEBUG_NODE(segment);
        BOOST_SPIRIT_DEBUG_NODE(param);
        BOOST_SPIRIT_DEBUG_NODE(pchar);
        BOOST_SPIRIT_DEBUG_NODE(query);
        BOOST_SPIRIT_DEBUG_NODE(fragment);
        BOOST_SPIRIT_DEBUG_NODE(uric);
        BOOST_SPIRIT_DEBUG_NODE(reserved);
        BOOST_SPIRIT_DEBUG_NODE(unreserved);
        BOOST_SPIRIT_DEBUG_NODE(mark);
        BOOST_SPIRIT_DEBUG_NODE(escaped);

        uri & uri_ref = self.uri_ref;

        uri_reference
            =   !(absolute_uri | relative_uri) >> !('#' >> fragment)
            ;

        absolute_uri
            =   (scheme[
                    absolute_uri.scheme_begin = arg1,
                    absolute_uri.scheme_end = arg2
                ] >> ':')[
                    var(uri_ref.scheme_begin) = absolute_uri.scheme_begin,
                    var(uri_ref.scheme_end) = absolute_uri.scheme_end
                ] >> (hier_part | opaque_part)[
                    var(uri_ref.scheme_specific_part_begin) = arg1,
                    var(uri_ref.scheme_specific_part_end) = arg2
                ]
            ;

        relative_uri
            =   (net_path | abs_path | rel_path) >> !('?' >> query)
            ;

        hier_part
            =   (net_path | abs_path) >> !('?' >> query)
            ;

        opaque_part
            =   uric_no_slash >> *uric
            ;

        uric_no_slash
            =   unreserved
            |   escaped
            |   ';'
            |   '?'
            |   ':'
            |   '@'
            |   '&'
            |   '='
            |   '+'
            |   '$'
            |   ','
            ;

        net_path
            =   "//" >> authority >> !abs_path
            ;

        abs_path
            =   ('/' >> path_segments)[
                    var(uri_ref.path_begin) = arg1,
                    var(uri_ref.path_end) = arg2
                ]
            ;

        rel_path
            =   (rel_segment >> !abs_path)[
                    var(uri_ref.path_begin) = arg1,
                    var(uri_ref.path_end) = arg2
                ]
            ;

        rel_segment
            =  +(   unreserved
                |   escaped
                |   ';'
                |   '@'
                |   '&'
                |   '='
                |   '+'
                |   '$'
                |   ','
                )
            ;

        scheme
            =   (alpha_p >> *(alpha_p | digit_p | '+' | '-' | '.'))
            ;

        authority
            =   (server | reg_name)[
                    var(uri_ref.authority_begin) = arg1,
                    var(uri_ref.authority_end) = arg2
                ]
            ;

        reg_name
            =  +(   unreserved
                |   escaped
                |   '$'
                |   ','
                |   ';'
                |   ':'
                |   '@'
                |   '&'
                |   '='
                |   '+'
                )
            ;

        server
            =  !(
                    !(userinfo[
                        server.userinfo_begin = arg1,
                        server.userinfo_end = arg2
                    ] >> '@')[
                        var(uri_ref.userinfo_begin) = server.userinfo_begin,
                        var(uri_ref.userinfo_end) = server.userinfo_end
                    ]
                    >> hostport
                )
            ;

        userinfo
            =  *(   unreserved
                |   escaped
                |   ';'
                |   ':'
                |   '&'
                |   '='
                |   '+'
                |   '$'
                |   ','
                )
            ;

        hostport
            =   host >> !(':' >> port)
            ;

        host
            =   (hostname | ipv4address)[
                    var(uri_ref.host_begin) = arg1,
                    var(uri_ref.host_end) = arg2
                ]
            ;

        hostname
            =   *(domainlabel >> '.') >> toplabel >> !ch_p('.')
            ;

        domainlabel
            =   alnum_p >> *(*ch_p('-') >> alnum_p)
            ;

        toplabel
            =   alpha_p >> *(*ch_p('-') >> alnum_p)
            ;

        ipv4address
            =   +digit_p >> '.' >> +digit_p >> '.' >> +digit_p >> '.'
                >> +digit_p
            ;

        port
            =   (*digit_p)[
                    var(uri_ref.port_begin) = arg1,
                    var(uri_ref.port_end) = arg2
                ]
            ;

        path_segments
            =   segment >> *('/' >> segment)
            ;

        segment
            =   *pchar >> *(';' >> param)
            ;

        param
            =   *pchar
            ;

        pchar
            =   unreserved
            |   escaped
            |   ':'
            |   '@'
            |   '&'
            |   '='
            |   '+'
            |   '$'
            |   ','
            ;

        query
            =   (*uric)[
                    var(uri_ref.query_begin) = arg1,
                    var(uri_ref.query_end) = arg2
                ]
            ;

        fragment
            =   (*uric)[
                    var(uri_ref.fragment_begin) = arg1,
                    var(uri_ref.fragment_end) = arg2
                ]
            ;

        uric
            =   reserved
            |   unreserved
            |   escaped
            ;

        reserved
            =   ch_p(';')
            |   '/'
            |   '?'
            |   ':'
            |   '@'
            |   '&'
            |   '='
            |   '+'
            |   '$'
            |   ','
            ;

        unreserved
            =   alnum_p
            |   mark
            ;

        mark
            =   ch_p('-')
            |   '_'
            |   '.'
            |   '!'
            |   '~'
            |   '*'
            |   '\''
            |   '('
            |   ')'
            ;

        escaped
            =   '%' >> xdigit_p >> xdigit_p
            ;
    }

    template <typename ScannerT>
    const boost::spirit::rule<ScannerT> &
    uri::grammar::definition<ScannerT>::start() const
    {
        return uri_reference;
    }

    uri::uri() throw (std::bad_alloc)
    {}

    uri::uri(const std::string & str)
        throw (openvrml::invalid_url, std::bad_alloc):
        str_(str)
    {
        using std::string;
        using namespace boost::spirit;

        grammar g(*this);

        string::const_iterator begin = this->str_.begin();
        string::const_iterator end = this->str_.end();

        if (!parse(begin, end, g, space_p).full) {
            throw openvrml::invalid_url();
        }
    }

    uri::operator std::string() const throw (std::bad_alloc)
    {
        return this->str_;
    }

    const std::string uri::scheme() const throw (std::bad_alloc)
    {
        return std::string(this->scheme_begin, this->scheme_end);
    }

    const std::string uri::scheme_specific_part() const
        throw (std::bad_alloc)
    {
        return std::string(this->scheme_specific_part_begin,
                           this->scheme_specific_part_end);
    }

    const std::string uri::authority() const throw (std::bad_alloc)
    {
        return std::string(this->authority_begin, this->authority_end);
    }

    const std::string uri::userinfo() const throw (std::bad_alloc)
    {
        return std::string(this->userinfo_begin, this->userinfo_end);
    }

    const std::string uri::host() const throw (std::bad_alloc)
    {
        return std::string(this->host_begin, this->host_end);
    }

    const std::string uri::port() const throw (std::bad_alloc)
    {
        return std::string(this->port_begin, this->port_end);
    }

    const std::string uri::path() const throw (std::bad_alloc)
    {
        return std::string(this->path_begin, this->path_end);
    }

    const std::string uri::query() const throw (std::bad_alloc)
    {
        return std::string(this->query_begin, this->query_end);
    }

    const std::string uri::fragment() const throw (std::bad_alloc)
    {
        return std::string(this->fragment_begin, this->fragment_end);
    }

    const uri uri::resolve_against(const uri & absolute_uri) const
        throw (std::bad_alloc)
    {
        using std::list;
        using std::string;

        assert(this->scheme().empty());
        assert(!absolute_uri.scheme().empty());

        string result = absolute_uri.scheme() + ':';

        if (!this->authority().empty()) {
            return uri(result + this->scheme_specific_part());
        } else {
            result += "//" + absolute_uri.authority();
        }

        string path = absolute_uri.path();
        const string::size_type last_slash_index = path.find_last_of('/');

        //
        // Chop off the leading slash and the last path segment (typically a
        // file name).
        //
        path = path.substr(1, last_slash_index);

        //
        // Append the relative path.
        //
        path += this->path();

        //
        // Put the path segments in a list to process them.
        //
        list<string> path_segments;
        string::size_type slash_index = 0;
        string::size_type segment_start_index = 0;
        do {
            slash_index = path.find('/', segment_start_index);
            string segment = path.substr(segment_start_index,
                                         slash_index - segment_start_index);
            if (!segment.empty()) {
                path_segments.push_back(segment);
            }
            segment_start_index = slash_index + 1;
        } while (slash_index != string::npos);

        //
        // Remove any "." segments.
        //
        path_segments.remove(".");

        //
        // Remove any ".." segments along with the segment that precedes them.
        //
        const list<string>::iterator begin(path_segments.begin());
        list<string>::iterator pos;
        for (pos = begin; pos != path_segments.end(); ++pos) {
            if (pos != begin && *pos == "..") {
                --(pos = path_segments.erase(pos));
                --(pos = path_segments.erase(pos));
            }
        }

        //
        // Reconstruct the path.
        //
        path = string();
        for (pos = path_segments.begin(); pos != path_segments.end(); ++pos) {
            path += '/' + *pos;
        }

        //
        // End in a slash?
        //
        if (*(this->path().end() - 1) == '/') { path += '/'; }

        result += path;

        const string query = this->query();
        if (!query.empty()) { result += '?' + query; }

        const string fragment = this->fragment();
        if (!fragment.empty()) { result += '#' + fragment; }

        uri result_uri;
        try {
            result_uri = uri(result);
        } catch (openvrml::invalid_url &) {
            assert(false); // If we constructed a bad URI, something is wrong.
        }

        return result_uri;
    }


# ifdef OPENVRML_ENABLE_GZIP
    namespace z {

        typedef int level;
        const level no_compression      = Z_NO_COMPRESSION;
        const level best_speed          = Z_BEST_SPEED;
        const level best_compression    = Z_BEST_COMPRESSION;
        const level default_compression = Z_DEFAULT_COMPRESSION;

        enum strategy {
            default_strategy    = Z_DEFAULT_STRATEGY,
            filtered            = Z_FILTERED,
            huffman_only        = Z_HUFFMAN_ONLY
        };

        class filebuf : public std::streambuf {
            enum { buffer_size = 16384 };
            char buffer[buffer_size];
            gzFile file;

        public:
            filebuf();
            virtual ~filebuf();

            bool is_open() const;
            filebuf * open(const char * path, int mode,
                           level = default_compression,
                           strategy = default_strategy);
            filebuf * close();

        protected:
            virtual int underflow();
            virtual int overflow(int = EOF);
        };

        class ifstream : public std::istream {
            filebuf fbuf;

        public:
            ifstream();
            explicit ifstream(const char * path, level = default_compression,
                              strategy = default_strategy);
            virtual ~ifstream();

            filebuf * rdbuf() const;
            bool is_open() const;
            void open(const char * path, level = default_compression,
                      strategy = default_strategy);
            void close();
        };

        //
        // filebuf
        //

        int const lookback(4);

        filebuf::filebuf(): file(0) {
            this->setg(this->buffer + lookback,  // beginning of putback area
                       this->buffer + lookback,  // read position
                       this->buffer + lookback); // end position
        }

        filebuf::~filebuf() {
            this->close();
        }

        bool filebuf::is_open() const {
            return (this->file != 0);
        }

        filebuf * filebuf::open(const char * path,
                                const int mode,
                                const level comp_level,
                                const strategy comp_strategy) {
            using std::ios;

            if (this->file) { return 0; }

            //
            // zlib only supports the "rb" and "wb" modes, so we bail on anything
            // else.
            //
            static const char read_mode_string[] = "rb";
            static const char write_mode_string[] = "wb";
            const char * mode_string = 0;
            if (mode == (ios::binary | ios::in)) {
                mode_string = read_mode_string;
            } else if (   (mode == (ios::binary | ios::out))
                       || (mode == (ios::binary | ios::out | ios::trunc))) {
                mode_string = write_mode_string;
            } else {
                return 0;
            }

            this->file = gzopen(path, mode_string);
            if (!this->file) { return 0; }

            gzsetparams(this->file, comp_level, comp_strategy);
            return this;
        }

        filebuf * filebuf::close() {
            if (!this->file) { return 0; }
            gzclose(this->file);
            this->file = 0;
            return this;
        }

        int filebuf::underflow() {
            if (this->gptr() < this->egptr()) { return *this->gptr(); }

            //
            // Process the size of the putback area; use the number of characters read,
            // but at most four.
            //
            int num_putback = this->gptr() - this->eback();
            if (num_putback > lookback) { num_putback = lookback; }

            std::copy(this->gptr() - num_putback, this->gptr(),
                      this->buffer + (lookback - num_putback));

            //
            // Read new characters.
            //
            int num = gzread(this->file,
                             this->buffer + lookback,
                             filebuf::buffer_size - lookback);

            if (num <= 0) { return EOF; } // Error condition or end of file.

            //
            // Reset the buffer pointers.
            //
            this->setg(buffer + (lookback - num_putback), // Beginning of putback area.
                       buffer + lookback,                 // Read position.
                       buffer + lookback + num);          // End of buffer.

            //
            // Return the next character.
            //
            return *this->gptr();
        }

        int filebuf::overflow(int c) {
            //
            // This probably ought to be buffered, but this will do for now.
            //
            if (c != EOF) {
                if (gzputc(file, c) == -1) { return EOF; }
            }
            return c;
        }


        //
        // ifstream
        //

        ifstream::ifstream(): std::basic_istream<char>(&fbuf) {}

        ifstream::ifstream(const char * path, level lev, strategy strat):
                std::basic_istream<char>(&fbuf) {
            this->open(path, lev, strat);
        }

        ifstream::~ifstream() {}

        filebuf * ifstream::rdbuf() const {
            return const_cast<filebuf *>(&this->fbuf);
        }

        bool ifstream::is_open() const { return this->fbuf.is_open(); }

        void ifstream::open(const char * path, level lev, strategy strat) {
            using std::ios;
            if (!this->fbuf.open(path, ios::binary | ios::in, lev, strat)) {
#   ifdef _WIN32
                this->clear(failbit);
#   else
                this->setstate(failbit);
#   endif
            }
        }

        void ifstream::close() { this->fbuf.close(); }
    }
# endif // OPENVRML_ENABLE_GZIP
} // namespace

//
// Including a .cpp file is strange, but it's exactly what we want to do here.
// This usage lets us put the ANTLR parser in an unnamed namespace.
//
#include "Vrml97Parser.cpp"
} // Close "namespace openvrml", opened in Vrml97Parser.cpp.

// Max time in seconds between updates. Make this user
// setable to balance performance with cpu usage.
#ifndef DEFAULT_DELTA
#define DEFAULT_DELTA 0.5
#endif

/**
 * @namespace openvrml
 *
 * @brief The OpenVRML Runtime Library
 */
namespace openvrml {

/**
 * @var const double pi
 *
 * @brief pi
 */

/**
 * @var const double pi_2
 *
 * @brief pi/2
 */

/**
 * @var const double pi_4
 *
 * @brief pi/4
 */

/**
 * @var const double inv_pi
 *
 * @brief 1/pi
 */

/**
 * @class resource_istream
 *
 * @brief An abstract input stream for network resources.
 *
 * <code>resource_istream</code> inherits <code>std::istream</code>, adding
 * functions to get the URI and the MIME content type associated with the
 * stream. Users of the library must provide an implementation of this class,
 * to be returned from <code>openvrml::browser::do_get_resource</code>.
 */

/**
 * @brief Construct.
 *
 * @param streambuf a stream buffer.
 */
resource_istream::resource_istream(std::streambuf * streambuf):
    std::istream(streambuf)
{}

/**
 * @brief Destroy.
 */
resource_istream::~resource_istream()
{}

/**
 * @fn const std::string resource_istream::url() const throw ()
 *
 * @brief Get the URL associated with the stream.
 *
 * @return the URL associated with the stream.
 */

/**
 * @fn const std::string resource_istream::type() const throw ()
 *
 * @brief Get the MIME content type associated with the stream.
 *
 * @return the MIME content type associated with the stream.
 */


/**
 * @class invalid_vrml
 *
 * @brief Exception thrown when the parser fails due to errors in the VRML
 *      input.
 */

/**
 * @var const std::string invalid_vrml::url
 *
 * @brief Resource identifier.
 */

/**
 * @var size_t invalid_vrml::line
 *
 * @brief Line number.
 */

/**
 * @var size_t invalid_vrml::column
 *
 * @brief Column number.
 */

/**
 * @brief Construct.
 *
 * @param url       resource identifier of the stream.
 * @param line      line number where the error was detected.
 * @param column    column number where the error was detected.
 * @param message   description of the error.
 */
invalid_vrml::invalid_vrml(const std::string & url,
                           const size_t line,
                           const size_t column,
                           const std::string & message):
    std::runtime_error(message),
    url(url),
    line(line),
    column(column)
{}

/**
 * @brief Destroy.
 */
invalid_vrml::~invalid_vrml() throw ()
{}


/**
 * @class viewer_in_use
 *
 * @brief Exception thrown when attempting to associate a <code>viewer</code>
 *        with a <code>browser</code> when the <code>viewer</code> is already
 *        associated with a <code>browser</code>.
 */

/**
 * @brief Construct.
 */
viewer_in_use::viewer_in_use():
    std::invalid_argument("viewer in use")
{}

/**
 * @brief Destroy.
 */
viewer_in_use::~viewer_in_use() throw ()
{}


/**
 * @class browser
 *
 * @brief Encapsulates a VRML browser.
 */

/**
 * @var browser::Vrml97Parser
 *
 * @brief VRML97 parser generated by ANTLR.
 */

/**
 * @var browser::vrml97_root_scope
 *
 * @brief Root scope that is initialized with the VRML97 node types.
 */

/**
 * @internal
 *
 * @class browser::node_class_map
 *
 * @brief The map of <code>node_class</code>es.
 */

/**
 * @brief Construct.
 *
 * @param b the <code>browser</code>.
 */
browser::node_class_map::node_class_map(browser & b)
{
    using namespace vrml97_node;
    this->map_["urn:X-openvrml:node:Anchor"] =
        node_class_ptr(new anchor_class(b));
    this->map_["urn:X-openvrml:node:Appearance"] =
        node_class_ptr(new appearance_class(b));
    this->map_["urn:X-openvrml:node:AudioClip"] =
        node_class_ptr(new audio_clip_class(b));
    this->map_["urn:X-openvrml:node:Background"] =
        node_class_ptr(new background_class(b));
    this->map_["urn:X-openvrml:node:Billboard"] =
        node_class_ptr(new billboard_class(b));
    this->map_["urn:X-openvrml:node:Box"] =
        node_class_ptr(new box_class(b));
    this->map_["urn:X-openvrml:node:Collision"] =
        node_class_ptr(new collision_class(b));
    this->map_["urn:X-openvrml:node:Color"] =
        node_class_ptr(new color_class(b));
    this->map_["urn:X-openvrml:node:ColorInterpolator"] =
        node_class_ptr(new color_interpolator_class(b));
    this->map_["urn:X-openvrml:node:Cone"] =
        node_class_ptr(new cone_class(b));
    this->map_["urn:X-openvrml:node:Coordinate"] =
        node_class_ptr(new coordinate_class(b));
    this->map_["urn:X-openvrml:node:CoordinateInterpolator"] =
        node_class_ptr(new coordinate_interpolator_class(b));
    this->map_["urn:X-openvrml:node:Cylinder"] =
        node_class_ptr(new cylinder_class(b));
    this->map_["urn:X-openvrml:node:CylinderSensor"] =
        node_class_ptr(new cylinder_sensor_class(b));
    this->map_["urn:X-openvrml:node:DirectionalLight"] =
        node_class_ptr(new directional_light_class(b));
    this->map_["urn:X-openvrml:node:ElevationGrid"] =
        node_class_ptr(new elevation_grid_class(b));
    this->map_["urn:X-openvrml:node:Extrusion"] =
        node_class_ptr(new extrusion_class(b));
    this->map_["urn:X-openvrml:node:Fog"] =
        node_class_ptr(new fog_class(b));
    this->map_["urn:X-openvrml:node:FontStyle"] =
        node_class_ptr(new font_style_class(b));
    this->map_["urn:X-openvrml:node:Group"] =
        node_class_ptr(new group_class(b));
    this->map_["urn:X-openvrml:node:ImageTexture"] =
        node_class_ptr(new image_texture_class(b));
    this->map_["urn:X-openvrml:node:IndexedFaceSet"] =
        node_class_ptr(new indexed_face_set_class(b));
    this->map_["urn:X-openvrml:node:IndexedLineSet"] =
        node_class_ptr(new indexed_line_set_class(b));
    this->map_["urn:X-openvrml:node:Inline"] =
        node_class_ptr(new inline_class(b));
    this->map_["urn:X-openvrml:node:LOD"] =
        node_class_ptr(new lod_class(b));
    this->map_["urn:X-openvrml:node:Material"] =
        node_class_ptr(new material_class(b));
    this->map_["urn:X-openvrml:node:MovieTexture"] =
        node_class_ptr(new movie_texture_class(b));
    this->map_["urn:X-openvrml:node:NavigationInfo"] =
        node_class_ptr(new navigation_info_class(b));
    this->map_["urn:X-openvrml:node:Normal"] =
        node_class_ptr(new normal_class(b));
    this->map_["urn:X-openvrml:node:NormalInterpolator"] =
        node_class_ptr(new normal_interpolator_class(b));
    this->map_["urn:X-openvrml:node:OrientationInterpolator"] =
        node_class_ptr(new orientation_interpolator_class(b));
    this->map_["urn:X-openvrml:node:PixelTexture"] =
        node_class_ptr(new pixel_texture_class(b));
    this->map_["urn:X-openvrml:node:PlaneSensor"] =
        node_class_ptr(new plane_sensor_class(b));
    this->map_["urn:X-openvrml:node:PointLight"] =
        node_class_ptr(new point_light_class(b));
    this->map_["urn:X-openvrml:node:PointSet"] =
        node_class_ptr(new point_set_class(b));
    this->map_["urn:X-openvrml:node:PositionInterpolator"] =
        node_class_ptr(new position_interpolator_class(b));
    this->map_["urn:X-openvrml:node:ProximitySensor"] =
        node_class_ptr(new proximity_sensor_class(b));
    this->map_["urn:X-openvrml:node:ScalarInterpolator"] =
        node_class_ptr(new scalar_interpolator_class(b));
    this->map_["urn:X-openvrml:node:Shape"] =
        node_class_ptr(new shape_class(b));
    this->map_["urn:X-openvrml:node:Sound"] =
        node_class_ptr(new sound_class(b));
    this->map_["urn:X-openvrml:node:Sphere"] =
        node_class_ptr(new sphere_class(b));
    this->map_["urn:X-openvrml:node:SphereSensor"] =
        node_class_ptr(new sphere_sensor_class(b));
    this->map_["urn:X-openvrml:node:SpotLight"] =
        node_class_ptr(new spot_light_class(b));
    this->map_["urn:X-openvrml:node:Switch"] =
        node_class_ptr(new switch_class(b));
    this->map_["urn:X-openvrml:node:Text"] =
        node_class_ptr(new text_class(b));
    this->map_["urn:X-openvrml:node:TextureCoordinate"] =
        node_class_ptr(new texture_coordinate_class(b));
    this->map_["urn:X-openvrml:node:TextureTransform"] =
        node_class_ptr(new texture_transform_class(b));
    this->map_["urn:X-openvrml:node:TimeSensor"] =
        node_class_ptr(new time_sensor_class(b));
    this->map_["urn:X-openvrml:node:TouchSensor"] =
        node_class_ptr(new touch_sensor_class(b));
    this->map_["urn:X-openvrml:node:Transform"] =
        node_class_ptr(new transform_class(b));
    this->map_["urn:X-openvrml:node:Viewpoint"] =
        node_class_ptr(new viewpoint_class(b));
    this->map_["urn:X-openvrml:node:VisibilitySensor"] =
        node_class_ptr(new visibility_sensor_class(b));
    this->map_["urn:X-openvrml:node:WorldInfo"] =
        node_class_ptr(new world_info_class(b));
}

/**
 * @brief Assign.
 *
 * @param map   the value to assign.
 */
browser::node_class_map &
browser::node_class_map::operator=(const node_class_map & map)
{
    boost::mutex::scoped_lock my_lock(this->mutex_), map_lock(map.mutex_);
    map_t temp(map.map_);
    swap(this->map_, temp);
    return *this;
}

namespace {
    typedef std::map<std::string, node_class_ptr> node_class_map_t;

    struct init_node_class : std::unary_function<void,
                                                 node_class_map_t::value_type>
    {
        init_node_class(viewpoint_node * initial_viewpoint, const double time)
            throw ():
            initial_viewpoint_(initial_viewpoint),
            time_(time)
        {}

        void operator()(const node_class_map_t::value_type & value) const
            throw ()
        {
            assert(value.second);
            value.second->initialize(this->initial_viewpoint_, this->time_);
        }

    private:
        viewpoint_node * initial_viewpoint_;
        double time_;
    };
}

/**
 * @brief Initialize the <code>node_class</code>es.
 *
 * @param initial_viewpoint the viewpoint_node that should be initially active.
 * @param timestamp         the current time.
 */
void browser::node_class_map::init(viewpoint_node * initial_viewpoint,
                                   const double timestamp)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    for_each(this->map_.begin(), this->map_.end(),
             init_node_class(initial_viewpoint, timestamp));
}

/**
 * @brief Insert a <code>node_class</code>.
 *
 * This operation will "fail" silently. That is, if a <code>node_class</code>
 * corresponding to @p id already exists in the map, the existing element will
 * simply be returned.
 *
 * @param id            the implementation identifier.
 * @param node_class    a <code>node_class</code>.
 *
 * @return the element in the node_class_map corresponding to @p id.
 */
const node_class_ptr
browser::node_class_map::insert(const std::string & id,
                                const node_class_ptr & node_class)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    return this->map_.insert(make_pair(id, node_class)).first->second;
}

/**
 * @brief Find a <code>node_class</code>.
 *
 * @param id    an implementation id.
 *
 * @return the <code>node_class</code> corresponding to @p id, or a null
 *         pointer if no such <code>node_class</code> exists in the map.
 */
const node_class_ptr
browser::node_class_map::find(const std::string & id) const
{
    const map_t::const_iterator pos = this->map_.find(id);
    return (pos != this->map_.end())
        ? pos->second
        : node_class_ptr();
}

namespace {

    struct render_node_class :
            std::unary_function<void, node_class_map_t::value_type> {
        explicit render_node_class(openvrml::viewer & viewer):
            viewer(&viewer)
        {}

        void operator()(const node_class_map_t::value_type & value) const
        {
            value.second->render(*this->viewer);
        }

    private:
        openvrml::viewer * viewer;
    };
}

/**
 * @brief Render the <code>node_class</code>es.
 *
 * @param v a viewer.
 */
void browser::node_class_map::render(openvrml::viewer & v)
{
    boost::mutex::scoped_lock lock(this->mutex_);
    for_each(this->map_.begin(), this->map_.end(), render_node_class(v));
}

/**
 * @enum browser::cb_reason
 *
 * @brief Valid reasons for browser callback.
 */

/**
 * @var browser::cb_reason browser::destroy_world_id
 *
 * @brief Destroy the world.
 */

/**
 * @var browser::cb_reason browser::replace_world_id
 *
 * @brief Replace the world.
 */

/**
 * @typedef browser::scene_cb
 *
 * @brief A pointer to a browser callback function.
 *
 * The callback function provoides a way to let the app know when a world is
 * loaded, changed, etc.
 */

/**
 * @var std::auto_ptr<null_node_class> browser::null_node_class_
 *
 * @brief "Null" class object for default nodes (e.g., default_viewpoint).
 */

/**
 * @var std::auto_ptr<null_node_type> browser::null_node_type_
 *
 * @brief "Null" type object for default nodes (e.g., default_viewpoint).
 */

/**
 * @var browser::node_class_map browser::node_class_map_
 *
 * @brief A map of URIs to node implementations.
 */

/**
 * @var script_node_class browser::script_node_class_
 *
 * @brief node_class for Script nodes in the browser.
 */

/**
 * @var scene * browser::scene_
 *
 * @brief Pointer to the root scene.
 */

/**
 * @var node_ptr browser::default_viewpoint_
 *
 * @brief The "default" viewpoint_node used when no viewpoint_node in the scene
 *        is bound.
 */

/**
 * @var viewpoint_node * browser::active_viewpoint_
 *
 * @brief The currently "active" viewpoint_node.
 */

/**
 * @var node_ptr browser::default_navigation_info_
 *
 * @brief The "default" navigation_info_node used when no navigation_info_node
 *        in the scene is bound.
 */

/**
 * @var navigation_info_node * browser::active_navigation_info_
 *
 * @brief The currently "active" navigation_info_node.
 */

/**
 * @var std::list<viewpoint_node *> browser::viewpoint_list
 *
 * @brief A list of all the Viewpoint nodes in the browser.
 */

/**
 * @typedef browser::bind_stack_t
 *
 * @brief A list of bound nodes.
 */

/**
 * @var browser::bind_stack_t browser::navigation_info_stack
 *
 * @brief The stack of bound NavigationInfo nodes.
 */

/**
 * @var std::list<node *> browser::navigation_infos
 *
 * @brief A list of all the NavigationInfo nodes in the browser.
 */

/**
 * @var std::list<node *> browser::scoped_lights
 *
 * @brief A list of all the scoped light nodes in the browser.
 */

/**
 * @var std::list<script_node *> browser::scripts
 *
 * @brief A list of all the Script nodes in the browser.
 */

/**
 * @var std::list<node *> browser::timers
 *
 * @brief A list of all the TimeSensor nodes in the browser.
 */

/**
 * @var std::list<node *> browser::audio_clips
 *
 * @brief A list of all the AudioClip nodes in the browser.
 */

/**
 * @var std::list<node *> browser::movies
 *
 * @brief A list of all the MovieTexture nodes in the browser.
 */

/**
 * @var bool browser::modified_
 *
 * @brief Flag to indicate whether the browser has been modified.
 */

/**
 * @var bool browser::new_view
 *
 * @brief Flag to indicate if the user has changed to a new view.
 */

/**
 * @var double browser::delta_time
 *
 * @brief Time elapsed since the last update.
 */

/**
 * @var openvrml::viewer * browser::viewer_
 *
 * @brief The current <code>viewer</code>.
 */

/**
 * @typedef browser::scene_cb_list_t
 *
 * @brief List of functions to call when the world is changed.
 */

/**
 * @struct browser::event
 *
 * @brief An event.
 *
 * An event has a value and a destination, and is associated with a time.
 */

/**
 * @var double browser::event::timestamp
 *
 * @brief The timestamp of the event.
 */

/**
 * @var field_value * browser::event::value
 *
 * @brief The value associated with the event.
 */

/**
 * @var node_ptr browser::event::to_node
 *
 * @brief The node the event is going to.
 */

/**
 * @var std::string browser::event::to_eventin
 *
 * @brief The eventIn of @a to_node the event is going to.
 */

/**
 * @var browser::scene_cb_list_t browser::scene_callbacks
 *
 * @brief List of functions to call when the world is changed.
 */

/**
 * @var double browser::frame_rate_
 *
 * @brief Frame rate.
 */

/**
 * @var browser::max_events
 *
 * @brief The maximum number of events which may be queued.
 *
 * Each browser can have a limited number of pending events.
 * Repeatedly allocating/freeing events is slow (it would be
 * nice to get rid of the field cloning, too), and if there are
 * so many events pending, we are probably running too slow to
 * handle them effectively anyway.
 */

/**
 * @var browser::event browser::event_mem
 *
 * @brief The event queue.
 *
 * @todo The event queue ought to be sorted by timestamp.
 */

/**
 * @var size_t browser::first_event
 *
 * @brief Index of the first pending event.
 */

/**
 * @var size_t browser::last_event
 *
 * @brief Index of the last pending event.
 */

/**
 * @brief Get the current time.
 */
double browser::current_time() throw ()
{
    double currentTime;
# ifdef _WIN32
    _timeb timebuffer;
    _ftime(&timebuffer);
    currentTime = double(timebuffer.time) + 1.0e-3 * double(timebuffer.millitm);
# else
    timeval tv;
    const int result = gettimeofday(&tv, 0);
    assert(result == 0);

    currentTime = double(tv.tv_sec) + 1.0e-6 * double(tv.tv_usec);
# endif
    return currentTime;
}

/**
 * @var std::ostream & browser::out
 *
 * @brief Output stream, generally for console output.
 */

/**
 * @var std::ostream & browser::err
 *
 * @brief Error output stream.
 */

/**
 * @var bool browser::flags_need_updating
 *
 * @brief Set by node::setBVolumeDirty on any node in this browser graph,
 *      cleared by update_flags.
 *
 * @c true if the bvolume dirty flag has been set on a node in the
 * browser graph, but has not yet been propegated to that node's
 * ancestors.
 */

/**
 * @brief Constructor.
 *
 * @param out   output stream for console output.
 * @param err   output stream for error console output.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
browser::browser(std::ostream & out, std::ostream & err)
    throw (std::bad_alloc):
    null_node_class_(new null_node_class(*this)),
    null_node_type_(new null_node_type(*null_node_class_)),
    node_class_map_(*this),
    script_node_class_(*this),
    scene_(new scene(*this)),
    default_viewpoint_(new default_viewpoint(*null_node_type_)),
    active_viewpoint_(node_cast<viewpoint_node *>(default_viewpoint_.get())),
    default_navigation_info_(new default_navigation_info(*null_node_type_)),
    active_navigation_info_(
        node_cast<navigation_info_node *>(default_navigation_info_.get())),
    modified_(false),
    new_view(false),
    delta_time(DEFAULT_DELTA),
    viewer_(0),
    frame_rate_(0.0),
    first_event(0),
    last_event(0),
    out(out),
    err(err),
    flags_need_updating(false)
{
    assert(this->active_viewpoint_);
    assert(this->active_navigation_info_);
}

/**
 * @brief Destructor.
 */
browser::~browser() throw ()
{
    const double now = browser::current_time();

    if (this->scene_) { this->scene_->shutdown(now); }
    assert(this->viewpoint_list.empty());
    assert(this->scoped_lights.empty());
    assert(this->scripts.empty());
    assert(this->timers.empty());
    assert(this->audio_clips.empty());
    assert(this->movies.empty());
}

/**
 * @brief Get the root nodes for the browser.
 *
 * @return the root nodes for the browser.
 */
const std::vector<node_ptr> & browser::root_nodes() const throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(this->scene_);
    return this->scene_->nodes();
}

/**
 * @brief Get the path to a node in the scene graph.
 *
 * @param n  the objective node.
 *
 * @return the path to @p node, starting with a root node, and ending with
 *         @p node. If @p node is not in the scene graph, the returned
 *         node_path is empty.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
const node_path browser::find_node(const node & n) const
    throw (std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(this->scene_);

    class FindNodeTraverser : public node_traverser {
        const node & objectiveNode;
        node_path & nodePath;

    public:
        FindNodeTraverser(const node & objectiveNode, node_path & nodePath)
            throw ():
            objectiveNode(objectiveNode),
            nodePath(nodePath)
        {}

        virtual ~FindNodeTraverser() throw()
        {}

    private:
        virtual void on_entering(node & n) throw (std::bad_alloc)
        {
            this->nodePath.push_back(&n);
            if (&n == &this->objectiveNode) { this->halt_traversal(); }
        }

        virtual void on_leaving(node & n) throw ()
        {
            if (!this->halted()) { this->nodePath.pop_back(); }
        }
    };

    node_path nodePath;
    FindNodeTraverser(n, nodePath).traverse(this->scene_->nodes());
    return nodePath;
}

/**
 * @brief Get the active viewpoint_node.
 *
 * The active viewpoint_node is the one currently associated with the user
 * view.
 *
 * @return the active viewpoint_node.
 */
viewpoint_node & browser::active_viewpoint() const throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return *this->active_viewpoint_;
}

/**
 * @brief Set the active viewpoint_node.
 *
 * @param viewpoint a viewpoint_node.
 */
void browser::active_viewpoint(viewpoint_node & viewpoint) throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    this->active_viewpoint_ = &viewpoint;
}

/**
 * @brief Reset the active viewpoint_node to the default.
 */
void browser::reset_default_viewpoint() throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(this->default_viewpoint_);
    this->active_viewpoint_ =
        node_cast<viewpoint_node *>(this->default_viewpoint_.get());
    assert(this->active_viewpoint_);
}

/**
 * @brief Get the active navigation_info_node.
 *
 * The active navigation_info_node is the one currently associated with the
 * user view.
 *
 * @return the active navigation_info_node.
 */
navigation_info_node & browser::active_navigation_info() const throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return *this->active_navigation_info_;
}

/**
 * @brief Set the active navigation_info_node.
 *
 * @param nav_info a navigation_info_node.
 */
void browser::active_navigation_info(navigation_info_node & nav_info) throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    this->active_navigation_info_ = &nav_info;
}

/**
 * @brief Reset the active navigation_info_node to the default.
 */
void browser::reset_default_navigation_info() throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(this->default_navigation_info_);
    this->active_navigation_info_ =
        node_cast<navigation_info_node *>(
            this->default_navigation_info_.get());
    assert(this->active_navigation_info_);
}

/**
 * @brief Add a viewpoint_node to the list of viewpoint_nodes for the browser.
 *
 * @param viewpoint a viewpoint_node.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 *
 * @pre @p viewpoint is not in the list of viewpoint_nodes for the browser.
 */
void browser::add_viewpoint(viewpoint_node & viewpoint) throw (std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(std::find(this->viewpoint_list.begin(), this->viewpoint_list.end(),
                     &viewpoint) == this->viewpoint_list.end());
    this->viewpoint_list.push_back(&viewpoint);
}

/**
 * @brief Remove a viewpoint_node from the list of viewpoint_nodes for the
 *      browser.
 *
 * @param viewpoint a viewpoint_node.
 *
 * @pre @p viewpoint is in the list of viewpoint_nodes for the browser.
 */
void browser::remove_viewpoint(viewpoint_node & viewpoint) throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(!this->viewpoint_list.empty());
    typedef std::list<viewpoint_node *> viewpoint_list_t;
    const viewpoint_list_t::iterator end = this->viewpoint_list.end();
    const viewpoint_list_t::iterator pos =
            std::find(this->viewpoint_list.begin(), end, &viewpoint);
    assert(pos != end);
    this->viewpoint_list.erase(pos);
}

/**
 * @brief Get the list of @link viewpoint_node viewpoint_nodes@endlink for the
 *      world.
 *
 * @return the list of @link viewpoint_node viewpoint_nodes@endlink for the
 *      world.
 */
const std::list<viewpoint_node *> & browser::viewpoints() const throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return this->viewpoint_list;
}

/**
 * @brief Set the current <code>viewer</code>.
 *
 * @param v <code>viewer</code>.
 *
 * @exception viewer_in_use if @p v is already associated with a
 *                          <code>browser</code>.
 */
void browser::viewer(openvrml::viewer * v) throw (viewer_in_use)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    if (v && v->browser_) { throw viewer_in_use(); }
    if (this->viewer_) { this->viewer_->browser_ = 0; }
    this->viewer_ = v;
    if (v) { v->browser_ = this; }
}

/**
 * @brief The current <code>viewer</code>.
 *
 * @return the current <code>viewer</code>.
 */
viewer * browser::viewer() const throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return this->viewer_;
}

/**
 * @brief Fetch a network resource.
 *
 * @param uri   a Uniform Resource Identifier.
 *
 * @return the requested resource as a stream.
 */
std::auto_ptr<openvrml::resource_istream>
browser::get_resource(const std::string & uri)
{
    return this->do_get_resource(uri);
}

/**
 * @brief Get the browser name.
 *
 * @return "OpenVRML"
 *
 * Specific browsers may wish to override this method.
 */
const char * browser::name() const throw ()
{
    return "OpenVRML";
}

/**
 * @brief Get the browser version.
 *
 * @return the version of openvrml.
 *
 * Specific browsers may wish to override this method.
 */
const char * browser::version() const throw ()
{
    return PACKAGE_VERSION;
}

/**
 * @brief Get the average navigation speed in meters per second.
 *
 * @return the average navigation speed.
 */
float browser::current_speed()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    navigation_info_node & nav_info = this->active_navigation_info();
    return nav_info.speed();
}

/**
 * @brief Get the URI for the world.
 *
 * @return the URI for the world.
 */
const std::string browser::world_url() const throw (std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(this->scene_);
    return this->scene_->url(); // Throws std::bad_alloc.
}

/**
 * @brief Set the URI for the world.
 *
 * This function does nothing other than change the URI returned by
 * the browser::world_url accessor. It does not result in loading a new world.
 *
 * @param str   a valid URI.
 *
 * @exception invalid_url       if @p str is not a valid URI.
 * @exception std::bad_alloc    if memory allocation fails.
 */
void browser::world_url(const std::string & str)
    throw (invalid_url, std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(this->scene_);
    this->scene_->url(str);
}

/**
 * @brief Replace the root nodes of the world.
 *
 * @param nodes new root nodes for the world.
 */
void browser::replace_world(const std::vector<node_ptr> & nodes)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    const double now = browser::current_time();
    this->scene_->shutdown(now);
    this->scene_->nodes(nodes);
    this->scene_->initialize(now);
    //
    // Initialize the node_classes.
    //
    static viewpoint_node * const initial_viewpoint = 0;
    this->node_class_map_.init(initial_viewpoint, now);
    this->modified(true);
    this->new_view = true; // Force resetUserNav
}

/**
 * @brief Load a VRML world into the browser.
 *
 * @param url       a URI.
 * @param parameter parameters for @p url.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void browser::load_url(const std::vector<std::string> & url,
                       const std::vector<std::string> & parameter)
    throw (std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);

    class root_scene : public openvrml::scene {
    public:
        explicit root_scene(openvrml::browser & b):
            scene(b, 0)
        {}

    private:
        virtual void scene_loaded()
        {
            boost::recursive_mutex::scoped_lock lock(this->browser().mutex_);

            try {
                using std::string;

                const double now = browser::current_time();
                this->initialize(now);

                //
                // Get the initial viewpoint_node, if any was specified.
                //
                viewpoint_node * initial_viewpoint = 0;
                const string viewpoint_node_id = uri(this->url()).fragment();
                if (!viewpoint_node_id.empty()) {
                    if (!this->nodes().empty()) {
                        const node_ptr & n = this->nodes()[0];
                        if (n) {
                            node * const vp =
                                n->scope()->find_node(viewpoint_node_id);
                            initial_viewpoint =
                                dynamic_cast<viewpoint_node *>(vp);
                        }
                    }
                }

                //
                // Initialize the node_classes.
                //
                this->browser().node_class_map_.init(initial_viewpoint, now);

                if (this->browser().active_viewpoint_
                    != node_cast<viewpoint_node *>(
                        this->browser().default_viewpoint_.get())) {
                    event_listener & listener =
                        this->browser().active_viewpoint_
                        ->event_listener("set_bind");
                    assert(dynamic_cast<sfbool_listener *>(&listener));
                    static_cast<sfbool_listener &>(listener)
                        .process_event(sfbool(true), now);
                }
            } catch (std::exception & ex) {
                std::ostream & err = this->browser().err;
                err << ex.what() << std::endl;
            }
            this->browser().modified(true);
            this->browser().new_view = true; // Force resetUserNav
        }
    };

    using std::for_each;
    using std::list;

    const double now = browser::current_time();

    //
    // Clear out the current scene.
    //
    if (this->scene_) { this->scene_->shutdown(now); }
    this->scene_.reset();
    this->active_viewpoint_ =
        node_cast<viewpoint_node *>(this->default_viewpoint_.get());
    assert(this->viewpoint_list.empty());
    assert(this->scoped_lights.empty());
    assert(this->scripts.empty());
    assert(this->timers.empty());
    assert(this->audio_clips.empty());
    assert(this->movies.empty());

    //
    // Create the new scene.
    //
    node_class_map new_map(*this);
    this->node_class_map_ = new_map;
    this->scene_.reset(new root_scene(*this));
    this->scene_->load(url);
}

/**
 * @brief Send a string to the user interface.
 *
 * The default implementation of this method simply prints @p description to
 * @a out.  Subclasses can override this method to direct messages to an
 * application's UI; for instance, a status bar.
 *
 * @param description   a string.
 */
void browser::description(const std::string & description)
{
    this->out << description << std::endl;
}

/**
 * @brief Generate nodes from a stream of VRML syntax.
 *
 * In addition to the exceptions listed, this method may throw any
 * exception that may result from reading the input stream.
 *
 * @param in    an input stream.
 *
 * @return the root nodes generated from @p in.
 *
 * @exception invalid_vrml      if @p in has invalid VRML syntax.
 * @exception std::bad_alloc    if memory allocation fails.
 */
const std::vector<node_ptr> browser::create_vrml_from_stream(std::istream & in)
{
    std::vector<node_ptr> nodes;
    try {
        Vrml97Scanner scanner(in);
        Vrml97Parser parser(scanner, "");
        parser.vrmlScene(*this, nodes);
    } catch (antlr::RecognitionException & ex) {
        throw invalid_vrml(ex.getFilename(),
                           ex.getLine(),
                           ex.getColumn(),
                           ex.getMessage());
    } catch (antlr::ANTLRException & ex) {
        throw std::runtime_error(ex.getMessage());
    }
    return nodes;
}

/**
 * @todo Implement me!
 */
void browser::create_vrml_from_url(const std::vector<std::string> & url,
                                   const node_ptr & node,
                                   const std::string & event)
{}

/**
 * @brief Execute browser callback functions.
 *
 * @param reason    the cb_reason to pass to the callback functions.
 */
void browser::do_callbacks(const cb_reason reason)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    scene_cb_list_t::iterator cb, cbend = this->scene_callbacks.end();
    for (cb = this->scene_callbacks.begin(); cb != cbend; ++cb) {
        (*cb)(reason);
    }
}

/**
 * @brief Add a callback function to be called when the world changes.
 *
 * @param cb    a browser callback function.
 */
void browser::add_world_changed_callback(const scene_cb cb)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    this->scene_callbacks.push_front(cb);
}

/**
 * @brief Get the current frame rate.
 *
 * @return the current frame rate.
 */
double browser::frame_rate() const
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return this->frame_rate_;
}

/**
 * @brief Queue an event for a node.
 *
 * Current events are in the array @a event_mem[first_event, last_event). If
 * @a first_event == @a last_event, the queue is empty. There is a fixed
 * maximum number of events. If we are so far behind that the queue is filled,
 * the oldest events get overwritten.
 */
void browser::queue_event(double timestamp,
                          field_value * value,
                          const node_ptr & to_node,
                          const std::string & to_eventin)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    event * e = &this->event_mem[this->last_event];
    e->timestamp = timestamp;
    e->value = value;
    e->to_node = to_node;
    e->to_eventin = to_eventin;
    this->last_event = (this->last_event + 1) % max_events;

    // If the event queue is full, discard the oldest (in terms of when it
    // was put on the queue, not necessarily in terms of earliest timestamp).
    if (this->last_event == this->first_event) {
        e = &this->event_mem[this->last_event];
        delete e->value;
        this->first_event = (this->first_event + 1) % max_events;
    }
}

/**
 * @brief Check if any events are waiting to be distributed.
 *
 * @return @c true if there are pending events, @c false otherwise.
 */
bool browser::events_pending()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return this->first_event != this->last_event;
}


/**
 * @brief Discard all pending events
 */
void browser::flush_events()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    while (this->first_event != this->last_event) {
        event *e = &this->event_mem[this->first_event];
        this->first_event = (this->first_event + 1) % max_events;
        delete e->value;
    }
}

/**
 * Called by the viewer when the cursor passes over, clicks, drags, or
 * releases a sensitive object (an Anchor or another grouping node with
 * an enabled TouchSensor child).
 */
void browser::sensitive_event(node * const n,
                              const double timestamp,
                              const bool is_over,
                              const bool is_active,
                              double * const point)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    if (n) {
        vrml97_node::anchor_node * a = n->to_anchor();
        if (a) {
            //
            // This should really be (isOver && !isActive && n->wasActive)
            // (ie, button up over the anchor after button down over the
            // anchor)
            //
            if (is_active && is_over) { a->activate(); }
        } else {
            //
            // The parent grouping node is registered for Touch/Drag Sensors.
            //
            grouping_node * const g = node_cast<grouping_node *>(n);
            if (g) {
                g->activate(timestamp, is_over, is_active, point);
                this->modified(true);
            }
        }
    }
}

namespace {
    template <typename T>
    struct UpdatePolledNode_ : std::unary_function<T, void> {
        explicit UpdatePolledNode_(double time): time(time) {}

        void operator()(T node) const { node->update(time); }

    private:
        double time;
    };

    template <typename FieldValue>
    void process_event(event_listener & listener,
                       const field_value & value,
                       const double timestamp)
    {
        assert(dynamic_cast<field_value_listener<FieldValue> *>(&listener));
        assert(dynamic_cast<const FieldValue *>(&value));
        static_cast<field_value_listener<FieldValue> &>(listener)
            .process_event(static_cast<const FieldValue &>(value), timestamp);
    }
}

/**
 * @brief Process events (update the browser).
 *
 * This method should be called after each frame is rendered.
 *
 * @return @c true if the browser needs to be rerendered, @c false otherwise.
 */
bool browser::update(double current_time)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);

    if (current_time <= 0.0) { current_time = browser::current_time(); }

    this->delta_time = DEFAULT_DELTA;

    // Update each of the timers.
    std::list<node *>::iterator i, end = this->timers.end();
    for (i = this->timers.begin(); i != end; ++i) {
        vrml97_node::time_sensor_node * t = (*i)->to_time_sensor();
        if (t) { t->update(current_time); }
    }

    // Update each of the clips.
    end = this->audio_clips.end();
    for (i = this->audio_clips.begin(); i != end; ++i) {
        vrml97_node::audio_clip_node * c = (*i)->to_audio_clip();
        if (c) { c->update(current_time); }
    }

    // Update each of the movies.
    end = this->movies.end();
    for (i = this->movies.begin(); i != end; ++i) {
        vrml97_node::movie_texture_node * m = (*i)->to_movie_texture();
        if (m) { m->update(current_time); }
    }

    //
    // Update each of the scripts.
    //
    std::for_each(this->scripts.begin(), this->scripts.end(),
                  UpdatePolledNode_<script_node *>(current_time));

    // Pass along events to their destinations
    while (this->first_event != this->last_event) {
        event * const e = &this->event_mem[this->first_event];
        this->first_event = (this->first_event + 1) % max_events;

        event_listener & listener = e->to_node->event_listener(e->to_eventin);
        switch (e->value->type()) {
        case field_value::sfbool_id:
            process_event<sfbool>(listener, *e->value, e->timestamp);
            break;
        case field_value::sfcolor_id:
            process_event<sfcolor>(listener, *e->value, e->timestamp);
            break;
        case field_value::sffloat_id:
            process_event<sffloat>(listener, *e->value, e->timestamp);
            break;
        case field_value::sfimage_id:
            process_event<sfimage>(listener, *e->value, e->timestamp);
            break;
        case field_value::sfint32_id:
            process_event<sfint32>(listener, *e->value, e->timestamp);
            break;
        case field_value::sfnode_id:
            process_event<sfnode>(listener, *e->value, e->timestamp);
            break;
        case field_value::sfrotation_id:
            process_event<sfrotation>(listener, *e->value, e->timestamp);
            break;
        case field_value::sfstring_id:
            process_event<sfstring>(listener, *e->value, e->timestamp);
            break;
        case field_value::sftime_id:
            process_event<sftime>(listener, *e->value, e->timestamp);
            break;
        case field_value::sfvec2f_id:
            process_event<sfvec2f>(listener, *e->value, e->timestamp);
            break;
        case field_value::sfvec3f_id:
            process_event<sfvec3f>(listener, *e->value, e->timestamp);
            break;
        case field_value::mfcolor_id:
            process_event<mfcolor>(listener, *e->value, e->timestamp);
            break;
        case field_value::mffloat_id:
            process_event<mffloat>(listener, *e->value, e->timestamp);
            break;
        case field_value::mfint32_id:
            process_event<mfint32>(listener, *e->value, e->timestamp);
            break;
        case field_value::mfnode_id:
            process_event<mfnode>(listener, *e->value, e->timestamp);
            break;
        case field_value::mfrotation_id:
            process_event<mfrotation>(listener, *e->value, e->timestamp);
            break;
        case field_value::mfstring_id:
            process_event<mfstring>(listener, *e->value, e->timestamp);
            break;
        case field_value::mftime_id:
            process_event<mftime>(listener, *e->value, e->timestamp);
            break;
        case field_value::mfvec2f_id:
            process_event<mfvec2f>(listener, *e->value, e->timestamp);
            break;
        case field_value::mfvec3f_id:
            process_event<mfvec3f>(listener, *e->value, e->timestamp);
            break;
        default:
            assert(false);
        }

        delete e->value;
    }

    // Signal a redisplay if necessary
    return this->modified();
}

/**
 * @brief Indicate whether the headlight is on.
 *
 * @return @c true if the headlight is on; @c false otherwise.
 */
bool browser::headlight_on()
{
    navigation_info_node & nav_info = this->active_navigation_info();
    return nav_info.headlight();
}

/**
 * @brief Draw this browser into the specified viewer
 */
void browser::render()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    if (!this->viewer_) { return; }

    if (this->new_view) {
        this->viewer_->reset_user_navigation();
        this->new_view = false;
    }
    navigation_info_node & nav_info = this->active_navigation_info();
    const float avatarSize = nav_info.avatar_size().empty()
        ? 0.25
        : nav_info.avatar_size()[0];
    const float visibilityLimit = nav_info.visibility_limit();

    // Activate the headlight.
    // ambient is supposed to be 0 according to the spec...
    if (this->headlight_on()) {
        static const color color(1.0, 1.0, 1.0);
        static const vec3f direction(0.0, 0.0, -1.0);
        static const float ambientIntensity = 0.3f;
        static const float intensity = 1.0;

        this->viewer_->insert_dir_light(ambientIntensity,
                                        intensity,
                                        color,
                                        direction);
    }

    // sets the viewpoint transformation
    //
    const mat4f t = this->active_viewpoint_->user_view_transform()
                    * this->active_viewpoint_->transformation();
    vec3f position, scale;
    rotation orientation;
    t.transformation(position, orientation, scale);
    this->viewer_->set_viewpoint(position,
                                 orientation,
                                 this->active_viewpoint_->field_of_view(),
                                 avatarSize,
                                 visibilityLimit);

    this->node_class_map_.render(*this->viewer_);

    // Top level object

    this->viewer_->begin_object(0);
    mat4f modelview = t.inverse();
    rendering_context rc(bounding_volume::partial, modelview);
    rc.draw_bounding_spheres = true;

    // Do the browser-level lights (Points and Spots)
    std::list<node *>::iterator li, end = this->scoped_lights.end();
    for (li = this->scoped_lights.begin(); li != end; ++li) {
        vrml97_node::abstract_light_node * x = (*li)->to_light();
        if (x) { x->renderScoped(*this->viewer_); }
    }

    //
    // Render the nodes.  scene_ may be 0 if the world failed to load.
    //
    if (this->scene_) {
        this->scene_->render(*this->viewer_, rc);
    }

    this->viewer_->end_object();

    // This is actually one frame late...
    this->frame_rate_ = this->viewer_->frame_rate();

    this->modified(false);

    // If any events were generated during render (ugly...) do an update
    if (this->events_pending()) { this->delta(0.0); }
}

/**
 * @brief Indicate whether rendering is necessary.
 *
 * @param value @c true to indicate that the browser state has changed and
 *              rerendering is necessary; @c false once rendering has occurred.
 */
void browser::modified(const bool value)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    this->modified_ = value;
}

/**
 * @brief Check if the browser has been modified.
 *
 * @return @c true if the browser has been modified, @c false otherwise.
 */
bool browser::modified() const
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return this->modified_;
}

/**
 * @brief Set the time until the next update is needed.
 *
 * @param d a time interval.
 */
void browser::delta(const double d)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    if (d < this->delta_time) { this->delta_time = d; }
}

/**
 * @brief Get the time interval between browser updates.
 *
 * @return the time interval between browser updates.
 */
double browser::delta() const
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return this->delta_time;
}

/**
 * @brief Add a scoped light node to the browser.
 *
 * @param light a light node.
 *
 * @pre @p light is not in the list of light nodes for the browser.
 */
void browser::add_scoped_light(vrml97_node::abstract_light_node & light)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(std::find(this->scoped_lights.begin(), this->scoped_lights.end(),
                     &light) == this->scoped_lights.end());
    this->scoped_lights.push_back(&light);
}

/**
 * @brief Remove a scoped light node from the browser.
 *
 * @param light the light node to remove.
 *
 * @pre @p light is in the list of light nodes for the browser.
 */
void browser::remove_scoped_light(vrml97_node::abstract_light_node & light)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(!this->scoped_lights.empty());
    const std::list<node *>::iterator end = this->scoped_lights.end();
    const std::list<node *>::iterator pos =
            std::find(this->scoped_lights.begin(), end, &light);
    assert(pos != end);
    this->scoped_lights.erase(pos);
}

/**
 * @brief Add a MovieTexture node to the browser.
 *
 * @param movie a MovieTexture node.
 *
 * @pre @p movie is not in the list of MovieTexture nodes for the browser.
 */
void browser::add_movie(vrml97_node::movie_texture_node & movie) {
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(std::find(this->movies.begin(), this->movies.end(), &movie)
            == this->movies.end());
    this->movies.push_back(&movie);
}

/**
 * @brief Remove a movie_texture node from the browser.
 *
 * @param movie the movie_texture node to remove.
 *
 * @pre @p movie is in the list of movie_texture nodes for the browser.
 */
void browser::remove_movie(vrml97_node::movie_texture_node & movie)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(!this->movies.empty());
    const std::list<node *>::iterator end = this->movies.end();
    const std::list<node *>::iterator pos =
            std::find(this->movies.begin(), end, &movie);
    assert(pos != end);
    this->movies.erase(pos);
}

/**
 * @brief Add a Script node to the browser.
 *
 * @param script    a Script node.
 *
 * @pre @p script is not in the list of Script nodes for the browser.
 */
void browser::add_script(script_node & script)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(std::find(this->scripts.begin(), this->scripts.end(), &script)
            == this->scripts.end());
    this->scripts.push_back(&script);
}

/**
 * @brief Remove a Script node from the browser.
 *
 * @param script    the Script node to remove.
 *
 * @pre @p script is in the list of Script nodes for the browser.
 */
void browser::remove_script(script_node & script)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(!this->scripts.empty());
    typedef std::list<script_node *> script_node_list_t;
    const script_node_list_t::iterator end = this->scripts.end();
    const script_node_list_t::iterator pos =
            std::find(this->scripts.begin(), end, &script);
    assert(pos != end);
    this->scripts.erase(pos);
}

/**
 * @brief Add a TimeSensor node to the browser.
 *
 * @param timer a TimeSensor node.
 *
 * @pre @p timer is not in the list of TimeSensor nodes for the browser.
 */
void browser::add_time_sensor(vrml97_node::time_sensor_node & timer)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(std::find(this->timers.begin(), this->timers.end(), &timer)
           == this->timers.end());
    this->timers.push_back(&timer);
}

/**
 * @brief Remove a time_sensor node from the browser.
 *
 * @param timer the time_sensor node to remove.
 *
 * @pre @p timer is in the list of time_sensor nodes for the browser.
 */
void browser::remove_time_sensor(vrml97_node::time_sensor_node & timer)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(!this->timers.empty());
    const std::list<node *>::iterator end = this->timers.end();
    const std::list<node *>::iterator pos =
            std::find(this->timers.begin(), end, &timer);
    assert(pos != end);
    this->timers.erase(pos);
}


/**
 * @brief Add an AudioClip node to the browser.
 *
 * @param audio_clip    an audio_clip node.
 *
 * @pre @p audio_clip is not in the list of audio_clip nodes for the browser.
 */
void browser::add_audio_clip(vrml97_node::audio_clip_node & audio_clip)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(std::find(this->audio_clips.begin(), this->audio_clips.end(),
                     &audio_clip) == this->audio_clips.end());
    this->audio_clips.push_back(&audio_clip);
}

/**
 * @brief Remove an audio_clip node from the browser.
 *
 * @param audio_clip    the audio_clip node to remove.
 *
 * @pre @p audio_clip is in the list of audio_clip nodes for the browser.
 */
void browser::remove_audio_clip(vrml97_node::audio_clip_node & audio_clip)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    assert(!this->audio_clips.empty());
    const std::list<node *>::iterator end = this->audio_clips.end();
    const std::list<node *>::iterator pos =
            std::find(this->audio_clips.begin(), end, &audio_clip);
    assert(pos != end);
    this->audio_clips.erase(pos);
}


/**
 * @brief Propagate the bvolume dirty flag from children to ancestors.
 *
 * The invariant is that if a node's bounding volume is out of date,
 * then the bounding volumes of all that nodes's ancestors must be
 * out of date. However, Node does not maintain a parent pointer. So
 * we must do a traversal of the entire browser graph to do the propagation.
 *
 * @see node::setBVolumeDirty
 * @see node::isBVolumeDirty
 */
void browser::update_flags()
{
//  Node* root = this->getRoot();
//  root->updateModified(0x002);
}


/**
 * @class bad_url
 *
 * @brief Thrown when there is a problem resolving a URI.
 */

/**
 * @brief Construct.
 *
 * @param message   Informative text.
 */
bad_url::bad_url(const std::string & message):
    std::runtime_error(message)
{}

/**
 * @brief Destroy.
 */
bad_url::~bad_url() throw ()
{}


/**
 * @class invalid_url
 *
 * @brief Thrown when parsing a URI fails.
 */

/**
 * @brief Construct.
 */
invalid_url::invalid_url():
    bad_url("Invalid URI.")
{}

/**
 * @brief Destroy.
 */
invalid_url::~invalid_url() throw ()
{}


/**
 * @class unreachable_url
 *
 * @brief Thrown when a URI cannot be reached.
 */

/**
 * @brief Construct.
 */
unreachable_url::unreachable_url():
    bad_url("Unreachable URI.")
{}

/**
 * @brief Destroy.
 */
unreachable_url::~unreachable_url() throw ()
{}


/**
 * @class no_alternative_url
 *
 * @brief Exception thrown when no URI in an alternative URI list can be
 *        resolved.
 */

/**
 * @brief Construct.
 */
no_alternative_url::no_alternative_url():
    bad_url("No alternative URI could be resolved.")
{}

/**
 * @brief Destroy.
 */
no_alternative_url::~no_alternative_url() throw ()
{}


/**
 * @class scene
 *
 * @brief A scene in the VRML world.
 */

/**
 * @var mfnode scene::nodes_
 *
 * @brief The nodes for the scene.
 */

/**
 * @var const std::string scene::url_
 *
 * @brief The URI for the scene.
 *
 * @a uri may be a relative or an absolute reference.
 */

/**
 * @var browser & scene::browser_
 *
 * @brief A reference to the browser associated with the scene.
 */

/**
 * @var scene * const scene::parent_
 *
 * @brief A pointer to the parent scene.
 *
 * If the scene is the root scene, @a parent will be 0.
 */

namespace {

    const uri createFileURL(const uri & relative_uri) throw (std::bad_alloc)
    {
        assert(relative_uri.scheme().empty());

        using std::string;

        string result = "file://";
        uri result_uri;

        try {
# ifdef _WIN32
            //
            // _fullpath returns a string starting with the drive letter; for
            // the URL, the path must begin with a '/'. So we simply put one at
            // the beginning of the buffer.
            //
            char buffer[_MAX_PATH] = { '/' };
            char * resolvedPath =
                _fullpath(buffer + 1, relative_uri.path().c_str(), _MAX_PATH);
            if (!resolvedPath) {
                //
                // XXX Failed; need to check errno to see what we should throw.
                //
                return uri(result);
            }
            std::replace_if(resolvedPath,
                            resolvedPath + strlen(resolvedPath) + 1,
                            bind2nd(equal_to<char>(), '\\'), '/');
            --resolvedPath;
            assert(resolvedPath == buffer);
# else
            char buffer[PATH_MAX];
            const char * resolvedPath = realpath(relative_uri.path().c_str(),
                                                 buffer);
            if (!resolvedPath) {
                //
                // XXX Failed; need to check errno to see what we should throw.
                //
                return uri(result);
            }
# endif

            result += resolvedPath;

            const string query = relative_uri.query();
            if (!query.empty()) { result += '?' + query; }

            const string fragment = relative_uri.fragment();
            if (!fragment.empty()) { result += '#' + fragment; }

            result_uri = uri(result);

        } catch (invalid_url &) {
            assert(false); // If we constructed a bad URI, something is wrong.
        }

        return result_uri;
    }
}

/**
 * @brief Construct.
 *
 * @param browser   the browser associated with the scene.
 * @param parent    the parent scene.
 */
scene::scene(openvrml::browser & browser, scene * parent) throw ():
    browser_(browser),
    parent_(parent)
{}

/**
 * @brief Destroy.
 */
scene::~scene() throw ()
{}

/**
 * @brief Get the associated <code>browser</code>.
 *
 * @return the associated <code>browser</code>.
 */
openvrml::browser & scene::browser() throw ()
{
    return this->browser_;
}

/**
 * @brief Get the parent <code>scene</code>.
 *
 * @return the parent <code>scene</code>, or 0 if this is the root
 *         <code>scene</code>.
 */
scene * scene::parent() const throw ()
{
    return this->parent_;
}

struct scene::load_scene {

    load_scene(openvrml::scene & scene,
               const std::vector<std::string> & url):
        scene_(&scene),
        url_(&url)
    {}
        
    void operator()() const
    {
        using std::string;
        using std::vector;

        openvrml::scene & scene = *this->scene_;
        const vector<string> & url = *this->url_;

        string absolute_uri;
        vector<node_ptr> nodes;
        for (vector<string>::size_type i = 0; i < url.size(); ++i) {
            try {
                using std::auto_ptr;

                //
                // Throw invalid_url if it isn't a valid URI.
                //
                uri test_uri(url[i]);

                const bool absolute = !test_uri.scheme().empty();
                if (absolute) {
                    absolute_uri = test_uri;
                } else if (!scene.parent()) {
                    //
                    // If we have a relative reference and the parent is
                    // null, then assume the reference is to the local file
                    // system:  convert the relative reference to a file
                    // URL.
                    //
                    absolute_uri = createFileURL(test_uri);
                } else {
                    //
                    // If we have a relative URI and parent is not null,
                    // try to resolve the relative reference against the
                    // parent's URI.
                    //
                    const uri parent_uri(scene.parent()->url());
                    absolute_uri = test_uri.resolve_against(parent_uri);
                }

                auto_ptr<resource_istream>
                    in(scene.browser().get_resource(absolute_uri));
                if (!(*in)) { throw unreachable_url(); }
                try {
                    Vrml97Scanner scanner(*in);
                    Vrml97Parser parser(scanner, absolute_uri);
                    parser.vrmlScene(scene.browser(), nodes);
                } catch (antlr::RecognitionException & ex) {
                    throw invalid_vrml(ex.getFilename(),
                                       ex.getLine(),
                                       ex.getColumn(),
                                       ex.getMessage());
                } catch (antlr::ANTLRException & ex) {
                    scene.browser().err << ex.getMessage() << std::endl;
                } catch (std::bad_alloc &) {
                    throw;
                } catch (...) {
                    throw unreachable_url();
                }
            } catch (bad_url & ex) {
                scene.browser().err << ex.what() << std::endl;
                continue;
            }
            //
            // If this is the root scene (that is, the parent is null),
            // then this->uri must be the absolute URI.
            //
            scene.url(scene.parent() ? url[i] : absolute_uri);
            break;
        }
        scene.nodes(nodes);
        scene.scene_loaded();
    }

private:
    openvrml::scene * const scene_;
    const std::vector<std::string> * const url_;
};

/**
 * @brief Load a world.
 *
 * The world specified by @p url is loaded asynchronously.
 * <code>scene::scene_loaded</code> is called when loading the world has
 * completed (i.e., when <code>scene::nodes</code> will return the world's
 * root <code>node</code>s).
 *
 * As this function executes asynchronously, note that it will not throw upon
 * encountering a malformed or unreachable URI, or syntactically incorrect
 * VRML.
 *
 * @param url   the URI for the world.  Per VRML97 convention, this is a list
 *              of alternative URIs.  The first one in the list to load
 *              successfully is used.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void scene::load(const std::vector<std::string> & url)
    throw (std::bad_alloc)
{
    using boost::scoped_ptr;
    using boost::thread;
    scoped_ptr<thread> scene_loading_thread(
        new thread(load_scene(*this, url)));
}

/**
 * @brief Initialize the scene.
 *
 * @param timestamp the current time.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void scene::initialize(const double timestamp) throw (std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    for (std::vector<node_ptr>::iterator node(this->nodes_.begin());
         node != this->nodes_.end();
         ++node) {
        assert(*node);
        (*node)->initialize(*this, timestamp);
        child_node * const child = node_cast<child_node *>(node->get());
        assert(child);
        child->relocate();
    }
}

/**
 * @fn const std::vector<node_ptr> & scene::nodes() const throw ()
 *
 * @brief Root nodes for the scene.
 *
 * @return the root nodes for the scene.
 */

/**
 * @brief Set the root nodes for the scene.
 *
 * @param n the new root nodes for the scene.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void scene::nodes(const std::vector<node_ptr> & n) throw (std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    this->nodes_ = n;
}

/**
 * @brief Get the absolute URI for the scene.
 *
 * @return the absolute URI for the scene.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
const std::string scene::url() const throw (std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    using std::string;
    return this->parent_
        ? string(uri(this->url_)
                 .resolve_against(uri(this->parent_->url())))
        : this->url_;
}

/**
 * @brief Set the URI for the scene.
 *
 * Generally this function is used in conjunction with the two-argument
 * constructor (that does not take an alternative URI list) and the
 * scene::nodes mutator function.
 *
 * @param str   a valid URI.
 *
 * @exception invalid_url       if @p str is not a valid URI.
 * @exception std::bad_alloc    if memory allocation fails.
 */
void scene::url(const std::string & str) throw (invalid_url, std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    uri id(str); // Make sure we have a valid URI.
    this->url_ = str;
}

/**
 * @brief Render the scene.
 *
 * @param viewer    a viewer to render to.
 * @param context   a rendering_context.
 */
void scene::render(openvrml::viewer & viewer, rendering_context context)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    for (std::vector<node_ptr>::iterator node(this->nodes_.begin());
         node != this->nodes_.end();
         ++node) {
        assert(*node);
        child_node * child = node_cast<child_node *>(node->get());
        if (child) { child->render_child(viewer, context); }
    }
}

/**
 * @brief Load a resource into @a browser.
 *
 * This method simply resolves any relative references in @p uri and calls
 * browser::load_url.
 *
 * @note There are a couple of edge cases here where we are probably doing the
 *      wrong thing:
 *       - If there is a URI in the list of the form "#NodeId" and it is not
 *         the first URI in the list, this URI will be loaded as if it were a
 *         new world rather than as a Viewpoint that should simply be bound.
 *       - If the first URI in the list is of the form "#NodeId" and no
 *         Viewpoint named "NodeId" exists in the scene, this method will not
 *         try any subsequent URIs in the list.
 *
 * @param url       an array of URIs. Per VRML97 convention, the first resource
 *                  in the sequence that can be reached will be loaded into the
 *                  browser.
 * @param parameter an array of parameters to be associated with the URIs in
 *                  @p uri.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 *
 * @todo This method currently fails silently if any of the URIs in @p url is
 *      invalid. Should this throw invalid_url?
 */
void scene::load_url(const std::vector<std::string> & url,
                     const std::vector<std::string> & parameter)
    throw (std::bad_alloc)
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);

    using std::string;

    if (!url.empty()) {
        if (url[0][0] == '#') {
# if 0
            //
            // If the first element in uri is a Viewpoint name, bind the
            // Viewpoint.
            //
            this->browser_.setViewpoint(uri[0].substr(1));
# endif
        } else {
            std::vector<std::string> absoluteURIs(url.size());
            for (size_t i = 0; i < absoluteURIs.size(); ++i) {
                try {
                    const uri urlElement(url[i]);
                    const string value =
                        urlElement.scheme().empty()
                            ? urlElement.resolve_against(uri(this->url()))
                            : urlElement;
                    absoluteURIs[i] = value;
                } catch (invalid_url & ex) {
                    OPENVRML_PRINT_EXCEPTION_(ex);
                }
            }
            this->browser_.load_url(absoluteURIs, parameter);
        }
    }
}

/**
 * @brief Shut down the nodes in the scene.
 *
 * This function @b must be called before the scene is destroyed.
 *
 * @param timestamp the current time.
 */
void scene::shutdown(const double timestamp) throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    for (std::vector<node_ptr>::iterator node(this->nodes_.begin());
         node != this->nodes_.end();
         ++node) {
        if (*node) { (*node)->shutdown(timestamp); }
    }
}

/**
 * @brief Function called once the scene has been loaded.
 *
 * <code>scene::load</code> calls this function once the scene has finished
 * loading. The default implementation does nothing.
 */
void scene::scene_loaded()
{}


/**
 * @internal
 *
 * @class vrml97_root_scope
 *
 * @brief Root namespace for VRML97 browsers.
 *
 * <code>vrml97_root_scope</code> is initialized with the VRML97 node types.
 */

namespace {
    /**
     * @internal
     */
    class vrml97_node_interface_set_ : public node_interface_set {
    public:
        vrml97_node_interface_set_(const node_interface * const begin,
                                   const node_interface * const end)
        {
            this->insert(begin, end);
        }
    };
}

/**
 * @brief Constructor.
 *
 * @param browser   the browser object.
 * @param uri       the URI associated with the stream being read.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
vrml97_root_scope::vrml97_root_scope(const browser & browser,
                                     const std::string & uri)
    throw (std::bad_alloc):
    scope(uri)
{
    const browser::node_class_map & node_class_map = browser.node_class_map_;
    node_class_ptr node_class;

    try {
        //
        // Anchor node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "addChildren"),
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "removeChildren"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfnode_id,
                               "children"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfstring_id,
                               "description"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "parameter"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "url"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxCenter"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxSize")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 8);
            node_class = node_class_map.find("urn:X-openvrml:node:Anchor");
            assert(node_class);
            this->add_type(node_class->create_type("Anchor", interface_set));
        }

        //
        // Appearance node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "material"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "texture"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "textureTransform")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 3);
            node_class = node_class_map.find("urn:X-openvrml:node:Appearance");
            assert(node_class);
            this->add_type(node_class->create_type("Appearance",
                                                   interface_set));
        }

        //
        // AudioClip node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfstring_id,
                               "description"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "loop"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "pitch"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sftime_id,
                               "startTime"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sftime_id,
                               "stopTime"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "url"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "duration_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 8);
            node_class = node_class_map.find("urn:X-openvrml:node:AudioClip");
            assert(node_class);
            this->add_type(node_class->create_type("AudioClip",
                                                   interface_set));
        }

        //
        // Background node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sfbool_id,
                               "set_bind"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "groundAngle"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfcolor_id,
                               "groundColor"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "backUrl"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "bottomUrl"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "frontUrl"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "leftUrl"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "rightUrl"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "topUrl"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "skyAngle"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfcolor_id,
                               "skyColor"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isBound")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 12);
            node_class = node_class_map.find("urn:X-openvrml:node:Background");
            assert(node_class);
            this->add_type(node_class->create_type("Background",
                                                   interface_set));
        }

        //
        // Billboard node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "addChildren"),
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "removeChildren"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "axisOfRotation"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfnode_id,
                               "children"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxCenter"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxSize")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 6);
            node_class = node_class_map.find("urn:X-openvrml:node:Billboard");
            assert(node_class);
            this->add_type(node_class->create_type("Billboard",
                                                   interface_set));
        }

        //
        // Box node
        //
        {
            static const node_interface interface =
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "size");
            static const vrml97_node_interface_set_
                interface_set(&interface, &interface + 1);
            node_class = node_class_map.find("urn:X-openvrml:node:Box");
            assert(node_class);
            this->add_type(node_class->create_type("Box", interface_set));
        }

        //
        // Collision node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "addChildren"),
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "removeChildren"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfnode_id,
                               "children"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "collide"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxCenter"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxSize"),
                node_interface(node_interface::field_id,
                               field_value::sfnode_id,
                               "proxy"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "collideTime")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 8);
            node_class = node_class_map.find("urn:X-openvrml:node:Collision");
            assert(node_class);
            this->add_type(node_class->create_type("Collision",
                                                   interface_set));
        }

        //
        // Color node
        //
        {
            static const node_interface interface =
                node_interface(node_interface::exposedfield_id,
                               field_value::mfcolor_id,
                               "color");
            static const vrml97_node_interface_set_
                interface_set(&interface, &interface + 1);
            node_class = node_class_map.find("urn:X-openvrml:node:Color");
            assert(node_class);
            this->add_type(node_class->create_type("Color", interface_set));
        }

        //
        // ColorInterpolator node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sffloat_id,
                               "set_fraction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "key"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfcolor_id,
                               "keyValue"),
                node_interface(node_interface::eventout_id,
                               field_value::sfcolor_id,
                               "value_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map
                .find("urn:X-openvrml:node:ColorInterpolator");
            assert(node_class);
            this->add_type(node_class->create_type("ColorInterpolator",
                                                   interface_set));
        }

        //
        // Cone node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "bottomRadius"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "height"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "side"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "bottom")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map.find("urn:X-openvrml:node:Cone");
            assert(node_class);
            this->add_type(node_class->create_type("Cone", interface_set));
        }

        //
        // Coordinate node
        //
        {
            static const node_interface interface =
                node_interface(node_interface::exposedfield_id,
                               field_value::mfvec3f_id,
                               "point");
            static const vrml97_node_interface_set_
                interface_set(&interface, &interface + 1);
            node_class = node_class_map.find("urn:X-openvrml:node:Coordinate");
            assert(node_class);
            this->add_type(node_class->create_type("Coordinate",
                                                   interface_set));
        }

        //
        // CoordinateInterpolator node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sffloat_id,
                               "set_fraction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "key"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfvec3f_id,
                               "keyValue"),
                node_interface(node_interface::eventout_id,
                               field_value::mfvec3f_id,
                               "value_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map
                .find("urn:X-openvrml:node:CoordinateInterpolator");
            assert(node_class);
            this->add_type(node_class->create_type("CoordinateInterpolator",
                                                   interface_set));
        }

        //
        // Cylinder node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "bottom"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "height"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "radius"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "side"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "top")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 5);
            node_class = node_class_map.find("urn:X-openvrml:node:Cylinder");
            assert(node_class);
            this->add_type(node_class->create_type("Cylinder",
                                                   interface_set));
        }

        //
        // CylinderSensor node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "autoOffset"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "diskAngle"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "enabled"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "maxAngle"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "minAngle"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "offset"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive"),
                node_interface(node_interface::eventout_id,
                               field_value::sfrotation_id,
                               "rotation_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec3f_id,
                               "trackPoint_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 9);
            node_class = node_class_map
                .find("urn:X-openvrml:node:CylinderSensor");
            assert(node_class);
            this->add_type(node_class->create_type("CylinderSensor",
                                                   interface_set));
        }

        //
        // DirectionalLight node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "ambientIntensity"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfcolor_id,
                               "color"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "direction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "intensity"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "on")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 5);
            node_class = node_class_map
                .find("urn:X-openvrml:node:DirectionalLight");
            assert(node_class);
            this->add_type(node_class->create_type("DirectionalLight",
                                                   interface_set));
        }

        //
        // ElevationGrid node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mffloat_id,
                               "set_height"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "color"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "normal"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "texCoord"),
                node_interface(node_interface::field_id,
                               field_value::mffloat_id,
                               "height"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "ccw"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "colorPerVertex"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "creaseAngle"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "normalPerVertex"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "solid"),
                node_interface(node_interface::field_id,
                               field_value::sfint32_id,
                               "xDimension"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "xSpacing"),
                node_interface(node_interface::field_id,
                               field_value::sfint32_id,
                               "zDimension"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "zSpacing")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 14);
            node_class = node_class_map
                .find("urn:X-openvrml:node:ElevationGrid");
            assert(node_class);
            this->add_type(node_class->create_type("ElevationGrid",
                                                   interface_set));
        }

        //
        // Extrusion node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mfvec2f_id,
                               "set_crossSection"),
                node_interface(node_interface::eventin_id,
                               field_value::mfrotation_id,
                               "set_orientation"),
                node_interface(node_interface::eventin_id,
                               field_value::mfvec2f_id,
                               "set_scale"),
                node_interface(node_interface::eventin_id,
                               field_value::mfvec3f_id,
                               "set_spine"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "beginCap"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "ccw"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "convex"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "creaseAngle"),
                node_interface(node_interface::field_id,
                               field_value::mfvec2f_id,
                               "crossSection"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "endCap"),
                node_interface(node_interface::field_id,
                               field_value::mfrotation_id,
                               "orientation"),
                node_interface(node_interface::field_id,
                               field_value::mfvec2f_id,
                               "scale"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "solid"),
                node_interface(node_interface::field_id,
                               field_value::mfvec3f_id,
                               "spine")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 14);
            node_class = node_class_map.find("urn:X-openvrml:node:Extrusion");
            assert(node_class);
            this->add_type(node_class->create_type("Extrusion",
                                                   interface_set));
        }

        //
        // Fog node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sfbool_id,
                               "set_bind"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfcolor_id,
                               "color"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfstring_id,
                               "fogType"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "visibilityRange"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isBound")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 5);
            node_class = node_class_map.find("urn:X-openvrml:node:Fog");
            assert(node_class);
            this->add_type(node_class->create_type("Fog", interface_set));
        }

        //
        // FontStyle node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::field_id,
                               field_value::mfstring_id,
                               "family"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "horizontal"),
                node_interface(node_interface::field_id,
                               field_value::mfstring_id,
                               "justify"),
                node_interface(node_interface::field_id,
                               field_value::sfstring_id,
                               "language"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "leftToRight"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "size"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "spacing"),
                node_interface(node_interface::field_id,
                               field_value::sfstring_id,
                               "style"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "topToBottom")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 9);
            node_class = node_class_map.find("urn:X-openvrml:node:FontStyle");
            assert(node_class);
            this->add_type(node_class->create_type("FontStyle",
                                                   interface_set));
        }

        //
        // Group node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "addChildren"),
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "removeChildren"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfnode_id,
                               "children"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxCenter"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxSize")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 5);
            node_class = node_class_map.find("urn:X-openvrml:node:Group");
            assert(node_class);
            this->add_type(node_class->create_type("Group", interface_set));
        }

        //
        // ImageTexture node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "url"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "repeatS"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "repeatT")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 3);
            node_class = node_class_map
                .find("urn:X-openvrml:node:ImageTexture");
            assert(node_class);
            this->add_type(node_class->create_type("ImageTexture",
                                                   interface_set));
        }

        //
        // IndexedFaceSet node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mfint32_id,
                               "set_colorIndex"),
                node_interface(node_interface::eventin_id,
                               field_value::mfint32_id,
                               "set_coordIndex"),
                node_interface(node_interface::eventin_id,
                               field_value::mfint32_id,
                               "set_normalIndex"),
                node_interface(node_interface::eventin_id,
                               field_value::mfint32_id,
                               "set_texCoordIndex"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "color"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "coord"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "normal"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "texCoord"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "ccw"),
                node_interface(node_interface::field_id,
                               field_value::mfint32_id,
                               "colorIndex"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "colorPerVertex"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "convex"),
                node_interface(node_interface::field_id,
                               field_value::mfint32_id,
                               "coordIndex"),
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "creaseAngle"),
                node_interface(node_interface::field_id,
                               field_value::mfint32_id,
                               "normalIndex"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "normalPerVertex"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "solid"),
                node_interface(node_interface::field_id,
                               field_value::mfint32_id,
                               "texCoordIndex")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 18);
            node_class = node_class_map
                .find("urn:X-openvrml:node:IndexedFaceSet");
            assert(node_class);
            this->add_type(node_class->create_type("IndexedFaceSet",
                                                   interface_set));
        }

        //
        // IndexedLineSet node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mfint32_id,
                               "set_colorIndex"),
                node_interface(node_interface::eventin_id,
                               field_value::mfint32_id,
                               "set_coordIndex"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "color"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "coord"),
                node_interface(node_interface::field_id,
                               field_value::mfint32_id,
                               "colorIndex"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "colorPerVertex"),
                node_interface(node_interface::field_id,
                               field_value::mfint32_id,
                               "coordIndex")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 7);
            node_class = node_class_map
                .find("urn:X-openvrml:node:IndexedLineSet");
            assert(node_class);
            this->add_type(node_class->create_type("IndexedLineSet",
                                                   interface_set));
        }

        //
        // Inline node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "url"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxCenter"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxSize")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 3);
            node_class = node_class_map.find("urn:X-openvrml:node:Inline");
            assert(node_class);
            this->add_type(node_class->create_type("Inline", interface_set));
        }

        //
        // LOD node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::mfnode_id,
                               "level"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "center"),
                node_interface(node_interface::field_id,
                               field_value::mffloat_id,
                               "range")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 3);
            node_class = node_class_map.find("urn:X-openvrml:node:LOD");
            assert(node_class);
            this->add_type(node_class->create_type("LOD", interface_set));
        }

        //
        // Material node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "ambientIntensity"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfcolor_id,
                               "diffuseColor"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfcolor_id,
                               "emissiveColor"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "shininess"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfcolor_id,
                               "specularColor"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "transparency")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 6);
            node_class = node_class_map.find("urn:X-openvrml:node:Material");
            assert(node_class);
            this->add_type(node_class->create_type("Material",
                                                   interface_set));
        }

        //
        // MovieTexture node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "loop"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "speed"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sftime_id,
                               "startTime"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sftime_id,
                               "stopTime"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "url"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "repeatS"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "repeatT"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "duration_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 9);
            node_class = node_class_map
                .find("urn:X-openvrml:node:MovieTexture");
            assert(node_class);
            this->add_type(node_class->create_type("MovieTexture",
                                                   interface_set));
        }

        //
        // NavigationInfo node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sfbool_id,
                               "set_bind"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "avatarSize"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "headlight"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "speed"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "type"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "visibilityLimit"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isBound")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 7);
            node_class = node_class_map
                .find("urn:X-openvrml:node:NavigationInfo");
            assert(node_class);
            this->add_type(node_class->create_type("NavigationInfo",
                                                   interface_set));
        }

        //
        // Normal node
        //
        {
            static const node_interface interface =
                node_interface(node_interface::exposedfield_id,
                               field_value::mfvec3f_id,
                               "vector");
            static const vrml97_node_interface_set_
                interface_set(&interface, &interface + 1);
            node_class = node_class_map.find("urn:X-openvrml:node:Normal");
            assert(node_class);
            this->add_type(node_class->create_type("Normal", interface_set));
        }

        //
        // NormalInterpolator node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sffloat_id,
                               "set_fraction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "key"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfvec3f_id,
                               "keyValue"),
                node_interface(node_interface::eventout_id,
                               field_value::mfvec3f_id,
                               "value_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map
                .find("urn:X-openvrml:node:NormalInterpolator");
            assert(node_class);
            this->add_type(node_class->create_type("NormalInterpolator",
                                                   interface_set));
        }

        //
        // OrientationInterpolator node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sffloat_id,
                               "set_fraction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "key"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfrotation_id,
                               "keyValue"),
                node_interface(node_interface::eventout_id,
                               field_value::sfrotation_id,
                               "value_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map
                .find("urn:X-openvrml:node:OrientationInterpolator");
            assert(node_class);
            this->add_type(node_class->create_type("OrientationInterpolator",
                                                   interface_set));
        }

        //
        // PixelTexture node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfimage_id,
                               "image"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "repeatS"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "repeatT")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 3);
            node_class = node_class_map
                .find("urn:X-openvrml:node:PixelTexture");
            assert(node_class);
            this->add_type(node_class->create_type("PixelTexture",
                                                   interface_set));
        }

        //
        // PlaneSensor node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "autoOffset"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "enabled"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec2f_id,
                               "maxPosition"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec2f_id,
                               "minPosition"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "offset"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec3f_id,
                               "trackPoint_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec3f_id,
                               "translation_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 8);
            node_class = node_class_map
                .find("urn:X-openvrml:node:PlaneSensor");
            assert(node_class);
            this->add_type(node_class->create_type("PlaneSensor",
                                                   interface_set));
        }

        //
        // PointLight node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "ambientIntensity"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "attenuation"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfcolor_id,
                               "color"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "intensity"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "location"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "on"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "radius")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 7);
            node_class = node_class_map.find("urn:X-openvrml:node:PointLight");
            assert(node_class);
            this->add_type(node_class->create_type("PointLight",
                                                   interface_set));
        }

        //
        // PointSet node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "color"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "coord")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 2);
            node_class = node_class_map.find("urn:X-openvrml:node:PointSet");
            assert(node_class);
            this->add_type(node_class->create_type("PointSet", interface_set));
        }

        //
        // PositionInterpolator node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sffloat_id,
                               "set_fraction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "key"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfvec3f_id,
                               "keyValue"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec3f_id,
                               "value_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map
                .find("urn:X-openvrml:node:PositionInterpolator");
            assert(node_class);
            this->add_type(node_class->create_type("PositionInterpolator",
                                                   interface_set));
        }

        //
        // ProximitySensor node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "center"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "size"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "enabled"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec3f_id,
                               "position_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfrotation_id,
                               "orientation_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "enterTime"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "exitTime")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 8);
            node_class = node_class_map
                .find("urn:X-openvrml:node:ProximitySensor");
            assert(node_class);
            this->add_type(node_class->create_type("ProximitySensor",
                                                   interface_set));
        }

        //
        // ScalarInterpolator node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sffloat_id,
                               "set_fraction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "key"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "keyValue"),
                node_interface(node_interface::eventout_id,
                               field_value::sffloat_id,
                               "value_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map
                .find("urn:X-openvrml:node:ScalarInterpolator");
            assert(node_class);
            this->add_type(node_class->create_type("ScalarInterpolator",
                                                   interface_set));
        }

        //
        // Shape node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "appearance"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "geometry")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 2);
            node_class = node_class_map.find("urn:X-openvrml:node:Shape");
            assert(node_class);
            this->add_type(node_class->create_type("Shape", interface_set));
        }

        //
        // Sound node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "direction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "intensity"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "location"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "maxBack"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "maxFront"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "minBack"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "minFront"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "priority"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "source"),
                node_interface(node_interface::field_id,
                               field_value::sfbool_id,
                               "spatialize")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 10);
            node_class = node_class_map.find("urn:X-openvrml:node:Sound");
            assert(node_class);
            this->add_type(node_class->create_type("Sound", interface_set));
        }

        //
        // Sphere node
        //
        {
            static const node_interface interface =
                node_interface(node_interface::field_id,
                               field_value::sffloat_id,
                               "radius");
            static const vrml97_node_interface_set_
                interface_set(&interface, &interface + 1);
            node_class = node_class_map.find("urn:X-openvrml:node:Sphere");
            assert(node_class);
            this->add_type(node_class->create_type("Sphere", interface_set));
        }

        //
        // SphereSensor node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "autoOffset"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "enabled"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfrotation_id,
                               "offset"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive"),
                node_interface(node_interface::eventout_id,
                               field_value::sfrotation_id,
                               "rotation_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec3f_id,
                               "trackPoint_changed")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 6);
            node_class = node_class_map
                .find("urn:X-openvrml:node:SphereSensor");
            assert(node_class);
            this->add_type(node_class->create_type("SphereSensor",
                                                   interface_set));
        }

        //
        // SpotLight node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "ambientIntensity"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "attenuation"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "beamWidth"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfcolor_id,
                               "color"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "cutOffAngle"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "direction"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "intensity"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "location"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "on"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "radius")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 10);
            node_class = node_class_map.find("urn:X-openvrml:node:SpotLight");
            assert(node_class);
            this->add_type(node_class->create_type("SpotLight",
                                                   interface_set));
        }

        //
        // Switch node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::mfnode_id,
                               "choice"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfint32_id,
                               "whichChoice")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 2);
            node_class = node_class_map.find("urn:X-openvrml:node:Switch");
            assert(node_class);
            this->add_type(node_class->create_type("Switch", interface_set));
        }

        //
        // Text node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::mfstring_id,
                               "string"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfnode_id,
                               "fontStyle"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mffloat_id,
                               "length"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "maxExtent")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map.find("urn:X-openvrml:node:Text");
            assert(node_class);
            this->add_type(node_class->create_type("Text", interface_set));
        }

        //
        // TextureCoordinate node
        //
        {
            static const node_interface interface =
                node_interface(node_interface::exposedfield_id,
                               field_value::mfvec2f_id,
                               "point");
            static const vrml97_node_interface_set_
                interface_set(&interface, &interface + 1);
            node_class = node_class_map
                .find("urn:X-openvrml:node:TextureCoordinate");
            assert(node_class);
            this->add_type(node_class->create_type("TextureCoordinate",
                                                   interface_set));
        }

        //
        // TextureTransform node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec2f_id,
                               "center"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "rotation"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec2f_id,
                               "scale"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec2f_id,
                               "translation")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 4);
            node_class = node_class_map
                .find("urn:X-openvrml:node:TextureTransform");
            assert(node_class);
            this->add_type(node_class->create_type("TextureTransform",
                                                   interface_set));
        }

        //
        // TimeSensor node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sftime_id,
                               "cycleInterval"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "enabled"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "loop"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sftime_id,
                               "startTime"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sftime_id,
                               "stopTime"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "cycleTime"),
                node_interface(node_interface::eventout_id,
                               field_value::sffloat_id,
                               "fraction_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "time")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 9);
            node_class = node_class_map.find("urn:X-openvrml:node:TimeSensor");
            assert(node_class);
            this->add_type(node_class->create_type("TimeSensor",
                                                   interface_set));
        }

        //
        // TouchSensor node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "enabled"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec3f_id,
                               "hitNormal_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec3f_id,
                               "hitPoint_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfvec2f_id,
                               "hitTexCoord_changed"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isOver"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "touchTime")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 7);
            node_class = node_class_map
                .find("urn:X-openvrml:node:TouchSensor");
            assert(node_class);
            this->add_type(node_class->create_type("TouchSensor",
                                                   interface_set));
        }

        //
        // Transform node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "addChildren"),
                node_interface(node_interface::eventin_id,
                               field_value::mfnode_id,
                               "removeChildren"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "center"),
                node_interface(node_interface::exposedfield_id,
                               field_value::mfnode_id,
                               "children"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfrotation_id,
                               "rotation"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "scale"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfrotation_id,
                               "scaleOrientation"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "translation"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxCenter"),
                node_interface(node_interface::field_id,
                               field_value::sfvec3f_id,
                               "bboxSize")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 10);
            node_class = node_class_map.find("urn:X-openvrml:node:Transform");
            assert(node_class);
            this->add_type(node_class->create_type("Transform",
                                                   interface_set));
        }

        //
        // Viewpoint node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::eventin_id,
                               field_value::sfbool_id,
                               "set_bind"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sffloat_id,
                               "fieldOfView"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "jump"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfrotation_id,
                               "orientation"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "position"),
                node_interface(node_interface::field_id,
                               field_value::sfstring_id,
                               "description"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "bindTime"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isBound")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 8);
            node_class = node_class_map.find("urn:X-openvrml:node:Viewpoint");
            assert(node_class);
            this->add_type(node_class->create_type("Viewpoint",
                                                   interface_set));
        }

        //
        // VisibilitySensor node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "center"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfbool_id,
                               "enabled"),
                node_interface(node_interface::exposedfield_id,
                               field_value::sfvec3f_id,
                               "size"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "enterTime"),
                node_interface(node_interface::eventout_id,
                               field_value::sftime_id,
                               "exitTime"),
                node_interface(node_interface::eventout_id,
                               field_value::sfbool_id,
                               "isActive")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 6);
            node_class = node_class_map
                .find("urn:X-openvrml:node:VisibilitySensor");
            assert(node_class);
            this->add_type(node_class->create_type("VisibilitySensor",
                                                   interface_set));
        }

        //
        // WorldInfo node
        //
        {
            static const node_interface interfaces[] = {
                node_interface(node_interface::field_id,
                               field_value::mfstring_id,
                               "info"),
                node_interface(node_interface::field_id,
                               field_value::sfstring_id,
                               "title")
            };
            static const vrml97_node_interface_set_
                interface_set(interfaces, interfaces + 2);
            node_class = node_class_map.find("urn:X-openvrml:node:WorldInfo");
            assert(node_class);
            this->add_type(node_class->create_type("WorldInfo",
                                                   interface_set));
        }
    } catch (std::invalid_argument & ex) {
        OPENVRML_PRINT_EXCEPTION_(ex);
    }
}

vrml97_root_scope::~vrml97_root_scope() throw () {}


null_node_class::null_node_class(openvrml::browser & browser) throw ():
    node_class(browser)
{}

null_node_class::~null_node_class() throw ()
{}

const node_type_ptr
null_node_class::do_create_type(const std::string & id,
                                const node_interface_set & interfaces) const
    throw ()
{
    assert(false);
    static const node_type_ptr nodeType;
    return nodeType;
}


null_node_type::null_node_type(null_node_class & nodeClass) throw ():
    node_type(nodeClass, std::string())
{}

null_node_type::~null_node_type() throw ()
{}

const node_interface_set & null_node_type::do_interfaces() const throw ()
{
    assert(false);
    static const node_interface_set interfaces;
    return interfaces;
}

const node_ptr
null_node_type::
do_create_node(const boost::shared_ptr<openvrml::scope> & scope,
               const initial_value_map & initial_values) const
    throw ()
{
    assert(false);
    static const node_ptr node;
    return node;
}


/**
 * @class doc
 *
 * @brief A class to contain document references.
 *
 * This is just a shell until a real http protocol library is found...
 */

/**
 * @var char * doc::url_
 *
 * @brief The URL.
 */

/**
 * @var std::ostream * doc::out_
 *
 * @brief A pointer to a std::ostream used for writing the resource.
 */

/**
 * @var FILE * doc::fp_
 *
 * @brief A file descriptor for reading the local copy of the resource.
 */

/**
 * @var char * doc::tmpfile_
 *
 * @brief Name of the temporary file created for the local copy of the
 *        resource.
 */

/**
 * @brief Constructor.
 *
 * @param url       an HTTP or file URL.
 * @param relative  the doc that @p url is relative to, or 0 if @p url is an
 *                  absolute URL.
 */
doc::doc(const std::string & url, const doc * relative):
    url_(0),
    out_(0),
    fp_(0),
    tmpfile_(0)
{
    if (!url.empty()) { this->seturl(url.c_str(), relative); }
}

/**
 * @brief Constructor.
 *
 * @param url       an HTTP or file URL.
 * @param relative  the doc2 that @p url is relative to, or 0 if @p url is an
 *                  absolute URL.
 */
doc::doc(const std::string & url, const doc2 * relative):
    url_(0),
    out_(0),
    fp_(0),
    tmpfile_(0)
{
    if (!url.empty()) { this->seturl(url.c_str(), relative); }
}

/**
 * @brief Destructor.
 */
doc::~doc()
{
    delete [] this->url_;
    delete this->out_;
    if (this->tmpfile_) {
        the_system->remove_file(this->tmpfile_);
        delete [] this->tmpfile_;
    }
}

namespace {
    const char * stripProtocol(const char *url)
    {
      const char *s = url;

#ifdef _WIN32
      if (strncmp(s+1,":/",2) == 0) return url;
#endif

      // strip off protocol if any
      while (*s && isalpha(*s)) ++s;

      if (*s == ':')
        return s + 1;

      return url;
    }

    bool isAbsolute(const char *url)
    {
      const char *s = stripProtocol(url);
      return ( *s == '/' || *(s+1) == ':' );
    }
}

/**
  * @brief Set the URL.
 *
 * @param url       the new URL.
 * @param relative  the doc that @p url is relative to, or 0 if @p url is an
 *                  absolute URL.
 */
void doc::seturl(const char * const url, const doc * const relative)
{
  delete [] url_;
  url_ = 0;

  if (url)
  {
      const char *path = "";

#ifdef _WIN32
// Convert windows path stream to standard URL
	  char *p = (char *)url;
	  for(;*p != '\0';p++)
		  if(*p == '\\')*p = '/';
#endif

      if ( relative && ! isAbsolute(url) )
	    path = relative->url_path();

      url_ = new char[strlen(path) + strlen(url) + 1];
      strcpy(url_, path);

      if (strlen(url)>2 && url[0] == '.' && url[1] == '/')
        strcat(url_, url+2); // skip "./"
      else
        strcat(url_, url);
  }
}

/**
 * @brief Set the URL.
 *
 * @param url       the new URL.
 * @param relative  the doc2 that @p url is relative to, or 0 if @p url is an
 *                  absolute URL.
 */
void doc::seturl(const char * const url, const doc2 * const relative)
{
    delete [] this->url_;
    this->url_ = 0;

    if (url) {
        std::string path;

#ifdef _WIN32
        // Convert windows path stream to standard URL
        char *p = (char *)url;
        for (; *p != '\0'; p++) { if (*p == '\\') { *p = '/'; } }
#endif

        if (relative && !isAbsolute(url)) { path = relative->url_path(); }

        this->url_ = new char[path.length() + strlen(url) + 1];
        strcpy(this->url_, path.c_str());

        if (strlen(url) > 2 && url[0] == '.' && url[1] == '/') {
            strcat(this->url_, url + 2); // skip "./"
        } else {
            strcat(this->url_, url);
        }
    }
}

/**
 * @brief Get the URL.
 *
 * @return the URL.
 */
const char * doc::url() const { return url_; }

/**
 * @brief Get the portion of the path likely to correspond to a file name
 *      without its extension.
 *
 * @return the portion of the last path element preceding the last '.' in the
 *      path, or an empty string if the last path element is empty.
 */
const char * doc::url_base() const
{
  if (! url_) return "";

  static char path[1024];
  char *p, *s = path;
  strncpy(path, url_, sizeof(path)-1);
  path[sizeof(path)-1] = '\0';
  if ((p = strrchr(s, '/')) != 0)
    s = p+1;
  else if ((p = strchr(s, ':')) != 0)
    s = p+1;

  if ((p = strrchr(s, '.')) != 0)
    *p = '\0';

  return s;
}

/**
 * @brief Get the portion of the path likely to correspond to a file name
 *      extension.
 *
 * @return the portion of the last path element succeeding the last '.' in the
 *      path, or an empty string if the last path element includes no '.'.
 */
const char * doc::url_ext() const
{
  if (! url_) return "";

  static char ext[20];
  char *p;

  if ((p = strrchr(url_, '.')) != 0)
    {
      strncpy(ext, p+1, sizeof(ext)-1);
      ext[sizeof(ext)-1] = '\0';
    }
  else
    ext[0] = '\0';

  return &ext[0];
}

/**
 * @brief Get the URL without the last component of the path.
 *
 * In spite of its name, this method does not return the URL's path.
 *
 * @return the portion of the URL including the scheme, the authority, and all
 *      but the last component of the path.
 */
const char * doc::url_path() const
{
  if (! url_) return "";

  static char path[1024];

  strcpy(path, url_);
  char *slash;
  if ((slash = strrchr(path, '/')) != 0)
    *(slash+1) = '\0';
  else
    path[0] = '\0';
  return &path[0];
}

/**
 * @brief Get the URL scheme.
 *
 * @return the URL scheme.
 */
const char * doc::url_protocol() const
{
  if (url_)
    {
      static char protocol[12];
      const char *s = url_;

#ifdef _WIN32
      if (strncmp(s+1,":/",2) == 0) return "file";
#endif

      for (unsigned int i=0; i<sizeof(protocol); ++i, ++s)
	{
	  if (*s == 0 || ! isalpha(*s))
	    {
	      protocol[i] = '\0';
	      break;
	    }
	  protocol[i] = tolower(*s);
	}
      protocol[sizeof(protocol)-1] = '\0';
      if (*s == ':')
	return protocol;
    }

  return "file";
}

/**
 * @brief Get the fragment identifier.
 *
 * @return the fragment identifier, including the leading '#', or an empty
 *      string if there is no fragment identifier.
 */
const char * doc::url_modifier() const
{
  char *mod = url_ ? strrchr(url_,'#') : 0;
  return mod;
}

/**
 * @brief Get the fully qualified name of a local file that is the downloaded
 *      resource at @a url_.
 *
 * @return the fully qualified name of a local file that is the downloaded
 *      resource at @a url_.
 */
const char * doc::local_name()
{
  static char buf[1024];
  if (filename(buf, sizeof(buf)))
    return &buf[0];
  return 0;
}

/**
 * @brief Get the path of the local file that is the downloaded resource at
 *      @a url_.
 *
 * @return the path of the local file that is the downloaded resource at
 *      @a url_.
 */
const char * doc::local_path()
{
  static char buf[1024];
  if (filename(buf, sizeof(buf)))
    {
      char *s = strrchr(buf, '/');
      if (s) *(s+1) = '\0';
      return &buf[0];
    }
  return 0;
}

/**
 * @brief Converts a url into a local filename.
 *
 * @retval fn   a character buffer to hold the local filename.
 * @param nfn   the number of elements in the buffer @p fn points to.
 */
bool doc::filename(char * fn, int nfn)
{
    using std::string;

    fn[0] = '\0';

    string s = stripProtocol(this->url_);
    char * e = 0;

    if ((e = strrchr(s.c_str(),'#')) != 0) { *e = '\0'; }

    const char *protocol = url_protocol();

    // Get a local copy of http files
    if (strcmp(protocol, "http") == 0) {
        if (tmpfile_) {
            // Already fetched it
            s = tmpfile_;
        } else if (!(s = the_system->http_fetch(this->url_)).empty()) {
            tmpfile_ = new char[s.length() + 1];
            strcpy(tmpfile_, s.c_str());
            s = tmpfile_;
	}
    }

    // Unrecognized protocol (need ftp here...)
    else if (strcmp(protocol, "file") != 0) {
        s.clear();
    }

#ifdef _WIN32
  // Does not like "//C:" skip "// "
    if (!s.empty()) {
        if(s.length() > 2 && s[0] == '/' && s[1] == '/') { s = s.substr(2); }
    }
#endif

    if (!s.empty()) {
        strncpy(fn, s.c_str(), nfn - 1);
        fn[nfn - 1] = '\0';
    }

    if (e) { *e = '#'; }

    return !s.empty();
}

/**
 * @brief Open a file.
 *
 * @return a pointer to a FILE struct for the opened file.
 *
 * Having both fopen and outputStream is dumb.
 */
FILE *doc::fopen(const char *mode)
{
    if (this->fp_) {
        OPENVRML_PRINT_MESSAGE_(std::string(this->url_ ? this->url_ : "")
                                + "is already open.");
    }

    char fn[256];
    if (filename(fn, sizeof(fn))) {
        if (strcmp(fn, "-") == 0) {
            if (*mode == 'r') {
                fp_ = stdin;
            } else if (*mode == 'w') {
                fp_ = stdout;
            }
        } else {
            fp_ = ::fopen( fn, mode );
        }
    }
    return fp_;
}

/**
 * @brief Close a file.
 *
 * Closes the file opened with doc::fopen.
 */
void doc::fclose()
{
  if (fp_ && (strcmp(url_, "-") != 0) && (strncmp(url_, "-#", 2) != 0))
    ::fclose(fp_);

  fp_ = 0;
  if (tmpfile_)
    {
      the_system->remove_file(tmpfile_);
      delete [] tmpfile_;
      tmpfile_ = 0;
    }
}

/**
 * @brief Get an output stream for writing to the resource.
 *
 * @return an output stream.
 */
std::ostream & doc::output_stream()
{
    this->out_ = new std::ofstream(stripProtocol(url_), std::ios::out);
    return *this->out_;
}


/**
 * @class doc2
 *
 * @brief A class to contain document references.
 *
 * doc2 is a hack of doc. When the ANTLR parser was added to OpenVRML, a doc
 * work-alike was needed that would read from a std::istream instead of a C
 * @c FILE @c *. doc2's purpose is to fill that need, and to remind us through
 * its ugliness just how badly both it and doc need to be replaced with an I/O
 * solution that doesn't suck.
 */

/**
 * @var char * doc2::url_
 *
 * @brief The URL.
 */

/**
 * @var char * doc2::tmpfile_
 *
 * @brief Name of the temporary file created for the local copy of the
 *        resource.
 */

/**
 * @var std::istream * doc2::istm_
 *
 * @brief A file descriptor for reading the local copy of the resource.
 */

/**
 * @var std::ostream * doc2::ostm_
 *
 * @brief A pointer to a std::ostream used for writing the resource.
 */

/**
 * @brief Constructor.
 *
 * @param url       an HTTP or file URL.
 * @param relative  the doc2 that @p url is relative to, or 0 if @p url is an
 *                  absolute URL.
 */
doc2::doc2(const std::string & url, const doc2 * relative):
    tmpfile_(0),
    istm_(0),
    ostm_(0)
{
    if (!url.empty()) {
        this->seturl(url, relative);
    }
}

/**
 * @brief Destructor.
 */
doc2::~doc2()
{
    delete istm_;
    delete ostm_;
    if (tmpfile_) {
        the_system->remove_file(tmpfile_);
        delete [] tmpfile_;
    }
}

namespace {
    const std::string stripProtocol(const std::string & url) {
        using std::string;
        const string::size_type colonPos = url.find_first_of(':');
        return (colonPos != string::npos)
                ? url.substr(colonPos + 1)
                : url;
    }

    bool isAbsolute(const std::string & url) {
        return stripProtocol(url)[0] == '/';
    }
}

/**
 * @brief Set the URL.
 *
 * @param url       the new URL.
 * @param relative  the doc2 that @p url is relative to, or 0 if @p url is an
 *                  absolute URL.
 */
void doc2::seturl(const std::string & url, const doc2 * relative) {
    using std::string;

    this->url_ = string();

    if (!url.empty()) {

        delete this->istm_;
        this->istm_ = 0;
        delete this->ostm_;
        this->ostm_ = 0;

        string path;

        if (relative && !isAbsolute(url)) {
            path = relative->url_path();
        }

        this->url_ = path;

        if (url.length() > 2 && url[0] == '.' && url[1] == '/') {
            this->url_ += url.substr(2);
        } else {
            this->url_ += url;
        }
    }
}

/**
 * @brief Get the URL.
 *
 * @return the URL.
 */
const std::string doc2::url() const { return this->url_; }

/**
 * @brief Get the portion of the path likely to correspond to a file name
 *      without its extension.
 *
 * @return the portion of the last path element preceding the last '.' in the
 *      path, or an empty string if the last path element is empty.
 */
const std::string doc2::url_base() const {
    using std::string;

    string::size_type lastSlashPos = this->url_.find_last_of('/');
    string::size_type lastDotPos = this->url_.find_last_of('.');

    string::size_type beginPos = (lastSlashPos != string::npos)
                               ? lastSlashPos + 1
                               : 0;
    string::size_type length = (lastDotPos != string::npos)
                             ? lastDotPos - beginPos
                             : this->url_.length() - 1 - beginPos;

    return (beginPos < this->url_.length())
            ? this->url_.substr(beginPos, length)
            : "";
}

/**
 * @brief Get the portion of the path likely to correspond to a file name
 *      extension.
 *
 * @return the portion of the last path element succeeding the last '.' in the
 *      path, or an empty string if the last path element includes no '.'.
 */
const std::string doc2::url_ext() const {
    using std::string;
    string::size_type lastDotPos = this->url_.find_last_of('.');
    return (lastDotPos != string::npos)
            ? this->url_.substr(lastDotPos + 1)
            : "";
}

/**
 * @brief Get the URL without the last component of the path.
 *
 * In spite of its name, this method does not return the URL's path.
 *
 * @return the portion of the URL including the scheme, the authority, and all
 *      but the last component of the path.
 */
const std::string doc2::url_path() const {
    using std::string;

    string::size_type lastSlashPos = this->url_.find_last_of('/');

    return (lastSlashPos != string::npos)
            ? this->url_.substr(0, lastSlashPos + 1)
            : this->url_;
}

/**
 * @brief Get the URL scheme.
 *
 * @return the URL scheme.
 */
const std::string doc2::url_protocol() const {
    using std::string;

    string::size_type firstColonPos = this->url_.find_first_of(':');
    return (firstColonPos != string::npos)
            ? this->url_.substr(0, firstColonPos)
            : "file";
}

/**
 * @brief Get the fragment identifier.
 *
 * @return the fragment identifier, including the leading '#', or an empty
 *      string if there is no fragment identifier.
 */
const std::string doc2::url_modifier() const {
    using std::string;
    string::size_type lastHashPos = this->url_.find_last_of('#');
    return (lastHashPos != string::npos)
            ? this->url_.substr(lastHashPos)
            : "";
}

/**
 * @brief Get the fully qualified name of a local file that is the downloaded
 *      resource at @a url_.
 *
 * @return the fully qualified name of a local file that is the downloaded
 *      resource at @a url_.
 */
const char * doc2::local_name() {
    static char buf[1024];
    if (filename(buf, sizeof(buf))) { return buf; }
    return 0;
}

/**
 * @brief Get the path of the local file that is the downloaded resource at
 *      @a url_.
 *
 * @return the path of the local file that is the downloaded resource at
 *      @a url_.
 */
const char * doc2::local_path() {
    static char buf[1024];

    if (filename(buf, sizeof(buf))) {

        char * s = strrchr(buf, '/');
        if (s) {
            *(s+1) = '\0';
        }

        return buf;
    }

    return 0;
}

/**
 * @brief Get an input stream for the resource.
 *
 * @return an input stream for the resource.
 */
std::istream & doc2::input_stream() {
    if (!this->istm_) {

        char fn[256];

        this->filename(fn, sizeof(fn));
        if (strcmp(fn, "-") == 0) {
            this->istm_ = &std::cin;
        } else {
# ifdef OPENVRML_ENABLE_GZIP
            this->istm_ = new z::ifstream(fn);
# else
            this->istm_ = new std::ifstream(fn);
# endif
        }
    }

    return *this->istm_;
}

/**
 * @brief Get an output stream for the resource.
 *
 * @return an output stream for the resource.
 */
std::ostream & doc2::output_stream() {
    if (!ostm_) {
        ostm_ = new std::ofstream(stripProtocol(url_).c_str(), std::ios::out);
    }
    return *this->ostm_;
}

/**
 * @brief Converts a url into a local filename.
 *
 * @retval fn   a character buffer to hold the local filename.
 * @param nfn   the number of elements in the buffer @p fn points to.
 */
bool doc2::filename(char * fn, const size_t nfn) {
    using std::copy;
    using std::string;

    fn[0] = '\0';

    string s;

    const string protocol = this->url_protocol();

    if (protocol == "file") {
# ifdef _WIN32
        string name = uri(this->url_).path().substr(1);
# else
        string name = uri(this->url_).path();
# endif
        size_t len = (name.length() < (nfn - 1))
                   ? name.length()
                   : nfn - 1;
        copy(name.begin(), name.begin() + len, fn);
        fn[len] = '\0';
        return true;
    } else if (protocol == "http") {
        //
        // Get a local copy of http files.
        //
        if (this->tmpfile_) {    // Already fetched it
            s = this->tmpfile_;
        } else if (!(s = the_system->http_fetch(this->url_.c_str())).empty()) {
            tmpfile_ = new char[s.length() + 1];
            strcpy(tmpfile_, s.c_str());
            s = tmpfile_;
        }
    }
    // Unrecognized protocol (need ftp here...)
    else {
        s.clear();
    }

    if (!s.empty()) {
        strncpy(fn, s.c_str(), nfn - 1);
        fn[nfn-1] = '\0';
    }

    return !s.empty();
}

} // namespace openvrml
