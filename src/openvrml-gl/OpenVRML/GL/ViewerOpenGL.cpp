// -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; -*-
//
// OpenVRML
//
// Copyright (C) 1998-2000  Chris Morley
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

# ifdef _WIN32
#   include <wtypes.h>
#   include <winbase.h>
#   include <windef.h>
#   include <wingdi.h>
# endif

# if HAVE_APPLE_OPENGL_FRAMEWORK
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
# else
#   include <GL/gl.h>
#   include <GL/glu.h>
# endif

# include <math.h>

# include <OpenVRML/System.h>
# include <OpenVRML/browser.h>
# include <OpenVRML/vrml97node.h>
# include <OpenVRML/bounding_volume.h>
# include <OpenVRML/VrmlFrustum.h>

# include "ViewerOpenGL.h"

// Textures are now done using OGL1.1 bindTexture API rather than
// display lists when this flag is set. Don't define this if you
// are still at OpenGL 1.0 (or get a newer OpenGL).

# define USE_TEXTURE_DISPLAY_LISTS 1

namespace {
    const float FPTOLERANCE(1.0e-6);

    inline bool fpzero(const float f) {
        return (fabs(f) <= FPTOLERANCE);
    }

    inline bool fpequal(const float a, const float b) {
        return fpzero(a - b);
    }

    inline double length(const float vec[3]) {
        return sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    }

    inline void normalize(float vec[3]) {
        const float len = float(length(vec));
        if (!fpzero(len)) {
            vec[0] /= len;
            vec[1] /= len;
            vec[2] /= len;
        }
    }

    class GLCapabilities {
    public:
        GLint maxModelviewStackDepth;

        GLCapabilities();
    };

    GLCapabilities::GLCapabilities()
    {
        glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH,
                      &this->maxModelviewStackDepth);
    }

    const GLCapabilities glCapabilities;
}

namespace OpenVRML {

/**
 * @brief OpenGL geometry renderer.
 */
namespace GL {

/**
 * @class ViewerOpenGL
 *
 * @brief Abstract class for display of VRML models using OpenGL/Mesa.
 *
 * A window-system specific subclass needs to redefine the pure
 * virtual methods.
 */

/**
 * @struct ViewerOpenGL::EventInfo
 *
 * @brief Input event.
 */

/**
 * @class ViewerOpenGL::ModelviewMatrixStack
 *
 * @brief Encapsulates and extended modelview matrix stack.
 *
 * OpenGL requires that implementations have a modelview matrix stack with a
 * maximum depth of only 32. Regardless of that, the maximum depth can be
 * expected to vary between implementations, and we don't want nesting of
 * Transform nodes in VRML worlds to be constrained by this limit.
 *
 * ModelviewMatrixStack uses the OpenGL modelview matrix stack until it fills
 * up, at which point any additional matrices that spill over are pushed onto
 * a conventional stack of mat4f.
 */

/**
 * @var size_t ViewerOpenGL::ModelviewMatrixStack::size
 *
 * @brief The current stack depth.
 */

/**
 * @var std::stack<mat4f> ViewerOpenGL::ModelviewMatrixStack::spillover
 *
 * @brief Any matrices that won't fit on the OpenGL modelview matrix stack get
 *      pushed onto this stack.
 */

/**
 * @brief Constructor.
 */
ViewerOpenGL::ModelviewMatrixStack::ModelviewMatrixStack():
    size(0)
{}

/**
 * @brief Push the current matrix onto the stack.
 *
 * @pre The current matrix is the modelview matrix.
 */
void ViewerOpenGL::ModelviewMatrixStack::push()
{
# ifndef NDEBUG
    GLint matrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
# endif
    assert(matrixMode == GL_MODELVIEW);
    if (this->size == size_t(glCapabilities.maxModelviewStackDepth - 1)) {
        mat4f mat;
        glGetFloatv(GL_MODELVIEW_MATRIX, &mat[0][0]);
        this->spillover.push(mat);
        glPopMatrix();
    }
    glPushMatrix();
    ++this->size;
}

/**
 * @brief Pop the current matrix off of the stack.
 *
 * @pre The current matrix is the modelview matrix.
 */
void ViewerOpenGL::ModelviewMatrixStack::pop()
{
# ifndef NDEBUG
    GLint matrixMode;
    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
# endif
    assert(matrixMode == GL_MODELVIEW);
    glPopMatrix();
    if (!this->spillover.empty()) {
        glLoadMatrixf(&this->spillover.top()[0][0]);
        this->spillover.pop();
        glPushMatrix();
    }
    --this->size;
}

/**
 * @struct ViewerOpenGL::LightInfo
 *
 * @brief Light information.
 */

namespace {
    //
    // The functions trackball and tb_project_to_sphere are derived from code
    // by Silicon Graphics, Inc.
    //
    // (c) Copyright 1993, 1994, Silicon Graphics, Inc.
    // ALL RIGHTS RESERVED
    // Permission to use, copy, modify, and distribute this software for
    // any purpose and without fee is hereby granted, provided that the above
    // copyright notice appear in all copies and that both the copyright notice
    // and this permission notice appear in supporting documentation, and that
    // the name of Silicon Graphics, Inc. not be used in advertising
    // or publicity pertaining to distribution of the software without specific,
    // written prior permission.
    //
    // THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
    // AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
    // INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
    // FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL SILICON
    // GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT,
    // SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY
    // KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING WITHOUT LIMITATION,
    // LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE, OR THE CLAIMS OF
    // THIRD PARTIES, WHETHER OR NOT SILICON GRAPHICS, INC.  HAS BEEN
    // ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
    // ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
    // POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
    //
    // US Government Users Restricted Rights
    // Use, duplication, or disclosure by the Government is subject to
    // restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
    // (c)(1)(ii) of the Rights in Technical Data and Computer Software
    // clause at DFARS 252.227-7013 and/or in similar or successor
    // clauses in the FAR or the DOD or NASA FAR Supplement.
    // Unpublished-- rights reserved under the copyright laws of the
    // United States.  Contractor/manufacturer is Silicon Graphics,
    // Inc., 2011 N.  Shoreline Blvd., Mountain View, CA 94039-7311.
    //
    // OpenGL(TM) is a trademark of Silicon Graphics, Inc.
    //
    //
    // Trackball code:
    //
    // Implementation of a virtual trackball.
    // Implemented by Gavin Bell, lots of ideas from Thant Tessman and
    //   the August '88 issue of Siggraph's "Computer Graphics," pp. 121-129.
    //

    //
    // Project an x, y pair onto a sphere of radius r OR a hyperbolic sheet
    // if we are away from the center of the sphere.
    //
    float tb_project_to_sphere(float r, float x, float y)
    {
        static const float sqrt2 = sqrt(2.0);
        static const float sqrt2_2 = sqrt2 / 2.0;

        float d, t, z;

        d = sqrt(x * x + y * y);
        if (d < r * sqrt2_2) { /* Inside sphere */
            z = sqrt(r * r - d * d);
        } else { /* On hyperbola */
            t = r / sqrt2;
            z = t * t / d;
        }
        return z;
    }

    //
    // Ok, simulate a track-ball.  Project the points onto the virtual
    // trackball, then figure out the axis of rotation, which is the cross
    // product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
    // Note:  This is a deformed trackball-- is a trackball in the center,
    // but is deformed into a hyperbolic sheet of rotation away from the
    // center.  This particular function was chosen after trying out
    // several variations.
    //
    // It is assumed that the arguments to this routine are in the range
    // (-1.0 ... 1.0)
    //
    const rotation trackball(float p1x, float p1y, float p2x, float p2y)
    {
        //
        // This size should really be based on the distance from the center of
        // rotation to the point on the object underneath the mouse.  That
        // point would then track the mouse as closely as possible.  This is a
        // simple example, though, so that is left as an Exercise for the
        // Programmer.
        //
        static const float trackballSize = 0.8;

        rotation result;

        if (p1x == p2x && p1y == p2y) {
            /* Zero rotation */
            return result;
        }

        //
        // First, figure out z-coordinates for projection of P1 and P2 to
        // deformed sphere
        //
        vec3f p1(p1x, p1y, tb_project_to_sphere(trackballSize, p1x, p1y));
        vec3f p2(p2x, p2y, tb_project_to_sphere(trackballSize, p2x, p2y));

        result.axis((p2 * p1).normalize());

        //
        // Figure out how much to rotate around that axis.
        //
        vec3f d = p1 - p2;
        float t = d.length() / (2.0 * trackballSize);

        //
        // Avoid problems with out-of-control values...
        //
        if (t > 1.0) { t = 1.0; }
        if (t < -1.0) { t = -1.0; }

        result.angle(2.0 * asin(t));

        return result;
    }
}

/**
 * @brief Construct a viewer for the specified browser.
 *
 * @param browser   the browser.
 */
ViewerOpenGL::ViewerOpenGL(OpenVRML::browser & browser):
    viewer(browser),
    tesselator(gluNewTess()),
    d_nSensitive(0),
    d_activeSensitive(0),
    d_overSensitive(0),
    d_selectMode(false),
    d_selectZ(0.0),
    d_rotationChanged(false)
{
    d_GLinitialized = false;
    d_blend = true;
    d_lit = true;
    d_texture = true;
    d_wireframe = false;

    // Don't make any GL calls here since the window probably doesn't exist.
    d_nObjects = 0;
    d_nestedObjects = 0;

    d_background[0] = d_background[1] = d_background[2] = 0.0;
    d_winWidth = 1;
    d_winHeight = 1;
    for (int i=0; i<MAX_LIGHTS; ++i)
      d_lightInfo[i].lightType = LIGHT_UNUSED;

    d_rotating = false;
    d_scaling = false;
    d_translating = false;
    this->curquat = quatf(trackball(0.0, 0.0, 0.0, 0.0));

    d_renderTime = 1.0;
    d_renderTime1 = 1.0;

    d_drawBSpheres = false;
    d_cull = true;
}

ViewerOpenGL::~ViewerOpenGL()
{
    gluDeleteTess(this->tesselator);
}

void ViewerOpenGL::initialize()
{
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

#if USE_STENCIL_SHAPE
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_ALWAYS, 1, 1);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
#endif

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  // should only trigger this after a non-uniform scale is seen...
  // on my system this is <5% speedup though.
  glEnable(GL_NORMALIZE);

  // blending is only enabled if d_blend and a non-zero transparency value is
  // passed in
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  d_GLinitialized = true;
}

namespace {

