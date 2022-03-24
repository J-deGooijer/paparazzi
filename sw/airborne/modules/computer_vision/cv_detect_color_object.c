/*
 * Copyright (C) 2019 Kirk Scheper <kirkscheper@gmail.com>
 *
 * This file is part of Paparazzi.
 *
 * Paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * @file modules/computer_vision/cv_detect_object.h
 * Assumes the object consists of a continuous color and checks
 * if you are over the defined object or not
 */

// Own header
#include "modules/computer_vision/cv_detect_color_object.h"
#include "modules/computer_vision/cv.h"
#include "subsystems/abi.h"
#include "std.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "pthread.h"

#define OBJECT_DETECTOR_VERBOSE TRUE

#define PRINT(string,...) fprintf(stderr, "[object_detector->%s()] " string,__FUNCTION__ , ##__VA_ARGS__)
#if OBJECT_DETECTOR_VERBOSE
#define VERBOSE_PRINT PRINT
#else
#define VERBOSE_PRINT(...)
#endif

static pthread_mutex_t mutex;

#ifndef COLOR_OBJECT_DETECTOR_FPS1
#define COLOR_OBJECT_DETECTOR_FPS1 0 ///< Default FPS (zero means run at camera fps)
#endif
#ifndef COLOR_OBJECT_DETECTOR_FPS2
#define COLOR_OBJECT_DETECTOR_FPS2 0 ///< Default FPS (zero means run at camera fps)
#endif

//Variables
uint8_t k_object = 0;
uint8_t object_amount = 100;
uint32_t object_matrix[100][8]; 
uint32_t rows = 520; //Should have a value assigned.
uint32_t cols = 240; //Should have a value assigned.
uint8_t matrix_edge[520][240];

// Filter Settings
uint8_t cod_lum_min1 = 0;
uint8_t cod_lum_max1 = 0;
uint8_t cod_cb_min1 = 0;
uint8_t cod_cb_max1 = 0;
uint8_t cod_cr_min1 = 0;
uint8_t cod_cr_max1 = 0;

uint8_t cod_lum_min2 = 0;
uint8_t cod_lum_max2 = 0;
uint8_t cod_cb_min2 = 0;
uint8_t cod_cb_max2 = 0;
uint8_t cod_cr_min2 = 0;
uint8_t cod_cr_max2 = 0;

bool cod_draw1 = false;
bool cod_draw2 = false;

// define global variables
struct color_object_t {
  int32_t x_c;
  int32_t y_c;
  int32_t object_matrix[100][8]; 
  uint32_t color_count;
  bool updated;
};
struct color_object_t global_filters[2];

// Function
uint32_t find_object_centroid(int (*return_img)[240], struct image_t *img, int32_t* p_xc, int32_t* p_yc, bool draw,
                              uint8_t lum_min, uint8_t lum_max,
                              uint8_t cb_min, uint8_t cb_max,
                              uint8_t cr_min, uint8_t cr_max);

/*
 * object_detector
 * @param img - input image to process
 * @param filter - which detection filter to process
 * @return img
 */
static struct image_t *object_detector(struct image_t *img, uint8_t filter)
{
  uint8_t lum_min, lum_max;
  uint8_t cb_min, cb_max;
  uint8_t cr_min, cr_max;
  bool draw;

  switch (filter){
    case 1:
      lum_min = cod_lum_min1;
      lum_max = cod_lum_max1;
      cb_min = cod_cb_min1;
      cb_max = cod_cb_max1;
      cr_min = cod_cr_min1;
      cr_max = cod_cr_max1;
      draw = cod_draw1;
      break;
    case 2:
      lum_min = cod_lum_min2;
      lum_max = cod_lum_max2;
      cb_min = cod_cb_min2;
      cb_max = cod_cb_max2;
      cr_min = cod_cr_min2;
      cr_max = cod_cr_max2;
      draw = cod_draw2;
      break;
    default:
      return img;
  };

