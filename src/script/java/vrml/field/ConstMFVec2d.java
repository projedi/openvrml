//
// OpenVRML
//
// Copyright 2008  Braden McDaniel
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

package vrml.field;

import vrml.ConstMField;

/**
 * Represents a read-only VRML MFVec2d field in Java.
 */
public class ConstMFVec2d extends ConstMField {

    /**
     * Construct from a peer handle.
     */
    ConstMFVec2d(long peer) {
        super(peer);
    }

    /**
     * Construct a read-only MFVec2d field.
     *
     * @param vec2s An array of sets of x,y values
     */
    public ConstMFVec2d(double vec2s[][]) {
        super(MFVec2d.createPeer(vec2s));
    }

    /**
     * Construct a read-only MFVec2d field.
     *
     * @param vec2s List of x,y pairs
     */
    public ConstMFVec2d(double vec2s[]) {
        this(vec2s.length / 2, vec2s);
    }

    /**
     * Construct a read-only MFVec2d field.
     *
     * @param size Number of SFVec2d elements passed in.
     * @param vec2s List of x,y pairs
     */
    public ConstMFVec2d(int size, double vec2s[]) {
        super(MFVec2d.createPeer(size, vec2s));
    }

    public native int getSize();

    /**
     * Retrieves the value of an MFVec2d field.
     *
     * @param vec2s 2D array of x,y pairs to be returned.
     */
    public native void getValue(double vec2s[][]);

    /**
     * Retrieves the value of an MFVec2d field.
     *
     * @param vec2s Array of x,y pairs to be returned.
     */
    public native void getValue(double vec2s[]);

    /**
     * Retrieves a specific element in an MFVec2d and
     * returns it as a double array.
     *
     * @param index Position of desired element
     * @param vec2s Element at specified position
     */
    public native void get1Value(int index, double vec2s[]);

    /**
     * Retrieves a specific element in an MFVec2d and
     * returns it as an SFVec2d.
     *
     * @param index Position of desired element
     * @param vec Element at specified position
     */
    public void get1Value(int index, SFVec2d vec) {
        double[] v = { 0.0, 0.0 };
        this.get1Value(index, v);
        vec.setValue(v);
    }
}