    // Call this after each frame for debugging...
    void checkErrors(const char * s)
    {
        GLenum glerr;
        while ((glerr = glGetError()) != GL_NO_ERROR) {
# ifdef macintosh
            if (glerr != 1285) // This avoids automatic switching between SW and
                               // HW renderers being continuously reported as an
                               // error
# endif
            theSystem->error("GL ERROR: %s %s\n", s, gluErrorString(glerr));
        }
    }
}


//
//  beginObject/endObject should correspond to grouping nodes.
//  Group-level scoping for directional lights, anchors, sensors
//  are handled here. Display lists can optionally be created
//  (but the retain flag is just a hint, not guaranteed). Retained
//  objects can be referred to later to avoid duplicating geometry.
//  OpenGL doesn't allow nested objects. The top-down approach of
//  putting entire groups in display lists is faster for static
//  scenes but uses more memory and means that if anything is changed,
//  the whole object must be tossed.
//  The bottom-up model wraps each piece of geometry in a dlist but
//  requires traversal of the entire scene graph to reference each dlist.
//  The decision about what groups to stuff in an object is punted to
//  the object itself, as it can decide whether it is mutable.
//
//  The OpenGL viewer never puts objects in display lists, so the
//  retain hint is ignored.

viewer::object_t ViewerOpenGL::begin_object(const char *,
                                            bool retain)
{
    // Finish setup stuff before first object
    if (1 == ++this->d_nObjects) {
        // Finish specifying the view (postponed to make Background easier)
        this->modelviewMatrixStack.push();
        if (!this->d_lit) { glDisable(GL_LIGHTING); }
    }

    ++this->d_nestedObjects;

    // Increment nesting level for group-scoped lights
    for (size_t i = 0; i < MAX_LIGHTS; ++i) {
        if (this->d_lightInfo[i].lightType == LIGHT_DIRECTIONAL) {
            ++this->d_lightInfo[i].nestingLevel;
        }
    }
    this->modelviewMatrixStack.push();
    return 0;
}

// End of group scope

void ViewerOpenGL::end_object()
{
    // Decrement nesting level for group-scoped lights and get rid
    // of any defined at this level
    for (size_t i = 0; i < MAX_LIGHTS; ++i) {
        if (this->d_lightInfo[i].lightType == LIGHT_DIRECTIONAL) {
            if (--this->d_lightInfo[i].nestingLevel < 0) {
                glDisable(GLenum(GL_LIGHT0 + i));
                this->d_lightInfo[i].lightType = LIGHT_UNUSED;
            }
        }
    }

    this->modelviewMatrixStack.pop();

    if (--this->d_nestedObjects == 0) {
        this->modelviewMatrixStack.pop();
    }
}


// These attributes need to be reset per geometry. Any attribute
// modified in a geometry function should be reset here. This is
// called after Appearance/Material has been set. Any attribute
// that can be modified by an Appearance node should not be modified
// here since these settings will be put in dlists with the geometry.

void ViewerOpenGL::beginGeometry()
{
  glPushAttrib( GL_ENABLE_BIT );
}

// This should be called BEFORE ending any dlists containing geometry,
// otherwise, attributes changed in the geometry insertion will impact
// the geometries rendered after the dlist is executed.

void ViewerOpenGL::endGeometry()
{
  glPopAttrib();
  glCullFace( GL_BACK );
  glShadeModel( GL_SMOOTH );
  glFrontFace( GL_CCW );

  // if needed...
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
}

viewer::rendering_mode ViewerOpenGL::mode()
{
    return d_selectMode
        ? pick_mode
        : draw_mode;
}

double ViewerOpenGL::frame_rate()
{
    return 1.0 / this->d_renderTime;
}

void ViewerOpenGL::reset_user_navigation()
{
    viewpoint_node & activeViewpoint = this->browser.active_viewpoint();
    activeViewpoint.user_view_transform(mat4f());

    this->curquat = quatf(trackball(0.0, 0.0, 0.0, 0.0));
    this->d_rotationChanged = true;

    wsPostRedraw();
}

void ViewerOpenGL::get_user_navigation(mat4f & M)
{
    // The Matrix M should be a unit matrix
    viewpoint_node & activeViewpoint = this->browser.active_viewpoint();
    M = activeViewpoint.user_view_transform() * M;
}

namespace {
    // Generate a normal from 3 indexed points.
    const vec3f indexFaceNormal(const int i1,
                                const int i2,
                                const int i3,
                                const std::vector<vec3f> & points)
    {
        const vec3f v1 = points[i2] - points[i3];
        const vec3f v2 = points[i2] - points[i1];
        return v1 * v2;
    }
}

/**
 * @brief Insert a background into a display list.
 *
 * @param groundAngle   ground angles.
 * @param groundColor   ground colors.
 * @param skyAngle      sky angles.
 * @param skyColor      sky colors.
 * @param whc           texture width, height, and number of components.
 * @param pixels        texture pixel data.
 *
 * @return display object identifier.
 */
viewer::object_t
ViewerOpenGL::insert_background(const std::vector<float> & groundAngle,
                                const std::vector<color> & groundColor,
                                const std::vector<float> & skyAngle,
                                const std::vector<color> & skyColor,
                                int * whc,
                                unsigned char ** pixels)
{
    float r = 0.0, g = 0.0, b = 0.0, a = 1.0;

    // Clear to last sky color
    if (!skyColor.empty()) {
        r = skyColor.back().r();
        g = skyColor.back().g();
        b = skyColor.back().b();
    }

    GLuint glid = 0;

    // Need to separate the geometry from the transformation so the
    // dlist doesn't have to get rebuilt for every mouse movement...
# if 0
    // Don't bother with a dlist if we aren't drawing anything
    if (!this->d_selectMode
            && (!skyAngle.empty() || !groundAngle.empty() || pixels)) {
        glid = glGenLists(1);
        glNewList(glid, GL_COMPILE_AND_EXECUTE);
    }
# endif

    glClearColor(r, g, b, a);
    GLuint mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
# if USE_STENCIL_SHAPE
    mask |= GL_STENCIL_BUFFER_BIT;
    glClear(mask);
# else
    glClear(mask);

    // Draw the background as big spheres centered at the view position
    if (!this->d_selectMode
            && (!skyAngle.empty() || !groundAngle.empty() || pixels)) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        this->modelviewMatrixStack.push();

        glScalef(1000.0, 1000.0, 1000.0);

        // Sphere constants
        const size_t nCirc = 8; // number of circumferential slices
        const double cd = 2.0 * pi / nCirc;

        double heightAngle0, heightAngle1 = 0.0;
        std::vector<color>::const_iterator c0, c1 = skyColor.begin();

        for (size_t nSky = 0; nSky < skyAngle.size(); ++nSky) {
            heightAngle0 = heightAngle1;
            heightAngle1 = skyAngle[nSky];
            c0 = c1;
            ++c1;

            double circAngle0, circAngle1 = 0.0;
            double sha0 = sin(heightAngle0), cha0 = cos(heightAngle0);
            double sha1 = sin(heightAngle1), cha1 = cos(heightAngle1);
            double sca0, cca0;
            double sca1 = sin(circAngle1), cca1 = cos(circAngle1);

            glBegin(GL_QUADS);
            for (size_t nc = 0; nc < nCirc; ++nc) {
                circAngle0 = circAngle1;
                circAngle1 = (nc + 1) * cd;
                sca0 = sca1;
                sca1 = sin(circAngle1);
                cca0 = cca1;
                cca1 = cos(circAngle1);

                glColor3fv(&(*c1)[0]);
                glVertex3f(sha1 * cca0, cha1, sha1 * sca0);
                glVertex3f(sha1 * cca1, cha1, sha1 * sca1);
                glColor3fv(&(*c0)[0]);
                glVertex3f(sha0 * cca1, cha0, sha0 * sca1);
                glVertex3f(sha0 * cca0, cha0, sha0 * sca0);
            }
            glEnd();
        }

        // Ground
        heightAngle1 = pi;
        c1 = groundColor.begin();

        for (size_t nGround = 0; nGround < groundAngle.size(); ++nGround) {
            heightAngle0 = heightAngle1;
            heightAngle1 = pi - groundAngle[nGround];
            c0 = c1;
            ++c1;

            double circAngle0, circAngle1 = 0.0;
            double sha0 = sin(heightAngle0), cha0 = cos(heightAngle0);
            double sha1 = sin(heightAngle1), cha1 = cos(heightAngle1);
            double sca0, cca0;
            double sca1 = sin(circAngle1), cca1 = cos(circAngle1);

            glBegin(GL_QUADS);
            for (size_t nc = 0; nc < nCirc; ++nc) {
                circAngle0 = circAngle1;
                circAngle1 = (nc + 1) * cd;
                sca0 = sca1;
                sca1 = sin(circAngle1);
                cca0 = cca1;
                cca1 = cos(circAngle1);

                glColor3fv(&(*c1)[0]);
                glVertex3f(sha1 * cca1, cha1, sha1 * sca1);
                glVertex3f(sha1 * cca0, cha1, sha1 * sca0);
                glColor3fv(&(*c0)[0]);
                glVertex3f(sha0 * cca0, cha0, sha0 * sca0);
                glVertex3f(sha0 * cca1, cha0, sha0 * sca1);
            }
            glEnd();
        }

        // Background textures are drawn on a transparent cube
        if (pixels && this->d_texture && !this->d_wireframe) {
            float v2[6][4][3] = {
              {{1,-1,1}, {-1,-1,1}, {-1,1,1}, {1,1,1}},     // Back
              {{-1,-1,1}, {1,-1,1}, {1,-1,-1}, {-1,-1,-1}}, // Bottom
              {{-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1}}, // Front
              {{-1,-1,1}, {-1,-1,-1}, {-1,1,-1}, {-1,1,1}}, // Left
              {{1,-1,-1}, {1,-1,1}, {1,1,1}, {1,1,-1}},     // Right
              {{-1,1,-1}, {1,1,-1}, {1,1,1}, {-1,1,1}}};    // Top

            // Tile big textures into 256x256 (or 256xsmaller or smallerx256) pieces

            float v3[6][4][3];
            int number_tiles;
            int number_vertices;
            const size_t NUM_SPLITS = 4;
            float v[NUM_SPLITS * NUM_SPLITS * 6][4][3];
            float size_x;
            float size_y;
            int number_splits_x;
            int number_splits_y;
            number_splits_x = NUM_SPLITS;
            number_splits_y = NUM_SPLITS;
            number_tiles = number_splits_x * number_splits_y;
            size_x = 2.0 / float(number_splits_x);
            size_y = 2.0 / float(number_splits_y);
            int i, j, k;
            for (j = 0; j < 16 * 6; j++) {
                for (k = 0; k < 4; k++) {
                    for (i = 0; i < 3; i++) { v[j][k][i] = 0; }
                }
            }

            for (j = 0; j < 6; j++) {
                for (k = 0; k < 4; k++) {
                    for (i = 0; i < 3; i++) {
                        v3[j][k][i] = v2[j][k][i] - v2[j][0][i];
                    }
                }
            }

            for (j = 0; j < 6; j++) {
                for (i = 0; i < 3; i++) {
                    v[j * number_tiles][0][i] = v2[j][0][i];
                    for (k = 0; k < 4; k++) {
                        v[j * number_tiles][k][i] =
                            v[j * number_tiles][0][i]
                            + v3[j][k][i] / number_splits_y;
                    }
                }
            }

            for (j = 0; j < 6; j++) {
                number_vertices = j * number_tiles + 1;
                for (k = 0; k < number_splits_y; k++) {
                    int num_line;
                    num_line = number_vertices - 1;
                    for (int l = 1; l < number_splits_x; l++) {
                        for (i = 0; i < 3; i++) {
                            v[number_vertices][0][i] =
                                v[number_vertices-1][0][i]
                                + v3[j][1][i] / number_splits_x;
                            for (int m = 0; m < 4; m++) {
                                v[number_vertices][m][i] =
                                    v[number_vertices][0][i]
                                    + v3[j][m][i] / number_splits_x;
                            }
                        }
                        number_vertices++;
                    }
                    if (k == (number_splits_y - 1)) { break; }
                    for (i = 0; i < 3; i++) {
                        v[number_vertices][0][i] =
                            v[num_line][0][i] + v3[j][3][i] / number_splits_y;
                        for (size_t m = 0; m < 4; m++) {
                            v[number_vertices][m][i] =
                                v[number_vertices][0][i]
                                + v3[j][m][i] / number_splits_y;
                        }
                    }
                    number_vertices++;
                }
            }

            glScalef(0.5, 0.5, 0.5);

            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            int vertices_number = 0;
            int t, lastT = -1;
            for (t = 0; t < 6; ++t, whc += 3) {
                for (j = 0; j < number_splits_y; j++) {
                    for (i = 0; i < number_splits_x; i++) {
                        // Check for non-zero width,height,coords and pixel data
                        if (whc[0] && whc[1] && whc[2] && pixels[t]) {
                            // Optimize for the case where the same texture is used
                            // do not work here....
                            // if (lastT == -1 || pixels[t] != pixels[lastT])
                            insertSubTexture(i * whc[0] / number_splits_x,
                                             j * whc[1] / number_splits_y,
                                             whc[0] / number_splits_x,
                                             whc[1] / number_splits_y,
                                             whc[0],
                                             whc[1],
                                             whc[2],
                                             false, false, pixels[t],
                                             false); // Don't put the textures in dlists

                            //
                            // The commented out code suggests what might be
                            // done to subdivide a texture with sides that are
                            // a *multiple* of 2 into textures that have sides
                            // that are a *power* of 2.
                            //
                            lastT = t;
                            glBegin(GL_QUADS);
                            // float len_x = 1.0 / number_splits_x;
                            // float len_y = 1.0 / number_splits_y;
                            // float x0 = i * len_x;
                            // float y0 = j * len_y;
                            // glTexCoord2f(x0, y0);
                            glTexCoord2f(0, 0);
                            glVertex3fv(v[vertices_number][0]);
                            // glTexCoord2f(x0 + len_x, y0);
                            glTexCoord2f(1, 0);
                            glVertex3fv(v[vertices_number][1]);
                            // glTexCoord2f(x0 + len_x, y0 + len_y);
                            glTexCoord2f(1, 1);
                            glVertex3fv(v[vertices_number][2]);
                            // glTexCoord2f(x0, y0 + len_y);
                            glTexCoord2f(0, 1);
                            glVertex3fv(v[vertices_number][3]);
                            glEnd();
                            vertices_number++;
                        }
                    }
                }
            }
            glDisable(GL_TEXTURE_2D);
        }

        // Put everything back the way it was
        this->modelviewMatrixStack.pop();

        if (this->d_lit) { glEnable(GL_LIGHTING); }
        glEnable(GL_DEPTH_TEST);
    }

# endif // USE_STENCIL_SHAPE

    if (glid) { glEndList(); }

    // Save bg color so we can choose a fg color (doesn't help bg textures...)
    this->d_background[0] = r;
    this->d_background[1] = g;
    this->d_background[2] = b;

    return object_t(glid);
}

/**
 * @brief Insert a box into a display list.
 *
 * @param size  box dimensions.
 *
 * @return display object identifier.
 */
viewer::object_t ViewerOpenGL::insert_box(const vec3f & size)
{
    GLuint glid = 0;

    if (!this->d_selectMode) {
        glid = glGenLists(1);
        glNewList(glid, GL_COMPILE_AND_EXECUTE);
    }

    static const GLint faces[6][4] =
    {
        {0, 1, 2, 3},
        {1, 5, 6, 2},
        {5, 4, 7, 6},
        {4, 0, 3, 7},
        {2, 6, 7, 3},
        {0, 4, 5, 1}
    };

    static const GLfloat n[6][3] =        // normals
    {
        {-1.0, 0.0, 0.0},
        {0.0, 0.0, 1.0},
        {1.0, 0.0, 0.0},
        {0.0, 0.0, -1.0},
        {0.0, 1.0, 0.0},
        {0.0, -1.0, 0.0}
    };

    GLfloat v[8][3];
    GLint i;

    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size.x() / 2;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = size.x() / 2;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size.y() / 2;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = size.y() / 2;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size.z() / 2;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = size.z() / 2;

    beginGeometry();
    glShadeModel(GL_FLAT);

    glBegin(GL_QUADS);
    for (i = 0; i < 6; ++i) {
        glNormal3fv(&n[i][0]);

        //glTexCoord2f(0.0, 1.0);
        glTexCoord2f(0.0, 0.0);
        glVertex3fv(&v[faces[i][0]][0]);

        //glTexCoord2f(1.0, 1.0);
        glTexCoord2f(1.0, 0.0);
        glVertex3fv(&v[faces[i][1]][0]);

        //glTexCoord2f(1.0, 0.0);
        glTexCoord2f(1.0, 1.0);
        glVertex3fv(&v[faces[i][2]][0]);

        //glTexCoord2f(0.0, 0.0);
        glTexCoord2f(0.0, 1.0);
        glVertex3fv(&v[faces[i][3]][0]);
    }
    glEnd();

    endGeometry();
    if (glid) { glEndList(); }

    return object_t(glid);
}

namespace {

    /**
     * @brief Build a cylinder object.
     *
     * @param height    the height for the cylinder.
     * @param radius    the radius for the cylinder.
     * @param numFacets the number of facets for the sides of the cylinder.
     * @retval c        the coordinates.
     * @retval tc       the texture coordinates.
     * @retval faces    the faces.
     *
     * It might be smarter to do just one, and reference it with scaling (but the
     * world creator could just as easily do that with DEF/USE ...).
     */
    void computeCylinder(const double height, const double radius,
                         const int numFacets, float c[][3],
                         float tc[][3], int faces[])
    {
        using OpenVRML::pi;

        double angle, x, y;
        int i, polyIndex;

        // Compute coordinates, texture coordinates:
        for (i = 0; i < numFacets; ++i) {
            angle = i * 2 * pi / numFacets;
            x = cos(angle);
            y = sin(angle);
            c[i][0] = radius * x;
            c[i][1] = 0.5 * height;
            c[i][2] = radius * y;
            c[numFacets+i][0] = radius * x;
            c[numFacets+i][1] = -0.5 * height;
            c[numFacets+i][2] = radius * y;

            if (tc) {
                double u = 0.75 - ((float) i) / numFacets;
                //double u = ((float) i) / numFacets + 0.25;
                //if ( u > 1.0 ) u -= 1.0;
                tc[i][0] = u; // ((float) i) / numFacets;
                tc[i][1] = 1.0;
                tc[i][2] = 0.0;
                tc[numFacets+i][0] = u; //((float) i) / numFacets;
                tc[numFacets+i][1] = 0.0;
                tc[numFacets+i][2] = 0.0;
            }
        }

        // And compute indices:
        for (i = 0; i < numFacets; ++i) {
            polyIndex = 5*i;
            faces[polyIndex + 0] = i;
            faces[polyIndex + 1] = (i+1) % numFacets;
            faces[polyIndex + 2] = (i+1) % numFacets + numFacets;
            faces[polyIndex + 3] = i + numFacets;
            faces[polyIndex + 4] = -1;
        }
    }
}

