#include "img_vtf.h"
#include "imagelib.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

dword Image_VTFMipSize(dword width, dword height, enum VTFImageFormat format)
{
    // Function assumes width and height already power of two
    switch (format)
    {
        case IMAGE_FORMAT_NONE:
            return 0;
        case IMAGE_FORMAT_DXT1:
        case IMAGE_FORMAT_DXT1_ONEBITALPHA:
            return (width * height) / 2;
        case IMAGE_FORMAT_DXT3:
        case IMAGE_FORMAT_DXT5:
        case IMAGE_FORMAT_I8:
        case IMAGE_FORMAT_P8:
        case IMAGE_FORMAT_A8:
            return width * height;
        case IMAGE_FORMAT_UV88:
        case IMAGE_FORMAT_IA88:
        case IMAGE_FORMAT_RGB565:
        case IMAGE_FORMAT_BGR565:
        case IMAGE_FORMAT_BGRA4444:
        case IMAGE_FORMAT_BGRX5551:
        case IMAGE_FORMAT_BGRA5551:
            return width * height * 2;
        case IMAGE_FORMAT_RGB888_BLUESCREEN:
        case IMAGE_FORMAT_BGR888_BLUESCREEN:
        case IMAGE_FORMAT_RGB888:
        case IMAGE_FORMAT_BGR888:
            return width * height * 3;
        case IMAGE_FORMAT_RGBA8888:
        case IMAGE_FORMAT_ABGR8888:
        case IMAGE_FORMAT_ARGB8888:
        case IMAGE_FORMAT_BGRA8888:
        case IMAGE_FORMAT_BGRX8888:
        case IMAGE_FORMAT_UVWQ8888:
        case IMAGE_FORMAT_UVLX8888:
            return width * height * 4;
        case IMAGE_FORMAT_RGBA16161616F:
        case IMAGE_FORMAT_RGBA16161616:
            return width * height * 8;
    }
    return 0;
}

qboolean Image_LoadVTF(const char *name, const byte *buffer, fs_offset_t filesize)
{
    VTFFileHeader_t *header = (VTFFileHeader_t *) buffer;
    if (header->ident != VTF_FOURCC ||
        header->version_mj != 7 ||
        header->version_mn > 5 || //Only strata engine uses 7.6
        header->header_size >= filesize//Truncated file
            )
        return false;

    image.width = header->width;
    image.height = header->height;
    if (header->version_mn >= 2)
    {
        image.depth = header->depth;
    }
    image.palette = NULL;
    switch (header->highres_format)
    {

        case IMAGE_FORMAT_NONE:
            return false;
        case IMAGE_FORMAT_RGBA8888:
            image.type = PF_RGBA_32;
            break;
        case IMAGE_FORMAT_ABGR8888:
            return false;
        case IMAGE_FORMAT_RGB888:
            image.type = PF_RGB_24;
            break;
        case IMAGE_FORMAT_BGR888:
            image.type = PF_BGR_24;
            break;
        case IMAGE_FORMAT_RGB565:
            return false;
        case IMAGE_FORMAT_I8:
            return false;
        case IMAGE_FORMAT_IA88:
            return false;
        case IMAGE_FORMAT_P8:
            return false;
        case IMAGE_FORMAT_A8:
            image.type = PF_LUMINANCE;
            break;
        case IMAGE_FORMAT_RGB888_BLUESCREEN:
            image.type = PF_RGB_24;
            break;
        case IMAGE_FORMAT_BGR888_BLUESCREEN:
            image.type = PF_BGR_24;
            break;
        case IMAGE_FORMAT_ARGB8888:
            return false;
        case IMAGE_FORMAT_BGRA8888:
            image.type = PF_BGRA_32;
            break;
        case IMAGE_FORMAT_DXT1:
            image.type = PF_DXT1;
            break;
        case IMAGE_FORMAT_DXT3:
            image.type = PF_DXT3;
            break;
        case IMAGE_FORMAT_DXT5:
            image.type = PF_DXT5;
            break;
        case IMAGE_FORMAT_BGRX8888:
            image.type = PF_BGRA_32;
            image.flags = TF_NOALPHA;
            break;
        case IMAGE_FORMAT_BGR565:
            return false;
        case IMAGE_FORMAT_BGRX5551:
            return false;
        case IMAGE_FORMAT_BGRA4444:
            return false;
        case IMAGE_FORMAT_DXT1_ONEBITALPHA:
            image.type = PF_DXT1;
            break;
        case IMAGE_FORMAT_BGRA5551:
            return false;
        case IMAGE_FORMAT_UV88:
            return false;
        case IMAGE_FORMAT_UVWQ8888:
            return false;
        case IMAGE_FORMAT_RGBA16161616F:
            return false;
        case IMAGE_FORMAT_RGBA16161616:
            return false;
        case IMAGE_FORMAT_UVLX8888:
            return false;
    }

    byte *image_data;

    if (header->version_mn >= 3)
    {
        VTFResourceEntryInfo_t *resources = (VTFResourceEntryInfo_t *) (buffer + sizeof(VTFFileHeader_t));
        qboolean found_highres = false;
        for (int res_id = 0; res_id < header->resouce_count; ++res_id)
        {
            VTFResourceEntryInfo_t *resource = &resources[res_id];
            if (resource->tag[0] == 0x30 && resource->tag[1] == 0x00 && resource->tag[2] == 0x00)
            {
                image_data = (byte *) (buffer + resource->offset);
                found_highres = true;
                break;
            }
        }
        if (!found_highres)
        {
            image_data = (byte *) (buffer + header->header_size +
                                   Image_VTFMipSize(header->lowres_width,
                                                    header->lowres_height,
                                                    header->lowres_format));
        }

    } else
    {
        image_data = (byte *) (buffer + header->header_size +
                               Image_VTFMipSize(header->lowres_width,
                                                header->lowres_height,
                                                header->lowres_format));
    }


    image.num_mips = header->mipmap_count;
    dword total_size = 0;
    for (int i = 0; i < header->mipmap_count; ++i)
    {
        int min_size = ImageCompressed(image.type) ? 4 : 1;
        int mip_width = max(min_size, image.width >> i);
        int mip_height = max(min_size, image.height >> i);
        total_size += Image_VTFMipSize(mip_width, mip_height, header->highres_format);
    }
    image.rgba = Mem_Malloc(host.imagepool, total_size);
    image.size = total_size;

    for (int mip = header->mipmap_count-1; mip >= 0; mip--)
    {
        int min_size = ImageCompressed(image.type) ? 4 : 1;
        int mip_width = max(min_size, image.width >> mip);
        int mip_height = max(min_size, image.height >> mip);
        dword mip_size = Image_VTFMipSize(mip_width, mip_height, header->highres_format);

        total_size-=mip_size;
        memcpy(image.rgba+total_size, image_data, mip_size);
        image_data += mip_size;

    }


    return 1;
}
