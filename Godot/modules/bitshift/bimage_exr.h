/*************************************************************************/
/*                    This file is part of:                              */
/*                    BITSHIFT GODOT PLUGIN                              */
/*                    http://bit-shift.io                                */
/*************************************************************************/
/* Copyright (c) 2017   Fabian Mathews.                                  */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef BIMAGE_EXR_H
#define BIMAGE_EXR_H

#include "scene/3d/spatial.h"

class FileAccess;

/**
    @author Fabian Mathews <supagu@gmail.com>

    Utility class for dealing with EXR images
*/

class BImageEXR : public Resource {

    GDCLASS(BImageEXR, Resource);

protected:

    Error _load_image(Ref<Image> p_image, FileAccess *f, Image::Format format, bool p_force_linear = false, float p_scale = 1.f);

    static void _bind_methods();
           
public:

    Error load_image(Ref<Image> p_image, const String& p_path, Image::Format format);
    Error save_image(const Ref<Image>& p_image, const String& p_path, Image::Format format);

    Ref<Image> generate_linear_gradient(int width);

    BImageEXR();
    ~BImageEXR();
};

#endif // BIMAGE_EXR_H