/**
 * @brief Insert a cone into a display list.
 *
 * @param height    height.
 * @param radius    radius at base.
 * @param bottom    show the bottom.
 * @param side      show the side.
 *
 * @return display object identifier.
 */
viewer::object_t ViewerOpenGL::insert_cone(const float height,
                                           const float radius,
                                           const bool bottom,
                                           const bool side)
{
    GLuint glid = 0;

    if (!this->d_selectMode) {
        glid = glGenLists(1);
        glNewList( glid, GL_COMPILE_AND_EXECUTE );
    }

    beginGeometry();
    if (!bottom || !side) { glDisable(GL_CULL_FACE); }

    if (bottom || side) {
        const int nfacets = 11;                // Number of polygons for sides
        const int npts = 2 * nfacets;
        const int nfaces = nfacets * 5;

        float c[npts][3];                // coordinates
        float tc[npts][3];                // texture coordinates
        int faces[nfaces];                // face lists

        // should only compute tc if a texture is present...
        computeCylinder(height, radius, nfacets, c, tc, faces);

        for (int i = 0; i < nfacets; ++i) { c[i][0] = c[i][2] = 0.0; }

        if (side) {
            float Ny = radius * radius / height;
            glBegin(GL_QUAD_STRIP);
            for (int i = 0; i < nfacets; ++i) {
                glNormal3f(c[i + nfacets][0], Ny, c[i + nfacets][2]);
                glTexCoord2fv(&tc[i + nfacets][0]);
                glVertex3fv(&c[i + nfacets][0]);
                glTexCoord2fv(&tc[i][0]);
                glVertex3fv(&c[i][0]);
            }

            glNormal3f(c[nfacets][0], Ny, c[nfacets][2]);
            glTexCoord2f(tc[nfacets][0] - 1.0, tc[nfacets][1]);
            glVertex3fv(&c[nfacets][0]);
            glTexCoord2f(tc[0][0] - 1.0, tc[0][1]);
            glVertex3fv(&c[0][0]);
            glEnd();
        }

        if (bottom) {
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0.0, -1.0, 0.0);
            glTexCoord2f(0.5, 0.5);
            glVertex3f(0.0, - 0.5 * height, 0.0);

            float angle = 0.5 * pi; // First v is at max x
            float aincr = 2.0 * pi / float(nfacets);
            for (int i = 0; i < nfacets; ++i, angle += aincr) {
                glTexCoord2f(      0.5 * (1.0 + sin(angle)),
                             1.0 - 0.5 * (1.0 + cos(angle)));
                glVertex3fv(&c[i + nfacets][0]);
            }
            glTexCoord2f(      0.5 * (1.0 + sin(angle)),
                         1.0 - 0.5 * (1.0 + cos(angle)));
            glVertex3fv(&c[nfacets][0]);
            glEnd();
        }
    }

    endGeometry();
    if (glid) { glEndList(); }

    return object_t(glid);
}

/**
 * @brief Insert a cylinder into a display list.
 *
 * @param height    height.
 * @param radius    radius.
 * @param bottom    show the bottom.
 * @param side      show the side.
 * @param top       show the top.
 *
 * @return display object identifier.
 */
viewer::object_t ViewerOpenGL::insert_cylinder(const float height,
                                               const float radius,
                                               const bool bottom,
                                               const bool side,
                                               const bool top)
{
    GLuint glid = 0;

    if (!this->d_selectMode) {
        glid = glGenLists(1);
        glNewList(glid, GL_COMPILE_AND_EXECUTE);
    }

    beginGeometry();
    if (!bottom || !side || !top) { glDisable(GL_CULL_FACE); }

    if (bottom || side || top) {
        const int nfacets = 8;                // Number of polygons for sides
        const int npts = 2 * nfacets;
        const int nfaces = nfacets * 5;

        float c[npts][3];                // coordinates
        float tc[npts][3];                // texture coordinates
        int faces[nfaces];                // face lists

        // should only compute tc if a texture is present...
        computeCylinder(height, radius, nfacets, c, tc, faces);

        if (side) {
            glBegin(GL_QUAD_STRIP);
            for (int i = 0; i < nfacets; ++i) {
                glNormal3f(c[i + nfacets][0], 0.0, c[i + nfacets][2]);
                glTexCoord2fv(&tc[i + nfacets][0]);
                glVertex3fv(&c[i + nfacets][0]);
                glTexCoord2fv(&tc[i][0]);
                glVertex3fv(&c[i][0]);
            }

            glNormal3f(c[nfacets][0], 0.0, c[nfacets][2]);
            glTexCoord2f(tc[nfacets][0] - 1.0, tc[nfacets][1]);
            glVertex3fv(&c[nfacets][0]);
            glTexCoord2f(tc[0][0]-1.0, tc[0][1]);
            glVertex3fv(&c[0][0]);
            glEnd();
        }

        if (bottom) {                // tex coords...
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0.0, -1.0, 0.0);
            glTexCoord2f(0.5, 0.5);
            glVertex3f(0.0, -0.5 * height, 0.0);

            float angle = 0.5 * pi; // First v is at max x
            float aincr = 2.0 * pi / float(nfacets);
            for (int i = 0; i < nfacets; ++i, angle += aincr) {
                glTexCoord2f(      0.5 * (1.0 + sin(angle)),
                             1.0 - 0.5 * (1.0 + cos(angle)));
                glVertex3fv(&c[i + nfacets][0]);
            }
            glTexCoord2f(      0.5 * (1.0 + sin(angle)),
                         1.0 - 0.5 * (1.0 + cos(angle)));
            glVertex3fv(&c[nfacets][0]);
            glEnd();
        }

        if (top) {                // tex coords...
            glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0.0, 1.0, 0.0);
            glTexCoord2f(0.5, 0.5);
            glVertex3f(0.0, 0.5 * height, 0.0);

            float angle = 0.75 * pi;
            float aincr = 2.0 * pi / float(nfacets);
            for (int i = nfacets-1; i >= 0; --i, angle += aincr) {
                glTexCoord2f(      0.5 * (1.0 + sin(angle)),
                             1.0 - 0.5 * (1.0 + cos(angle)));
                glVertex3fv(&c[i][0]);
            }
            glTexCoord2f(      0.5 * (1.0 + sin(angle)),
                         1.0 - 0.5 * (1.0 + cos(angle)));
            glVertex3fv(&c[nfacets - 1][0]);
            glEnd();
        }
    }

    endGeometry();
    if (glid) { glEndList(); }

    return object_t(glid);
}

namespace {

    // Compute a normal at vert i,j of an ElevationGrid.

    const vec3f elevationVertexNormal(
            const int i, const int j,
            const int nx, const int nz,
            const float dx, const float dz,
            const std::vector<float>::const_iterator height)
    {
        vec3f Vx, Vz;

        if (i > 0 && i < nx - 1) {
            Vx[0] = 2.0 * dx;
            Vx[1] = *(height + 1) - *(height - 1);
        } else if (i == 0) {
            Vx[0] = dx;
            Vx[1] = *(height + 1) - *(height);
        } else {
            Vx[0] = dx;
            Vx[1] = *(height) - *(height - 1);
        }
        Vx[2] = 0.0;

        Vz[0] = 0.0;
        if (j > 0 && j < nz - 1) {
            Vz[1] = *(height + nx) - *(height - nx);
            Vz[2] = 2.0 * dz;
        } else if (j == 0) {
            Vz[1] = *(height+nx) - *(height);
            Vz[2] = dz;
        } else {
            Vz[1] = *(height) - *(height - nx);
            Vz[2] = dz;
        }

        return Vz * Vx;
    }
}


/**
 * @brief Insert an elevation grid into a display list.
 *
 * @param mask
 * @param height        height field.
 * @param xDimension    vertices in the x direction.
 * @param zDimension    vertices in the z direction.
 * @param xSpacing      distance between vertices in the x direction.
 * @param zSpacing      distance between vertices in the z direction.
 * @param color         colors.
 * @param normal        normals.
 * @param texCoord      texture coordinates.
 */
viewer::object_t
ViewerOpenGL::insert_elevation_grid(const unsigned int mask,
                                    const std::vector<float> & height,
                                    const int32 xDimension,
                                    const int32 zDimension,
                                    const float xSpacing,
                                    const float zSpacing,
                                    const std::vector<color> & color,
                                    const std::vector<vec3f> & normal,
                                    const std::vector<vec2f> & texCoord)
{
    int32 i, j;
    float x, z;

    GLuint glid = 0;

    if (!this->d_selectMode) {
        glid = glGenLists(1);
        glNewList( glid, GL_COMPILE_AND_EXECUTE );
    }

    this->beginGeometry();

    // Face orientation & culling
    glFrontFace((mask & mask_ccw) ? GL_CCW : GL_CW);
    if (!(mask & mask_solid)) { glDisable(GL_CULL_FACE); }

    std::vector<float>::const_iterator h = height.begin();
    std::vector<OpenVRML::color>::const_iterator c = color.begin();
    std::vector<vec3f>::const_iterator n = normal.begin();
    std::vector<vec2f>::const_iterator tc = texCoord.begin();

    // x varies fastest
    for (j = 0; j < zDimension - 1; ++j) {
        float s0, s1, t0 = 0.0, t1 = 0.0;

        z = zSpacing * j;
        if (texCoord.empty()) {
            t0 = float(j)     / (zDimension - 1);
            t1 = float(j + 1) / (zDimension - 1);
        }

        glBegin(GL_QUAD_STRIP);

        for (i = 0; i < xDimension; ++i) {
            x = xSpacing * i;

            if (!color.empty() && ((mask & mask_color_per_vertex)
                                    || (i < xDimension - 1))) {
                glColor3fv(&(*c)[0]);
            }

            if (!normal.empty() && ((mask & mask_normal_per_vertex)
                                    || (i < xDimension - 1))) {
                glNormal3fv(&(*n)[0]);
            } else if (normal.empty()) {
                if (mask & mask_normal_per_vertex) {
                    const vec3f N =
                            elevationVertexNormal(i, j,
                                                  xDimension, zDimension,
                                                  xSpacing, zSpacing,
                                                  h);
                    glNormal3fv(&N[0]);
                } else if (i < xDimension - 1) {
                    //
                    // Normal per face.
                    //
                    const vec3f vx(xSpacing, *(h + 1) - *h, 0.0);
                    const vec3f vz(0.0, *(h + xDimension) - *h, zSpacing);
                    glNormal3fv(&(vx * vz)[0]);
                }
            }

            if (!texCoord.empty()) {
                s0 = tc->x();
                t0 = tc->y();
                s1 = (tc + xDimension * 2)->x();
                t1 = (tc + xDimension * 2)->y();
                ++tc;
            } else {
                s0 = s1 = float(i) / (xDimension - 1);
            }

            glTexCoord2f(s0, t0);
            glVertex3f(x, *h, z);

            // Vertex from next row
            if (!color.empty() && (mask & mask_color_per_vertex)) {
                glColor3fv(&(*(c + xDimension))[0]);
            }

            if (mask & mask_normal_per_vertex) {
                if (!normal.empty()) {
                    glNormal3fv(&(*(n + xDimension))[0]);
                } else {
                    const vec3f N =
                            elevationVertexNormal(i, j + 1,
                                                  xDimension, zDimension,
                                                  xSpacing, zSpacing,
                                                  h + xDimension);
                    glNormal3fv(&N[0]);
                }
            }

            glTexCoord2f(s1, t1);
            glVertex3f(x, *(h + xDimension), z + zSpacing);

            ++h;
            if (!color.empty()) { ++c; }
            if (!normal.empty()) { ++n; }
        }

        glEnd();
    }

    this->endGeometry();
    if (glid) { glEndList(); }
    return object_t(glid);
}


// Tesselator callback

# if defined(__CYGWIN__) || defined(__MINGW32__)
#   define OPENVRML_GL_CALLBACK_ __attribute__ ((__stdcall__))
# elif defined (_WIN32)
#   define OPENVRML_GL_CALLBACK_ APIENTRY
# else
#   define OPENVRML_GL_CALLBACK_
# endif
extern "C" {
    typedef GLvoid (OPENVRML_GL_CALLBACK_* TessCB)();
}

namespace {

    // Extrusion cap tessellation for non-convex shapes

    struct TessExtrusion {
        const float * c; // coordinates array [nVerts * 3]
        const float * crossSection; // crossSection coordinates [nCrossSection * 2]
        float tcDeltaU, tcDeltaV;
        float tcScaleU, tcScaleV;
        int vOffset;
        vec3f N; // Normal
    };

    void OPENVRML_GL_CALLBACK_ tessExtrusionBegin(const GLenum type, void * const pdata)
    {
        TessExtrusion * const p = static_cast<TessExtrusion *>(pdata);
        glBegin(type);
        glNormal3fv(&p->N[0]);
    }

    void OPENVRML_GL_CALLBACK_ tessExtrusionVertex(void * const vdata, void * const pdata)
    {
        const int j = reinterpret_cast<int>(vdata);
        TessExtrusion * const p = static_cast<TessExtrusion *>(pdata);

        glTexCoord2f((p->crossSection[2 * j] - p->tcDeltaU) * p->tcScaleU,
                     (p->crossSection[2 * j + 1] - p->tcDeltaV) * p->tcScaleV);
        glVertex3fv(&(p->c[3 * (j + p->vOffset)]));
    }
}

