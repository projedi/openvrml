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

# ifdef HAVE_CONFIG_H
#   include <config.h>
# endif

# include <algorithm>
# include <numeric>
# include <assert.h>
# include "field.h"
# include "private.h"
# include "node.h"

// Printing methods



namespace OpenVRML {

/**
 * @brief Stream output.
 *
 * @param out           an output stream.
 * @param fieldValue    a FieldValue.
 *
 * @return @p out.
 */
std::ostream & operator<<(std::ostream & out, const FieldValue & fieldValue) {
    return fieldValue.print(out);
}

/**
 * @class FieldValue
 *
 * @brief Abstract base class for the VRML field types.
 */

/**
 * @enum FieldValue::Type
 *
 * @brief Used to identify FieldValue types.
 *
 * These tags are typically used to designate an expected type or avoid a
 * <code>dynamic_cast</code>.
 */

/**
 * @var FieldValue::invalidType
 *
 * @brief Zero value typically used to indicate failure.
 */

/**
 * @var FieldValue::sfbool
 *
 * @brief Designates an SFBool.
 */

/**
 * @var FieldValue::sfcolor
 *
 * @brief Designates an SFColor.
 */

/**
 * @var FieldValue::sffloat
 *
 * @brief Designates an SFFloat.
 */

/**
 * @var FieldValue::sfimage
 *
 * @brief Designates an SFImage.
 */

/**
 * @var FieldValue::sfint32
 *
 * @brief Designates an SFInt32.
 */

/**
 * @var FieldValue::sfnode
 *
 * @brief Designates an SFNode.
 */

/**
 * @var FieldValue::sfrotation
 *
 * @brief Designates an SFRotation.
 */

/**
 * @var FieldValue::sfstring
 *
 * @brief Designates an SFString.
 */

/**
 * @var FieldValue::sftime
 *
 * @brief Designates an SFTime.
 */

/**
 * @var FieldValue::sfvec2f
 *
 * @brief Designates an SFVec2f.
 */

/**
 * @var FieldValue::sfvec3f
 *
 * @brief Designates an SFVec3f.
 */

/**
 * @var FieldValue::mfcolor
 *
 * @brief Designates an MFColor.
 */

/**
 * @var FieldValue::mffloat
 *
 * @brief Designates an MFFloat.
 */

/**
 * @var FieldValue::mfint32
 *
 * @brief Designates an MFInt32.
 */

/**
 * @var FieldValue::mfnode
 *
 * @brief Designates an MFNode.
 */

/**
 * @var FieldValue::mfrotation
 *
 * @brief Designates an MFRotation.
 */

/**
 * @var FieldValue::mfstring
 *
 * @brief Designates an MFString.
 */

/**
 * @var FieldValue::mftime
 *
 * @brief Designates an MFTime.
 */

/**
 * @var FieldValue::mfvec2f
 *
 * @brief Designates an MFVec2f.
 */

/**
 * @var FieldValue::mfvec3f
 *
 * @brief Designates an MFVec3f.
 */

/**
 * @brief Destructor.
 */
FieldValue::~FieldValue() throw () {}

/**
 * @fn FieldValue * FieldValue::clone() const throw (std::bad_alloc)
 *
 * @brief Virtual copy constructor.
 *
 * @return a new FieldValue identical to this one.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 *
 * @todo This should probably return an std::auto_ptr<FieldValue>.
 */

/**
 * @fn FieldValue & FieldValue::assign(const FieldValue & value) throw (std::bad_cast, std::bad_alloc)
 *
 * @brief Virtual assignment.
 *
 * @param value the value to assign to the object.
 *
 * @return this object.
 *
 * @exception std::bad_cast if value is not of the same concrete type as this
 *                          object.
 */

/**
 * @fn std::ostream & FieldValue::print(std::ostream & out) const
 *
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */

/**
 * @fn FieldValue::Type FieldValue::type() const throw ()
 *
 * @brief Get the field type.
 *
 * @return the Type enumerant corresponding to this FieldValue's type
 */

namespace {
    const char * ftn[] = {
        "<invalid field type>",
        "SFBool",
        "SFColor",
        "SFFloat",
        "SFImage",
        "SFInt32",
        "SFNode",
        "SFRotation",
        "SFString",
        "SFTime",
        "SFVec2f",
        "SFVec3f",
        "MFColor",
        "MFFloat",
        "MFInt32",
        "MFNode",
        "MFRotation",
        "MFString",
        "MFTime",
        "MFVec2f",
        "MFVec3f"
    };
}

/**
 * @brief Stream output.
 *
 * @relates FieldValue
 *
 * @param out   an output stream.
 * @param type  a FieldValue type identifier.
 *
 * @return @p out.
 */
std::ostream & operator<<(std::ostream & out, const FieldValue::Type type) {
    return out << ftn[type];
}

/**
 * @brief Stream input.
 *
 * @relates FieldValue
 *
 * @param in    an input stream.
 * @param type  a FieldValue type identifier.
 *
 * @return @p in.
 */
std::istream & operator>>(std::istream & in, FieldValue::Type & type) {
    std::string str;
    in >> str;
    const char * const * const begin = ftn + FieldValue::sfbool;
    const char * const * const end = ftn + FieldValue::mfvec3f + 1;
    const char * const * pos = std::find(begin, end, str);
    type = (pos == end)
         ? FieldValue::invalidType
         : static_cast<FieldValue::Type>
                (FieldValue::invalidType + (pos - begin));
    return in;
}

/**
 * @class SFBool
 *
 * @brief Encapsulates an SFBool value.
 */

/**
 * @brief Constructor.
 *
 * @param value initial value
 */
SFBool::SFBool(bool value) throw (): d_value(value) {}

/**
 * @brief Destructor.
 */
SFBool::~SFBool() throw () {}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFBool::clone() const throw (std::bad_alloc) {
    return new SFBool(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFBool.
 */
FieldValue & SFBool::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFBool &>(value));
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFBool::print(std::ostream & out) const {
    return out << (d_value ? "TRUE" : "FALSE");
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfbool.
 */
FieldValue::Type SFBool::type() const throw () { return sfbool; }

/**
 * @brief Get the value.
 *
 * @return the value of this SFBool
 */
bool SFBool::get() const throw () { return this->d_value; }

/**
 * @brief Set the value.
 *
 * @param value the new value
 */
void SFBool::set(bool value) throw () { this->d_value = value; }


/**
 * @class SFColor
 *
 * @brief Encapsulates an SFColor value.
 */

/**
 * @typedef SFColor::ConstArrayReference
 *
 * @brief A reference to the SFColor's 3-element array.
 */

/**
 * @brief Construct with the default value, (0, 0, 0).
 */
SFColor::SFColor() throw () {
    this->d_rgb[0] = 0.0f;
    this->d_rgb[1] = 0.0f;
    this->d_rgb[2] = 0.0f;
}

/**
 * @brief Construct a SFColor.
 *
 * @param rgb a 3-element array
 */
SFColor::SFColor(ConstArrayReference rgb) throw () {
    this->d_rgb[0] = rgb[0];
    this->d_rgb[1] = rgb[1];
    this->d_rgb[2] = rgb[2];
}

/**
 * @brief Construct a SFColor
 *
 * @param r red component
 * @param g green component
 * @param b blue component
 */
SFColor::SFColor(float r, float g, float b) throw () {
    d_rgb[0] = r;
    d_rgb[1] = g;
    d_rgb[2] = b;
}

/**
 * @brief Destructor.
 */
SFColor::~SFColor() throw () {}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFColor::clone() const throw (std::bad_alloc) {
    return new SFColor(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFColor.
 */
FieldValue & SFColor::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFColor &>(value));
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFColor::print(std::ostream & out) const {
    return out << d_rgb[0] << ' ' << d_rgb[1] << ' ' << d_rgb[2];
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfcolor.
 */
FieldValue::Type SFColor::type() const throw () { return sfcolor; }

/**
 * @brief Array element dereference operator (const version).
 *
 * @param index an index from 0 - 2
 */
float SFColor::operator[](size_t index) const throw () {
    assert(index < 3);
    return this->d_rgb[index];
}

/**
 * @brief Array element dereference operator (non-const version).
 *
 * @param index an index from 0 - 2
 */
float & SFColor::operator[](size_t index) throw () {
    assert(index < 3);
    return this->d_rgb[index];
}

/**
 * @brief Get the red component.
 *
 * @return the red component value
 */
float SFColor::getR() const throw () { return this->d_rgb[0]; }

/**
 * @brief Get the green component.
 *
 * @return the green component value
 */
float SFColor::getG() const throw () { return this->d_rgb[1]; }

/**
 * @brief Get the blue component.
 * @return the blue component value
 */
float SFColor::getB() const throw () { return this->d_rgb[2]; }

/**
 * @brief Get the value.
 *
 * @return a reference to a 3-element array comprising the RGB value
 */
SFColor::ConstArrayReference SFColor::get() const throw () {
    return this->d_rgb;
}

/**
 * @brief Set the value.
 *
 * @param rgb a 3-element vector comprising a RGB value
 */
void SFColor::set(ConstArrayReference rgb) throw () {
    this->d_rgb[0] = rgb[0];
    this->d_rgb[1] = rgb[1];
    this->d_rgb[2] = rgb[2];
}

// Conversion functions between RGB each in [0,1] and HSV with  
// h in [0,360), s,v in [0,1]. From Foley, van Dam p615-616.

/**
 * @brief Convert a color from HSV to RGB.
 *
 * Convert from HSV (with(with <var>h</var> in [0,360), <var>s</var>,
 * <var>v</var> in [0,1]) to RGB (with each component in [0,1]).
 *
 * @param hsv a 3-element array comprising an HSV value
 * @retval rgb a 3-element array comprising an RGB value
 */
void SFColor::HSVtoRGB(ConstArrayReference hsv, ArrayReference rgb) throw ()
{
    float h = hsv[0];
    if (hsv[1] == 0.0) {
        rgb[0] = rgb[1] = rgb[2] = hsv[2];
    } else {
        if (h >= 360.0) {
            h -= 360.0;
        }
        h /= 60.0;
        const double i = floor(h);
        const double f = h - i;
        const float p = hsv[2] * (1.0 - hsv[1]);
        const float q = hsv[2] * (1.0 - hsv[1] * f);
        const float t = hsv[2] * (1.0 - hsv[1] * (1.0 - f));
        switch (static_cast<int>(i)) {
	    default:
	    case 0: rgb[0] = hsv[2]; rgb[1] = t; rgb[2] = p; break;
	    case 1: rgb[0] = q; rgb[1] = hsv[2]; rgb[2] = p; break;
	    case 2: rgb[0] = p; rgb[1] = hsv[2]; rgb[2] = t; break;
	    case 3: rgb[0] = p; rgb[1] = q; rgb[2] = hsv[2]; break;
	    case 4: rgb[0] = t; rgb[1] = p; rgb[2] = hsv[2]; break;
	    case 5: rgb[0] = hsv[2]; rgb[1] = p; rgb[2] = q; break;
	}
    }
}

/**
 * @brief Convert a color from RGB to HSV.
 *
 * Convert from RGB (with each component in [0,1]) to HSV (with <var>h</var> in
 * [0,360), <var>s</var>, <var>v</var> in [0,1]).
 *
 * @param rgb a 3-element array comprising an RGB value
 * @retval hsv a 3-element array comprising an HSV value
 */
void SFColor::RGBtoHSV(ConstArrayReference rgb, ArrayReference hsv) throw ()
{
    const float maxrgb = *std::max_element(rgb, rgb + 3);
    const float minrgb = *std::min_element(rgb, rgb + 3);
    
    hsv[0] = 0.0;
    hsv[1] = (maxrgb > 0.0) ? ((maxrgb - minrgb) / maxrgb) : 0.0;
    hsv[2] = maxrgb;
    
    if (hsv[1] != 0.0) {
        const float rc = (maxrgb - rgb[0]) / (maxrgb - minrgb);
        const float gc = (maxrgb - rgb[1]) / (maxrgb - minrgb);
        const float bc = (maxrgb - rgb[2]) / (maxrgb - minrgb);
        
        if (rgb[0] == maxrgb) {
            hsv[0] = bc - gc;
        } else if (rgb[1] == maxrgb) {
            hsv[0] = 2 + rc - bc;
        } else {
            hsv[0] = 4 + gc - rc;
        }
        
        hsv[0] *= 60.0;
        if (hsv[0] < 0.0) {
            hsv[0] += 360.0;
        }
    }
}

/**
 * @brief Set the value using HSV.
 *
 * @param h the hue component
 * @param s the saturation component
 * @param v the value component
 */
void SFColor::setHSV(float h, float s, float v) throw () {
    const float hsv[3] = { h, s, v };
    HSVtoRGB(hsv, this->d_rgb);
}

/**
 * @brief Get the value expressed in HSV.
 *
 * @retval hsv a 3-element array comprising the HSV value.
 */
void SFColor::getHSV(ArrayReference hsv) const throw () {
    RGBtoHSV(this->d_rgb, hsv);
}


/**
 * @class SFFloat
 *
 * @brief Encapsulates an SFFloat value.
 */

/**
 * @brief Constructor.
 *
 * @param value initial value
 */
SFFloat::SFFloat(float value) throw (): d_value(value) {}

/**
 * @brief Destructor.
 */
SFFloat::~SFFloat() throw () {}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFFloat::print(std::ostream & out) const {
    return out << d_value;
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFFloat::clone() const throw (std::bad_alloc) {
    return new SFFloat(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFFloat.
 */
FieldValue & SFFloat::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFFloat &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sffloat.
 */
FieldValue::Type SFFloat::type() const throw () { return sffloat; }

/**
 * @brief Get value.
 *
 * @return the SFFloat value
 */
float SFFloat::get() const throw () { return this->d_value; }

/**
 * @brief Set value.
 *
 * @param value the new value
 */
void SFFloat::set(float value) throw () { this->d_value = value; }

/**
 * @class SFImage
 *
 * A single uncompressed 2-dimensional pixel image. The first
 * hexadecimal value is the lower left pixel and the last value is the
 * upper right pixel.Pixel values are limited to 256 levels of
 * intensity. A one-component image specifies one-byte greyscale
 * values. A two-component image specifies the intensity in the first
 * (high) byte and the alpha opacity in the second (low) byte. A
 * three-component image specifies the red component in the first
 * (high) byte, followed by the green and blue components.
 * Four-component images specify the alpha opacity byte after
 * red/green/blue.
 */

/**
 * Construct the default SFImage.
 */
SFImage::SFImage() throw (): d_w(0), d_h(0), d_nc(0), d_pixels(0) {}

/**
 * @brief Create an SFImage.
 *
 * Note that the pixels read from lower left to upper right, which
 * is a reflection around the y-axis from the "normal" convention.
 * <p>
 * Note also that width and height are specified in pixels, and a
 * pixel may be more than one byte wide. For example, an image with
 * a width and height of 16, and nc==3, would have a pixel array
 * w*h*nc = 16*16*3 = 768 bytes long. See the class intro above for
 * the interpretation of different pixel depths.
 * 
 * @param width width in pixels
 * @param height height in pixels
 * @param components number of components/pixel (see above)
 * @param pixels the caller owns the bytes, so this ctr makes a copy
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
SFImage::SFImage(size_t width, size_t height, size_t components,
                 const unsigned char * pixels) throw (std::bad_alloc):
        d_w(0L), d_h(0L), d_nc(0L), d_pixels(0L) {
    const size_t nbytes = width * height * components;
    this->d_pixels = new unsigned char[nbytes];
    this->d_w = width;
    this->d_h = height;
    this->d_nc = components;
    std::copy(pixels, pixels + (width * height * components), this->d_pixels);
}

/**
 * @brief Copy constructor.
 *
 * @param sfimage   the SFImage object to copy.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
SFImage::SFImage(const SFImage & sfimage) throw (std::bad_alloc):
        d_w(0), d_h(0), d_nc(0), d_pixels(0) {
    const size_t nbytes = sfimage.d_w * sfimage.d_h * sfimage.d_nc;
    this->d_pixels = new unsigned char[nbytes];
    this->d_w = sfimage.d_w;
    this->d_h = sfimage.d_h;
    this->d_nc = sfimage.d_nc;
    std::copy(sfimage.d_pixels, sfimage.d_pixels + nbytes, this->d_pixels);
}

/**
 * @brief Destructor.
 */
SFImage::~SFImage() throw () { delete [] d_pixels; }

/**
 * @brief Assignment.
 *
 * @param sfimage   the SFImage value to assign to the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
SFImage & SFImage::operator=(const SFImage & sfimage) throw (std::bad_alloc) {
    if (this != &sfimage) {
        delete [] this->d_pixels;
        this->d_w = this->d_h = this->d_nc = 0L;
        const size_t nbytes = sfimage.d_w * sfimage.d_h * sfimage.d_nc;
        this->d_pixels = new unsigned char[nbytes];
        this->d_w = sfimage.d_w;
        this->d_h = sfimage.d_h;
        this->d_nc = sfimage.d_nc;
        std::copy(sfimage.d_pixels, sfimage.d_pixels + nbytes, this->d_pixels);
    }
    return *this;
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFImage::print(std::ostream & out) const {
    out << d_w << " " << d_h << " " << d_nc;

    size_t np = d_w * d_h;
    unsigned char * p = d_pixels;

    for (size_t i = 0; i < np; ++i) {
        unsigned int pixval = 0;
        for (size_t j=0; j<d_nc; ++j) { pixval = (pixval << 8) | *p++; }
        out << " " << pixval;
    }
    return out;
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFImage::clone() const throw (std::bad_alloc) {
    return new SFImage(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFImage.
 */
FieldValue & SFImage::assign(const FieldValue & value)
        throw (std::bad_cast, std::bad_alloc) {
    return (*this = dynamic_cast<const SFImage &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfimage.
 */
FieldValue::Type SFImage::type() const throw () { return sfimage; }

/**
 * @brief Get the image width.
 *
 * @return the image width
 */
size_t SFImage::getWidth() const throw () { return this->d_w; }

/**
 * @brief Get the image height.
 *
 * @return the image height
 */
size_t SFImage::getHeight() const throw () { return this->d_h; }

/**
 * @brief Get the number of components.
 *
 * @return the number of components
 */
size_t SFImage::getComponents() const throw () { return this->d_nc; }

/**
 * @brief Get the pixel data.
 *
 * @return a pointer to the array of pixel data.
 */
const unsigned char * SFImage::getPixels() const throw () {
    return this->d_pixels;
}

/**
 * @brief Set the image.
 *
 * @param width width in pixels
 * @param height height in pixels
 * @param components number of components
 * @param pixels array of (width * height * components) bytes comprising the
 *        image data.
 */
void SFImage::set(size_t width, size_t height, size_t components,
                  const unsigned char * pixels)
        throw (std::bad_alloc) {
    delete this->d_pixels;
    
    this->d_w = width;
    this->d_h = height;
    this->d_nc = components;
    
    this->d_pixels = new unsigned char[width * height * components];
    std::copy(pixels, pixels + (width * height * components), this->d_pixels);
}

/**
 * @class SFInt32
 *
 * @brief Encapsulates an SFInt32 value.
 */

/**
 * @brief Constructor.
 * @param value initial value
 */
SFInt32::SFInt32(long value) throw (): d_value(value) {}

/**
 * @brief Destructor.
 */
SFInt32::~SFInt32() throw () {}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFInt32::print(std::ostream & out) const {
    return out << d_value;
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFInt32::clone() const throw (std::bad_alloc) {
    return new SFInt32(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFInt32.
 */
FieldValue & SFInt32::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFInt32 &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfint32.
 */
FieldValue::Type SFInt32::type() const throw () { return sfint32; }

/**
 * @brief Get value.
 *
 * @return the integer value
 */
long SFInt32::get() const throw () { return this->d_value; }

/**
 * @brief Set value.
 *
 * @param value the new integer value
 */
void SFInt32::set(long value) throw () { this->d_value = value; }


/**
 * @class SFNode
 *
 * @brief Encapsulates an SFNode.
 */

/**
 * @brief Constructor.
 *
 * @param node a NodePtr
 */
SFNode::SFNode(const NodePtr & node) throw (): node(node) {}

/**
 * @brief Destructor.
 */
SFNode::~SFNode() throw () {}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFNode::print(std::ostream & out) const {
    if (!this->node) {
        return out << "NULL" << std::endl;
    }
    return out << *this->node << std::endl;
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFNode::clone() const throw (std::bad_alloc) {
    return new SFNode(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFNode.
 */
FieldValue & SFNode::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFNode &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfnode.
 */
FieldValue::Type SFNode::type() const throw () { return FieldValue::sfnode; }

/**
 * @brief Get value.
 *
 * @return a smart pointer to this object's Node
 */
const NodePtr & SFNode::get() const throw () { return this->node; }

/**
 * @brief Set value.
 *
 * @param node a smart pointer to a Node, or to 0 if setting this
 *             SFNode to <code>NULL</code>
 */
void SFNode::set(const NodePtr & node) throw () { this->node = NodePtr(node); }


/**
 * @class SFRotation
 *
 * @brief Encapsulates an SFRotation.
 *
 * Per the VRML97 specification, the axis of an SFRotation is a normalized
 * vector (5.8). The specification leaves undefined how to deal with an
 * attempt to construct an SFRotation from an axis vector that is not
 * normalized. In order to allow users of the library to minimize the number
 * of normalizations, OpenVRML takes the following approach:
 *
 * <ul>
 * <li>Attempts to construct an SFRotation axis from a vector that is not
 * normalized will yield an assertion failure (abort) unless NDEBUG is defined
 * when compiling the library (in which case truly wacky behavior could
 * result).
 * <li>Assignment to individual components of the axis will result in the
 * axis being re-normalized upon each assignment.
 * </ul>
 */

/**
 * @typedef SFRotation::ConstArrayReference
 *
 * @brief A reference to the SFRotation's 4-element array.
 */

/**
 * @brief Default constructor.
 *
 * Construct with the default value, (0, 0, 1, 0).
 */
SFRotation::SFRotation() throw () {
    this->d_x[0] = 0.0; // x
    this->d_x[1] = 0.0; // y
    this->d_x[2] = 1.0; // z
    this->d_x[3] = 0.0; // angle
}

/**
 * @brief Constructor.
 *
 * @param rot   a 4-element array.
 *
 * @pre The first three elements of the argument array constitute a
 *      normalized vector.
 */
SFRotation::SFRotation(ConstArrayReference rot) throw () {
    using OpenVRML_::fpequal;
    using OpenVRML_::length;
    
    //
    // Make sure axis is normalized.
    //
    assert(fpequal(length(rot), 1.0));
    
    std::copy(rot, rot + 4, this->d_x);
}

/**
 * @brief Constructor.
 *
 * @param x the <var>x</var>-component of the axis of rotation
 * @param y the <var>y</var>-component of the axis of rotation
 * @param z the <var>z</var>-component of the axis of rotation
 * @param angle the rotation angle
 *
 * @pre The first three arguments constitute a normalized vector.
 */
SFRotation::SFRotation(float x, float y, float z, float angle) throw () {
    using OpenVRML_::fpequal;
    using OpenVRML_::length;
    
    this->d_x[0] = x;
    this->d_x[1] = y;
    this->d_x[2] = z;
    
    //
    // Make sure axis is normalized.
    //
    assert(fpequal(length(this->d_x), 1.0));
    
    this->d_x[3] = angle;
}

/**
 * @brief Constructor.
 *
 * @param axis a normalized vector to use as the axis of rotation
 * @param angle the rotation angle
 *
 * @pre The first argument is a normalized vector.
 */
SFRotation::SFRotation(const SFVec3f & axis, float angle) throw () {
    using OpenVRML_::fpequal;
    using OpenVRML_::length;
    
    //
    // Make sure axis is normalized.
    //
    assert(fpequal(length(axis.get()), 1.0));
    
    std::copy(axis.get(), axis.get() + 3, this->d_x);
    this->d_x[3] = angle;
}

/**
 * @brief Construct a rotation between two vectors.
 *
 * Construct a SFRotation equal to the rotation between two different
 * vectors.
 *
 * @param fromVector the starting vector
 * @param toVector the ending vector
 */
SFRotation::SFRotation(const SFVec3f & fromVector, const SFVec3f & toVector)
        throw () {
    this->setAxis(fromVector.cross(toVector));
    this->d_x[3] = acos(fromVector.dot(toVector) /
                        (fromVector.length() * toVector.length()));
}

/**
 * @brief Destructor.
 */
SFRotation::~SFRotation() throw () {}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFRotation::print(std::ostream & out) const {
    return out <<d_x[0] << " " << d_x[1] << " " << d_x[2] << " " << d_x[3];
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFRotation::clone() const throw (std::bad_alloc) {
    return new SFRotation(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFRotation.
 */
FieldValue & SFRotation::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFRotation &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfrotation.
 */
FieldValue::Type SFRotation::type() const throw () { return sfrotation; }

/**
 * @brief Get the <var>x</var>-component of the rotation axis.
 *
 * @return the <var>x</var>-component of the rotation axis.
 */
float SFRotation::getX() const throw () { return this->d_x[0]; }

namespace {
    void normalizeAxis_(float axis[3]) throw () {
        using OpenVRML_::length;
        using OpenVRML_::fpequal;
        using OpenVRML_::normalize;
    
        const float axisLength = length(axis);
        if (fpequal(axisLength, 0.0)) {
            axis[2] = 1.0;
        } else {
            normalize(axis);
        }
    }
}

/**
 * @brief Set the <var>x</var>-component of the rotation axis.
 *
 * @param value
 */
void SFRotation::setX(float value) throw () {
    this->d_x[0] = value;
    normalizeAxis_(this->d_x);
}

/**
 * @brief Get the <var>y</var>-component of the rotation axis.
 *
 * @return the <var>y</var>-component of the rotation axis
 */
float SFRotation::getY() const throw () { return this->d_x[1]; }

/**
 * @brief Set the <var>y</var>-component of the rotation axis.
 *
 * @param value
 */
void SFRotation::setY(float value) throw () {
    this->d_x[1] = value;
    normalizeAxis_(this->d_x);
}

/**
 * @brief Get the <var>z</var>-component of the rotation axis.
 *
 * @return the <var>z</var>-component of the rotation axis
 */
float SFRotation::getZ() const throw () { return this->d_x[2]; }

/**
 * @brief Set the <var>z</var>-component of the rotation axis
 *
 * @param value
 */
void SFRotation::setZ(float value) throw () {
    this->d_x[2] = value;
    normalizeAxis_(this->d_x);
}

/**
 * @brief Get the rotation angle.
 *
 * @return the rotation angle
 */
float SFRotation::getAngle() const throw () { return this->d_x[3]; }

/**
 * @brief Set the rotation angle.
 *
 * @param value
 */
void SFRotation::setAngle(float value) throw () { this->d_x[3] = value; }

/**
 * @brief Get the value of this rotation.
 *
 * @return a reference to a 4-element array.
 */
SFRotation::ConstArrayReference SFRotation::get() const throw () {
    return this->d_x;
}

/**
 * @brief Set the value of this rotation.
 *
 * @param rot a 4-element array
 *
 * @pre The first three elements of <var>rot</var> constitute a normalized
 *      vector.
 */
void SFRotation::set(ConstArrayReference rot) throw ()
{
    using OpenVRML_::fpequal;
    using OpenVRML_::length;
    
    //
    // Make sure axis is normalized.
    //
    assert(fpequal(length(rot), 1.0));
    
    std::copy(rot, rot + 4, this->d_x);
}

/**
 * @brief Get the axis of rotation as a SFVec3f.
 *
 * @return the axis of rotation
 */
const SFVec3f SFRotation::getAxis() const throw () {
    return SFVec3f(this->d_x[0], this->d_x[1], this->d_x[2]);
}

/**
 * @brief Set the axis of rotation using a SFVec3f.
 *
 * @param axis the new rotation axis
 *
 * @pre <var>axis</var> is a normalized vector.
 */
void SFRotation::setAxis(const SFVec3f & axis) throw () {
    using OpenVRML_::fpequal;
    using OpenVRML_::length;
    
    //
    // Make sure axis is normalized.
    //
    assert(fpequal(length(axis.get()), 1.0));
    
    std::copy(axis.get(), axis.get() + 3, this->d_x);
}

/**
 * @brief Get the inverse.
 *
 * @return a SFRotation that is the inverse of this one
 */
const SFRotation SFRotation::inverse() const throw () {
    SFRotation result(*this);
    result.d_x[3] = -d_x[3];
    return result;
}

namespace {
    void multQuat(const float quat1[4], const float quat2[4], float result[4])
            throw () {
        result[3] = quat1[3]*quat2[3] - quat1[0]*quat2[0]
                - quat1[1]*quat2[1] - quat1[2]*quat2[2];

        result[0] = quat1[3]*quat2[0] + quat1[0]*quat2[3]
                + quat1[1]*quat2[2] - quat1[2]*quat2[1];

        result[1] = quat1[3]*quat2[1] + quat1[1]*quat2[3]
                + quat1[2]*quat2[0] - quat1[0]*quat2[2];

        result[2] = quat1[3]*quat2[2] + quat1[2]*quat2[3]
                + quat1[0]*quat2[1] - quat1[1]*quat2[0];
    }
    
    void sfrotToQuat(const SFRotation & rot, float quat[4])
            throw () {
        const float sintd2 = sin(rot.getAngle() * 0.5);
        const float len = sqrt((rot.getX() * rot.getX())
                             + (rot.getY() * rot.getY())
                             + (rot.getZ() * rot.getZ()));
        const float f = sintd2 / len;
        quat[3] = cos(rot.getAngle() * 0.5);
        quat[0] = rot.getX() * f;
        quat[1] = rot.getY() * f;
        quat[2] = rot.getZ() * f;
    }
    
    void quatToSFRot(const float quat[4], SFRotation & rot)
            throw () {
        double sina2 = sqrt(quat[0] * quat[0]
                          + quat[1] * quat[1]
                          + quat[2] * quat[2]);
        const double angle = 2.0 * atan2(sina2, double(quat[3]));

        if (sina2 >= 1e-8) {
	    sina2 = 1.0 / sina2;
            rot.setX(quat[0] * sina2);
            rot.setY(quat[1] * sina2);
            rot.setZ(quat[2] * sina2);
            rot.setAngle(angle);
        } else {
            static const float r[4] = { 0.0, 1.0, 0.0, 0.0 };
            rot.set(r);
        }
    }
}

/**
 * @brief Multiply two rotations.
 *
 * @param rot   the rotation by which to multiply this one
 *
 * @return the result rotation
 */
const SFRotation SFRotation::multiply(const SFRotation & rot) const throw () {
    // convert to quaternions
    float quatUS[4], quatVec[4];
    sfrotToQuat(*this, quatUS);
    sfrotToQuat(rot, quatVec);
    
    // multiply quaternions
    float resultQuat[4];
    multQuat(quatUS, quatVec, resultQuat);
    
    // now convert back to axis/angle
    SFRotation result;
    quatToSFRot(resultQuat, result);
    return result;
}

/**
 * @brief Multiply the matrix corresponding to this rotation by a vector.
 *
 * @todo IMPLEMENT ME!
 *
 * @param vec vector by which to multiply this rotation
 * @return the result of multiplying this rotation by vec
 */
const SFVec3f SFRotation::multVec(const SFVec3f & vec) const {
    return SFVec3f();
}

/**
 * @brief Perform a <b>S</b>pherical <b>L</b>inear Int<b>ERP</b>olation.
 *
 * @param destRotation the destination rotation
 * @param t the interval fraction
 */
const SFRotation SFRotation::slerp(const SFRotation & destRotation,
                                   const float t) const throw () {
    using OpenVRML_::fptolerance;
    
    float fromQuat[4], toQuat[4];
    sfrotToQuat(*this, fromQuat);
    sfrotToQuat(destRotation, toQuat);
    
    //
    // Calculate cosine.
    //
    double cosom = std::inner_product(fromQuat, fromQuat + 4, toQuat, 0.0);
    
    //
    // Adjust signs (if necessary).
    //
    float to1[4];
    if (cosom < 0.0) {
        cosom = -cosom;
        to1[0] = -toQuat[0];
        to1[1] = -toQuat[1];
        to1[2] = -toQuat[2];
        to1[3] = -toQuat[3];
    } else {
        std::copy(toQuat, toQuat + 4, to1);
    }
    
    //
    // Calculate coefficients.
    //
    double scale0, scale1;
    if ((1.0 - cosom) > fptolerance) {
        const double omega = acos(cosom);
        const double sinom = sin(omega);
        scale0 = sin((1.0 - t) * omega) / sinom;
        scale1 = sin(t * omega) / sinom;
    } else {
        //
        // "From" and "to" quaternions are very close, so do linear
        // interpolation.
        //
        scale0 = 1.0 - t;
        scale1 = t;
    }
    
    //
    // Calculate the final values.
    //
    float resultQuat[4];
    resultQuat[0] = (scale0 * fromQuat[0]) + (scale1 * to1[0]);
    resultQuat[1] = (scale0 * fromQuat[1]) + (scale1 * to1[1]);
    resultQuat[2] = (scale0 * fromQuat[2]) + (scale1 * to1[2]);
    resultQuat[3] = (scale0 * fromQuat[3]) + (scale1 * to1[3]);
    
    SFRotation result;
    quatToSFRot(resultQuat, result);
    return result;
}


/**
 * @class SFString
 *
 * @brief Encapsulates an SFString.
 */

/**
 * @brief Constructor.
 *
 * @param value
 */
SFString::SFString(const std::string & value) throw (std::bad_alloc):
        value(value) {}

/**
 * @brief Destructor.
 */
SFString::~SFString() throw () {}

/**
 * @brief Get value.
 *
 * @return a string
 */
const std::string & SFString::get() const throw () { return this->value; }

/**
 * @brief Set value.
 *
 * @param value
 */
void SFString::set(const std::string & value) throw (std::bad_alloc) {
    this->value = value;
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFString::clone() const throw (std::bad_alloc) {
    return new SFString(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFString.
 */
FieldValue & SFString::assign(const FieldValue & value)
        throw (std::bad_cast, std::bad_alloc) {
    return (*this = dynamic_cast<const SFString &>(value));
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFString::print(std::ostream & out) const {
    return out << '\"' << this->value.c_str() << '\"';
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfstring.
 */
FieldValue::Type SFString::type() const throw () { return sfstring; }


/**
 * @class SFTime
 *
 * @brief Encapsulates an SFTime value.
 */

/**
 * @brief Constructor
 *
 * @param value initial value
 */
SFTime::SFTime(double value) throw (): d_value(value) {}

/**
 * @brief Destructor.
 */
SFTime::~SFTime() throw () {}

FieldValue * SFTime::clone() const throw (std::bad_alloc) {
    return new SFTime(*this);
}

FieldValue & SFTime::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFTime &>(value));
}

std::ostream & SFTime::print(std::ostream & os) const
{ return (os << d_value); }

FieldValue::Type SFTime::type() const throw () { return sftime; }

/**
 * @brief Get value.
 *
 * @return the value.
 */
double SFTime::get() const throw () { return this->d_value; }

/**
 * @brief Set value.
 *
 * @param value the new value
 */
void SFTime::set(double value) throw () { this->d_value = value; }


/**
 * @class SFVec2f
 *
 * @brief Encapsulates a SFVec2f value.
 */

/**
 * @typedef SFVec2f::ConstArrayReference
 *
 * @brief A reference to the SFVec2f's 2-element array.
 */

/**
 * @brief Construct a SFVec2f with the default values, (0, 0).
 */
SFVec2f::SFVec2f() throw () { this->d_x[0] = this->d_x[1] = 0; }
        
/**
 * @brief Construct a SFVec2f.
 *
 * @param vec a 2-element array
 */
SFVec2f::SFVec2f(ConstArrayReference vec) throw () {
    this->d_x[0] = vec[0];
    this->d_x[1] = vec[1];
}
        
/**
 * @brief Construct a SFVec2f.
 *
 * @param x the <var>x</var>-component
 * @param y the <var>y</var>-component
 */
SFVec2f::SFVec2f(float x, float y) throw () {
    this->d_x[0] = x;
    this->d_x[1] = y;
}

/**
 * @brief Destructor.
 */
SFVec2f::~SFVec2f() throw () {}

/**
 * @brief Array element dereference operator (const version).
 *
 * @param index a value from 0 - 1. 0 corresponds to the
 *              <var>x</var>-component, and 1 corresponds to the
 *              <var>y</var>-component.
 */
float SFVec2f::operator[](size_t index) const throw () {
    assert(index < 2);
    return this->d_x[index];
}

/**
 * @brief Array element dereference operator (non-const version).
 *
 * @param index a value from 0 - 1. 0 corresponds to the
 *              <var>x</var>-component, and 1 corresponds to the
 *              <var>y</var>-component.
 */
float & SFVec2f::operator[](size_t index) throw () {
    assert(index < 2);
    return this->d_x[index];
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFVec2f::print(std::ostream & out) const {
    return out << d_x[0] << " " << d_x[1];
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFVec2f::clone() const throw (std::bad_alloc) {
    return new SFVec2f(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFVec2f.
 */
FieldValue & SFVec2f::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFVec2f &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfvec2f.
 */
FieldValue::Type SFVec2f::type() const throw () { return sfvec2f; }

/**
 * @brief Get the x component.
 */
float SFVec2f::getX() const throw () { return this->d_x[0]; }
    
/**
 * @brief Set the x component.
 */
void SFVec2f::setX(float value) throw () { this->d_x[0] = value; }

/**
 * @brief Get the y component.
 */
float SFVec2f::getY() const throw () { return this->d_x[1]; }
        
/**
 * @brief Set the y component.
 */
void SFVec2f::setY(float value) throw () { this->d_x[1] = value; }

/**
 * @brief Get the value of this vector.
 *
 * @returns a reference to a 2-element array.
 */
SFVec2f::ConstArrayReference SFVec2f::get() const throw () {
    return this->d_x;
}

/**
 * @brief Set the value of this vector.
 *
 * @param vec a 2-element array.
 */
void SFVec2f::set(ConstArrayReference vec) throw ()
{
    this->d_x[0] = vec[0];
    this->d_x[1] = vec[1];
}

/**
 * @brief Add two vectors.
 *
 * @param vec the vector to add to this one.
 *
 * @return a SFVec2f with a value that is the passed SFVec2f added,
 *         componentwise, to this object.
 */
const SFVec2f SFVec2f::add(const SFVec2f & vec) const throw () {
    SFVec2f result(*this);
    result.d_x[0] += this->d_x[0];
    result.d_x[1] += this->d_x[1];
    return result;
}

/**
 * @brief Divide this vector by a scalar.
 * @param number a scalar value.
 * @return a SFVec2f with a value that is the object divided by the
 *         passed numeric value.
 */
const SFVec2f SFVec2f::divide(float number) const throw () {
    SFVec2f result(*this);
    result.d_x[0] /= number;
    result.d_x[1] /= number;
    return result;
}

/**
 * @brief Dot product.
 *
 * @param vec
 *
 * @return the dot product of this vector and vec.
 */
double SFVec2f::dot(const SFVec2f & vec) const throw () {
    return (this->d_x[0] * vec.d_x[0]) + (this->d_x[1] * vec.d_x[1]);
}

/**
 * @brief Geometric length.
 *
 * @return the length of this vector.
 */
double SFVec2f::length() const throw () {
    return sqrt(d_x[0] * d_x[0] + d_x[1] * d_x[1]);
}

/**
 * @brief Multiply by a scalar.
 *
 * @param number a scalar value
 *
 * @return a SFVec2f with a value that is the object multiplied by the
 *         passed numeric value.
 */
const SFVec2f SFVec2f::multiply(float number) const throw () {
    SFVec2f result(*this);
    result.d_x[0] *= number;
    result.d_x[1] *= number;
    return result;
}

/**
 * @brief Negate.
 *
 * @return a SFVec2f that the result of negating this vector.
 */
const SFVec2f SFVec2f::negate() const throw () {
    SFVec2f result;
    result.d_x[0] = -this->d_x[0];
    result.d_x[1] = -this->d_x[1];
    return result;
}

/**
 * @brief Normalize.
 *
 * @return a SFVec2f that is this vector normalized.
 */
const SFVec2f SFVec2f::normalize() const throw () {
    using OpenVRML_::fpzero;
    
    const double len = this->length();
    if (fpzero(len)) { return *this; }
    SFVec2f result(*this);
    result.d_x[0] /= len;
    result.d_x[1] /= len;
    return result;
}

/**
 * @brief Take the difference of two vectors.
 * @param vec the vector to subtract from this one
 * @return a SFVec2f that is the difference between this vector and vec
 */
const SFVec2f SFVec2f::subtract(const SFVec2f & vec) const throw () {
    SFVec2f result(*this);
    result.d_x[0] -= vec.d_x[0];
    result.d_x[1] -= vec.d_x[1];
    return result;
}


/**
 * @class SFVec3f
 *
 * @brief Encapsulates a SFVec3f value.
 */

/**
 * @typedef SFVec3f::ConstArrayReference
 *
 * @brief A reference to the SFVec3f's 3-element array.
 */

/**
 * @brief Construct a SFVec3f with the default value, (0, 0, 0).
 */
SFVec3f::SFVec3f() throw () {}

/**
 * @brief Construct a SFVec3f.
 *
 * @param vec a 3-element array
 */
SFVec3f::SFVec3f(ConstArrayReference vec) throw ()
{
    std::copy(vec, vec + 3, this->d_x);
}

/**
 * @brief Construct a SFVec3f.
 *
 * @param x the <var>x</var>-component
 * @param y the <var>y</var>-component
 * @param z the <var>z</var>-component
 */
SFVec3f::SFVec3f(float x, float y, float z) throw ()
{ d_x[0] = x; d_x[1] = y; d_x[2] = z; }

/**
 * @brief Destructor.
 */
SFVec3f::~SFVec3f() throw () {}

/**
 * @brief Array element dereference operator (const version).
 *
 * @param index a value from 0 - 2. 0 corresponds to the
 *              <var>x</var>-component, 1 corresponds to the
 *              <var>y</var>-component, and 2 corresponds to the
 *              <var>z</var>-component.
 */
float SFVec3f::operator[](size_t index) const throw () {
    assert(index < 3);
    return this->d_x[index];
}

/**
 * @brief Array element dereference operator (non-const version).
 *
 * @param index a value from 0 - 2. 0 corresponds to the
 *              <var>x</var>-component, 1 corresponds to the
 *              <var>y</var>-component, and 2 corresponds to the
 *              <var>z</var>-component.
 */
float & SFVec3f::operator[](size_t index) throw () {
    assert(index < 3);
    return this->d_x[index];
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & SFVec3f::print(std::ostream & out) const {
    return out << d_x[0] << " " << d_x[1] << " " << d_x[2];
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * SFVec3f::clone() const throw (std::bad_alloc) {
    return new SFVec3f(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @param value the new value to give the object.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast if @p value is not an SFBool.
 */
FieldValue & SFVec3f::assign(const FieldValue & value) throw (std::bad_cast) {
    return (*this = dynamic_cast<const SFVec3f &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::sfvec3f.
 */
FieldValue::Type SFVec3f::type() const throw () { return sfvec3f; }

/**
 * @brief Get the <var>x</var>-component.
 *
 * @return the <var>x</var>-component of this vector
 */
float SFVec3f::getX() const throw () { return this->d_x[0]; }

/**
 * @brief Set the <var>x</var>-component.
 *
 * @param value
 */
void SFVec3f::setX(float value) throw () { this->d_x[0] = value; }

/**
 * @brief Get the <var>y</var>-component.
 *
 * @return the <var>y</var>-component of this vector
 */
float SFVec3f::getY() const throw () { return this->d_x[1]; }

/**
 * @brief Set the <var>y</var>-component.
 *
 * @param value
 */
void SFVec3f::setY(float value) throw () { this->d_x[1] = value; }

/**
 * @brief Get the <var>z</var>-component.
 *
 * @return the <var>z</var>-component of this vector
 */
float SFVec3f::getZ() const throw () { return this->d_x[2]; }

/**
 * @brief Set the <var>z</var>-component.
 *
 * @param value
 */
void SFVec3f::setZ(float value) throw () { this->d_x[2] = value; }

/**
 * @brief Get the vector value.
 *
 * @return a reference to a 3-element array
 */
SFVec3f::ConstArrayReference SFVec3f::get() const throw () {
    return this->d_x;
}

/**
 * @brief Set the vector value.
 *
 * @param vec   a 3-element array
 */
void SFVec3f::set(ConstArrayReference vec) throw ()
{
    this->d_x[0] = vec[0];
    this->d_x[1] = vec[1];
    this->d_x[2] = vec[2];
}

/**
 * @brief Add this vector and vec component-wise.
 *
 * @param vec   a vector.
 */
const SFVec3f SFVec3f::add(const SFVec3f & vec) const throw () {
    SFVec3f result(*this);
    result.d_x[0] += vec.d_x[0];
    result.d_x[1] += vec.d_x[1];
    result.d_x[2] += vec.d_x[2];
    return result;
}

/**
 * @brief Get the cross product of this vector and vec.
 *
 * @param vec   a vector.
 */
const SFVec3f SFVec3f::cross(const SFVec3f & vec) const throw () {
    SFVec3f result;
    result.d_x[0] = (this->d_x[1] * vec.d_x[2]) - (this->d_x[2] * vec.d_x[1]);
    result.d_x[1] = (this->d_x[2] * vec.d_x[0]) - (this->d_x[0] * vec.d_x[2]);
    result.d_x[2] = (this->d_x[0] * vec.d_x[1]) - (this->d_x[1] * vec.d_x[0]);
    return result;
}

/**
 * @brief Get the result of dividing this vector by number.
 *
 * @param number    a scalar value.
 */
const SFVec3f SFVec3f::divide(float number) const throw () {
    SFVec3f result(*this);
    result.d_x[0] /= number;
    result.d_x[1] /= number;
    result.d_x[2] /= number;
    return result;
}

/**
 * @brief Get the dot product of this vector and vec.
 *
 * @param vec   a vector.
 */
double SFVec3f::dot(const SFVec3f & vec) const throw () {
    return ((this->d_x[0] * vec.d_x[0]) + (this->d_x[1] * vec.d_x[1])
            + (this->d_x[2] * vec.d_x[2]));
}

/**
 * @brief Get the length of this vector.
 *
 * @return the geometric length of the vector.
 */
double SFVec3f::length() const throw () {
    using OpenVRML_::fpzero;
    
    const double len = sqrt((d_x[0] * d_x[0])
                          + (d_x[1] * d_x[1])
                          + (d_x[2] * d_x[2]));
    return fpzero(len) ? 0.0 : len;
}

/**
 * @brief Multiply by a scalar.
 *
 * @param number
 *
 * @return the product
 */
const SFVec3f SFVec3f::multiply(float number) const throw () {
    SFVec3f result(*this);
    result.d_x[0] *= number;
    result.d_x[1] *= number;
    result.d_x[2] *= number;
    return result;
}

/**
 * @brief Negate.
 *
 * @return the negatation of this vector
 */
const SFVec3f SFVec3f::negate() const throw () {
    SFVec3f result(*this);
    result.d_x[0] = -result.d_x[0];
    result.d_x[1] = -result.d_x[1];
    result.d_x[2] = -result.d_x[2];
    return result;
}

/**
 * @brief Normalize.
 *
 * @return a copy of this vector normalized
 */
const SFVec3f SFVec3f::normalize() const throw () {
    using OpenVRML_::fpzero;
    
    const double len = this->length();
    SFVec3f result(*this);
    if (!fpzero(len)) {
        result.d_x[0] /= len;
        result.d_x[1] /= len;
        result.d_x[2] /= len;
    }
  
    return result;
}

/**
 * @brief Subtract.
 *
 * @param vec
 *
 * @return the difference between this vector and vec
 */
const SFVec3f SFVec3f::subtract(const SFVec3f & vec) const throw () {
    SFVec3f result(*this);
    result.d_x[0] -= vec.d_x[0];
    result.d_x[1] -= vec.d_x[1];
    result.d_x[2] -= vec.d_x[2];
    return result;
}


namespace {
    
    template <typename ElementType, size_t ArraySize>
    class array_vector {
        ElementType (*data_)[ArraySize];
        size_t size_;
        size_t capacity_;

    public:
        typedef ElementType value_type[ArraySize];
        typedef value_type & reference;
        typedef const value_type & const_reference;
        typedef ElementType (*iterator)[ArraySize];
        typedef const ElementType (*const_iterator)[ArraySize];
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        explicit array_vector(size_type size = 0):
            data_(new ElementType[size][ArraySize]),
            size_(size),
            capacity_(size)
        {}

        array_vector(size_type size, const_reference value):
            data_(new ElementType[size][ArraySize]),
            size_(size),
            capacity_(size)
        {
            for (iterator i(this->begin()); i != this->end(); ++i) {
                std::copy(value, value + ArraySize, *i);
            }
        }

        array_vector(const_iterator begin, const_iterator end):
            data_(new ElementType[end - begin][ArraySize]),
            size_(end - begin),
            capacity_(end - begin)
        {
            for (iterator i(begin), j(this->begin()); i != end; ++i, ++j) {
                std::copy(*i, *i + ArraySize, *j);
            }
        }

        array_vector(const array_vector & av):
            data_(new ElementType[av.size_][ArraySize]),
            size_(av.size_),
            capacity_(av.capacity_)
        {
            const_iterator i(av.begin());
            iterator j(this->begin());
            for (; i != av.end(); ++i, ++j) {
                std::copy(*i, *i + ArraySize, *j);
            }
        }

        ~array_vector()
        {
            delete [] this->data_;
        }

        size_t size() const
        {
            return this->size_;
        }

        bool empty() const
        {
            return this->size_ == 0;
        }

        size_t max_size() const
        {
            return size_type(-1) / sizeof(ElementType[ArraySize]);
        }

        size_t capacity() const
        {
            return this->capacity_;
        }

        void reserve(size_type capacity)
        {
            if (this->capacity_ < capacity) {
                value_type * data = new value_type[capacity];
                for (iterator i(this->begin()), j(data); i != this->end();
                        ++i, ++j) {
                    std::copy(*i, *i + ArraySize, *j);
                }
                delete [] this->data_;
                this->data_ = data;
                this->capacity_ = capacity;
            }
        }

        array_vector & operator=(const array_vector & av)
        {
            array_vector<ElementType, ArraySize> temp(av);
            this->swap(temp);
            return *this;
        }

        void assign(size_t size, const_reference value)
        {
            if (this->capacity_ < size) {
                delete [] this->data_;
                this->data = new value_type[size];
                this->capacity = size;
            }
            for (iterator i(begin), j(this->begin()); i != end; ++i, ++j) {
                std::copy(*i, *i + ArraySize, *j);
            }
            this->size_ = size;
        }

        void assign(const_iterator begin, const_iterator end)
        {
            if (this->capacity_ < end - begin) {
                delete [] this->data_;
                this->data = new value_type[end - begin];
                this->capacity = end - begin;
            }
            for (iterator i(begin), j(this->begin()); i != end; ++i, ++j) {
                std::copy(*i, *i + ArraySize, *j);
            }
            this->size_ = end - begin;
        }

        void swap(array_vector & av) throw ()
        {
            std::swap(this->data_, av.data_);
            std::swap(this->size_, av.size_);
            std::swap(this->capacity_, av.capacity_);
        }

        const_reference at(size_type index) const
        {
            if (!(index < this->size_)) { throw std::out_of_range(); }
            return this->data_[index];
        }

        reference at(size_type index)
        {
            if (!(index < this->size_)) { throw std::out_of_range(); }
            return this->data_[index];
        }

        const_reference operator[](size_type index) const
        {
            return this->data_[index];
        }

        reference operator[](size_type index)
        {
            return this->data_[index];
        }

        const_reference front() const
        {
            return this->data_[0];
        }

        reference front()
        {
            return this->data_[0];
        }

        const_reference back() const
        {
            return this->data_[this->size_ - 1];
        }

        reference back()
        {
            return this->data_[this->size_ - 1];
        }

        const_iterator begin() const
        {
            return this->data_;
        }

        iterator begin()
        {
            return this->data_;
        }

        const_iterator end() const
        {
            return this->data_ + this->size_;
        }

        iterator end()
        {
            return this->data_ + this->size_;
        }

        const_reverse_iterator rbegin() const
        {
            return std::reverse_iterator(this->end());
        }

        reverse_iterator rbegin()
        {
            return std::const_reverse_iterator(this->end());
        }

        const_reverse_iterator rend() const
        {
            return std::reverse_iterator(this->begin());
        }

        reverse_iterator rend()
        {
            return std::const_reverse_iterator(this->end());
        }

        iterator insert(iterator pos, const_reference value)
        {
            size_type n = pos - this->begin();
            this->insert(pos, 1, value);
            return this->begin() + n;
        }
        
        void insert(iterator pos, size_t num, const_reference value)
        {
            if (this->capacity_ < this->size_ + num) {
                value_type * data = new value_type[this->size_ + num];
                iterator i(this->begin()), j(data);
                for (; i != pos; i++, j++) {
                    std::copy(*i, *i + ArraySize, *j);
                }
                std::copy(value, value + ArraySize, *j);
                ++j;
                for (; i != this->end(); ++i, ++j) {
                    std::copy(*i, *i + ArraySize, *j);
                }
                delete [] this->data_;
                this->data_ = data;
                this->capacity_ = this->size_ + num;
            } else {
                iterator i;
                for (i = this->end() - num - 1; i >= pos; --i) {
                    std::copy(*i, *i + ArraySize, *(i + num));
                }
                for (i = pos; i < pos + num; ++i) {
                    std::copy(value, value + ArraySize, *i);
                }
            }
            ++this->size_;
        }
        
        void insert(iterator pos, const_iterator begin, const_iterator end)
        {
            const ptrdiff_t num = end - begin;
            if (this->capacity_ < this->size_ + num) {
                value_type * data = new value_type[this->size_ + num];
                iterator i(this->begin()), j(data);
                for (; i != pos; i++, j++) {
                    std::copy(*i, *i + ArraySize, *j);
                }
                for (i = begin; i != end; i++, j++) {
                    std::copy(*i, *i + ArraySize, *j);
                }
                for (i = this->begin() + num; i != this->end(); ++i, ++j) {
                    std::copy(*i, *i + ArraySize, *j);
                }
                delete [] this->data_;
                this->data_ = data;
                this->capacity_ = this->size_ + num;
            } else {
                iterator i;
                for (i = this->end() - num - 1; i >= pos; --i) {
                    std::copy(*i, *i + ArraySize, *(i + num));
                }
                for (i = pos; i < pos + num; ++i) {
                    std::copy(value, value + ArraySize, *i);
                }
            }
            ++this->size_;
        }

        void push_back(const_reference value)
        {
            if (this->size_ == this->capacity_) {
                value_type * data = new value_type[this->capacity_ * 2];
                for (iterator i(this->begin()), j(data); i != this->end();
                        ++i, ++j) {
                    std::copy(*i, *i + ArraySize, *j);
                }
                delete [] this->data_;
                this->data_ = data;
                this->capacity_ *= 2;
            }
            std::copy(value, value + ArraySize, this->data_[this->size_]);
            ++this->size_;
        }

        void pop_back()
        {
            --this->size_;
        }

        iterator erase(iterator pos)
        {
            if (pos + 1 != this->end()) {
                for (iterator i(pos + 1), j(pos); i != this->end(); ++i, ++i) {
                    std::copy(*i, *i + ArraySize, *j);
                }
            }
            --this->size_;
            return pos;
        }

        iterator erase(iterator begin, iterator end)
        {
            for (iterator i(end), j(begin); i != this->end(); ++i, ++j) {
                std::copy(*i, *i + ArraySize, *j);
            }
            this->size_ -= end - begin;
            return begin;
        }

        void resize(size_type size)
        {
            ElementType value[ArraySize] = {};
            this->resize(size, value);
        }

        void resize(size_type size, const_reference value)
        {
            if (size < this->size_) {
                this->erase(this->begin() + size, this->end());
            } else {
                this->insert(this->end(), size - this->size_, value);
            }
        }

        void clear()
        {
            this->erase(this->begin(), this->end());
        }
    };
}

typedef array_vector<float, 3> ColorVec;

/**
 * @class MFColor
 *
 * @brief Encapsulates a MFColor.
 */

/**
 * @var void * MFColor::data
 *
 * @brief Internal representation.
 */

/**
 * @brief Construct from a float array.
 *
 * @param length    the number of RGB triplets in the array
 * @param values    a pointer to an array of 3-element arrays comprising color
 *                  values to initialize the MFColor.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFColor::MFColor(size_t length, SFColor::ConstArrayPointer values)
    throw (std::bad_alloc):
    data(new ColorVec(length))
{
    if (values) {
        for (size_t i(0); i < length; ++i) { this->setElement(i, values[i]); }
    }
}

/**
 * @brief Copy constructor.
 *
 * @param mfcolor   the object to copy.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFColor::MFColor(const MFColor & mfcolor) throw (std::bad_alloc):
    data(new ColorVec(*static_cast<const ColorVec *>(mfcolor.data)))
{}

/**
 * @brief Destructor.
 */
MFColor::~MFColor() throw ()
{
    delete static_cast<ColorVec *>(this->data);
}

/**
 * @brief Assignment operator.
 *
 * @param mfcolor value to assign to this object
 *
 * @return a reference to this object
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFColor & MFColor::operator=(const MFColor & mfcolor) throw (std::bad_alloc)
{
    MFColor temp(mfcolor);
    static_cast<ColorVec *>(temp.data)
        ->swap(*static_cast<ColorVec *>(this->data));
    return *this;
}

/**
 * @brief Get element.
 *
 * @return a 3-element array comprising an RGB triplet
 */
SFColor::ConstArrayReference MFColor::getElement(size_t index) const throw ()
{
    assert(index < this->getLength());
    return (*static_cast<const ColorVec *>(this->data))[index];
}

/**
 * @brief Set element.
 *
 * @param index the index of the element to set
 * @param value a 3-element float array comprising the new color value
 */
void MFColor::setElement(size_t index, SFColor::ConstArrayReference value)
    throw ()
{
    assert(index < this->getLength());
    std::copy(value, value + 3, (*static_cast<ColorVec *>(this->data))[index]);
}

/**
 * @brief Get the length.
 *
 * @return the number of color values (RGB triplets)
 */
size_t MFColor::getLength() const throw ()
{
    return static_cast<ColorVec *>(this->data)->size();
}

/**
 * @brief Set the length.
 *
 * If the new length is greater than the current length, the additional values
 * are initialized to the default color, (0, 0, 0). If the new length is less
 * than the current length, the array is truncated.
 *
 * @param length new length
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFColor::setLength(size_t length) throw (std::bad_alloc)
{
    static_cast<ColorVec *>(this->data)->resize(length);
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p value should be inserted.
 * @param value an RGB triplet.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFColor::insertElement(const size_t index,
                            SFColor::ConstArrayReference value)
    throw (std::bad_alloc)
{
    ColorVec & data = *static_cast<ColorVec *>(this->data);
    data.insert(data.begin() + index, value);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFColor::removeElement(size_t index) throw ()
{
    ColorVec & data = *static_cast<ColorVec *>(this->data);
    data.erase(data.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFColor::clone() const throw (std::bad_alloc)
{
    return new MFColor(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFColor object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFColor::assign(const FieldValue & value)
        throw (std::bad_cast, std::bad_alloc)
{
    return (*this = dynamic_cast<const MFColor &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::mfcolor.
 */
FieldValue::Type MFColor::type() const throw ()
{
    return FieldValue::mfcolor;
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFColor::print(std::ostream & out) const
{
    const ColorVec & data = *static_cast<ColorVec *>(this->data);
    out << '[';
    for (ColorVec::const_iterator i(data.begin()); i != data.end() - 1; ++i) {
        out << (*i)[0] << ' ' << (*i)[1] << ' ' << (*i)[2] << ", ";
    }
    return out << data.back()[0] << ' ' << data.back()[1] << ' '
               << data.back()[2] << ']';
}


/**
 * @class MFFloat
 *
 * @brief Encapsulates a MFFloat.
 */

/**
 * @brief Construct from a float array.
 *
 * @param length the number of floats in the array
 * @param values a pointer to a float array
 */
MFFloat::MFFloat(size_t length, const float * values) throw (std::bad_alloc):
    data(length)
{
    if (values) {
        std::copy(values, values + length, this->data.begin());
    }
}

/**
 * @brief Destructor.
 */
MFFloat::~MFFloat() throw ()
{}

/**
 * @brief Get element.
 *
 * @param index
 */
const float & MFFloat::getElement(size_t index) const throw ()
{
    assert(index < this->getLength());
    return this->data[index];
}

/**
 * @brief Set element.
 *
 * @param index
 * @param value
 */
void MFFloat::setElement(const size_t index, const float value) throw ()
{
    assert(index < this->getLength());
    this->data[index] = value;
}

/**
 * @brief Get the length.
 *
 * @return the number of float values
 */
size_t MFFloat::getLength() const throw ()
{
    return this->data.size();
}

/**
 * @brief Set the length.
 *
 * If the new length is greater than the current length, the additional values
 * are initialized to the default (0.0). If the new length is less
 * than the current length, the array is truncated.
 *
 * @param length new length
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFFloat::setLength(const size_t length) throw (std::bad_alloc)
{
    this->data.resize(length);
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p value should be inserted.
 * @param value a float value.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFFloat::insertElement(const size_t index, const float value)
    throw (std::bad_alloc)
{
    this->data.insert(this->data.begin() + index, value);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFFloat::removeElement(size_t index) throw ()
{
    this->data.erase(this->data.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFFloat::clone() const throw (std::bad_alloc)
{
    return new MFFloat(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFFloat object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFFloat::assign(const FieldValue & value)
    throw (std::bad_cast, std::bad_alloc)
{
    return (*this = dynamic_cast<const MFFloat &>(value));
}

/**
 * @brief Get the type identifier for the class.
 *
 * @return the type identifer for the class.
 */
FieldValue::Type MFFloat::type() const throw ()
{
    return FieldValue::mffloat;
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFFloat::print(std::ostream & out) const
{
    out << '[';
    for (std::vector<float>::const_iterator i(this->data.begin());
            i != this->data.end() - 1; ++i) {
        out << *i << ", ";
    }
    return out << this->data.back() << ']';
}


/**
 * @class MFInt32
 *
 * @brief Encapsulates a MFInt32.
 */

/**
 * @brief Construct from a long array.
 *
 * @param length    the number of integer values
 * @param values    a pointer to a long array
 */
MFInt32::MFInt32(const size_t length, const long * const values)
    throw (std::bad_alloc):
    data(length)
{
    if (values) {
        std::copy(values, values + length, this->data.begin());
    }
}

/**
 * @brief Destructor.
 */
MFInt32::~MFInt32() throw ()
{}

/**
 * @brief Get element.
 *
 * @param index
 */
const long & MFInt32::getElement(const size_t index) const throw ()
{
    assert(index < this->getLength());
    return this->data[index];
}

/**
 * @brief Set element.
 *
 * @param index
 * @param value
 */
void MFInt32::setElement(const size_t index, const long value) throw ()
{
    assert(index < this->getLength());
    this->data[index] = value;
}

/**
 * @brief Get the length.
 *
 * @return the number of integer values
 */
size_t MFInt32::getLength() const throw ()
{
    return this->data.size();
}

/**
 * @brief Set the length.
 *
 * If the new length is greater than the current length, the additional values
 * are initialized to the default (0.0). If the new length is less
 * than the current length, the array is truncated.
 *
 * @param length new length
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFInt32::setLength(size_t length) throw (std::bad_alloc)
{
    this->data.resize(length);
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p value should be inserted.
 * @param value an integer value.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFInt32::insertElement(const size_t index, const long value)
    throw (std::bad_alloc)
{
    this->data.insert(this->data.begin() + index, value);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFInt32::removeElement(size_t index) throw ()
{
    this->data.erase(this->data.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFInt32::clone() const throw (std::bad_alloc)
{
    return new MFInt32(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFInt32 object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFInt32::assign(const FieldValue & value)
    throw (std::bad_cast, std::bad_alloc)
{
    return (*this = dynamic_cast<const MFInt32 &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::mfint32.
 */
FieldValue::Type MFInt32::type() const throw ()
{
    return FieldValue::mfint32;
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFInt32::print(std::ostream & out) const {
    out << '[';
    for (std::vector<long>::const_iterator i(this->data.begin());
            i != this->data.end() - 1; ++i) {
        out << *i << ", ";
    }
    return out << this->data.back() << ']';
}


/**
 * @class MFNode
 *
 * @brief Encapsulates a MFNode.
 */

/**
 * @brief Construct from an array of Node pointers.
 *
 * @param length the length of the array
 * @param nodes a pointer to an array of Node pointers
 */
MFNode::MFNode(const size_t length, const NodePtr * nodes)
        throw (std::bad_alloc):
        nodes(length) {
    if (nodes) {
        std::copy(nodes, nodes + length, this->nodes.begin());
    }
}

/**
 * @brief Destructor.
 */
MFNode::~MFNode() throw () {}

/**
 * @brief Get element.
 *
 * @param index
 * @return a smart pointer to a Node
 */
const NodePtr & MFNode::getElement(const size_t index) const throw () {
    assert(index < this->nodes.size());
    return this->nodes[index];
}

/**
 * @brief Remove all elements.
 */
void MFNode::clear() throw () { this->nodes.clear(); }

/**
 * @brief Set element.
 *
 * @param index
 * @param node
 */
void MFNode::setElement(const size_t index, const NodePtr & node) throw () {
    assert(index < this->nodes.size());
    this->nodes[index] = node;
}

/**
 * @brief Get the length.
 *
 * @return the number of nodes in the array
 */
size_t MFNode::getLength() const throw () { return this->nodes.size(); }

/**
 * @brief Set the length.
 *
 * Set the length of the node array. If the new length is less than the
 * current length, the array is truncated. If the length is greater than
 * the current length, the new positions at the end of the array are filled
 * with null NodePtrs.
 *
 * @param length the new length
 */
void MFNode::setLength(const size_t length) throw (std::bad_alloc) {
    this->nodes.resize(length);
}

/**
 * @brief Determine if a node exists in this MFNode.
 *
 * @param node
 */
bool MFNode::exists(const Node & node) const {
    for (std::vector<NodePtr>::const_iterator i(this->nodes.begin());
            i != this->nodes.end(); ++i) {
        if (i->get() == &node) {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Add a node.
 *
 * Add @p node to the array if it isn't already part of this MFNode.
 * This method will <strong>not</strong> add NULLs.
 *
 * @param node a pointer to the node to add
 *
 * @return @c true if a node was added, @c false otherwise.
 */
bool MFNode::addNode(const NodePtr & node) {
    assert(node);
    if (!this->exists(*node)) {
        this->nodes.push_back(node);
        return true;
    }
    
    return false;
}

/**
 * @brief Remove a node.
 *
 * Remove <var>node</var> from the array, if it exists here. This method
 * will <strong>not</strong> remove NULLs.
 *
 * @param node a pointer to the node to remove
 * @return <code>true</code> if a node was removed, <code>false</code>
 *         otherwise
 */
bool MFNode::removeNode(const Node & node) {
    for (std::vector<NodePtr>::iterator i(this->nodes.begin());
            i != this->nodes.end(); ++i) {
        if (i->get() == &node) {
            this->nodes.erase(i);
	    return true;
        }
    }
    return false;
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p node should be inserted.
 * @param node  a NodePtr.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFNode::insertElement(size_t index, const NodePtr & node)
        throw (std::bad_alloc) {
  assert(index < this->nodes.size());
  this->nodes.insert(this->nodes.begin() + index, node);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFNode::removeElement(size_t index) throw () {
  assert(index < this->nodes.size());
  this->nodes.erase(this->nodes.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFNode::clone() const throw (std::bad_alloc) {
    return new MFNode(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFNode object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFNode::assign(const FieldValue & value)
        throw (std::bad_cast, std::bad_alloc) {
    return (*this = dynamic_cast<const MFNode &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::mfnode.
 */
FieldValue::Type MFNode::type() const throw () { return mfnode; }

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFNode::print(std::ostream & out) const {
    if (this->nodes.size() != 1) { out << '['; }
    for (std::vector<NodePtr>::const_iterator i(this->nodes.begin());
            i != this->nodes.end(); ++i) {
        out << **i << std::endl;
    }
    if (this->nodes.size() != 1) { out << ']'; }
    
    return out;
}

typedef array_vector<float, 4> RotationVec;

/**
 * @class MFRotation
 *
 * @brief Encapsulates an MFRotation.
 */

/**
 * @var void * MFRotation::data
 *
 * @brief Internal representation.
 */

/**
 * @brief Construct from an array of rotation values.
 *
 * @param length    the number of rotation values in the passed array.
 * @param values    a pointer to an array of 4-element arrays comprising
 *                  rotation values to initialize the MFRotation.
 */
MFRotation::MFRotation(size_t length, SFRotation::ConstArrayPointer values)
    throw (std::bad_alloc):
    data(new RotationVec(length))
{
    if (values) {
        for (size_t i(0); i < length; ++i) { this->setElement(i, values[i]); }
    }
}

/**
 * @brief Copy constructor.
 *
 * @param mfrotation the object to copy
 */
MFRotation::MFRotation(const MFRotation & mfrotation) throw (std::bad_alloc):
    data(new RotationVec(*static_cast<const RotationVec *>(mfrotation.data)))
{}

/**
 * @brief Destructor.
 */
MFRotation::~MFRotation() throw ()
{
    delete static_cast<RotationVec *>(this->data);
}

/**
 * @brief Assignment operator.
 *
 * @param mfrotation the object to copy into this one
 */
MFRotation & MFRotation::operator=(const MFRotation & mfrotation)
    throw (std::bad_alloc)
{
    MFRotation temp(mfrotation);
    static_cast<RotationVec *>(temp.data)
        ->swap(*static_cast<RotationVec *>(this->data));
    return *this;
}

/**
 * @brief Get element.
 *
 * @param index
 */
SFRotation::ConstArrayReference MFRotation::getElement(size_t index) const
    throw ()
{
    assert(index < this->getLength());
    return (*static_cast<const RotationVec *>(this->data))[index];
}

/**
 * @brief Set element.
 *
 * @param index
 * @param value
 */
void MFRotation::setElement(size_t index, SFRotation::ConstArrayReference value)
    throw ()
{
    assert(index < this->getLength());
    std::copy(value, value + 4,
              (*static_cast<RotationVec *>(this->data))[index]);
}

/**
 * @brief Get the length.
 *
 * @return the number of rotation values
 */
size_t MFRotation::getLength() const throw ()
{
    return static_cast<RotationVec *>(this->data)->size();
}

/**
 * @brief Set the length.
 *
 * If the new length is smaller than the current length, the array is
 * truncated. If the new length is greater than the current length, the
 * new values are initialized to the default rotation (0, 0, 1, 0).
 *
 * @param length the new length
 */
void MFRotation::setLength(size_t length) throw (std::bad_alloc)
{
    static_cast<RotationVec *>(this->data)->resize(length);
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p value should be inserted.
 * @param value a rotation value.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFRotation::insertElement(size_t index,
                               SFRotation::ConstArrayReference value)
        throw (std::bad_alloc)
{
    RotationVec & data = *static_cast<RotationVec *>(this->data);
    data.insert(data.begin() + index, value);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFRotation::removeElement(const size_t index) throw ()
{
    RotationVec & data = *static_cast<RotationVec *>(this->data);
    data.erase(data.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFRotation::clone() const throw (std::bad_alloc)
{
    return new MFRotation(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFRotation object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFRotation::assign(const FieldValue & value)
    throw (std::bad_cast, std::bad_alloc)
{
    return (*this = dynamic_cast<const MFRotation &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::mfrotation.
 */
FieldValue::Type MFRotation::type() const throw ()
{
    return FieldValue::mfrotation;
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFRotation::print(std::ostream & out) const
{
    const RotationVec & data = *static_cast<RotationVec *>(this->data);
    out << '[';
    for (RotationVec::const_iterator i(data.begin()); i != data.end() - 1; ++i) {
        out << (*i)[0] << ' ' << (*i)[1] << ' ' << (*i)[2] << ", ";
    }
    return out << data.back()[0] << ' ' << data.back()[1] << ' '
               << data.back()[2] << ']';
}


/**
 * @class MFString
 *
 * @brief Encapsulates a MFString.
 */

/**
 * @brief Constructor.
 *
 * @param length the length of the passed array
 * @param values an array of std::string
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFString::MFString(size_t length, const std::string * values)
        throw (std::bad_alloc):
        values(length) {
    if (values) { std::copy(values, values + length, this->values.begin()); }
}

/**
 * @brief Copy constructor.
 *
 * @param mfstring  the object to copy.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFString::MFString(const MFString & mfstring) throw (std::bad_alloc):
        values(mfstring.values) {}

/**
 * @brief Destructor.
 */
MFString::~MFString() throw () {}

/**
 * @brief Assignment operator.
 *
 * @param mfstring  the object to copy into this one
 */
MFString & MFString::operator=(const MFString & mfstring)
        throw (std::bad_alloc) {
    if (this != &mfstring) { this->values = mfstring.values; }
    return *this;
}

/**
 * @brief Get an element of the array.
 *
 * @param index the index of the element to retrieve
 *
 * @return the array element
 */
const std::string & MFString::getElement(size_t index) const throw () {
    assert(index < this->values.size());
    return this->values[index];
}

/**
 * @brief Set an element of the array.
 *
 * @param index the index of the element to set
 * @param value the new value
 */
void MFString::setElement(size_t index, const std::string & value)
        throw (std::bad_alloc) {
    assert(index < this->values.size());
    this->values[index] = value;
}

/**
 * @brief Get the length.
 *
 * @return the number of values in the array
 */
size_t MFString::getLength() const throw () { return this->values.size(); }

/**
 * Set the length of the vector of std::strings. If the new length is
 * greater than the current length, the new positions are filled with
 * empty std::strings.
 *
 * @param length the new length
 */
void MFString::setLength(const size_t length) throw (std::bad_alloc) {
    this->values.resize(length);
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p value should be inserted.
 * @param value a string.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFString::insertElement(size_t index, const std::string & value)
        throw (std::bad_alloc) {
    assert(index < this->values.size());
    this->values.insert(this->values.begin() + index, value);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFString::removeElement(size_t index) throw () {
    assert(index < this->values.size());
    this->values.erase(this->values.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFString::clone() const throw (std::bad_alloc) {
    return new MFString(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFString object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFString::assign(const FieldValue & value)
        throw (std::bad_cast, std::bad_alloc) {
    return (*this = dynamic_cast<const MFString &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::mfstring.
 */
FieldValue::Type MFString::type() const throw () { return mfstring; }

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFString::print(std::ostream & os) const {
  int n = getLength();

  if (n != 1) os << '[';
  for (int i=0; i<n; ++i)
    os << '\"' << (this->values[i]).c_str() << "\" ";
  if (n != 1) os << ']';

  return os;
}


/**
 * @class MFTime
 *
 * @brief Encapsulates a MFTime.
 */

/**
 * @brief Construct from an array of time values.
 *
 * @param length    the number of time values in the passed array.
 * @param values    a pointer to an array of time values.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFTime::MFTime(const size_t length, const double * const values)
    throw (std::bad_alloc):
    data(length)
{
    if (values) {
        std::copy(values, values + length, this->data.begin());
    }
}

/**
 * @brief Destructor.
 */
MFTime::~MFTime() throw ()
{}

/**
 * @brief Get element.
 *
 * @param index
 */
const double & MFTime::getElement(const size_t index) const throw ()
{
    assert(index < this->getLength());
    return this->data[index];
}

/**
 * @brief Set element.
 *
 * @param index
 * @param value
 */
void MFTime::setElement(const size_t index, const double value) throw ()
{
    assert(index < this->getLength());
    this->data[index] = value;
}

/**
 * @brief Get the length.
 *
 * @return the number of float values
 */
size_t MFTime::getLength() const throw ()
{
    return this->data.size();
}

/**
 * @brief Set the length.
 *
 * If the new length is greater than the current length, the additional values
 * are initialized to the default (0.0). If the new length is less
 * than the current length, the array is truncated.
 *
 * @param length new length
 */
void MFTime::setLength(const size_t length) throw (std::bad_alloc)
{
    this->data.resize(length);
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p value should be inserted.
 * @param value a time value.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFTime::insertElement(const size_t index, const double value)
    throw (std::bad_alloc)
{
    this->data.insert(this->data.begin() + index, value);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFTime::removeElement(const size_t index) throw ()
{
    this->data.erase(this->data.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFTime::clone() const throw (std::bad_alloc)
{
    return new MFTime(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFTime object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFTime::assign(const FieldValue & value)
    throw (std::bad_cast, std::bad_alloc)
{
    return (*this = dynamic_cast<const MFTime &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::mftime.
 */
FieldValue::Type MFTime::type() const throw ()
{
    return FieldValue::mftime;
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFTime::print(std::ostream & out) const
{
    out << '[';
    for (std::vector<double>::const_iterator i(this->data.begin());
            i != this->data.end() - 1; ++i) {
        out << *i << ", ";
    }
    return out << this->data.back() << ']';
}


typedef array_vector<float, 2> Vec2fVec;

/**
 * @class MFVec2f
 *
 * @brief Encapsulates an MFVec2f.
 */

/**
 * @brief Construct from an array of vector values.
 *
 * @param length the number of vector values in the passed array
 * @param values    a pointer to an array of 2-element arrays comprising vector
 *                  values to initialize the MFVec2f.
 */
MFVec2f::MFVec2f(size_t length, SFVec2f::ConstArrayPointer values)
    throw (std::bad_alloc):
    data(new Vec2fVec(length))
{
    if (values) {
        for (size_t i(0); i < length; ++i) { this->setElement(i, values[i]); }
    }
}

/**
 * @brief Copy constructor.
 *
 * @param mfvec2f the object to copy
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFVec2f::MFVec2f(const MFVec2f & mfvec2f) throw (std::bad_alloc):
    data(new Vec2fVec(*static_cast<const Vec2fVec *>(mfvec2f.data)))
{}

/**
 * @brief Destructor.
 */
MFVec2f::~MFVec2f() throw ()
{
    delete static_cast<Vec2fVec *>(this->data);
}

/**
 * @brief Assignment operator.
 *
 * @param mfvec2f the object to copy into this one
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFVec2f & MFVec2f::operator=(const MFVec2f & mfvec2f) throw (std::bad_alloc)
{
    MFVec2f temp(mfvec2f);
    static_cast<Vec2fVec *>(temp.data)
        ->swap(*static_cast<Vec2fVec *>(this->data));
    return *this;
}

/**
 * @brief Get element.
 *
 * @param index
 */
SFVec2f::ConstArrayReference MFVec2f::getElement(size_t index) const throw ()
{
    assert(index < this->getLength());
    return (*static_cast<const Vec2fVec *>(this->data))[index];
}

/**
 * @brief Set element.
 *
 * @param index
 * @param value
 */
void MFVec2f::setElement(size_t index, SFVec2f::ConstArrayReference value)
    throw ()
{
    assert(index < this->getLength());
    std::copy(value, value + 2, (*static_cast<Vec2fVec *>(this->data))[index]);
}

/**
 * @brief Get the length.
 *
 * @return the number of vector values
 */
size_t MFVec2f::getLength() const throw ()
{
    return static_cast<const Vec2fVec *>(this->data)->size();
}

/**
 * @brief Set the length.
 *
 * If the new length is smaller than the current length, the array is
 * truncated. If the new length is greater than the current length, the
 * new values are initialized to the default vector (0, 0).
 *
 * @param length the new length
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFVec2f::setLength(size_t length) throw (std::bad_alloc) {
    static_cast<Vec2fVec *>(this->data)->resize(length);
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p value should be inserted.
 * @param value a 2D vector.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFVec2f::insertElement(size_t index, SFVec2f::ConstArrayReference value)
    throw (std::bad_alloc)
{
    Vec2fVec & data = *static_cast<Vec2fVec *>(this->data);
    data.insert(data.begin() + index, value);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFVec2f::removeElement(size_t index) throw ()
{
    Vec2fVec & data = *static_cast<Vec2fVec *>(this->data);
    data.erase(data.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFVec2f::clone() const throw (std::bad_alloc)
{
    return new MFVec2f(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFVec2f object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFVec2f::assign(const FieldValue & value)
    throw (std::bad_cast, std::bad_alloc)
{
    return (*this = dynamic_cast<const MFVec2f &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::mfvec2f.
 */
FieldValue::Type MFVec2f::type() const throw ()
{
    return FieldValue::mfvec2f;
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFVec2f::print(std::ostream & out) const {
    const Vec2fVec & data = *static_cast<Vec2fVec *>(this->data);
    out << '[';
    for (Vec2fVec::const_iterator i(data.begin()); i != data.end() - 1; ++i) {
        out << (*i)[0] << ' ' << (*i)[1] << ", ";
    }
    return out << data.back()[0] << ' ' << data.back()[1] << ']';
}


typedef array_vector<float, 3> Vec3fVec;

/**
 * @class MFVec3f
 *
 * @brief Encapsulates an MFVec3f.
 */

/**
 * @brief Construct from an array of vector values.
 *
 * @param length    the number of vector values in the passed array
 * @param values    a pointer to an array of 3-element arrays comprising color
 *                  values to initialize the MFColor.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFVec3f::MFVec3f(size_t length, SFVec3f::ConstArrayPointer values)
    throw (std::bad_alloc):
    data(new Vec3fVec(length))
{
    if (values) {
        for (size_t i(0); i < length; ++i) { this->setElement(i, values[i]); }
    }
}

/**
 * @brief Copy constructor.
 *
 * @param mfvec3f the object to copy
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
MFVec3f::MFVec3f(const MFVec3f & mfvec3f) throw (std::bad_alloc):
    data(new Vec3fVec(*static_cast<const Vec3fVec *>(mfvec3f.data)))
{}

/**
 * @brief Destructor.
 */
MFVec3f::~MFVec3f() throw ()
{
    delete static_cast<Vec3fVec *>(this->data);
}

/**
 * @brief Assignment operator.
 *
 * @param mfvec3f the object to copy into this one
 */
MFVec3f & MFVec3f::operator=(const MFVec3f & mfvec3f) throw (std::bad_alloc)
{
    MFVec3f temp(mfvec3f);
    static_cast<Vec3fVec *>(temp.data)
        ->swap(*static_cast<Vec3fVec *>(this->data));
    return *this;
}

/**
 * @brief Get element.
 *
 * @param index
 */
SFVec3f::ConstArrayReference MFVec3f::getElement(size_t index) const throw ()
{
    assert(index < this->getLength());
    return (*static_cast<const Vec3fVec *>(this->data))[index];
}

/**
 * @brief Set element.
 *
 * @param index
 * @param value
 */
void MFVec3f::setElement(size_t index, SFVec3f::ConstArrayReference value)
    throw ()
{
    assert(index < this->getLength());
    std::copy(value, value + 3, (*static_cast<Vec3fVec *>(this->data))[index]);
}

/**
 * @brief Get the length.
 *
 * @return the number of vector values
 */
size_t MFVec3f::getLength() const throw ()
{
    return static_cast<const Vec3fVec *>(this->data)->size();
}

/**
 * @brief Set the length.
 *
 * If the new length is smaller than the current length, the array is
 * truncated. If the new length is greater than the current length, the
 * new values are initialized to the default vector (0, 0, 0).
 *
 * @param length the new length
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFVec3f::setLength(size_t length) throw (std::bad_alloc) {
    static_cast<Vec3fVec *>(this->data)->resize(length);
}

/**
 * @brief Insert an element into the sequence.
 *
 * @param index the index where the new @p value should be inserted.
 * @param value a 3D vector.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
void MFVec3f::insertElement(size_t index, SFVec3f::ConstArrayReference value)
    throw (std::bad_alloc)
{
    Vec3fVec & data = *static_cast<Vec3fVec *>(this->data);
    data.insert(data.begin() + index, value);
}

/**
 * @brief Remove an element from the sequence.
 *
 * @param index the index of the value to remove.
 *
 * @todo Right now this fails silently if @p index is out of range. We should
 *      either fail with an assertion or throw std::out_of_range.
 */
void MFVec3f::removeElement(size_t index) throw ()
{
    Vec2fVec & data = *static_cast<Vec2fVec *>(this->data);
    data.erase(data.begin() + index);
}

/**
 * @brief Virtual copy constructor.
 *
 * @return a pointer to a copy of the object.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue * MFVec3f::clone() const throw (std::bad_alloc)
{
    return new MFVec3f(*this);
}

/**
 * @brief Virtual assignment.
 *
 * @return a reference to the object.
 *
 * @exception std::bad_cast     if  @p value is not an MFVec3f object.
 * @exception std::bad_alloc    if memory allocation fails.
 */
FieldValue & MFVec3f::assign(const FieldValue & value)
    throw (std::bad_cast, std::bad_alloc)
{
    return (*this = dynamic_cast<const MFVec3f &>(value));
}

/**
 * @brief Get the FieldValue::Type associated with this class.
 *
 * @return @c FieldValue::mfvec3f.
 */
FieldValue::Type MFVec3f::type() const throw ()
{
    return FieldValue::mfvec3f;
}

/**
 * @brief Print to an output stream.
 *
 * @param out   an output stream.
 *
 * @return @p out.
 */
std::ostream & MFVec3f::print(std::ostream & out) const
{
    const Vec3fVec & data = *static_cast<Vec3fVec *>(this->data);
    out << '[';
    for (Vec3fVec::const_iterator i(data.begin()); i != data.end() - 1; ++i) {
        out << (*i)[0] << ' ' << (*i)[1] << ' ' << (*i)[2] << ", ";
    }
    return out << data.back()[0] << ' ' << data.back()[1] << ' '
               << data.back()[2] << ']';
}

} // namespace OpenVRML
