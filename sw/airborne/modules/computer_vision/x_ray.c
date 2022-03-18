#include "modules/computer_vision/cv_detect_color_object.h"
#include "modules/computer_vision/cv.h"
#include "subsystems/abi.h"
#include "std.h"

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "pthread.h"


uint8_t object_amount = 100;
float object_matrix[object_amount][14];
uint8_t k_object = 0;


uint8_t test_matrix[22][37] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };

int rows=22;
int cols=37;


void x_ray(uint8_t bin_mat[rows][cols], uint8_t* k_object, float object_matrix[object_amount][14])
{
    uint8_t i_start = 0;
    uint8_t i_end = rows;
    uint8_t j_start = 0;
    uint8_t j_end = cols;
    bool running = true;

    while (running)
    {
        bool stopvar = false;
        for(uint16_t i = i_start; i<i_end;i++)
        {
            for(uint16_t j = j_start; j<j_end;j++)
            {
                if(bin_mat[i][j]==1)
                {
                    bool inside_object = false;
                    for (uint8_t K = 0; K < k_object; K++)
                    {
                        if(i>=object_matrix[K][0] && i<=object_matrix[K][6] && j>=object_matrix[K][5] && j<=object_matrix[K][3])
                        {
                            inside_object=true;
                            for(uint8_t z = 0; z< object_matrix[K][3]-j;z++)
                            {
                                K++;
                            }
                        }
                        if(inside_object)
                        {
                            break;
                        }
                    }
                    if(!inside_object)
                    {
                        object_matrix[k_object][0]=i;
                        object_matrix[k_object][1]=j;
                        stopvar=true;
                        break;
                    }
                }
            }
            if(stopvar)
            {
                right_maxima_finder(bin_mat, object_matrix[k_object][0], object_matrix[k_object][1], k_object, object_matrix);
                left_maxima_finder(bin_mat, object_matrix[k_object][0], object_matrix[k_object][1], k_object, object_matrix);
                lower_maxima_finder(bin_mat, object_matrix[k_object][2], object_matrix[k_object][3], k_object, object_matrix);
                i_start = object_matrix[k_object][0];
                k_object+=1;
                break;
            }
            if(j==cols-1&&i==rows-1)
            {
                running=false;
            }
        }
    }
}