void ViewerOpenGL::insertExtrusionCaps(const unsigned int mask,
                                       const size_t nSpine,
                                       const std::vector<vec3f> & c,
                                       const std::vector<vec2f> & cs)
{
    // Determine x,z ranges for top & bottom tex coords
    float xz[4] = { cs[0].x(), cs[0].x(), cs[0].y(), cs[0].y() };
    std::vector<vec2f>::const_iterator csp = cs.begin();
    for (size_t nn = 1; nn < cs.size(); ++nn, ++csp) {
        if (csp->x() < xz[0]) {
            xz[0] = csp->x();
        } else if (csp->x() > xz[1]) {
            xz[1] = csp->x();
        }
        if (csp->y() < xz[2]) {
            xz[2] = csp->y();
        } else if (csp->y() > xz[3]) {
            xz[3] = csp->y();
        }
    }

    float dx = xz[1] - xz[0];
    float dz = xz[3] - xz[2];
    if (!fpzero(dx)) { dx = 1.0 / dx; }
    if (!fpzero(dz)) { dz = 1.0 / dz; }

    // If geometry is in dlists, should just always use the tesselator...

    const bool equalEndpts = cs.front() == cs.back();

    if (!(mask & mask_convex)) {
        gluTessCallback(this->tesselator, GLU_TESS_BEGIN_DATA,
                        reinterpret_cast<TessCB>(tessExtrusionBegin));
        gluTessCallback(this->tesselator, GLU_TESS_VERTEX_DATA,
                        reinterpret_cast<TessCB>(tessExtrusionVertex));
        gluTessCallback(this->tesselator, GLU_TESS_END, glEnd);

        if (mask & mask_bottom) {
            TessExtrusion bottom = { &c[0][0],
                                     &cs[0][0],
                                     xz[0], xz[2],
                                     dx, dz,
                                     0 };
            bottom.N = indexFaceNormal(0, 1, 2, c);

            gluTessBeginPolygon(this->tesselator, &bottom);
            gluTessBeginContour(this->tesselator);
            GLdouble v[3];
            // Mesa tesselator doesn;t like closed polys
            int j = equalEndpts ? cs.size() - 2 : cs.size() - 1;
            for (; j >= 0; --j) {
                v[0] = c[j].x();
                v[1] = c[j].y();
                v[2] = c[j].z();
                gluTessVertex(this->tesselator, v, reinterpret_cast<void *>(j));
            }
            gluTessEndContour(this->tesselator);
            gluTessEndPolygon(this->tesselator);
        }

        if (mask & mask_top) {
            int n = (nSpine - 1) * cs.size();
            TessExtrusion top = { &c[0][0],
                                  &cs[0][0],
                                  xz[0], xz[2],
                                  dx, dz,
                                  n };
            top.N = indexFaceNormal(n + 2, n + 1, n, c);

            gluTessBeginPolygon(this->tesselator, &top);
            gluTessBeginContour(this->tesselator);

            GLdouble v[3];
            // Mesa tesselator doesn;t like closed polys
            size_t j = equalEndpts ? 1 : 0;
            for (; j < cs.size(); ++j) {
                v[0] = c[j + n].x();
                v[1] = c[j + n].y();
                v[2] = c[j + n].z();
                gluTessVertex(this->tesselator, v, reinterpret_cast<void *>(j));
            }
            gluTessEndContour(this->tesselator);
            gluTessEndPolygon(this->tesselator);
        }
    } else {
        //
        // Convex (or not GLU1.2 ...)
        //
        vec3f N; // Normal

        if (mask & mask_bottom) {
            glBegin(GL_POLYGON);
            N = indexFaceNormal(0, 1, 2, c);
            glNormal3fv(&N[0]);

            for (int j = cs.size() - 1; j >= 0; --j) {
                glTexCoord2f((cs[j].x() - xz[0]) * dx,
                             (cs[j].y() - xz[2]) * dz);
                glVertex3fv(&c[j][0]);
            }
            glEnd();
        }

        if (mask & mask_top) {
            int n = (nSpine - 1) * cs.size();
            glBegin(GL_POLYGON);
            N = indexFaceNormal(n + 2, n + 1, n, c);
            glNormal3fv(&N[0]);

            for (size_t j = 0; j < cs.size(); ++j) {
                glTexCoord2f((cs[j].x() - xz[0]) * dx,
                             (cs[j].y() - xz[2]) * dz);
                glVertex3fv(&c[j + n][0]);
            }
            glEnd();
        }
    }
}

namespace {

    /**
     * @brief Build an extrusion.
     *
     * @param orientation
     * @param scale
     * @param crossSection
     * @param spine
     * @retval c
     * @retval tc
     */
    void computeExtrusion_(const std::vector<rotation> & orientation,
                           const std::vector<vec2f> & scale,
                           const std::vector<vec2f> & crossSection,
                           const std::vector<vec3f> & spine,
                           std::vector<vec3f> & c,
                           std::vector<vec2f> & tc)
    {
        size_t i, j, ci;

        // Xscp, Yscp, Zscp- columns of xform matrix to align cross section
        // with spine segments.
        vec3f Xscp(1.0, 0.0, 0.0);
        vec3f Yscp(0.0, 1.0, 0.0);
        vec3f Zscp(0.0, 0.0, 1.0);
        vec3f lastZ;

        // Is the spine a closed curve (last pt == first pt)?
        bool spineClosed = spine.back() == spine.front();

        // Is the spine a straight line?
        bool spineStraight = true;
        for (i = 1; i < spine.size() - 1; ++i) {
            const vec3f v = (spine[i - 1] - spine[i])
                            * (spine[i + 1] - spine[i]);
            if (!fpzero(v.length())) {
                spineStraight = false;
                lastZ = v.normalize();
                break;
            }
        }

        // If the spine is a straight line, compute a constant SCP xform
        if (spineStraight) {
            const vec3f v1(0.0, 1.0, 0.0);
            const vec3f v2 = spine.back() - spine.front();
            vec3f v3 = v2 * v1;
            double len = v3.length();
            if (!fpzero(len)) {
                //
                // Not aligned with Y axis.
                //
                v3 *= (1.0 / len);

                const rotation orient(v3, acos(v1.dot(v2))); // Axis/angle
                const mat4f scp = mat4f::rotation(orient); // xform matrix
                for (size_t k = 0; k < 3; ++k) {
                    Xscp[k] = scp[0][k];
                    Yscp[k] = scp[1][k];
                    Zscp[k] = scp[2][k];
                }
            }
        }

        // Orientation matrix
        mat4f om;
        if (orientation.size() == 1 && !fpzero(orientation.front().angle())) {
            om = mat4f::rotation(orientation.front());
        }

        using std::vector;
        vector<vec2f>::const_iterator s = scale.begin();
        vector<rotation>::const_iterator r = orientation.begin();
        // Compute coordinates, texture coordinates:
        for (i = 0, ci = 0; i < spine.size(); ++i, ci += crossSection.size()) {

            // Scale cross section
            for (j = 0; j < crossSection.size(); ++j) {
                c[ci + j].x(s->x() * crossSection[j].x());
                c[ci + j].y(0.0);
                c[ci + j].z(s->y() * crossSection[j].y());
            }

            // Compute Spine-aligned Cross-section Plane (SCP)
            if (!spineStraight) {
                int yi1, yi2, si1, s1i2, s2i2;

                if (spineClosed && (i == 0 || i == spine.size() - 1)) {
                    yi1 = spine.size() - 2;
                    yi2 = 1;
                    si1 = 0;
                    s1i2 = spine.size() - 2;
                    s2i2 = 1;
                } else if (i == 0) {
                    yi1 = 0;
                    yi2 = 1;
                    si1 = 1;
                    s1i2 = 0;
                    s2i2 = 2;
                } else if (i == spine.size() - 1) {
                    yi1 = spine.size() - 2;
                    yi2 = spine.size() - 1;
                    si1 = spine.size() - 2;
                    s1i2 = spine.size() - 3;
                    s2i2 = spine.size() - 1;
                } else {
                    yi1 = i - 1;
                    yi2 = i + 1;
                    si1 = i;
                    s1i2 = i - 1;
                    s2i2 = i + 1;
                }

                Yscp = (spine[yi2] - spine[yi1]).normalize();

                lastZ = Zscp; // Save last Zscp.
                Zscp = (spine[s1i2] - spine[si1]) * (spine[s2i2] - spine[si1]);

                float VlenZ = Zscp.length();
                if (fpzero(VlenZ)) {
                    Zscp = lastZ;
                } else {
                    Zscp *= (1.0 / VlenZ);
                }

                if (i > 0 && Zscp.dot(lastZ) < 0.0) { Zscp *= -1.0; }

                Xscp = Yscp * Zscp;
            }

            // Rotate cross section into SCP
            for (j = 0; j < crossSection.size(); ++j) {
                float cx, cy, cz;
                cx = c[ci + j].x() * Xscp.x()
                   + c[ci + j].y() * Yscp.x()
                   + c[ci + j].z() * Zscp.x();
                cy = c[ci + j].x() * Xscp.y()
                   + c[ci + j].y() * Yscp.y()
                   + c[ci + j].z() * Zscp.y();
                cz = c[ci + j].x() * Xscp.z()
                   + c[ci + j].y() * Yscp.z()
                   + c[ci + j].z() * Zscp.z();
                c[ci + j].x(cx);
                c[ci + j].y(cy);
                c[ci + j].z(cz);
            }

            //
            // Apply orientation.
            //
            if (!fpzero(r->angle())) {
                if (orientation.size() > 1) { om = mat4f::rotation(*r); }

                for (j = 0; j < crossSection.size(); ++j) {
                    float cx, cy, cz;
                    cx = c[ci + j].x() * om[0][0]
                       + c[ci + j].y() * om[1][0]
                       + c[ci + j].z() * om[2][0];
                    cy = c[ci + j].x() * om[0][1]
                       + c[ci + j].y() * om[1][1]
                       + c[ci + j].z() * om[2][1];
                    cz = c[ci + j].x() * om[0][2]
                       + c[ci + j].y() * om[1][2]
                       + c[ci + j].z() * om[2][2];
                    c[ci + j].x(cx);
                    c[ci + j].y(cy);
                    c[ci + j].z(cz);
                }
            }

            //
            // Translate cross section.
            //
            for (j = 0; j < crossSection.size(); ++j) {
                c[ci + j] += spine[i];

                // Texture coords
                tc[ci + j].x(float(j) / (crossSection.size() - 1));
                tc[ci + j].y(float(i) / (spine.size() - 1));
            }

            if (scale.size() > 1) { ++s; }
            if (orientation.size() > 1) { ++r; }
        }
    }

    /**
     * @brief Build an extrusion.
     *
     * @param orientation
     * @param scale
     * @param crossSection
     * @param spine
     * @retval c
     * @retval tc
     * @retval faces
     */
    void computeExtrusion_(const std::vector<rotation> & orientation,
                           const std::vector<vec2f> & scale,
                           const std::vector<vec2f> & crossSection,
                           const std::vector<vec3f> & spine,
                           std::vector<vec3f> & c,
                           std::vector<vec2f> & tc,
                           std::vector<int32> & faces)
    {
        computeExtrusion_(orientation, scale, crossSection, spine, c, tc);

        //
        // Compute face indices.
        //
        size_t polyIndex = 0;
        for (size_t i = 0, ci = 0; i < spine.size() - 1;
                ++i, ci += crossSection.size()) {
            for (size_t j = 0; j < crossSection.size() - 1; ++j) {
                faces[polyIndex + 0] = ci + j;
                faces[polyIndex + 1] = ci + j + 1;
                faces[polyIndex + 2] = ci + j + 1 + crossSection.size();
                faces[polyIndex + 3] = ci + j + crossSection.size();
                faces[polyIndex + 4] = -1;
                polyIndex += 5;
            }
        }
    }
}

/**
 * @brief Insert an extrusion into a display list.
 *
 * @param mask
 * @param spine         spine points.
 * @param crossSection  cross-sections.
 * @param orientation   cross-section orientations.
 * @param scale         cross-section scales.
 *
 * @return display object identifier.
 */
viewer::object_t
ViewerOpenGL::insert_extrusion(unsigned int mask,
                               const std::vector<vec3f> & spine,
                               const std::vector<vec2f> & crossSection,
                               const std::vector<rotation> & orientation,
                               const std::vector<vec2f> & scale)
{
    using std::vector;
    vector<vec3f> c(crossSection.size() * spine.size());
    vector<vec2f> tc(crossSection.size() * spine.size());

    computeExtrusion_(orientation, scale, crossSection, spine, c, tc);

    GLuint glid = 0;

    if (!this->d_selectMode) {
        glid = glGenLists(1);
        glNewList(glid, GL_COMPILE_AND_EXECUTE);
    }

    this->beginGeometry();

    // Face orientation & culling
    glFrontFace((mask & mask_ccw) ? GL_CCW : GL_CW);
    if (!(mask & mask_solid)) { glDisable(GL_CULL_FACE); }

    // Handle creaseAngle, correct normals, ...
    int n = 0;
    for (size_t i = 0; i < spine.size() - 1; ++i, n += crossSection.size()) {
        glBegin(GL_QUAD_STRIP);
        for (size_t j = 0; j < crossSection.size(); ++j) {
            // Compute normals
            vec3f v1 = j < crossSection.size() - 1
                     ? c[n + j + 1] - c[n + j]
                     : c[n + j] - c[n + j - 1];
            vec3f v2 = c[n + j + crossSection.size()] - c[n + j];
            v1 *= v2;
            glNormal3fv(&v1[0]);

            glTexCoord2fv(&tc[n + j + crossSection.size()][0]);
            glVertex3fv(&c[n + j + crossSection.size()][0]);
            glTexCoord2fv(&tc[n + j][0]);
            glVertex3fv(&c[n + j][0]);
          }
          glEnd();
      }

    // Draw caps. Convex can only impact the caps of an extrusion.
    if (mask & (mask_bottom | mask_top)) {
        this->insertExtrusionCaps(mask,
                                  spine.size(), c,
                                  crossSection);
    }

    this->endGeometry();
    if (glid) { glEndList(); }
    return object_t(glid);
}

/**
 * @brief Insert a line set into a display list.
 *
 * @param coord             coordinates.
 * @param coordIndex        coordinate indices.
 * @param colorPerVertex    whether colors are applied per-vertex or per-face.
 * @param color             colors.
 * @param colorIndex        color indices.
 *
 * @return display object identifier.
 */
viewer::object_t
ViewerOpenGL::insert_line_set(const std::vector<vec3f> & coord,
                              const std::vector<int32> & coordIndex,
                              bool colorPerVertex,
                              const std::vector<color> & color,
                              const std::vector<int32> & colorIndex)
{
    GLuint glid = 0;

    if (coord.size() < 2) { return 0; }

    if (!this->d_selectMode) {
        glid = glGenLists(1);
        glNewList(glid, GL_COMPILE_AND_EXECUTE);
    }

    this->beginGeometry();

    // Lighting, texturing don't apply to line sets
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    if (!color.empty() && !colorPerVertex) { glShadeModel(GL_FLAT); }

    glBegin(GL_LINE_STRIP);
    if (!color.empty() && !colorPerVertex) {
        glColor3fv(&color[!colorIndex.empty() ? colorIndex[0] : 0][0]);
    }

    size_t nl = 0;
    for (size_t i = 0; i < coordIndex.size(); ++i) {
        if (coordIndex[i] == -1) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            ++nl;
            if (i < coordIndex.size() - 1
                    && !color.empty() && !colorPerVertex) {
                const int32 index = !colorIndex.empty()
                                  ? colorIndex[nl]
                                  : nl;
                if (size_t(index) < color.size()) {
                    glColor3fv(&color[index][0]);
                }
            }
        } else {
            if (!color.empty() && colorPerVertex) {
                const int32 index = !colorIndex.empty()
                                  ? colorIndex[i]
                                  : coordIndex[i];
                if (size_t(index) < color.size()) {
                    glColor3fv(&color[index][0]);
                }
            }
            if (size_t(coordIndex[i]) < coord.size()) {
                glVertex3fv(&coord[coordIndex[i]][0]);
            }
        }
    }

    glEnd();

    this->endGeometry();

    if (glid) { glEndList(); }

    return object_t(glid);
}

