// -*- mode: c++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 78 -*-
//
// OpenVRML
//
// Copyright 2006, 2007, 2008  Braden McDaniel
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

# include "metadata_double.h"
# include "metadata_float.h"
# include "metadata_integer.h"
# include "metadata_set.h"
# include "metadata_string.h"
# include <openvrml/browser.h>

# ifdef HAVE_CONFIG_H
#   include <config.h>
# endif

extern "C"
# ifdef _WIN32
__declspec(dllexport)
# else
OPENVRML_API
# endif
void
openvrml_register_node_metatypes(openvrml::node_metatype_registry & registry)
{
    using boost::shared_ptr;
    using openvrml::node_metatype;
    using namespace openvrml_node_x3d_core;

    openvrml::browser & b = registry.browser();

    registry.register_node_metatype(metadata_double_metatype::id,
                                    shared_ptr<node_metatype>(
                                        new metadata_double_metatype(b)));
    registry.register_node_metatype(metadata_float_metatype::id,
                                    shared_ptr<node_metatype>(
                                        new metadata_float_metatype(b)));
    registry.register_node_metatype(metadata_integer_metatype::id,
                                    shared_ptr<node_metatype>(
                                        new metadata_integer_metatype(b)));
    registry.register_node_metatype(metadata_set_metatype::id,
                                    shared_ptr<node_metatype>(
                                        new metadata_set_metatype(b)));
    registry.register_node_metatype(metadata_string_metatype::id,
                                    shared_ptr<node_metatype>(
                                        new metadata_string_metatype(b)));
}