  int32_t x_c, y_c;
  int object_matrix[100][8];
  //uint32_t Edges = edge_finder(img,matrix_edge[rows][cols]);
  //uint32_t xray = x_ray(matrix_edge, &k_object, object_matrix, object_amount);
  



  // Filter and find centroid
  // uint32_t count = find_object_centroid(img, &x_c, &y_c, draw, lum_min, lum_max, cb_min, cb_max, cr_min, cr_max);
  // VERBOSE_PRINT("Color count %d: %u, threshold %u, x_c %d, y_c %d\n", camera, object_count, count_threshold, x_c, y_c);
  // VERBOSE_PRINT("centroid %d: (%d, %d) r: %4.2f a: %4.2f\n", camera, x_c, y_c,
  //       hypotf(x_c, y_c) / hypotf(img->w * 0.5, img->h * 0.5), RadOfDeg(atan2f(y_c, x_c)));
  // VERBOSE_PRINT(img->w, img->h);
  CallFunction(img, 100); //Vul hier de object amount in, 100 lijkt me voor nu meer dan voldoende, mag lager.


  pthread_mutex_lock(&mutex);
  // global_filters[filter-1].color_count = count;
  // global_filters[filter-1].x_c = x_c;
  // global_filters[filter-1].y_c = y_c;
  // global_filters[filter-1].updated = true;
  pthread_mutex_unlock(&mutex);

  return img;
}

struct image_t *object_detector1(struct image_t *img);
struct image_t *object_detector1(struct image_t *img)
{
  return object_detector(img, 1);
}

struct image_t *object_detector2(struct image_t *img);
struct image_t *object_detector2(struct image_t *img)
{
  return object_detector(img, 2);
}

void color_object_detector_init(void)
{
  memset(global_filters, 0, 2*sizeof(struct color_object_t));
  pthread_mutex_init(&mutex, NULL);
// #ifdef COLOR_OBJECT_DETECTOR_CAMERA1
// #ifdef COLOR_OBJECT_DETECTOR_LUM_MIN1
//   cod_lum_min1 = COLOR_OBJECT_DETECTOR_LUM_MIN1;
//   cod_lum_max1 = COLOR_OBJECT_DETECTOR_LUM_MAX1;
//   cod_cb_min1 = COLOR_OBJECT_DETECTOR_CB_MIN1;
//   cod_cb_max1 = COLOR_OBJECT_DETECTOR_CB_MAX1;
//   cod_cr_min1 = COLOR_OBJECT_DETECTOR_CR_MIN1;
//   cod_cr_max1 = COLOR_OBJECT_DETECTOR_CR_MAX1;
// #endif
// #ifdef COLOR_OBJECT_DETECTOR_DRAW1
//   cod_draw1 = COLOR_OBJECT_DETECTOR_DRAW1;
// #endif

//   cv_add_to_device(&COLOR_OBJECT_DETECTOR_CAMERA1, object_detector1, COLOR_OBJECT_DETECTOR_FPS1);
// #endif

#ifdef COLOR_OBJECT_DETECTOR_CAMERA2
#ifdef COLOR_OBJECT_DETECTOR_LUM_MIN2
  cod_lum_min2 = COLOR_OBJECT_DETECTOR_LUM_MIN2;
  cod_lum_max2 = COLOR_OBJECT_DETECTOR_LUM_MAX2;
  cod_cb_min2 = COLOR_OBJECT_DETECTOR_CB_MIN2;
  cod_cb_max2 = COLOR_OBJECT_DETECTOR_CB_MAX2;
  cod_cr_min2 = COLOR_OBJECT_DETECTOR_CR_MIN2;
  cod_cr_max2 = COLOR_OBJECT_DETECTOR_CR_MAX2;
#endif
#ifdef COLOR_OBJECT_DETECTOR_DRAW2
  cod_draw2 = COLOR_OBJECT_DETECTOR_DRAW2;
#endif

  cv_add_to_device(&COLOR_OBJECT_DETECTOR_CAMERA2, object_detector2, COLOR_OBJECT_DETECTOR_FPS2);
#endif
}