/**
 * @brief Insert a point set into a display list.
 *
 * @param coord     points.
 * @param color     colors.
 *
 * @return display object identifier.
 */
viewer::object_t
ViewerOpenGL::insert_point_set(const std::vector<vec3f> & coord,
                               const std::vector<color> & color)
{
    GLuint glid = 0;

    if (!this->d_selectMode) {
        glid = glGenLists(1);
        glNewList(glid, GL_COMPILE_AND_EXECUTE);
    }

    this->beginGeometry();

    // Lighting, texturing don't apply to points
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glBegin(GL_POINTS);

    for (size_t i = 0; i < coord.size(); ++i) {
        if (i < color.size()) { glColor3fv(&color[i][0]); }
        glVertex3fv(&coord[i][0]);
    }

    glEnd();
    this->endGeometry();
    if (glid) { glEndList(); }

    return object_t(glid);
}

namespace {

    void computeBounds(size_t npoints, const float * points, float (&bounds)[6])
    {
        if (npoints == 0) {
            std::fill(bounds, bounds + 6, 0.0);
        } else {
            bounds[0] = bounds[1] = points[0]; // xmin, xmax
            bounds[2] = bounds[3] = points[1]; // ymin, ymax
            bounds[4] = bounds[5] = points[2]; // zmin, zmax

            for (size_t i = 1; i < npoints; ++i) {
                points += 3;
                if (points[0] < bounds[0])      bounds[0] = points[0];
                else if (points[0] > bounds[1]) bounds[1] = points[0];
                if (points[1] < bounds[2])      bounds[2] = points[1];
                else if (points[1] > bounds[3]) bounds[3] = points[1];
                if (points[2] < bounds[4])      bounds[4] = points[2];
                else if (points[2] > bounds[5]) bounds[5] = points[2];
            }
        }
    }

    void texGenParams(float bounds[],  // xmin,xmax, ymin,ymax, zmin,zmax
                      int axes[2],     // s, t
                      float params[4]) // s0, 1/sSize, t0, 1/tSize
    {
        axes[0] = 0;
        axes[1] = 1;
        params[0] = params[1] = params[2] = params[3] = 0.0;

        for (size_t nb = 0; nb < 3; ++nb) {
            float db = bounds[2 * nb + 1] - bounds[2 * nb];
            if (db > params[1]) {
                axes[1] = axes[0];
                axes[0] = nb;
                params[2] = params[0];
                params[3] = params[1];
                params[0] = bounds[2 * nb];
                params[1] = db;
            } else if (db > params[3]) {
                axes[1] = nb;
                params[2] = bounds[2 * nb];
                params[3] = db;
            }
        }

        // If two of the dimensions are zero, give up.
        if (fpzero(params[1]) || fpzero(params[3])) { return; }

        params[1] = 1.0 / params[1];
        params[3] = 1.0 / params[3];
    }

    /**
     * @internal
     */
    struct ShellData {
        unsigned int mask;
        const std::vector<vec3f> & coord;
        const std::vector<int32> & coordIndex;
        const std::vector<OpenVRML::color> & color;
        const std::vector<int32> & colorIndex;
        const std::vector<vec3f> & normal;
        const std::vector<int32> & normalIndex;
        const std::vector<vec2f> & texCoord;
        const std::vector<int32> & texCoordIndex;
        int *texAxes;
        float *texParams;
        size_t nf, i;
    };

    void insertShellConvex(ShellData * const s)
    {
        vec3f N;
        size_t i, nf = 0;                        // Number of faces

        for (i = 0; i < s->coordIndex.size(); ++i) {
            if (i == 0 || s->coordIndex[i] == -1) {
                if (i > 0) { glEnd(); }
                if (i == s->coordIndex.size() - 1) { break; }

                glBegin(GL_POLYGON);

                // Per-face attributes
                if (!s->color.empty()
                        && !(s->mask & viewer::mask_color_per_vertex)) {
                    const size_t index = !s->colorIndex.empty()
                                       ? s->colorIndex[nf]
                                       : nf;
                    glColor3fv(&s->color[index][0]);
                }

                if (! (s->mask & viewer::mask_normal_per_vertex)) {
                    int i1 = (i == 0)
                           ? 0
                           : i + 1;
                    if (!s->normal.empty()) {
                        const size_t index = !s->normalIndex.empty()
                                           ? s->normalIndex[nf]
                                           : nf;
                        glNormal3fv(&s->normal[index][0]);
                    } else if (i < s->coordIndex.size() - 4
                        && s->coordIndex[i1] >= 0
                        && s->coordIndex[i1 + 1] >= 0
                        && s->coordIndex[i1 + 2] >= 0) {
                        N = indexFaceNormal(s->coordIndex[i1],
                                            s->coordIndex[i1 + 1],
                                            s->coordIndex[i1 + 2],
                                            s->coord);

                        // Flip normal if primitiv-orientation is clockwise
                        if (!(s->mask & viewer::mask_ccw)) { N = -N; }
                        glNormal3fv(&N[0]);
                    }
                }

                ++nf;
            }

            if (s->coordIndex[i] >= 0) {
                // Per-vertex attributes
                if (!s->color.empty()
                        && (s->mask & viewer::mask_color_per_vertex)) {
                    const size_t index = !s->colorIndex.empty()
                                       ? s->colorIndex[i]
                                       : s->coordIndex[i];
                    glColor3fv(&s->color[index][0]);
                }

                if (s->mask & viewer::mask_normal_per_vertex) {
                    if (!s->normal.empty()) {
                        const size_t index = !s->normalIndex.empty()
                                           ? s->normalIndex[i]
                                           : s->coordIndex[i];
                        glNormal3fv(&s->normal[index][0]);
                    } else {
                        ; // Generate per-vertex normal here...
                    }
                }

                const vec3f & v = s->coord[s->coordIndex[i]];
                if (!s->texCoord.empty()) {
                    const size_t index = !s->texCoordIndex.empty()
                                       ? s->texCoordIndex[i]
                                       : s->coordIndex[i];
                    glTexCoord2fv(&s->texCoord[index][0]);
                } else {
                    float c0, c1;
                    c0 = (v[s->texAxes[0]] - s->texParams[0]) * s->texParams[1];
                    c1 = (v[s->texAxes[1]] - s->texParams[2]) * s->texParams[3];
                    glTexCoord2f(c0, c1);
                }

                glVertex3fv(&v[0]);
            }
        }

        // Watch out for no terminating -1 in face list
        // two ways to break out:
        //   i > 0 && i == coordIndex.size() - 1 && coordIndex[i] == -1
        //   i == coordIndex.size()
        //
        if (i >= s->coordIndex.size()) {
            if (s->coordIndex[i - 1] >= 0) { glEnd(); }
        } else {
            if (s->coordIndex[i] >= 0) { glEnd(); }
        }
    }

    void OPENVRML_GL_CALLBACK_ tessShellBegin(GLenum type, void * pdata)
    {
        ShellData * s = static_cast<ShellData *>(pdata);

        glBegin(type);

        // Per-face attributes
        if (!s->color.empty() && !(s->mask & viewer::mask_color_per_vertex)) {
            const size_t index = !s->colorIndex.empty()
                               ? s->colorIndex[s->nf]
                               : s->nf;
            glColor3fv(&s->color[index][0]);
        }

        if (!(s->mask & viewer::mask_normal_per_vertex)) {
            int i1 = (s->i == 0)
                   ? 0
                   : s->i - 1;
            if (!s->normal.empty()) {
                const size_t index = !s->normalIndex.empty()
                                   ? s->normalIndex[s->nf]
                                   : s->nf;
                glNormal3fv(&s->normal[index][0]);
            } else if (s->i < s->coordIndex.size() - 4
                    && s->coordIndex[i1] >= 0
                    && s->coordIndex[i1 + 1] >= 0
                    && s->coordIndex[i1 + 2] >= 0) {
                vec3f normal = indexFaceNormal(s->coordIndex[i1],
                                               s->coordIndex[i1 + 1],
                                               s->coordIndex[i1 + 2],
                                               s->coord);
                // Flip normal if primitiv-orientation is clockwise.
                if (!(s->mask & viewer::mask_ccw)) { normal = -normal; }
                glNormal3fv(&normal[0]);
              }
          }
    }

    void OPENVRML_GL_CALLBACK_ tessShellVertex(void * vdata, void * pdata)
    {
        int i = int(vdata);
        ShellData * s = static_cast<ShellData *>(pdata);

        // Per-vertex attributes
        if (!s->color.empty() && (s->mask & viewer::mask_color_per_vertex)) {
            const size_t index = !s->colorIndex.empty()
                               ? s->colorIndex[i]
                               : s->coordIndex[i];
            glColor3fv(&s->color[index][0]);
        }

        if (s->mask & viewer::mask_normal_per_vertex) {
            if (!s->normal.empty()) {
                const size_t index = !s->normalIndex.empty()
                                   ? s->normalIndex[i]
                                   : s->coordIndex[i];
                glNormal3fv(&s->normal[index][0]);
            } else {
                ; // Generate per-vertex normal here...
            }
        }

        const vec3f & v = s->coord[s->coordIndex[i]];
        if (!s->texCoord.empty()) {
            const size_t index = !s->texCoordIndex.empty()
                               ? s->texCoordIndex[i]
                               : s->coordIndex[i];
            glTexCoord2fv(&s->texCoord[index][0]);
        } else {
            float c0, c1;
            c0 = (v[s->texAxes[0]] - s->texParams[0]) * s->texParams[1];
            c1 = (v[s->texAxes[1]] - s->texParams[2]) * s->texParams[3];
            glTexCoord2f(c0, c1);
        }

        glVertex3fv(&v[0]);
    }

    void insertShellTess(GLUtesselator * tesselator, ShellData * s)
    {
        gluTessCallback(tesselator, GLU_TESS_BEGIN_DATA,
                        reinterpret_cast<TessCB>(tessShellBegin));
        gluTessCallback(tesselator, GLU_TESS_VERTEX_DATA,
                        reinterpret_cast<TessCB>(tessShellVertex));
        gluTessCallback(tesselator, GLU_TESS_END, glEnd);

        size_t i;
        for (i = 0; i < s->coordIndex.size(); ++i) {
            if (i == 0 || s->coordIndex[i] == -1) {
                if (i > 0) {
                    gluTessEndContour(tesselator);
                    gluTessEndPolygon(tesselator);
                    ++s->nf;
                }
                if (i == s->coordIndex.size() - 1) { break; }
                gluTessBeginPolygon(tesselator, s);
                gluTessBeginContour(tesselator);
                s->i = i;
            }

            if (s->coordIndex[i] >= 0) {
                GLdouble v[3] = { s->coord[s->coordIndex[i]].x(),
                                  s->coord[s->coordIndex[i]].y(),
                                  s->coord[s->coordIndex[i]].z() };
                gluTessVertex(tesselator, v, (void*)i);
            }
        }

        // Watch out for no terminating -1 in face list
        if (i > 1 && s->coordIndex[i - 1] >= 0) {
            gluTessEndContour(tesselator);
            gluTessEndPolygon(tesselator);
        }
    }
}


/**
 * @brief Insert a shell into a display list.
 *
 * @param mask
 * @param coord         coordinates.
 * @param coordIndex    coordinate indices.
 * @param color         colors.
 * @param colorIndex    color indices.
 * @param normal        normals.
 * @param normalIndex   normal indices.
 * @param texCoord      texture coordinates.
 * @param texCoordIndex texture coordinate indices.
 *
 * @return display object identifier.
 */
viewer::object_t
ViewerOpenGL::insert_shell(unsigned int mask,
                           const std::vector<vec3f> & coord,
                           const std::vector<int32> & coordIndex,
                           const std::vector<color> & color,
                           const std::vector<int32> & colorIndex,
                           const std::vector<vec3f> & normal,
                           const std::vector<int32> & normalIndex,
                           const std::vector<vec2f> & texCoord,
                           const std::vector<int32> & texCoordIndex)
{
    if (coordIndex.size() < 4) { return 0; } // 3 pts and a trailing -1

    // Texture coordinate generation parameters.
    int texAxes[2];                        // Map s,t to x,y,z
    float texParams[4];                // s0, 1/sSize, t0, 1/tSize

    // Compute bounding box for texture coord generation and lighting.
    if (texCoord.empty()) { // || any positional lights are active...
        float bounds[6]; // xmin,xmax, ymin,ymax, zmin,zmax
        computeBounds(coord.size(), &coord[0][0], bounds);

        // do the bounds intersect the radius of any active positional lights...

        texGenParams(bounds, texAxes, texParams);
        if (fpzero(texParams[1]) || fpzero(texParams[3])) { return 0; }
    }

    GLuint glid = 0;

    if (!this->d_selectMode) {
        glid = glGenLists(1);
        glNewList(glid, GL_COMPILE_AND_EXECUTE);
    }

    this->beginGeometry();

    // Face orientation & culling
    glFrontFace((mask & mask_ccw) ? GL_CCW : GL_CW);
    if (!(mask & mask_solid)) { glDisable(GL_CULL_FACE); }

    // Color per face
    if (!color.empty() && ! (mask & mask_color_per_vertex)) {
        glShadeModel(GL_FLAT);
    }

    // -------------------------------------------------------

    // Generation of per vertex normals isn't implemented yet...
    if (normal.empty() && (mask & mask_normal_per_vertex)) {
        mask &= ~mask_normal_per_vertex;
    }

    // -------------------------------------------------------

    // Should build tri strips (probably at the VrmlNode level)...

    ShellData s = {
        mask,
        coord, coordIndex,
        color, colorIndex,
        normal, normalIndex,
        texCoord, texCoordIndex,
        texAxes, texParams, 0
    };

    // Handle non-convex polys
    if (!(mask & mask_convex)) {
        insertShellTess(this->tesselator, &s);
    } else {
        insertShellConvex(&s);
    }

    endGeometry();
    if (glid) { glEndList(); }

    return object_t(glid);
}

