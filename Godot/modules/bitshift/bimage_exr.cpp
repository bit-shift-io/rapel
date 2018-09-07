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
#include "bimage_exr.h"
#include "butil.h"
#include "btexture_io_tool.h"
#include <assert.h>
#include <vector>
#include "core/os/file_access.h"
#include "thirdparty/tinyexr/tinyexr.h"

Ref<Image> BImageEXR::generate_linear_gradient(int width) {
    Ref<Image> image;
    image.instance();

    float step = 1.f / float(width);

    int height = 64;
    image->create(width, height, false, Image::FORMAT_RGBH);

    BTextureIOTool image_io;
    image_io.set_image(image);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float val = float(x) * step;
            image_io.set_pixel(x, y, Color(val, val, val));
        }
    }

    return image;
}

Error BImageEXR::load_image(Ref<Image> p_image, const String& p_path, Image::Format format) {
    Error error;
    FileAccess *file = FileAccess::open(p_path, FileAccess::READ, &error);
    if (error != OK)
        return error;

    error = _load_image(p_image, file, format);
    memdelete(file);
    return error;
}

Error BImageEXR::_load_image(Ref<Image> p_image, FileAccess *f, Image::Format format, bool p_force_linear, float p_scale) {

    PoolVector<uint8_t> src_image;
    int src_image_len = f->get_len();
    ERR_FAIL_COND_V(src_image_len == 0, ERR_FILE_CORRUPT);
    src_image.resize(src_image_len);

    PoolVector<uint8_t>::Write w = src_image.write();

    f->get_buffer(&w[0], src_image_len);

    f->close();

    // Re-implementation of tinyexr's LoadEXRFromMemory using Godot types to store the Image data
    // and Godot's error codes.
    // When debugging after updating the thirdparty library, check that we're still in sync with
    // their API usage in LoadEXRFromMemory.

    EXRVersion exr_version;
    EXRImage exr_image;
    EXRHeader exr_header;
    const char *err = NULL;

    InitEXRHeader(&exr_header);

    int ret = ParseEXRVersionFromMemory(&exr_version, w.ptr(), src_image_len);
    if (ret != TINYEXR_SUCCESS) {

            return ERR_FILE_CORRUPT;
    }

    ret = ParseEXRHeaderFromMemory(&exr_header, &exr_version, w.ptr(), src_image_len, &err);
    if (ret != TINYEXR_SUCCESS) {
            if (err) {
                    ERR_PRINTS(String(err));
            }
            return ERR_FILE_CORRUPT;
    }

    // Read HALF channel as FLOAT. (GH-13490)
    for (int i = 0; i < exr_header.num_channels; i++) {
            if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
                    exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
            }
    }

    InitEXRImage(&exr_image);
    ret = LoadEXRImageFromMemory(&exr_image, &exr_header, w.ptr(), src_image_len, &err);
    if (ret != TINYEXR_SUCCESS) {
            if (err) {
                    ERR_PRINTS(String(err));
            }
            return ERR_FILE_CORRUPT;
    }

    // RGBA
    int idxR = -1;
    int idxG = -1;
    int idxB = -1;
    int idxA = -1;
    for (int c = 0; c < exr_header.num_channels; c++) {
            if (strcmp(exr_header.channels[c].name, "R") == 0) {
                    idxR = c;
            } else if (strcmp(exr_header.channels[c].name, "G") == 0) {
                    idxG = c;
            } else if (strcmp(exr_header.channels[c].name, "B") == 0) {
                    idxB = c;
            } else if (strcmp(exr_header.channels[c].name, "A") == 0) {
                    idxA = c;
            }
    }

    if (idxR == -1) {
            ERR_PRINT("TinyEXR: R channel not found.");
            // @todo { free exr_image }
            return ERR_FILE_CORRUPT;
    }

    if (idxG == -1 && (format == Image::FORMAT_RGBF || format == Image::FORMAT_RGBAF)) {
            ERR_PRINT("TinyEXR: G channel not found.")
            // @todo { free exr_image }
            return ERR_FILE_CORRUPT;
    }

    if (idxB == -1 && (format == Image::FORMAT_RGBF || format == Image::FORMAT_RGBAF)) {
            ERR_PRINT("TinyEXR: B channel not found.")
            // @todo { free exr_image }
            return ERR_FILE_CORRUPT;
    }

    if (idxA == -1 && (format == Image::FORMAT_RGBAF)) {
            ERR_PRINT("TinyEXR: A channel not found.")
            // @todo { free exr_image }
            return ERR_FILE_CORRUPT;
    }

    // EXR image data loaded, now parse it into Godot-friendly image data

    int sz = sizeof(float);
    if (format == Image::FORMAT_RF)
        sz = sizeof(float);
    else if (format == Image::FORMAT_RGBF)
        sz = sizeof(float) * 3;
    else if (format == Image::FORMAT_RGBAF)
        sz = sizeof(float) * 4;

    PoolVector<uint8_t> imgdata;
    imgdata.resize(exr_image.width * exr_image.height * sz);

    PoolVector<uint8_t>::Write wd8 = imgdata.write();
    float *iw = (float *)wd8.ptr();

    // Assume `out_rgba` have enough memory allocated.
    for (int i = 0; i < exr_image.width * exr_image.height; i++) {

        *iw++ = reinterpret_cast<float **>(exr_image.images)[idxR][i];

        if (format == Image::FORMAT_RGBF) {
            *iw++ = reinterpret_cast<float **>(exr_image.images)[idxG][i];
            *iw++ = reinterpret_cast<float **>(exr_image.images)[idxB][i];
        }

        if (format == Image::FORMAT_RGBAF) {
            assert(idxA > 0);
            *iw++ = reinterpret_cast<float **>(exr_image.images)[idxA][i];
        }
    }

    p_image->create(exr_image.width, exr_image.height, false, format, imgdata);

    w = PoolVector<uint8_t>::Write();

    FreeEXRHeader(&exr_header);
    FreeEXRImage(&exr_image);

    return OK;
}

