// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; -*-
//
// OpenVRML
//
// Copyright 2004, 2005  Braden McDaniel
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

# include "event.h"

/**
 * @class openvrml::event_listener
 *
 * @brief Abstract base class of event listeners.
 */

/**
 * @internal
 *
 * @var openvrml::node & openvrml::event_listener::node_
 *
 * @brief The <code>node</code> to which the <code>event_listener</code>
 *        belongs.
 */

/**
 * @brief Construct.
 *
 * @param node  the <code>node</code> to which the <code>event_listener</code>
 *              belongs.
 */
openvrml::event_listener::event_listener(openvrml::node & node) throw ():
    node_(node)
{}

/**
 * @brief Destroy.
 */
openvrml::event_listener::~event_listener() throw ()
{}

/**
 * @brief The <code>node</code> to which the <code>event_listener</code>
 *        belongs.
 *
 * @return the <code>node</code> to which the <code>event_listener</code>
 *         belongs.
 */
openvrml::node & openvrml::event_listener::node() throw ()
{
    return this->node_;
}

/**
 * @fn openvrml::field_value::type_id openvrml::event_listener::type() const throw ()
 *
 * @brief The <code>field_value::type_id</code> for the type accepted by the
 *        event listener.
 *
 * @return The <code>field_value::type_id</code> for the type accepted by the
 *         event listener.
 */

/**
 * @class openvrml::field_value_listener
 *
 * @brief Concrete event listener template.
 */

/**
 * @fn openvrml::field_value_listener<FieldValue>::field_value_listener(openvrml::node & node) throw ()
 *
 * @brief Construct.
 *
 * @param node  the node to which the event_listener belongs.
 */

/**
 * @fn openvrml::field_value_listener<FieldValue>::~field_value_listener() throw ()
 *
 * @brief Destroy.
 */

/**
 * @fn openvrml::field_value::type_id openvrml::field_value_listener<FieldValue>::type() const throw ()
 *
 * @brief <code>FieldValue::field_value_type_id</code>.
 *
 * @return <code>FieldValue::field_value_type_id</code>.
 */

/**
 * @fn void openvrml::field_value_listener<FieldValue>::process_event(const FieldValue & value, double timestamp) throw (std::bad_alloc)
 *
 * @brief Process an event.
 *
 * @param value     the event value.
 * @param timestamp the current time.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */

/**
 * @fn void openvrml::field_value_listener<FieldValue>::do_process_event(const FieldValue & value, double timestamp) throw (std::bad_alloc)
 *
 * @brief Called by
 *      <code>field_value_listener&lt;FieldValue&gt;::do_process_event</code>.
 *
 * Subclasses must implement this function.
 *
 * @param value     the event value.
 * @param timestamp the current time.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */

/**
 * @typedef openvrml::sfbool_listener
 *
 * @brief sfbool event listener.
 */

/**
 * @typedef openvrml::sfcolor_listener
 *
 * @brief sfcolor event listener.
 */

/**
 * @typedef openvrml::sffloat_listener
 *
 * @brief sffloat event listener.
 */

/**
 * @typedef openvrml::sfimage_listener
 *
 * @brief sfimage event listener.
 */

/**
 * @typedef openvrml::sfint32_listener
 *
 * @brief sfint32 event listener.
 */

/**
 * @typedef openvrml::sfnode_listener
 *
 * @brief sfnode event listener.
 */

/**
 * @typedef openvrml::sfrotation_listener
 *
 * @brief sfrotation event listener.
 */

/**
 * @typedef openvrml::sfstring_listener
 *
 * @brief sfstring event listener.
 */

/**
 * @typedef openvrml::sftime_listener
 *
 * @brief sftime event listener.
 */

/**
 * @typedef openvrml::sfvec2f_listener
 *
 * @brief sfvec2f event listener.
 */

/**
 * @typedef openvrml::sfvec3f_listener
 *
 * @brief sfvec3f event listener.
 */

/**
 * @typedef openvrml::mfcolor_listener
 *
 * @brief mfcolor event listener.
 */

/**
 * @typedef openvrml::mffloat_listener
 *
 * @brief mffloat event listener.
 */

/**
 * @typedef openvrml::mfint32_listener
 *
 * @brief mfint32 event listener.
 */

/**
 * @typedef openvrml::mfnode_listener
 *
 * @brief mfnode event listener.
 */

/**
 * @typedef openvrml::mfrotation_listener
 *
 * @brief mfrotation event listener.
 */

/**
 * @typedef openvrml::mfstring_listener
 *
 * @brief mfstring event listener.
 */

/**
 * @typedef openvrml::mftime_listener
 *
 * @brief mftime event listener.
 */

/**
 * @typedef openvrml::mfvec2f_listener
 *
 * @brief mfvec2f event listener.
 */

/**
 * @typedef openvrml::mfvec3f_listener
 *
 * @brief mfvec3f event listener.
 */


/**
 * @class openvrml::event_emitter
 *
 * @brief Abstract base class of event emitters.
 */