namespace {

    void computeSphere(const double radius, const int numLatLong,
                       float c[][3], float tc[][3], int *faces)
    {
        using OpenVRML::pi;
        using OpenVRML::pi_2;

        double r, angle, x, y, z;
        int i, j, polyIndex;

        // Compute coordinates, texture coordinates:
        for (i = 0; i < numLatLong; ++i) {
            /*y = 2.0 * ( ((double)i) / (numLatLong-1) ) - 1.0;*/
            angle = ( i * pi / (numLatLong-1) ) - pi_2;
            y = sin( angle );
            r = sqrt( 1.0 - y*y );
            for (j = 0; j < numLatLong; ++j) {
                angle = 2 * pi * ((double)j) / numLatLong;
                x = - sin(angle)*r;
                z = - cos(angle)*r;
                c[i * numLatLong + j][0] = radius * x;
                c[i * numLatLong + j][1] = radius * y;
                c[i * numLatLong + j][2] = radius * z;
                if (tc) {
                    tc[i * numLatLong + j][0] = ((float) j)/(numLatLong);
                    tc[i * numLatLong + j][1] = ((float) i)/(numLatLong);
                    tc[i * numLatLong + j][2] = 0.0;
                }
            }
        }

        // And compute indices:
        if (faces) {
            for (i = 0; i < numLatLong-1; ++i) {
                for (j = 0; j < numLatLong; ++j) {
                    polyIndex = 5 * (i * numLatLong + j);
                    faces[polyIndex + 0] = i * numLatLong + j;
                    faces[polyIndex + 1] = i * numLatLong + (j + 1) % numLatLong;
                    faces[polyIndex + 2] = (i + 1) * numLatLong + (j + 1) % numLatLong;
                    faces[polyIndex + 3] = (i + 1) * numLatLong + j;
                    faces[polyIndex + 4] = -1;  // quad
                }
            }
        }
    }
}

/**
 * @brief Insert a sphere into a display list.
 *
 * @param radius    sphere radius.
 *
 * @return display object identifier.
 */
viewer::object_t ViewerOpenGL::insert_sphere(const float radius)
{
  GLuint glid = 0;

  if (! d_selectMode)
    {
      glid = glGenLists(1);
      glNewList( glid, GL_COMPILE_AND_EXECUTE );
    }

  const int numLatLong = 10;
  const int npts = numLatLong * numLatLong;

  float c[ npts ][ 3 ];
  float tc[ npts ][ 3 ];

  // should only compute tc if a texture is present...
  computeSphere(radius, numLatLong, c, tc, 0);

  beginGeometry();

  for ( int i = 0; i < numLatLong-1; ++i)
    {
      int n = i * numLatLong;

      glBegin( GL_QUAD_STRIP );

      for ( int j = 0; j < numLatLong; ++j )
        {
          glTexCoord2f( tc[n+j+numLatLong][0], tc[n+j+numLatLong][1] );
          glNormal3fv( &c[n+j+numLatLong][0] );
          glVertex3fv( &c[n+j+numLatLong][0] );

          glTexCoord2f( tc[n+j][0], tc[n+j][1] );
          glNormal3fv( &c[n+j][0] );
          glVertex3fv( &c[n+j][0] );
        }

      glTexCoord2f( 1.0, tc[n+numLatLong][1] );
      glNormal3fv( &c[n+numLatLong][0] );
      glVertex3fv( &c[n+numLatLong][0] );

      glTexCoord2f( 1.0, tc[n][1] );
      glNormal3fv( &c[n][0] );
      glVertex3fv( &c[n][0] );

      glEnd();
    }

  endGeometry();
  if (glid) glEndList();

  return object_t(glid);
}

/**
 * @brief Insert a directional light into a display list.
 *
 * @param ambientIntensity  ambient intensity.
 * @param intensity         intensity.
 * @param color             color.
 * @param direction         direction.
 *
 * @return display object identifier.
 */
viewer::object_t ViewerOpenGL::insert_dir_light(const float ambientIntensity,
                                                const float intensity,
                                                const color & color,
                                                const vec3f & direction)
{
    float amb[4] = { ambientIntensity * color.r(),
                     ambientIntensity * color.g(),
                     ambientIntensity * color.b(),
                     1.0 };
    float dif[4] = { intensity * color.r(),
                     intensity * color.g(),
                     intensity * color.b(),
                     1.0 };
    float pos[4] = { direction.x(), direction.y(), -direction.z(), 0.0 };

    // Find an unused light, give up if none left.
    int i;
    for (i = 0; i < MAX_LIGHTS; ++i) {
        if (this->d_lightInfo[i].lightType == LIGHT_UNUSED) { break; }
    }
    if (i == MAX_LIGHTS) { return 0; }

    this->d_lightInfo[i].lightType = LIGHT_DIRECTIONAL;
    this->d_lightInfo[i].nestingLevel = 0;
    GLenum light = GLenum(GL_LIGHT0 + i);

    glEnable(light);
    glLightfv(light, GL_AMBIENT, amb);
    glLightfv(light, GL_DIFFUSE, dif);
    glLightfv(light, GL_POSITION, pos);

    // Disable any old point/spot settings
    glLightf(light, GL_CONSTANT_ATTENUATION, 1.0);
    glLightf(light, GL_LINEAR_ATTENUATION, 0.0);
    glLightf(light, GL_QUADRATIC_ATTENUATION, 0.0);

    glLightf(light, GL_SPOT_CUTOFF, 180.0);
    glLightf(light, GL_SPOT_EXPONENT, 0.0);

    return 0;
}

/**
 * @brief Insert a point light into a display list.
 *
 * @param ambientIntensity  ambient intensity.
 * @param attenuation       attenuation.
 * @param color             color.
 * @param intensity         intensity.
 * @param location          location.
 * @param radius            radius.
 *
 * @return display object identifier.
 *
 * @todo Only objects within radius should be lit by each PointLight.
 *      Test each object drawn against each point light and enable
 *      the lights accordingly? Get light and geometry into consistent
 *      coordinates first.
 */
viewer::object_t ViewerOpenGL::insert_point_light(const float ambientIntensity,
                                                  const vec3f & attenuation,
                                                  const color & color,
                                                  const float intensity,
                                                  const vec3f & location,
                                                  const float radius)
{
    float amb[4] = { ambientIntensity * color.r(),
                     ambientIntensity * color.g(),
                     ambientIntensity * color.b(),
                     1.0 };
    float dif[4] = { intensity * color.r(),
                     intensity * color.g(),
                     intensity * color.b(),
                     1.0 };
    float pos[4] = { location.x(), location.y(), location.z(), 1.0 };

    // Find an unused light, give up if none left.
    int i;
    for (i = 0; i < MAX_LIGHTS; ++i) {
        if (this->d_lightInfo[i].lightType == LIGHT_UNUSED) { break; }
    }
    if (i == MAX_LIGHTS) { return 0; }

    this->d_lightInfo[i].lightType = LIGHT_POSITIONAL;
    this->d_lightInfo[i].location = location;
    this->d_lightInfo[i].radiusSquared = radius * radius;

    GLenum light(GL_LIGHT0 + i);

    // should be enabled/disabled per geometry based on distance & radius...
    glEnable(light);
    glLightfv(light, GL_AMBIENT, amb);
    glLightfv(light, GL_DIFFUSE, dif);
    glLightfv(light, GL_POSITION, pos);

    glLightf(light, GL_CONSTANT_ATTENUATION, attenuation[0]);
    glLightf(light, GL_LINEAR_ATTENUATION, attenuation[1]);
    glLightf(light, GL_QUADRATIC_ATTENUATION, attenuation[2]);

    // Disable old spot settings
    glLightf(light, GL_SPOT_CUTOFF, 180.0);
    glLightf(light, GL_SPOT_EXPONENT, 0.0);

    return 0;
}

/**
 * @brief Insert a point light into a display list.
 *
 * @param ambientIntensity  ambient intensity.
 * @param attenuation       attenuation.
 * @param beamWidth         beam width.
 * @param color             color.
 * @param cutOffAngle       cut-off angle.
 * @param direction         direction.
 * @param intensity         intensity.
 * @param location          location.
 * @param radius            radius.
 *
 * @return display object identifier.
 *
 * @todo Same comments as for PointLight apply here.
 */
viewer::object_t ViewerOpenGL::insert_spot_light(const float ambientIntensity,
                                                 const vec3f & attenuation,
                                                 const float beamWidth,
                                                 const color & color,
                                                 const float cutOffAngle,
                                                 const vec3f & direction,
                                                 const float intensity,
                                                 const vec3f & location,
                                                 const float radius)
{
    float amb[4] = { ambientIntensity * color.r(),
                     ambientIntensity * color.g(),
                     ambientIntensity * color.b(),
                     1.0 };
    float dif[4] = { intensity * color.r(),
                     intensity * color.g(),
                     intensity * color.b(),
                     1.0 };
    float pos[4] = { location.x(), location.y(), location.z(), 1.0 };


    // Find an unused light, give up if none left.
    int i;
    for (i = 0; i < MAX_LIGHTS; ++i) {
        if (this->d_lightInfo[i].lightType == LIGHT_UNUSED) { break; }
    }
    if (i == MAX_LIGHTS) { return 0; }

    this->d_lightInfo[i].lightType = LIGHT_POSITIONAL;
    this->d_lightInfo[i].location = location;
    this->d_lightInfo[i].radiusSquared = radius * radius;

    GLenum light(GL_LIGHT0 + i);

    // should be enabled/disabled per geometry based on distance & radius...
    glEnable(light);
    glLightfv(light, GL_AMBIENT, amb);
    glLightfv(light, GL_DIFFUSE, dif);
    glLightfv(light, GL_POSITION, pos);

    glLightf(light, GL_CONSTANT_ATTENUATION, attenuation[0]);
    glLightf(light, GL_LINEAR_ATTENUATION, attenuation[1]);
    glLightf(light, GL_QUADRATIC_ATTENUATION, attenuation[2]);

    glLightfv(light, GL_SPOT_DIRECTION, &direction[0]);
    glLightf(light, GL_SPOT_CUTOFF, cutOffAngle * 180.0 / pi);
    // The exponential dropoff is not right/spec compliant...
    glLightf(light, GL_SPOT_EXPONENT, beamWidth < cutOffAngle ? 1.0 : 0.0);

    return 0;
}


// Lightweight copy

viewer::object_t ViewerOpenGL::insert_reference(object_t existingObject)
{
    glCallList(GLuint(existingObject));
    return 0;
}

// Remove an object from the display list

void ViewerOpenGL::remove_object(object_t key)
{
    glDeleteLists(GLuint(key), 1 );
}


void ViewerOpenGL::enable_lighting(bool lightsOn)
{
    if (lightsOn) {
        if (d_lit) { glEnable(GL_LIGHTING); }
    } else {
        glDisable(GL_LIGHTING);
    }
}

/**
 * @brief Set the color.
 *
 * @param rgb   red, green, and blue components.
 * @param a     alpha (transparency) component.
 */
void ViewerOpenGL::set_color(const color & rgb, const float a)
{
    glColor4f(rgb.r(), rgb.g(), rgb.b(), a);
}

/**
 * @brief Set the fog.
 *
 * @param color             fog color.
 * @param visibilityRange   the distance at which objects are fully obscured by
 *                          fog.
 * @param type              fog type.
 */
void ViewerOpenGL::set_fog(const color & color,
                           const float visibilityRange,
                           const char * const type)
{
    static const std::string exponential("EXPONENTIAL");
    const GLfloat fogColor[4] = { color.r(), color.g(), color.b(), 1.0 };
    const GLint fogMode = (type == exponential)
                        ? GL_EXP
                        : GL_LINEAR;

    glEnable(GL_FOG);
    glFogf(GL_FOG_START, 1.5); // XXX What should this be?
    glFogf(GL_FOG_END, visibilityRange);
    glFogi(GL_FOG_MODE, fogMode);
    glFogfv(GL_FOG_COLOR, fogColor);
}

/**
 * @brief Set the material.
 *
 * @param ambientIntensity  ambient intensity.
 * @param diffuseColor      diffuse color.
 * @param emissiveColor     emissive color.
 * @param shininess         shininess.
 * @param specularColor     specular color.
 * @param transparency      transparency.
 */
void ViewerOpenGL::set_material(const float ambientIntensity,
                                const color & diffuseColor,
                                const color & emissiveColor,
                                const float shininess,
                                const color & specularColor,
                                const float transparency)
{
    const float alpha = 1.0 - transparency;

    const float ambient[4] = { ambientIntensity * diffuseColor.r(),
                               ambientIntensity * diffuseColor.g(),
                               ambientIntensity * diffuseColor.b(),
                               alpha };
    const float diffuse[4] = { diffuseColor.r(),
                               diffuseColor.g(),
                               diffuseColor.b(),
                               alpha };
    const float emission[4] = { emissiveColor.r(),
                                emissiveColor.g(),
                                emissiveColor.b(),
                                alpha };
    const float specular[4] = { specularColor.r(),
                                specularColor.g(),
                                specularColor.b(),
                                alpha };

    // XXX doesn't work right yet (need alpha render pass...)
    if (this->d_blend && ! fpzero(transparency)) { glEnable(GL_BLEND); }

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    //
    // In OGL standard range of shininess is [0.0,128.0]
    // In VRML97 the range is [0.0,1.0]
    //
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * 128);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

    if (diffuseColor == color(0.0, 0.0, 0.0)
            && specularColor == color(0.0, 0.0, 0.0)) {
        glDisable(GL_LIGHTING);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, diffuse);
        glColor4fv(emission);
    } else {
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
        glColor4fv(diffuse);
    }
}


// This hack is necessary because setting the color mode needs to know
// about the appearance (presence & components of texture) and the geometry
// (presence of colors). Putting this stuff in either insertTexture or
// insert<geometry> causes problems when the texture or geometry node is
// USE'd with a different context.

void ViewerOpenGL::set_material_mode(int textureComponents,
                                     bool colors)
{
  if (textureComponents && d_texture && ! d_wireframe)
    {
      glEnable( GL_TEXTURE_2D );

      // This is a hack: if per-{face,vertex} colors are specified,
      // they take precedence over textures with GL_MODULATE. The
      // textures won't be lit this way but at least they show up...
      if (textureComponents > 2 && colors)
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
      else
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    }
  else
    glDisable( GL_TEXTURE_2D );

  if (colors && textureComponents < 3 /* && lighting enabled... */ )
    glEnable( GL_COLOR_MATERIAL );
  else
    glDisable( GL_COLOR_MATERIAL );

}