/*
 * find_object_centroid
 *
 * Finds the centroid of pixels in an image within filter bounds.
 * Also returns the amount of pixels that satisfy these filter bounds.
 *
 * @param img - input image to process formatted as YUV422.
 * @param p_xc - x coordinate of the centroid of color object
 * @param p_yc - y coordinate of the centroid of color object
 * @param lum_min - minimum y value for the filter in YCbCr colorspace
 * @param lum_max - maximum y value for the filter in YCbCr colorspace
 * @param cb_min - minimum cb value for the filter in YCbCr colorspace
 * @param cb_max - maximum cb value for the filter in YCbCr colorspace
 * @param cr_min - minimum cr value for the filter in YCbCr colorspace
 * @param cr_max - maximum cr value for the filter in YCbCr colorspace
 * @param draw - whether or not to draw on image
 * @return number of pixels of image within the filter bounds.
 */
// uint32_t find_object_centroid(int (*obstacle_data)[4],struct image_t *img, int32_t* p_xc, int32_t* p_yc, bool draw,
//                               uint8_t lum_min, uint8_t lum_max,
//                               uint8_t cb_min, uint8_t cb_max,
//                               uint8_t cr_min, uint8_t cr_max)
// {
//   uint32_t cnt = 0;
//   uint32_t tot_x = 0;
//   uint32_t tot_y = 0;
//   uint8_t *buffer = img->buf;

//   // Go through all the pixels
//   for (uint16_t y = 0; y < img->h; y++) {
//     for (uint16_t x = 0; x < img->w; x ++) {
//       // Check if the color is inside the specified values
//       // VERBOSE_PRINT("x,y: %f/%f", x, y);
//       uint8_t *yp, *up, *vp;
//       if (x % 2 == 0) {
//         // Even x
//         up = &buffer[y * 2 * img->w + 2 * x];      // U
//         yp = &buffer[y * 2 * img->w + 2 * x + 1];  // Y1
//         vp = &buffer[y * 2 * img->w + 2 * x + 2];  // V
//         //yp = &buffer[y * 2 * img->w + 2 * x + 3]; // Y2
//       } else {
//         // Uneven x
//         up = &buffer[y * 2 * img->w + 2 * x - 2];  // U
//         //yp = &buffer[y * 2 * img->w + 2 * x - 1]; // Y1
//         vp = &buffer[y * 2 * img->w + 2 * x];      // V
//         yp = &buffer[y * 2 * img->w + 2 * x + 1];  // Y2
//       }
//       if ( (*yp >= lum_min) && (*yp <= lum_max) &&
//            (*up >= cb_min ) && (*up <= cb_max ) &&
//            (*vp >= cr_min ) && (*vp <= cr_max )) {
//         cnt ++;
//         tot_x += x;
//         tot_y += y;
//         if (draw){
//           *yp = 255;  // make pixel brighter in image
//         }
//       }
//     }
//   }
//   if (cnt > 0) {
//     *p_xc = (int32_t)roundf(tot_x / ((float) cnt) - img->w * 0.5f);
//     *p_yc = (int32_t)roundf(img->h * 0.5f - tot_y / ((float) cnt));
//   } else {
//     *p_xc = 0;
//     *p_yc = 0;
//   }
//   return cnt;
// }

