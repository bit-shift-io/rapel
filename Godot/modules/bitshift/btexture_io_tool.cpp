  
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
#include "btexture_io_tool.h"
#include <vector>

void BTextureIOTool::set_image(const Ref<Image>& p_image) {
    // unlock old image
    if (image.is_valid() && !image.is_null()) {
        image->unlock();
    }

    image = p_image;
    if (image->is_compressed()) {
        ERR_EXPLAIN("Texture '" + texture->get_name() + "' is compressed, making it unusable in BTextureIOTool, attempting to decompress");
        image->decompress();
    }

    image->lock();

    //pixel_size = Image::get_format_pixel_size(image->get_format());
    width = image->get_width();
    height = image->get_height();

    width_m1 = width - 1;
    height_m1 = height - 1;

    //r = image->get_data().read();
}

void BTextureIOTool::set_texture(const Ref<Texture>& texture) {
    this->texture = texture;
    if (texture.is_null()) {
        image = Ref<Image>();
        width = 0;
        height = 0;
        width_m1 = 0;
        height_m1 = 0;
        return;
    }

    image = texture->get_data();
    if (image.is_null()) {
        ERR_EXPLAIN("Image is null in BTextureIOTool::set_texture: " + texture->get_name());
        return;
    }
    if (image->is_compressed()) {
        ERR_EXPLAIN("Image is compressed and must not be! Check godot texture settings is lossless or lossy. In BTextureIOTool::set_texture: " + texture->get_name());
        return;
    }

    set_image(image);
}

void BTextureIOTool::compute_min_max(float& min, float& max) {
    min = FLT_MAX;
    max = -FLT_MAX;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float v = get_pixel(x, y).r;
            min = MIN(min, v);
            max = MAX(max, v);
        }
    }
}

typedef std::vector<float> FloatArray;
typedef std::vector<FloatArray> Matrix;

Matrix getGaussian(int height, int width, double sigma)
{
    Matrix kernel(height, FloatArray(width));
    double sum=0.0;
    int i,j;

    for (i=0 ; i<height ; i++) {
        for (j=0 ; j<width ; j++) {
            kernel[i][j] = exp(-(i*i+j*j)/(2*sigma*sigma))/(2*M_PI*sigma*sigma);
            sum += kernel[i][j];
        }
    }

    for (i=0 ; i<height ; i++) {
        for (j=0 ; j<width ; j++) {
            kernel[i][j] /= sum;
        }
    }

    return kernel;
}

// https://gist.github.com/OmarAflak/aca9d0dc8d583ff5a5dc16ca5cdda86a
void BTextureIOTool::gaussian_blur(int size, float sigma) {
    Matrix filter = getGaussian(size, size, sigma);

    Ref<Image> newImage;
    newImage.instance();
    newImage->create(width, height, false, image->get_format());
    newImage->lock();

    for (uint32_t i=0 ; i<height ; i++) {
        for (uint32_t j=0 ; j<width ; j++) {
            for (uint32_t h=i ; h<i+size ; h++) {
                for (uint32_t w=j ; w<j+size ; w++) {

                    // out of bounds
                    if (w < 0 || h < 0 || w >= width || h >= height) {
                        continue;
                    }

                    Color p = newImage->get_pixel(j, i);


                    Color d = get_pixel(w, h);

                    p += d * filter[h-i][w-j];

                    newImage->set_pixel(j, i, p);
                    //newImage[d][i][j] += filter[h-i][w-j]*image[d][h][w];
                }
            }
        }
    }

    newImage->unlock();
    image = newImage;
}

BTextureIOTool::BTextureIOTool() {
    
}

BTextureIOTool::~BTextureIOTool() {
    // unlock old image
    if (image.is_valid() && !image.is_null()) {
        image->unlock();
    }
}