void ViewerOpenGL::set_sensitive(node * object)
{
    if (object) {
        // should make this dynamic...
        if (this->d_nSensitive == MAXSENSITIVE) {
            theSystem->error("Internal Error: too many sensitive objects.\n");
            return;
        }

        // push name, increment object count
        this->d_sensitiveObject[this->d_nSensitive] = object;
        glPushName(++this->d_nSensitive); // array index+1
    } else {
        glPopName( );
    }
}


// Scale an image to make sizes powers of two. This puts the data back
// into the memory pointed to by pixels, so there better be enough.

void ViewerOpenGL::scale_texture(size_t w, size_t h,
                                 size_t newW, size_t newH,
                                 size_t nc,
                                 unsigned char* pixels)
{
  GLenum fmt[] = { GL_LUMINANCE,        // single component
                   GL_LUMINANCE_ALPHA,        // 2 components
                   GL_RGB,                // 3 components
                   GL_RGBA                // 4 components
  };

  unsigned char *newpix = new unsigned char[nc*newW*newH];

  glPixelStorei( GL_PACK_ALIGNMENT, 1 );
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  if (0 == gluScaleImage( fmt[nc-1], w, h, GL_UNSIGNED_BYTE, pixels,
                          newW, newH, GL_UNSIGNED_BYTE, newpix))
    memcpy(pixels, newpix, nc*newW*newH);

  delete [] newpix;
}


//
// Pixels are lower left to upper right by row.
//
viewer::texture_object_t
ViewerOpenGL::insert_texture(size_t w, size_t h, size_t nc,
                             bool repeat_s,
                             bool repeat_t,
                             const unsigned char * pixels,
                             bool retainHint)
{
  GLenum fmt[] = { GL_LUMINANCE,        // single component
                   GL_LUMINANCE_ALPHA,        // 2 components
                   GL_RGB,                // 3 components
                   GL_RGBA                // 4 components
  };

  GLuint glid = 0;

  if (d_selectMode) return 0;

  // Enable blending if needed
  if (d_blend && (nc == 2 || nc == 4))
    glEnable(GL_BLEND);

#if USE_TEXTURE_DISPLAY_LISTS
  if (retainHint) glGenTextures(1, &glid);
  glBindTexture( GL_TEXTURE_2D, glid );
#endif

  // Texturing is enabled in setMaterialMode
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  glTexImage2D( GL_TEXTURE_2D, 0, nc, w, h, 0,
                fmt[nc-1], GL_UNSIGNED_BYTE, pixels);

  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                   repeat_s ? GL_REPEAT : GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                   repeat_t ? GL_REPEAT : GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

  return texture_object_t(glid);
}


//
// Pixels are lower left to upper right by row.
//

viewer::texture_object_t
ViewerOpenGL::insertSubTexture(size_t xoffset, size_t yoffset,
                               size_t w, size_t h,
                               size_t whole_w,size_t whole_h,size_t nc,
                               bool repeat_s,
                               bool repeat_t,
                               const unsigned char *pixels,
                               bool retainHint)
{

  GLenum fmt[] = { GL_LUMINANCE,        // single component
                   GL_LUMINANCE_ALPHA,        // 2 components
                   GL_RGB,                // 3 components
                   GL_RGBA                // 4 components
  };

  GLuint glid = 0;

  if (d_selectMode) return 0;

  // Enable blending if needed
  if (d_blend && (nc == 2 || nc == 4))
    glEnable(GL_BLEND);

#if USE_TEXTURE_DISPLAY_LISTS
  if (retainHint) glGenTextures(1, &glid);
  glBindTexture( GL_TEXTURE_2D, glid );
#endif

  // Texturing is enabled in setMaterialMode
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  GLubyte* texturepart=new GLubyte[w*h*nc];
  for (size_t i = 0; i < h; i++)
    memcpy(texturepart+i*w*nc,pixels+(i+yoffset)*whole_w*nc+xoffset*nc, w*nc);

  glTexImage2D( GL_TEXTURE_2D, 0, nc, w, h, 0,
                fmt[nc-1], GL_UNSIGNED_BYTE, texturepart);

  delete [] texturepart;
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                   repeat_s ? GL_REPEAT : GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                   repeat_t ? GL_REPEAT : GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

  return texture_object_t(glid);
}


void ViewerOpenGL::insert_texture_reference(texture_object_t t, int nc)
{
#if USE_TEXTURE_DISPLAY_LISTS
  // Enable blending if needed
  if (d_blend && (nc == 2 || nc == 4))
    glEnable(GL_BLEND);

  glBindTexture( GL_TEXTURE_2D, (GLuint) t );
#endif
}


void ViewerOpenGL::remove_texture_object(texture_object_t t)
{
#if USE_TEXTURE_DISPLAY_LISTS
  GLuint glid = (GLuint) t;
  glDeleteTextures( 1, &glid );
#endif
}

/**
 * @brief Set the texture transform.
 *
 * Texture coordinate transform
 * Tc' = -C x S x R x C x T x Tc
 *
 * @param center        center.
 * @param rotation      rotation.
 * @param scale         scale.
 * @param translation   translation.
 */
void ViewerOpenGL::set_texture_transform(const vec2f & center,
                                         float rotation,
                                         const vec2f & scale,
                                         const vec2f & translation)
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glTranslatef(-center.x(), -center.y(), 0.0);
    glScalef(scale.x(), scale.y(), 1.0);
    if (!fpzero(rotation)) { glRotatef(rotation * 180.0 / pi, 0.0, 0.0, 1.0); }

    glTranslatef(center.x(), center.y(), 0.0);
    glTranslatef(translation.x(), translation.y(), 0.0);

    glMatrixMode(GL_MODELVIEW);
}

namespace {

    /**
     * Compute a target and up vector from position/orientation/distance.
     */
    void computeView(const vec3f & position,
                     const rotation & orientation,
                     const float distance,
                     vec3f & target,
                     vec3f & up)
    {
        // Graphics Gems, p 466. Convert between axis/angle and rotation matrix

        const double s = sin(orientation.angle());
        const double c = cos(orientation.angle());
        const double t = 1.0 - c;

        // Transform [0,0,1] by the orientation to determine sight line
        target.x(t * orientation.x() * orientation.z() + s * orientation.y());
        target.y(t * orientation.y() * orientation.z() - s * orientation.x());
        target.z(t * orientation.z() * orientation.z() + c);

        // Move along that vector the specified distance away from position[]
        target = target * -distance + position;

        // Transform [0,1,0] by the orientation to determine up vector
        up.x(t * orientation.x() * orientation.y() - s * orientation.z());
        up.y(t * orientation.y() * orientation.y() + c);
        up.z(t * orientation.y() * orientation.z() + s * orientation.x());
    }
}

/**
 * @brief Set the viewpoint.
 *
 * @param position          position.
 * @param orienation        orientation.
 * @param fieldOfView       field of view.
 * @param avatarSize        avatar size.
 * @param visibilityLimit   visiblity limit.
 */
void ViewerOpenGL::set_viewpoint(const vec3f & position,
                                 const rotation & orientation,
                                 const float fieldOfView,
                                 const float avatarSize,
                                 const float visibilityLimit)
{
    glMatrixMode( GL_PROJECTION );
    if (!this->d_selectMode) { glLoadIdentity(); }

    float field_of_view = fieldOfView * 180.0 / pi;
    float aspect = float(this->d_winWidth) / this->d_winHeight;
    float znear = (avatarSize > 0.0)
                ? 0.5 * avatarSize
                : 0.01;
    float zfar = (visibilityLimit > 0.0)
               ? visibilityLimit
               : 30000.0;
    gluPerspective(field_of_view, aspect, znear, zfar);

    VrmlFrustum frust(field_of_view, aspect, znear, zfar);
    this->frustum(frust);

    glMatrixMode(GL_MODELVIEW);

    // Guess some distance along the sight line to use as a target...
    float d = 10.0 * avatarSize;
    if (d < znear || d > zfar) { d = 0.2 * (avatarSize + zfar); }

    vec3f target, up;
    computeView(position, orientation, d, target, up);

    gluLookAt(position.x(), position.y(), position.z(),
              target.x(), target.y(), target.z(),
              up.x(), up.y(), up.z());
}


// The viewer knows the current viewpoint

void ViewerOpenGL::transform_points(int np, float *p)
{
  float m[16];
  glGetFloatv (GL_MODELVIEW_MATRIX, m);

  float x, y, z;
  for (int i=0; i<np; ++i)
    {
      x = m[0]*p[0] + m[4]*p[1] + m[8]*p[2] + m[12];
      y = m[1]*p[0] + m[5]*p[1] + m[9]*p[2] + m[13];
      z = m[2]*p[0] + m[6]*p[1] + m[10]*p[2] + m[14];

      p[0] = x;
      p[1] = y;
      p[2] = z;
      p += 3;
    }

}

/**
 * @brief Multiply the current modelview matrix by @p mat.
 *
 * @param mat   a matrix.
 */
void ViewerOpenGL::transform(const mat4f & mat)
{
    glMultMatrixf(&mat[0][0]);
}

//
//  Viewer callbacks (called from window system specific functions)
//

// update is called from a timer callback and from checkSensitive
void ViewerOpenGL::update(const double timeNow) {
    if (this->browser.update(timeNow)) {
        checkErrors("update");
        this->wsPostRedraw();
    }

    // Set an alarm clock for the next update time.
    this->wsSetTimer(this->browser.delta());
}

void ViewerOpenGL::redraw()
{
  if (! d_GLinitialized) initialize();

  double start = browser::current_time();


  glDisable( GL_FOG );                // this is a global attribute
  glDisable( GL_TEXTURE_2D );

  glEnable( GL_CULL_FACE );
  glFrontFace( GL_CCW );
  glCullFace( GL_BACK );

  if (d_lit) glEnable( GL_LIGHTING );
  glDisable( GL_COLOR_MATERIAL );

  glDisable(GL_BLEND);

  glShadeModel( GL_SMOOTH );

  d_nObjects = 0;
  d_nestedObjects = 0;

  d_nSensitive = 0;

  // Clean out any defined lights
  for (int i=0; i<MAX_LIGHTS; ++i)
    {
      d_lightInfo[i].lightType = LIGHT_UNUSED;
      GLenum light = (GLenum) (GL_LIGHT0 + i);
      glDisable( light );
    }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  this->browser.render(*this);

  wsSwapBuffers();

  d_renderTime1 = d_renderTime;
  d_renderTime = browser::current_time() - start;
}

void ViewerOpenGL::resize(int width, int height)
{
  if (width < 2) width = 2;
  if (height < 2) height = 2;
  glViewport(0, 0, width, height);
  d_winWidth = width;
  d_winHeight = height;
}


void ViewerOpenGL::input(EventInfo * e)
{
    switch (e->event) {
    case EVENT_KEY_DOWN:
        handleKey(e->what);
        break;
    case EVENT_MOUSE_MOVE:
        this->checkSensitive(e->x, e->y, EVENT_MOUSE_MOVE);
        break;
    case EVENT_MOUSE_CLICK:
    case EVENT_MOUSE_RELEASE:
        this->handleButton(e);
        break;
    case EVENT_MOUSE_DRAG:
        handleMouseDrag(e->x, e->y);
        break;
    }
}

void ViewerOpenGL::rot_trackball(float x1, float y1, float x2, float y2)
{
    const rotation rot = trackball(x1, y1, x2, y2);
    this->rotate(rot);
}

void ViewerOpenGL::rotate(const rotation & rot) throw ()
{
    this->lastquat = quatf(rot);
    if (fpzero(rot.angle())) { return; }

    viewpoint_node & activeViewpoint = this->browser.active_viewpoint();
    const mat4f & viewpointTransformation = activeViewpoint.transformation();
    const mat4f & currentUserViewTransform =
            activeViewpoint.user_view_transform();

    mat4f oldCameraTransform =
            currentUserViewTransform * viewpointTransformation;

    vec3f currentTranslation, currentScale;
    rotation currentRotation;
    oldCameraTransform.transformation(currentTranslation,
                                      currentRotation,
                                      currentScale);

    const mat4f r = mat4f::rotation(rot);

    const mat4f prevOrientation = mat4f::rotation(currentRotation);

    const mat4f t = mat4f::translation(currentTranslation);

    const mat4f newCameraTransform =
            prevOrientation * (t * (r * viewpointTransformation.inverse()));

    activeViewpoint.user_view_transform(newCameraTransform);

    mat4f rotationMatrix;
    glGetFloatv(GL_MODELVIEW_MATRIX, &rotationMatrix[0][0]);
    rotationMatrix[3][0] = 0.0;
    rotationMatrix[3][1] = 0.0;
    rotationMatrix[3][2] = 0.0;

    vec3f d = rotationMatrix * rot.axis();
    quatf q(rotation(d, rot.angle()));
    this->curquat = q * this->curquat;
    this->d_rotationChanged = true;

    wsPostRedraw();
}

void ViewerOpenGL::step(float x, float y, float z)
{
    mat4f t = mat4f::translation(vec3f(x, y, z));
    viewpoint_node & activeViewpoint = this->browser.active_viewpoint();
    activeViewpoint
            .user_view_transform(t * activeViewpoint.user_view_transform());
    wsPostRedraw();
}