Error BImageEXR::save_image(const Ref<Image>& p_image, const String& p_path, Image::Format format) {
    BTextureIOTool image_io;
    image_io.set_image(p_image);

    int width = image_io.width;
    int height = image_io.height;

    // https://github.com/syoyo/tinyexr
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    if (format == Image::FORMAT_RF)
        image.num_channels = 1;
    else if (format == Image::FORMAT_RGBF)
        image.num_channels = 3;
    else if (format == Image::FORMAT_RGBAF)
        image.num_channels = 4;


    std::vector<float> images[4];
    for (int i = 0; i < image.num_channels; ++i) {
        images[i].resize(width * height);
    }

    // Split RGBRGBRGB... into R, G and B layer
    /*
    for (int i = 0; i < width * height; i++) {
      images[0][i] = rgb[3*i+0];
      images[1][i] = rgb[3*i+1];
      images[2][i] = rgb[3*i+2];
    }
    */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = (y * width) + x;
            Color rgb = image_io.get_pixel(x, y);
            images[0][i] = rgb.r;

            if (format == Image::FORMAT_RGBF) {
                images[1][i] = rgb.g;
                images[2][i] = rgb.b;
            }

            if (format == Image::FORMAT_RGBAF) {
                images[3][i] = rgb.a;
            }
        }
    }

    float* image_ptr[4];
    if (format == Image::FORMAT_RF) {
        image_ptr[0] = &(images[0].at(0)); // R
    }
    else if (format == Image::FORMAT_RGBF) {
        image_ptr[0] = &(images[2].at(0)); // B
        image_ptr[1] = &(images[1].at(0)); // G
        image_ptr[2] = &(images[0].at(0)); // R
    }
    else if (format == Image::FORMAT_RGBAF) {
        image_ptr[0] = &(images[3].at(0)); // A
        image_ptr[1] = &(images[2].at(0)); // B
        image_ptr[2] = &(images[1].at(0)); // G
        image_ptr[3] = &(images[0].at(0)); // R
    }

    image.images = (unsigned char**)image_ptr;
    image.width = width;
    image.height = height;

    header.num_channels = image.num_channels;
    header.channels = (EXRChannelInfo *)memalloc(sizeof(EXRChannelInfo) * header.num_channels);
    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    /*
    strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
    strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
    strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';
*/
    if (format == Image::FORMAT_RF) {
        strncpy(header.channels[0].name, "R", 255); header.channels[0].name[strlen("R")] = '\0';
    }
    else if (format == Image::FORMAT_RGBF) {
        strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
        strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
        strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';
    }
    else if (format == Image::FORMAT_RGBAF) {
        strncpy(header.channels[0].name, "A", 255); header.channels[0].name[strlen("A")] = '\0';
        strncpy(header.channels[1].name, "B", 255); header.channels[1].name[strlen("B")] = '\0';
        strncpy(header.channels[2].name, "G", 255); header.channels[2].name[strlen("G")] = '\0';
        strncpy(header.channels[3].name, "R", 255); header.channels[3].name[strlen("R")] = '\0';
    }

    header.pixel_types = (int *)memalloc(sizeof(int) * header.num_channels);
    header.requested_pixel_types = (int *)memalloc(sizeof(int) * header.num_channels);
    for (int i = 0; i < header.num_channels; i++) {
      header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
    }

    Error error;
    FileAccess *file = FileAccess::open(p_path, FileAccess::WRITE, &error);

    size_t alloced_size = width * height * 8 * image.num_channels;
    unsigned char *raw_pointers = (unsigned char *)memalloc(alloced_size);

    const char* err;
    //int ret = SaveEXRImageToFile(&image, &header, argv[2], &err);
    size_t size = SaveEXRImageToMemory(&image, &header, &raw_pointers, &err);
    assert(size <= alloced_size);

    file->store_buffer(raw_pointers, size);
    /*
    if (ret != TINYEXR_SUCCESS) {
      fprintf(stderr, "Save EXR err: %s\n", err);
      return ret;
    }
    printf("Saved exr file. [ %s ] \n", argv[2]);
*/
    //free(rgb);

    memfree(header.channels);
    memfree(header.pixel_types);
    memfree(header.requested_pixel_types);

    memdelete(file);

    //memfree(raw_pointers);

    return OK;
}

void BImageEXR::_bind_methods() {
    ClassDB::bind_method(D_METHOD("save_image"), &BImageEXR::save_image);
    ClassDB::bind_method(D_METHOD("load_image"), &BImageEXR::load_image);
}

BImageEXR::BImageEXR() {
}

BImageEXR::~BImageEXR() {
}


