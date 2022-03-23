#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "pthread.h"


const int object_amount = 100;
//float object_matrix[object_amount][14];
float object_matrix[100][14];
int k_object = 0;
int rows = 22;
int cols = 37;

int test_matrix[22][37] = {
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
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    };



void right_maxima_finder(int bin_mat[rows][cols],int i, int j, int k_object, float object_matrix[100][14] /*int object_matrix[100][14]*/)
{   
    int local_i = i; int local_j = j;
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

void left_maxima_finder(int bin_mat[rows][cols],int i, int j, int k_object, float object_matrix[object_amount][14])
{   
    int local_i = i; int local_j = j;
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

void lower_maxima_finder(int bin_mat[rows][cols],int i, int j, int k_object, float object_matrix[object_amount][14])
{   
    int local_i = i; int local_j = j;
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

void x_ray(int bin_mat[rows][cols], int* k_object, float object_matrix[object_amount][14])
{
    int i_start = 0;
    int i_end = rows;
    int j_start = 0;
    int j_end = cols;
    int j_running=0;

    bool running = true;

    while (running)
    {
        bool stopvar = false;
        for(int i = i_start; i<i_end;i++)
        {
            for(int j = j_start; j<j_end;j++)
            {
                j_running=j;
                if(bin_mat[i][j]==1)
                {
                    bool inside_object = false;
                    for (int K = 0; K < *k_object; K++)
                    {
                        if(i>=object_matrix[K][0] && i<=object_matrix[K][6] && j>=object_matrix[K][5] && j<=object_matrix[K][3])
                        {
                            inside_object=true;
                            for(int z = 0; z< object_matrix[K][3]-j;z++)
                            {
                                j++;
                            }
                        }
                        if(inside_object)
                        {
                            break;
                        }
                    }
                    if(!inside_object)
                    {
                        object_matrix[*k_object][0]=i;
                        object_matrix[*k_object][1]=j;
                        stopvar=true;
                        break;
                    }
                }
            }
            if(stopvar)
            {
                printf("%d", (int)object_matrix[*k_object][0]);
                right_maxima_finder(bin_mat, (int)object_matrix[*k_object][0], (int)object_matrix[*k_object][1], *k_object, object_matrix);
                left_maxima_finder(bin_mat, (int)object_matrix[*k_object][0], (int)object_matrix[*k_object][1], *k_object, object_matrix);
                lower_maxima_finder(bin_mat, (int)object_matrix[*k_object][2], (int)object_matrix[*k_object][3], *k_object, object_matrix);
                i_start = object_matrix[*k_object][0];
                (*k_object)+=1;
                break;
            }
            if(j_running==cols-1&&i==rows-1)
            {
                running=false;
            }
        }
    }
}

int main() {
    x_ray(test_matrix,&k_object,object_matrix);
    //right_maxima_finder(test_matrix,3,9,&k_object,object_matrix);
    //left_maxima_finder(test_matrix,3,9,&k_object,object_matrix);
    //lower_maxima_finder(test_matrix,3,9,&k_object,object_matrix);
    printf("\nk = %d", k_object);

    printf("\nThe Array elements are:\n");
    // outer loop for row
    for(int i=0; i<10; i++) {
    // inner loop for column
    for(int j=0; j<8; j++) {
      printf("%f ", object_matrix[i][j]);
    }
    printf("\n"); // new line
  }

  return 0;
} 