/**
 * @var class openvrml::event_emitter::node
 *
 * @brief The implementation of <code>node</code> calls
 *        <code>event_emitter::emit_event</code>.
 *
 * The only things that should be emitting events are <code>node</code>s.
 * Subclasses of <code>node</code> should call <code>node::emit_event</code> to
 * emit an event.
 */

/**
 * @brief Create an event_emitter.
 *
 * @param value value to emit.
 *
 * @return an event_emitter.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
std::auto_ptr<openvrml::event_emitter>
openvrml::event_emitter::create(const field_value & value)
    throw (std::bad_alloc)
{
    std::auto_ptr<event_emitter> emitter;
    switch (value.type()) {
    case field_value::sfbool_id:
        emitter.reset(new sfbool_emitter(static_cast<const sfbool &>(value)));
        break;
    case field_value::sfcolor_id:
        emitter.reset(
            new sfcolor_emitter(static_cast<const sfcolor &>(value)));
        break;
    case field_value::sffloat_id:
        emitter.reset(
            new sffloat_emitter(static_cast<const sffloat &>(value)));
        break;
    case field_value::sfimage_id:
        emitter.reset(
            new sfimage_emitter(static_cast<const sfimage &>(value)));
        break;
    case field_value::sfint32_id:
        emitter.reset(
            new sfint32_emitter(static_cast<const sfint32 &>(value)));
        break;
    case field_value::sfnode_id:
        emitter.reset(new sfnode_emitter(static_cast<const sfnode &>(value)));
        break;
    case field_value::sfstring_id:
        emitter.reset(
            new sfstring_emitter(static_cast<const sfstring &>(value)));
        break;
    case field_value::sfrotation_id:
        emitter.reset(
            new sfrotation_emitter(static_cast<const sfrotation &>(value)));
        break;
    case field_value::sftime_id:
        emitter.reset(new sftime_emitter(static_cast<const sftime &>(value)));
        break;
    case field_value::sfvec2f_id:
        emitter.reset(
            new sfvec2f_emitter(static_cast<const sfvec2f &>(value)));
        break;
    case field_value::sfvec3f_id:
        emitter.reset(
            new sfvec3f_emitter(static_cast<const sfvec3f &>(value)));
        break;
    case field_value::mfcolor_id:
        emitter.reset(
            new mfcolor_emitter(static_cast<const mfcolor &>(value)));
        break;
    case field_value::mffloat_id:
        emitter.reset(
            new mffloat_emitter(static_cast<const mffloat &>(value)));
        break;
    case field_value::mfint32_id:
        emitter.reset(
            new mfint32_emitter(static_cast<const mfint32 &>(value)));
        break;
    case field_value::mfnode_id:
        emitter.reset(new mfnode_emitter(static_cast<const mfnode &>(value)));
        break;
    case field_value::mfstring_id:
        emitter.reset(
            new mfstring_emitter(static_cast<const mfstring &>(value)));
        break;
    case field_value::mfrotation_id:
        emitter.reset(
            new mfrotation_emitter(static_cast<const mfrotation &>(value)));
        break;
    case field_value::mftime_id:
        emitter.reset(new mftime_emitter(static_cast<const mftime &>(value)));
        break;
    case field_value::mfvec2f_id:
        emitter.reset(
            new mfvec2f_emitter(static_cast<const mfvec2f &>(value)));
        break;
    case field_value::mfvec3f_id:
        emitter.reset(
            new mfvec3f_emitter(static_cast<const mfvec3f &>(value)));
        break;
    default:
        assert(false);
    }
    return emitter;
}

/**
 * @internal
 *
 * @var boost::recursive_mutex openvrml::event_emitter::mutex_
 *
 * @brief Object mutex.
 */

/**
 * @internal
 *
 * @var const openvrml::field_value & openvrml::event_emitter::value_
 *
 * @brief A reference to the field_value for the event_emitter.
 */

/**
 * @typedef openvrml::event_emitter::listener_set
 *
 * @brief Set of event_listeners.
 */

/**
 * @var openvrml::event_emitter::listener_set openvrml::event_emitter::listeners_
 *
 * @brief The listeners registered for this emitter.
 *
 * When emit_event is called, each of the registered listeners will be sent an
 * event.
 */

/**
 * @var double openvrml::event_emitter::last_time_
 *
 * @brief The timestamp of the last event emitted.
 */

/**
 * @brief Construct.
 *
 * @param value <code>field_value</code> associated with this emitter.
 */
openvrml::event_emitter::event_emitter(const field_value & value) throw ():
    value_(value)
{}

/**
 * @brief Destroy.
 */
openvrml::event_emitter::~event_emitter() throw ()
{}

/**
 * @brief Get the mutex for the <code>event_emitter</code>.
 *
 * @return a reference to the <code>event_emitter</code>'s mutex.
 */
boost::recursive_mutex & openvrml::event_emitter::mutex() const throw ()
{
    return this->mutex_;
}

