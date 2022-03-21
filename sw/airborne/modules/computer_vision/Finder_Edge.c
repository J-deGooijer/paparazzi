


uint8_t[20][20] FindObjectBlobs(struct image_t* img, 
        uint8_t lum_min, uint8_t lum_max, 
        uint8_t cb_min,  uint8_t cb_max,
        uint8_t cr_min, uint8_t cr_max)
{
    uint8_t[20][20] return_img;
    uint8_t* buffer = img->buf;
    for (uint16_t y = 0; y < img->h; y++) {
        for (uint16_t x = 0; x < img->w; x ++) {
        // Check if the color is inside the specified values
        uint8_t *yp, *up, *vp;
        if (x % 2 == 0) {
            // Even x
            up = &buffer[y * 2 * img->w + 2 * x];      // U
            yp = &buffer[y * 2 * img->w + 2 * x + 1];  // Y1
            vp = &buffer[y * 2 * img->w + 2 * x + 2];  // V
            //yp = &buffer[y * 2 * img->w + 2 * x + 3]; // Y2
        } else {
            // Uneven x
            up = &buffer[y * 2 * img->w + 2 * x - 2];  // U
            //yp = &buffer[y * 2 * img->w + 2 * x - 1]; // Y1
            vp = &buffer[y * 2 * img->w + 2 * x];      // V
            yp = &buffer[y * 2 * img->w + 2 * x + 1];  // Y2
        }
        if ( (*yp >= lum_min) && (*yp <= lum_max) &&
            (*up >= cb_min ) && (*up <= cb_max ) &&
            (*vp >= cr_min ) && (*vp <= cr_max )) {
            return_img[y][x]=1;
        }
        else
        {
            return_img[y][x]=0;
        }
        }
    }

    return return_img;
    
}