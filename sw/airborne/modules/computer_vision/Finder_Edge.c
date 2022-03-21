
void FindObjectBlobs(int *return_img[20][20], struct image_t* img, 
        uint8_t lum_min, uint8_t lum_max, 
        uint8_t cb_min,  uint8_t cb_max,
        uint8_t cr_min, uint8_t cr_max)
{
    // uint8_t[20][20] return_img;
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

    return false;
    
}

void edge_definer(int *matrix_edge[100][100], float bin_mat[100][100]){
    int row = sizeof(bin_mat) / sizeof(bin_mat[0]);
    int column = sizeof(bin_mat[0])/sizeof(bin_mat[0][0]);
    
    for (int i; i<row; ++i){
        int temp = 0;
        for (int j; j<column; ++j){
            if (temp != bin_mat[i][j]){
                matrix_edge[i][j] = 1;
                if (temp==0){
                    temp = 1;
                }else{
                    temp = 0;
                }
            }
        }
    }

    for (int j; j<column; ++j){
        int temp = 0;
        for (int i; i<row; ++i){
            if (temp != bin_mat[i][j]){
                matrix_edge[i][j] = 1;
                if (temp == 0){
                    temp = 1;
                }else{
                    temp = 0;
                }
            }
        }
    }

    return false;
}

void edge_finder(int *matrix_edge[100][100], struct image_t im){
    int bin_mat1[100][100] = filter_color(im,180,253,100,150,130,140);//orange pole and chairs
    int bin_mat2[100][100] = filter_color(im,70,120,150,160,100,120);//Blue chair
    int bin_mat3[100][100] = filter_color(im,100,200,90,130,160,240);//Orange

    int bin_mat_tot[100][100];

    for (int i=0; i<100; ++i){
        for (int j=0; j<100; ++j){
            int bin_mat_tot[i][j] = bin_mat1[i][j]+bin_mat2[i][j]+bin_mat3[i][j];
        }
    } 

    edge_definer(&matrix_egde, bin_mat_tot);

    return false;
}

