// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; -*-
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

# ifndef OPENVRML_SYSTEM_H
#   define OPENVRML_SYSTEM_H

#   include <stddef.h>
#   include <string>
#   include <OpenVRML/common.h>

namespace OpenVRML {

    class mfstring;

    class OPENVRML_SCOPE system {
    public:
        virtual bool load_url(const std::string & url,
                             const mfstring & parameters);
        virtual int connect_socket( const char *host, int port );
        virtual const char *http_host(const char *url, int *port);
        virtual const char *http_fetch( const char *url );
        virtual void remove_file( const char *fn );
    };

    extern OPENVRML_SCOPE system * the_system;
}

#endif // OPENVRML_SYSTEM_H