void FindObjectBlobs(uint8_t (*return_img)[240], struct image_t* img, 
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

void edge_definer(uint8_t (*matrix_edge)[240], uint8_t (*bin_mat)[240]){
    // uint32_t row = sizeof(bin_mat) / sizeof(bin_mat[0]);
    // uint32_t column = sizeof(bin_mat[0])/sizeof(bin_mat[0][0]);
    uint32_t row = 520;
    uint32_t column = 240;

    for (uint32_t i; i<row; ++i){
        uint8_t temp = 0;
        for (uint32_t j; j<column; ++j){
            if (temp != bin_mat[i][j]){   // if bin_mat[i][j] == 1
                matrix_edge[i][j] = 1;
                if (temp==0){
                    temp = 1;
                }else{
                    temp = 0;
                }
            }
        }
    }

    for (uint32_t j; j<column; ++j){
        uint8_t temp = 0;
        for (uint32_t i; i<row; ++i){
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

void edge_finder(struct image_t *im, uint8_t matrix_edge[520][240]){
    uint8_t bin_mat1[520][240];
    FindObjectBlobs(bin_mat1, im,180,253,100,150,130,140);//orange pole and chairs
    uint8_t bin_mat2[520][240];
    FindObjectBlobs(bin_mat2, im,70,120,150,160,100,120);//Blue chair
    uint8_t bin_mat3[520][240];
    FindObjectBlobs(bin_mat3, im,100,200,90,130,160,240);//Orange

    uint8_t bin_mat_tot[520][240];

    for (uint32_t i=0; i<520; ++i){
       for (uint32_t j=0; j<240; ++j){
            bin_mat_tot[i][j] = bin_mat1[i][j]+bin_mat2[i][j]+bin_mat3[i][j];
        }
    } 

    // printf("\nThe Array elements are:\n");
    // // outer loop for row
    // for(int i=0; i<520; i++) {
    //   // inner loop for column
    //   for(int j=0; j<240; j++) {
    //     printf("%d ", bin_mat_tot[i][j]);
    //   }
    //printf("\n"); // new line
  //}

    edge_definer(matrix_edge, bin_mat_tot);

    return false;
}


void right_maxima_finder(uint8_t bin_mat[rows][cols],uint32_t i, uint32_t j, uint8_t k_object, uint32_t object_matrix[100][8])
{   
    uint32_t local_i = i; uint32_t local_j = j;
    bool running = true;
    bool running2 = true;
    while(running) {
      while(running2) {
        if(j == cols-1) {
          running=false;
        } 
        else if(bin_mat[local_i][local_j+1] == 1) {
          local_j++;
          running=false;
        }
        else if(bin_mat[local_i+1][local_j+1] == 1) {
          local_i++;
          local_j++;
          running=false;
        }
        else if(bin_mat[local_i+1][local_j] == 1) {
          local_i++;
          running=false;
        }
        else {
          running2=false;
        }
      }
    }
    object_matrix[k_object][2] = local_i;
    object_matrix[k_object][3] = local_j;
}

void left_maxima_finder(uint8_t bin_mat[rows][cols],uint32_t i, uint32_t j, uint8_t k_object, uint32_t object_matrix[object_amount][8])
{   
    uint32_t local_i = i; uint32_t local_j = j;
    bool running = true;
    bool running2 = true;
    while(running) {
      while(running2) {
        if(j == 0) {
          running=false;
        } 
        else if(bin_mat[local_i][local_j-1] == 1) {
          local_j--;
          running=false;
        }
        else if(bin_mat[local_i+1][local_j-1] == 1) {
          local_i++;
          local_j--;
          running=false;
        }
        else if(bin_mat[local_i+1][local_j] == 1) {
          local_i++;
          running=false;
        }
        else {
          running2=false;
        }
      }
    }
    object_matrix[k_object][4] = local_i;
    object_matrix[k_object][5] = local_j;
}

void lower_maxima_finder(uint8_t bin_mat[rows][cols],uint32_t i, uint32_t j, uint8_t k_object, uint32_t object_matrix[object_amount][8])
{   
    uint32_t local_i = i; uint32_t local_j = j;
    bool running = true;
    bool running2 = true;
    while(running) {
      while(running2) {
        if(j == rows-1) {
          running=false;
        } 
        else if(bin_mat[local_i+1][local_j] == 1) {
          local_i++;
          running=false;
        }
        else if(bin_mat[local_i+1][local_j-1] == 1) {
          local_i++;
          local_j--;
          running=false;
        }
        else if(bin_mat[local_i][local_j-1] == 1) {
          local_j--;
          running=false;
        }
        else {
          running2=false;
        }
      }
    }
    object_matrix[k_object][6] = local_i;
    object_matrix[k_object][7] = local_j;
}

void x_ray(uint8_t bin_mat[rows][cols], uint8_t* k_object, uint32_t object_matrix[object_amount][8], uint8_t object_amount)
{
    uint32_t i_start = 0;
    uint32_t i_end = rows;
    uint32_t j_start = 0;
    uint32_t j_end = cols;
    uint32_t j_running=0;
    *k_object=0;

    bool running = true;

    while (running)
    {
        bool stopvar = false;
        for(uint32_t i = i_start; i<i_end;i++)
        {
            for(uint32_t j = j_start; j<j_end;j++)
            {
                j_running=j;
                if(bin_mat[i][j]==1)
                {
                    bool inside_object = false;
                    for (uint8_t K = 0; K < *k_object; K++)
                    {
                        if(i>=object_matrix[K][0] && i<=object_matrix[K][6] && j>=object_matrix[K][5] && j<=object_matrix[K][3])
                        {
                            inside_object=true;
                            for(uint32_t z = 0; z< object_matrix[K][3]-j;z++)
                            {
                                j++;
                            }
                        }
                        if(inside_object)
                        {
                            break; //Breaks out of the K loop
                        }
                    }
                    if(!inside_object)
                    {
                        object_matrix[*k_object][0]=i;
                        object_matrix[*k_object][1]=j;
                        stopvar=true;
                        break; //Breaks out of the j loop
                    }
                }
            }
            if(stopvar)
            {
                printf("%d", (uint32_t)object_matrix[*k_object][0]);
                right_maxima_finder(bin_mat, (uint32_t)object_matrix[*k_object][0], (uint32_t)object_matrix[*k_object][1], *k_object, object_matrix);
                left_maxima_finder(bin_mat, (uint32_t)object_matrix[*k_object][0], (uint32_t)object_matrix[*k_object][1], *k_object, object_matrix);
                lower_maxima_finder(bin_mat, (uint32_t)object_matrix[*k_object][2], (uint32_t)object_matrix[*k_object][3], *k_object, object_matrix);
                i_start = object_matrix[*k_object][0];
                (*k_object)+=1;
                if(*k_object>=object_amount-1)
                {
                    running=false;
                }
                break; //Breaks out of the i loop
            }
            if(j_running==cols-1&&i==rows-1)
            {
                running=false;
            }
        }
    }
}

void CallFunction(struct image_t *img, uint8_t object_amount)
{
    edge_finder(img,matrix_edge[rows][cols]);
    printf("\nThe Array elements are:\n");
    // outer loop for row
    for(int i=0; i<520; i++) {
      // inner loop for column
      for(int j=0; j<240; j++) {
        printf("%d ", matrix_edge[i][j]);
      }
    printf("\n"); // new line
  }
    x_ray(matrix_edge, &k_object, object_matrix, object_amount);


}

void color_object_detector_periodic(void)
{
  static struct color_object_t local_filters[2];
  pthread_mutex_lock(&mutex);
  memcpy(local_filters, global_filters, 2*sizeof(struct color_object_t));
  pthread_mutex_unlock(&mutex);

  if(local_filters[0].updated){
    AbiSendMsgVISUAL_DETECTION(COLOR_OBJECT_DETECTION1_ID, local_filters[0].x_c, local_filters[0].y_c,
        0, 0, local_filters[0].color_count, 0);
    local_filters[0].updated = false;
  }
  if(local_filters[1].updated){
    AbiSendMsgOF_OBSTACLE_DATA(OPTIC_FLOW_OBSTACLE_DATA1_ID, &local_filters[1]);
    local_filters[1].updated = false;
  }
}