void ViewerOpenGL::zoom(float z)
{
    GLint viewport[4];
    GLdouble modelview[16], projection[16];
    glGetIntegerv (GL_VIEWPORT, viewport);
    glGetDoublev (GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev (GL_PROJECTION_MATRIX, projection);
    Vrml97Node::NavigationInfo * nav =
            this->browser.bindable_navigation_info_top();
    GLdouble x_c = d_winWidth/2;
    GLdouble y_c = d_winHeight/2;
    GLdouble z_c = 0.5;
    float visibilityLimit=0.0;
    if (nav) { visibilityLimit = nav->getVisibilityLimit(); }
    if (fpzero(visibilityLimit)) { visibilityLimit = 30000.0; }
    GLdouble ox, oy, oz;
    gluUnProject(x_c, y_c, z_c, modelview, projection, viewport, &ox, &oy, &oz);
    z_c = z_c - 100.0 * z / visibilityLimit;
    GLdouble dx, dy, dz;
    gluUnProject(x_c, y_c, z_c, modelview, projection, viewport, &dx, &dy, &dz);
    dx -= ox;
    dy -= oy;
    dz -= oz;
    double dist = dx * dx + dy * dy + dz * dz;
    if (dist < 1.0e-25) { return; }
    dist = sqrt(dist);
    float speed = 1.0;
    if (nav) { speed = nav->getSpeed(); }
    dist = speed / dist;
    if (fpzero(dist)) { return; }
    dx *= dist;
    dy *= dist;
    dz *= dist;
    const vec3f translation(dx, dy, dz);
    mat4f t = mat4f::translation(translation);
    viewpoint_node & activeViewpoint = this->browser.active_viewpoint();
    const mat4f & userViewTransform = activeViewpoint.user_view_transform();
    activeViewpoint.user_view_transform(t * userViewTransform);
    wsPostRedraw();
}

void ViewerOpenGL::handleKey(int key)
{
    using std::find;
    using std::list;

    switch (key) {
    case KEY_LEFT:
          step(-1, 0, 0);
      break;

    case KEY_RIGHT:
          step(1, 0, 0);
     break;

    case KEY_UP:  // move forward along line of sight
      zoom(1);
      break;

    case KEY_DOWN: // move backwards along line of sight
      zoom(-1);
      break;

    case 'a':  // look up
      rot_trackball(0.0, 0.45, 0.0, 0.55);
      break;

    case 'z':  // look down
      rot_trackball(0.0, 0.55, 0.0, 0.45);
      break;

    case 'A':  // translate up
      step(0,1,0);
      break;

    case 'Z':  // translate down
      step(0,-1,0);
      break;

    case ',':                   // Look left
      rot_trackball(0.55, 0.0, 0.45, 0.0);
      break;

    case '.':                   // Look right
      rot_trackball(0.45, 0.0, 0.55, 0.0);
      break;

    //
    // XXX This can't work well. The list of Viewpoints in the world could
    // XXX change, and the resulting ordering would give unexpected results.
    // XXX The ordering of Viewpoints in the list should be the same as the
    // XXX order in which the nodes are encountered in a normal traversal of
    // XXX the scene graph.
    //
    case KEY_PAGE_DOWN:
        {
            viewpoint_node & currentViewpoint =
                this->browser.active_viewpoint();
            const list<viewpoint_node *> & viewpoints =
                this->browser.viewpoints();
            list<viewpoint_node *>::const_iterator pos =
                    find(viewpoints.begin(), viewpoints.end(),
                         &currentViewpoint);
            if (pos != viewpoints.end()) {
                ++pos;
                if (pos == viewpoints.end()) { pos = viewpoints.begin(); }
                (*pos)->process_event("set_bind", sfbool(true),
                                      browser::current_time());
            }
        }
        wsPostRedraw();
        break;

    case KEY_PAGE_UP:
        {
            viewpoint_node & currentViewpoint =
                this->browser.active_viewpoint();
            const list<viewpoint_node *> & viewpoints =
                this->browser.viewpoints();
            list<viewpoint_node *>::const_iterator pos =
                    find(viewpoints.begin(), viewpoints.end(),
                         &currentViewpoint);
            if (pos != viewpoints.end()) {
                if (pos == viewpoints.begin()) {
                    pos = viewpoints.end();
                }
                --pos;
                (*pos)->process_event("set_bind", sfbool(true),
                                      browser::current_time());
            }
        }
        wsPostRedraw();
        break;

    case 'b':
      d_blend = ! d_blend;
      wsPostRedraw();
      theSystem->inform(" Alpha blending %sabled.",
                     d_blend ? "en" : "dis");
      break;

    case 'd':
      d_drawBSpheres = ! d_drawBSpheres;
      theSystem->inform(" bspheres %sabled.", d_drawBSpheres ? "en" : "dis");
      wsPostRedraw();
      break;

    case 'c':
      d_cull = ! d_cull;
      theSystem->inform(" culling %sabled.", d_cull ? "en" : "dis");
      wsPostRedraw();
      break;

    case 'l':
      d_lit = ! d_lit;
      theSystem->inform(" Lighting %sabled.", d_lit ? "en" : "dis");
      wsPostRedraw();
      break;

    case KEY_HOME:
    case 'r':                        // Reset view
      this->reset_user_navigation();
      break;

    case 't':
      d_texture = ! d_texture;
      wsPostRedraw();
      theSystem->inform(" Texture mapping %sabled.",
                     d_texture ? "en" : "dis");
      break;

    case 'w':                        // Wireframe (should disable texturing)
      d_wireframe = ! d_wireframe;
      glPolygonMode(GL_FRONT_AND_BACK, d_wireframe ? GL_LINE : GL_FILL);
      wsPostRedraw();
      theSystem->inform(" Drawing polygons in %s mode.", d_wireframe ? "wireframe" : "filled");
      break;

    default:
      break;
    }
}

// Mouse button up/down

void ViewerOpenGL::handleButton( EventInfo *e)
{
  d_rotating = d_scaling = d_translating = false;

  // Check for a sensitive object first
  if (e->what == 0 &&
      checkSensitive( e->x, e->y, e->event ) )
    return;

  d_activeSensitive = 0;

  // Nothing under the mouse
  if (e->event == EVENT_MOUSE_RELEASE)
    wsSetCursor( CURSOR_INHERIT );
  else
    switch (e->what)                // button
      {
      case 0:
        wsSetCursor( CURSOR_CYCLE );
        d_rotating = true;
        d_beginx = e->x;
        d_beginy = e->y;
        break;

      case 1:
        wsSetCursor( CURSOR_UP_DOWN );
        d_scaling = true;
        d_beginx = e->x;
        d_beginy = e->y;
        break;

      case 2:
        wsSetCursor( CURSOR_CROSSHAIR );
        d_translating = true;
        d_beginx = e->x;
        d_beginy = e->y;
        break;
      }
}


// Mouse movement with button down

void ViewerOpenGL::handleMouseDrag(int x, int y)
{
 if (d_activeSensitive)
    {
      (void) checkSensitive( x, y, EVENT_MOUSE_DRAG );
    }
  else if (d_rotating)
    {
      rot_trackball((2.0 * d_beginx - d_winWidth) / d_winWidth,
                (d_winHeight - 2.0 * d_beginy) / d_winHeight,
                (2.0 * x - d_winWidth) / d_winWidth,
                (d_winHeight - 2.0 * y) / d_winHeight);
      d_beginx = x;
      d_beginy = y;
    }
  // This is not scaling, it is now moving in screen Z coords
  else if (d_scaling)
    {
      zoom((float) (d_beginy - y) / d_winHeight );
      d_beginx = x;
      d_beginy = y;
    }
  else if (d_translating)
    {
      step( (float) (x - d_beginx) / d_winWidth,
            (float) (d_beginy - y) / d_winHeight,
             0.0 );
      d_beginx = x;
      d_beginy = y;
    }

}


/**
 * Check for pickable objects.
 */
bool ViewerOpenGL::checkSensitive(const int x, const int y,
                                  const EventType mouseEvent) {
    double timeNow = browser::current_time();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLuint selectBuffer[4 * MAXSENSITIVE];
    glSelectBuffer(4 * MAXSENSITIVE, selectBuffer);

    glRenderMode(GL_SELECT);
    this->d_selectMode = true;

    glInitNames();
    glPushName(0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(GLdouble(x), GLdouble(viewport[3] - y),
                  2.0, 2.0, viewport);

    // Set up the global attributes
    glDisable(GL_FOG);
    glDisable(GL_TEXTURE_2D);

    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);

    glShadeModel(GL_FLAT);

    this->d_nObjects = 0;
    this->d_nestedObjects = 0;

    this->d_nSensitive = 0;

    // Clean out any defined lights
    for (int i=0; i < MAX_LIGHTS; ++i) {
        this->d_lightInfo[i].lightType = LIGHT_UNUSED;
        GLenum light = GLenum(GL_LIGHT0 + i);
        glDisable(light);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    this->browser.render(*this);

    this->d_selectMode = false;

    // select closest hit
    GLuint closest = 0, minz = 0xffffffff;

    int selected = 0;   // nothing selected
    int hits = glRenderMode(GL_RENDER);
    if (hits > 0) {
        // selectBuffer[] = { N1, z1, z2, name1, ..., nameN1, N2, ... }
        GLuint * sel = selectBuffer;

        for (int nh=0; nh < hits; ++nh, sel += (3 + sel[0])) {
            if (sel[1] <= minz) {
                minz = sel[1];  // z1
                closest = sel[2 + sel[0]];  // nameN (most deeply nested)
            }
        }

        if (closest > 0 && closest <= GLuint(this->d_nSensitive)) {
            selected = closest;
        }
    }

    this->wsSetCursor(selected ? CURSOR_INFO : CURSOR_INHERIT);

    //
    // Compute & store the world coords of the pick if something
    // was already active or is about to become active. The window
    // Z coord is retained when a drag is started on a sensitive
    // so the drag stays in the same plane even if the mouse moves
    // off the original sensitive object.
    //
    double selectCoord[3] = { 0.0, 0.0, 0.0 };

    if (this->d_activeSensitive || selected) {
        if (!this->d_activeSensitive) {
            this->d_selectZ = minz / double(0xffffffff);
        }

        GLint viewport[4];
        GLdouble modelview[16], projection[16];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);
  //      glGetDoublev (GL_MODELVIEW_MATRIX, modelview);

        //
        // make modelview as a unit matrix as this is taken care in the core side
        // during render traversal.
        //
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                modelview[4 * i + j] = (i == j) ? 1.0 : 0.0;
            }
        }

        GLdouble dx, dy, dz;
        gluUnProject(GLdouble(x), GLdouble(viewport[3] - y), this->d_selectZ,
                     modelview, projection, viewport,
                     &dx, &dy, &dz);

        selectCoord[0] = dx;
        selectCoord[1] = dy;
        selectCoord[2] = dz;
    }

    bool wasActive = false;
  // To unset PickMatrix...
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Sanity check. This can happen when the world gets replaced
    // by clicking on an anchor - the current sensitive object goes
    // away, but these variables are not reset.
    if (this->d_activeSensitive > this->d_nSensitive) {
        this->d_activeSensitive = 0;
    }
    if (this->d_overSensitive > this->d_nSensitive) {
        this->d_overSensitive = 0;
    }

    // An active sensitive object "grabs" the mouse until button released
    if (this->d_activeSensitive) {
        if (mouseEvent == EVENT_MOUSE_RELEASE
                || mouseEvent == EVENT_MOUSE_MOVE) {
            this->browser.sensitive_event(
                this->d_sensitiveObject[this->d_activeSensitive - 1],
                timeNow,
                selected == this->d_activeSensitive,
                false,
                selectCoord);
            this->d_activeSensitive = 0;
        } else {
            // _DRAG
            this->browser.sensitive_event(
                this->d_sensitiveObject[this->d_activeSensitive - 1],
                timeNow,
                selected == this->d_activeSensitive,
                true,
                selectCoord);
        }
        wasActive = true;
    } else if (mouseEvent == EVENT_MOUSE_CLICK && selected) {
        //
        // A click down over a sensitive object initiates an active grab and
        // mouse over events are no longer relevant.
        //
        if (d_overSensitive && d_overSensitive != selected) {
            this->browser.sensitive_event(
                this->d_sensitiveObject[this->d_overSensitive - 1],
                timeNow,
                false, false, // isOver, isActive
                selectCoord);
            this->d_overSensitive = 0;
        }
        this->d_activeSensitive = selected;
        this->browser.sensitive_event(
            this->d_sensitiveObject[this->d_activeSensitive - 1],
            timeNow,
            true, true, // isOver, isActive
            selectCoord);
    } else if (mouseEvent == EVENT_MOUSE_MOVE) {
        // Handle isOver events (coords are bogus)
        if (d_overSensitive && d_overSensitive != selected) {
            this->browser.sensitive_event(
                this->d_sensitiveObject[this->d_overSensitive - 1],
                timeNow,
                false, false, // isOver, isActive
                selectCoord);
        }
        this->d_overSensitive = selected;
        if (this->d_overSensitive) {
            this->browser.sensitive_event(
                this->d_sensitiveObject[this->d_overSensitive - 1],
                timeNow,
                true, false,  // isOver, isActive
                selectCoord);
        }
    }

    // Was event handled here?
    if (this->d_activeSensitive || wasActive) { this->update(timeNow); }

    // Everything is handled except down clicks where nothing was selected
    // and up clicks where nothing was active.
    return this->d_activeSensitive || wasActive;
}

void
ViewerOpenGL::draw_bounding_sphere(
    const bounding_sphere & bs,
    const bounding_volume::intersection intersection)
{
    static const GLfloat green[] = { 0.25f, 1.0f, 0.25f, 1.0f };
    static const GLfloat red[] = { 1.0f, 0.5f, 0.5f, 1.0f };
    static const GLfloat grey[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    if (!this->d_drawBSpheres || bs.maximized() || bs.radius() == -1.0) {
        return;
    }
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    //glEnable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    GLUquadricObj * sph = 0;
    glMatrixMode(GL_MODELVIEW);
    this->modelviewMatrixStack.push();
    const vec3f & c = bs.center();
    glTranslatef(c.x(), c.y(), c.z());
    sph = gluNewQuadric();
    switch (intersection) {
    case bounding_volume::outside:
        //glDisable(GL_LIGHTING);
        //glEnable(GL_LIGHTING);
        gluQuadricDrawStyle(sph, GLU_LINE);
        //gluQuadricDrawStyle(sph, GLU_POINT);
        glColor3f(0.5, 0.5, 0.5);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, grey);
        gluSphere(sph, bs.radius(), 5, 5);
        break;

    case bounding_volume::partial:
        //glEnable(GL_LIGHTING);
        gluQuadricNormals(sph, GLU_SMOOTH);
        //gluQuadricDrawStyle(sph, GLU_FILL);
        gluQuadricDrawStyle(sph, GLU_LINE);
        glColor3f(0.25, 1.0, 0.25);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
        gluSphere(sph, bs.radius(), 8, 8);
        break;

    case bounding_volume::inside:
        //glEnable(GL_LIGHTING);
        gluQuadricNormals(sph, GLU_SMOOTH);
        //gluQuadricDrawStyle(sph, GLU_FILL);
        gluQuadricDrawStyle(sph, GLU_LINE);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
        glColor3f(1.0, 0.5, 0.5);
        gluSphere(sph, bs.radius(), 8, 8);
        break;

    default:
        //glEnable(GL_LIGHTING);
        gluQuadricNormals(sph, GLU_SMOOTH);
        gluQuadricDrawStyle(sph, GLU_LINE);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, grey);
        glColor3f(0.5, 0.5, 0.5);
        gluSphere(sph, bs.radius(), 8, 8);
    }
    gluDeleteQuadric(sph);
    this->modelviewMatrixStack.pop();
}

} // namespace GL

} // namespace OpenVRML
