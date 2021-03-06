// -*- mode: c++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 78 -*-
//
// Copyright 2004, 2005, 2006, 2007  Braden McDaniel
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; if not, see <http://www.gnu.org/licenses/>.
//

# include <cstdlib>
# include <iostream>
# include <sstream>
# include "test_resource_fetcher.h"

int main()
{
    using namespace std;
    using namespace openvrml;

    try {
        test_resource_fetcher fetcher;
        browser b(fetcher, std::cout, std::cerr);

        const char vrmlstring[] =
            "Anchor {"
            "  children []"
            "  description \"\""
            "  parameter []"
            "  url []"
            "  bboxCenter 0 0 0"
            "  bboxSize -1 -1 -1"
            "}";
        stringstream vrmlstream(vrmlstring);

        vector<boost::intrusive_ptr<node> > nodes =
            b.create_vrml_from_stream(vrmlstream);
        if (nodes.size() != 1) {
            return EXIT_FAILURE;
        }
        if (!nodes[0]) {
            return EXIT_FAILURE;
        }
        if (nodes[0]->type().id() != "Anchor") {
            return EXIT_FAILURE;
        }
    } catch (...) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