/**
 * @brief A reference to the <code>field_value</code> for the
 *        <code>event_emitter</code>.
 *
 * @return a reference to the <code>field_value</code> for the
 *         <code>event_emitter</code>.
 */
const openvrml::field_value & openvrml::event_emitter::value() const throw ()
{
    return this->value_;
}

/**
 * @brief Registered listeners.
 *
 * @return the set of registered event_listeners.
 */
const openvrml::event_emitter::listener_set &
openvrml::event_emitter::listeners() const throw ()
{
    return this->listeners_;
}

/**
 * @brief Registered listeners.
 *
 * @return the set of registered event_listeners.
 */
openvrml::event_emitter::listener_set & openvrml::event_emitter::listeners()
    throw ()
{
    return this->listeners_;
}

/**
 * @brief The timestamp of the last event emitted.
 *
 * @return the timestamp of the last event emitted.
 */
double openvrml::event_emitter::last_time() const throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    return this->last_time_;
}

/**
 * @brief Set the timestamp of the last event emitted.
 *
 * @param t the timestamp of the last event emitted.
 */
void openvrml::event_emitter::last_time(const double t) throw ()
{
    boost::recursive_mutex::scoped_lock lock(this->mutex_);
    this->last_time_ = t;
}

/**
 * @fn void openvrml::event_emitter::emit_event(double timestamp) throw (std::bad_alloc)
 *
 * @brief Emit an event.
 *
 * @param timestamp the current time.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 *
 * This function is called by node::emit_event.
 */


/**
 * @class openvrml::field_value_emitter
 *
 * @brief Concrete event emitter template.
 */

/**
 * @fn openvrml::field_value_emitter::field_value_emitter(const FieldValue & value) throw ()
 *
 * @brief Construct.
 */

/**
 * @fn openvrml::field_value_emitter::~field_value_emitter() throw ()
 *
 * @brief Destroy.
 */

/**
 * @fn void openvrml::field_value_emitter::emit_event(double timestamp) throw (std::bad_alloc)
 *
 * @brief Emit an event.
 *
 * @param timestamp the current time.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */

/**
 * @fn bool openvrml::field_value_emitter::add(field_value_listener<FieldValue> & listener) throw (std::bad_alloc)
 *
 * @brief Add an event listener.
 *
 * @param listener  the listener to add.
 *
 * @return @c true if @p listener was added; @c false if @p listener was not
 *         added (if it was already registered for the emitter).
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */

/**
 * @fn bool openvrml::field_value_emitter::remove(field_value_listener<FieldValue> & listener)
 *
 * @brief Remove an event listener.
 *
 * @param listener  the listener to remove.
 *
 * @return @c true if @p listener was removed; @c false if @p listener was not
 *         removed (if it was not registered for the emitter).
 */

/**
 * @typedef openvrml::sfbool_emitter
 *
 * @brief sfbool event emitter.
 */

/**
 * @typedef openvrml::sfcolor_emitter
 *
 * @brief sfcolor event emitter.
 */

/**
 * @typedef openvrml::sffloat_emitter
 *
 * @brief sffloat event emitter.
 */

/**
 * @typedef openvrml::sfimage_emitter
 *
 * @brief sfimage event emitter.
 */

/**
 * @typedef openvrml::sfint32_emitter
 *
 * @brief sfint32 event emitter.
 */

/**
 * @typedef openvrml::sfnode_emitter
 *
 * @brief sfnode event emitter.
 */

/**
 * @typedef openvrml::sfrotation_emitter
 *
 * @brief sfrotation event emitter.
 */

/**
 * @typedef openvrml::sfstring_emitter
 *
 * @brief sfstring event emitter.
 */

/**
 * @typedef openvrml::sftime_emitter
 *
 * @brief sftime event emitter.
 */

/**
 * @typedef openvrml::sfvec2f_emitter
 *
 * @brief sfvec2f event emitter.
 */

/**
 * @typedef openvrml::sfvec3f_emitter
 *
 * @brief sfvec3f event emitter.
 */

/**
 * @typedef openvrml::mfcolor_emitter
 *
 * @brief mfcolor event emitter.
 */

/**
 * @typedef openvrml::mffloat_emitter
 *
 * @brief mffloat event emitter.
 */

/**
 * @typedef openvrml::mfint32_emitter
 *
 * @brief mfint32 event emitter.
 */

/**
 * @typedef openvrml::mfnode_emitter
 *
 * @brief mfnode event emitter.
 */

/**
 * @typedef openvrml::mfrotation_emitter
 *
 * @brief mfrotation event emitter.
 */

/**
 * @typedef openvrml::mfstring_emitter
 *
 * @brief mfstring event emitter.
 */

/**
 * @typedef openvrml::mftime_emitter
 *
 * @brief mftime event emitter.
 */

/**
 * @typedef openvrml::mfvec2f_emitter
 *
 * @brief mfvec2f event emitter.
 */

/**
 * @typedef openvrml::mfvec3f_emitter
 *
 * @brief mfvec3f event emitter.
 */
