/*
 * Copyright (C) Kirk Scheper <kirkscheper@gmail.com>
 *
 * This file is part of paparazzi
 *
 */
/**
 * @file "modules/orange_avoider/orange_avoider_guided.c"
 * @author Kirk Scheper
 * This module is an example module for the course AE4317 Autonomous Flight of Micro Air Vehicles at the TU Delft.
 * This module is used in combination with a color filter (cv_detect_color_object) and the guided mode of the autopilot.
 * The avoidance strategy is to simply count the total number of orange pixels. When above a certain percentage threshold,
 * (given by color_count_frac) we assume that there is an obstacle and we turn.
 *
 * The color filter settings are set using the cv_detect_color_object. This module can run multiple filters simultaneously
 * so you have to define which filter to use with the ORANGE_AVOIDER_VISUAL_DETECTION_ID setting.
 * This module differs from the simpler orange_avoider.xml in that this is flown in guided mode. This flight mode is
 * less dependent on a global positioning estimate as witht the navigation mode. This module can be used with a simple
 * speed estimate rather than a global position.
 *
 * Here we also need to use our onboard sensors to stay inside of the cyberzoo and not collide with the nets. For this
 * we employ a simple color detector, similar to the orange poles but for green to detect the floor. When the total amount
 * of green drops below a given threshold (given by floor_count_frac) we assume we are near the edge of the zoo and turn
 * around. The color detection is done by the cv_detect_color_object module, use the FLOOR_VISUAL_DETECTION_ID setting to
 * define which filter to use.
 */

#include "modules/orange_avoider/orange_avoider_guided.h"
#include "firmwares/rotorcraft/guidance/guidance_h.h"
#include "firmwares/rotorcraft/navigation.h"
#include "generated/airframe.h"
#include "state.h"
#include "modules/core/abi.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h> 

#define NAV_C
#include "generated/flight_plan.h"

#define ORANGE_AVOIDER_VERBOSE TRUE

#define PRINT(string,...) fprintf(stderr, "[orange_avoider_guided->%s()] " string,__FUNCTION__ , ##__VA_ARGS__)
#if ORANGE_AVOIDER_VERBOSE
#define VERBOSE_PRINT PRINT
#else
#define VERBOSE_PRINT(...)
#endif


static uint8_t calculateForwards(struct EnuCoor_i *new_coor, float distanceMeters);
uint8_t chooseRandomIncrementAvoidance(void);
uint8_t chooseIncrementAvoidance(void);
uint8_t CheckWall(struct EnuCoor_i new_coor);
uint8_t RotationOperation(float *x, float *y, float *psi);

enum navigation_state_t {
  SAFE,
  OBSTACLE_FOUND,
  SEARCH_FOR_SAFE_HEADING,
  OUT_OF_BOUNDS,
  REENTER_ARENA,
  OPTIC_FLOW
};

enum OF_cal_state_t{
  RIGHT,
  LEFT,
  MID
};

// define settings
float oag_color_count_frac = 0.18f;       // obstacle detection threshold as a fraction of total of image
float oag_floor_count_frac = 0.05f;       // floor detection threshold as a fraction of total of image
float oag_max_speed = 1.f;               // max flight speed [m/s]
float oag_heading_rate = RadOfDeg(30.f);  // heading change setpoint for avoidance [rad/s]

float oag_oob_vx = 0.1f;
float oag_oob_vy = 0.7f;
float oag_oob_rate = 90.0f;
float oag_oob_dist = 5.0f;

int wall;

int count;



// define and initialise global variables
enum navigation_state_t navigation_state = SEARCH_FOR_SAFE_HEADING;   // current state in state machine
enum OF_cal_state_t OF_cal_state = RIGHT;
int32_t color_count = 0;                // orange color count from color filter for obstacle detection
float avoidance_heading_direction = 0;  // heading change direction for avoidance [rad/s]
int16_t obstacle_free_confidence = 0;   // a measure of how certain we are that the way ahead if safe.

const int16_t max_trajectory_confidence = 5;  // number of consecutive negative object detections to be sure we are obstacle free

// This call back will be used to receive the color count from the orange detector
#ifndef ORANGE_AVOIDER_VISUAL_DETECTION_ID
#error This module requires two color filters, as such you have to define ORANGE_AVOIDER_VISUAL_DETECTION_ID to the orange filter
#error Please define ORANGE_AVOIDER_VISUAL_DETECTION_ID to be COLOR_OBJECT_DETECTION1_ID or COLOR_OBJECT_DETECTION2_ID in your airframe
#endif
static abi_event color_detection_ev;
static void color_detection_cb(uint8_t __attribute__((unused)) sender_id,
                               int16_t __attribute__((unused)) pixel_x, int16_t __attribute__((unused)) pixel_y,
                               int16_t __attribute__((unused)) pixel_width, int16_t __attribute__((unused)) pixel_height,
                               int32_t quality, int16_t __attribute__((unused)) extra)
{
  color_count = quality;
}


#ifndef OPTIC_FLOW_OBSTACLE_DATA_ID
#define OPTIC_FLOW_OBSTACLE_DATA_ID ABI_BROADCAST
#endif
static abi_event optic_flow_obstacle_data_ev;
static void optic_flow_obstacle_data_cb(uint8_t __attribute__((unused))sender_id,
					float data_matrix)
{
  //do stuff with data matrix
}

/*
 * Initialisation function
 */
void orange_avoider_guided_init(void)
{
  // Initialise random values
  srand(time(NULL));
  chooseRandomIncrementAvoidance();

  // bind our colorfilter callbacks to receive the color filter outputs
  AbiBindMsgVISUAL_DETECTION(ORANGE_AVOIDER_VISUAL_DETECTION_ID, &color_detection_ev, color_detection_cb);
  AbiBindMsgOF_OBSTACLE_DATA(OPTIC_FLOW_OBSTACLE_DATA_ID, &optic_flow_obstacle_data_ev, optic_flow_obstacle_data_cb);
}

/*
 * Function that checks it is safe to move forwards, and then sets a forward velocity setpoint or changes the heading
 */
void orange_avoider_guided_periodic(void)
{
  // Only run the mudule if we are in the correct flight mode
  if (guidance_h.mode != GUIDANCE_H_MODE_GUIDED) {
    navigation_state = SEARCH_FOR_SAFE_HEADING;
    obstacle_free_confidence = 0;
    return;
  }

  // compute current color thresholds
  int32_t color_count_threshold = oag_color_count_frac * front_camera.output_size.w * front_camera.output_size.h;
  // VERBOSE_PRINT("Color_count: %d  threshold: %d state: %d \n", color_count, color_count_threshold, navigation_state);

  // update our safe confidence using color threshold
  if(color_count < color_count_threshold){
    obstacle_free_confidence++;
  } else {
    obstacle_free_confidence -= 2;  // be more cautious with positive obstacle detections
  }

  // bound obstacle_free_confidence
  Bound(obstacle_free_confidence, 0, max_trajectory_confidence);

  float speed_sp = fminf(oag_max_speed, 0.4f * obstacle_free_confidence);

  // VERBOSE_PRINT("x/y/psi: %f/%f/%f\n", POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->x),POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->y), DegOfRad(stateGetNedToBodyEulers_f()->psi));

  switch (navigation_state){
    case SAFE: ;
      count = 0;
      VERBOSE_PRINT("Safe\n");
      VERBOSE_PRINT("ofc: %f\n", obstacle_free_confidence);
      struct EnuCoor_i new_coor;
      calculateForwards(&new_coor, 1.0f);

      if (!InsideObstacleZone(POS_FLOAT_OF_BFP(new_coor.x),POS_FLOAT_OF_BFP(new_coor.y))){//(floor_count < floor_count_threshold || fabsf(floor_centroid_frac) > 0.12){
        // VERBOSE_PRINT("x/y: %s %s", new_coor.x, new_coor.y);
        CheckWall(new_coor);
        navigation_state = OUT_OF_BOUNDS;
      } else if (obstacle_free_confidence == 0){
        navigation_state = OBSTACLE_FOUND;
      } else {
        guidance_h_set_guided_body_vel(speed_sp, 0);
      }

      break;
    case OBSTACLE_FOUND:
      VERBOSE_PRINT("Obstacle found\n");
      // stop
      guidance_h_set_guided_heading(stateGetNedToBodyEulers_f()->psi);

      guidance_h_set_guided_heading_rate(avoidance_heading_direction * oag_heading_rate);
      guidance_h_set_guided_body_vel(0.0f, avoidance_heading_direction*1.0);
      // make sure we have a couple of good readings before declaring the way safe
      if (obstacle_free_confidence >= 2){
        guidance_h_set_guided_heading(stateGetNedToBodyEulers_f()->psi);
        navigation_state = SAFE;
      }

      break;
    case SEARCH_FOR_SAFE_HEADING:
      VERBOSE_PRINT("Search for safe heading\n")
      // put the stuff here that sues the data matrix to find clear path to other side of cyberzoo

      // if no path is found, rotat a bit more, or fly a bit and try again

      navigation_state = SAFE;
      
      break;
    case OUT_OF_BOUNDS: ;
      VERBOSE_PRINT("Out of bounds\n");
      // stop

      chooseIncrementAvoidance();
      guidance_h_set_guided_body_vel(0.0f, 0.0f);//oag_oob_vx, avoidance_heading_direction*oag_oob_vy);
      // start turn back into arena
      guidance_h_set_guided_heading_rate(avoidance_heading_direction * RadOfDeg(oag_oob_rate));

      navigation_state = REENTER_ARENA;

      break;
    case REENTER_ARENA: ;
      VERBOSE_PRINT("Reenter\n");
      // Check if coords a couple meters in front are still in the arena
      struct EnuCoor_i new_coor2;
      calculateForwards(&new_coor2, oag_oob_dist);
      if (InsideObstacleZone(POS_FLOAT_OF_BFP(new_coor2.x), POS_FLOAT_OF_BFP(new_coor2.y))){
        // return to heading mode
        guidance_h_set_guided_heading(stateGetNedToBodyEulers_f()->psi);
        guidance_h_set_guided_body_vel(0.0f,0.0f);//oag_max_speed, 0.0f);

        OF_cal_state = RIGHT;
        navigation_state = OPTIC_FLOW;
      }

      break;
    case OPTIC_FLOW:
      count = count+1;
      switch (OF_cal_state){
        case RIGHT:
          VERBOSE_PRINT("right\n");
          guidance_h_set_guided_body_vel(0.0f,1.0f);
          if (count>2){
            guidance_h_set_guided_body_vel(0.0f,0.0f);
            AbiSendMsgOF_CALIBRATION(OPTIC_FLOW_CALIBRATION1_ID, 1, POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->x), POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->y));
            OF_cal_state = LEFT;
          }
          break;
        case LEFT:
          VERBOSE_PRINT("left\n");
          guidance_h_set_guided_body_vel(0.0f,-1.0f);
          if (count>6){
            guidance_h_set_guided_body_vel(0.0f,0.0f);
            AbiSendMsgOF_CALIBRATION(OPTIC_FLOW_CALIBRATION1_ID, 2, POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->x), POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->y));
            OF_cal_state = MID;
          }
          break;
        case MID:
          VERBOSE_PRINT("mid\n");
          guidance_h_set_guided_body_vel(0.0f,1.0f);
          if (count>8){
            guidance_h_set_guided_body_vel(0.0f,0.0f);
            AbiSendMsgOF_CALIBRATION(OPTIC_FLOW_CALIBRATION1_ID, 3, POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->x), POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->y));
            navigation_state = SEARCH_FOR_SAFE_HEADING;
          }
          break;
        default:
          break;
      }      
      break;
    default:
      break;
  }
  return;
}

/*
 * Sets the variable 'incrementForAvoidance' randomly positive/negative
 */
uint8_t chooseRandomIncrementAvoidance(void)
{
  // Randomly choose CW or CCW avoiding direction
  if (rand() % 2 == 0) {
    avoidance_heading_direction = 1.f;
    // VERBOSE_PRINT("Set avoidance increment to: %f\n", avoidance_heading_direction * oag_heading_rate);
  } else {
    avoidance_heading_direction = -1.f;
    // VERBOSE_PRINT("Set avoidance increment to: %f\n", avoidance_heading_direction * oag_heading_rate);
  }
  return false;
}

/*
 * Chooses a way for the drone to turn. Usually the shortest turn away from the wall, but in the corners the decision making is more complex.
 */
uint8_t chooseIncrementAvoidance(void)
{
  float x = POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->x);
  float y = POS_FLOAT_OF_BFP(stateGetPositionEnu_i()->y);
  float psi = DegOfRad(stateGetNedToBodyEulers_f()->psi);

  RotationOperation(&x, &y, &psi);

  if (psi<0){
    psi = psi+360;
  }else if (psi>360){
    psi = psi-360;
  }
  
  int quad; // What quadrant does the heading point to
  if (0 <= psi && psi <= 90){
    quad = 1;
  }else if (90 <= psi && psi <= 180){
    quad = 2;
  }else if (180 <= psi && psi <= 270){
    quad = 3;
  } else if (270 <= psi && psi <= 360){
    quad = 4;
  }

  int corner; // In what quadrant is the drone
  if (x>0 && y>0){
    corner = 1;
  }else if (x>0 && y<0){
    corner = 2;
  }else if (x<0 && y<0){
    corner = 3;
  }else if (x<0 && y>0){
    corner = 4;
  }

  float rest = fmod(psi, 90.0);
  
  if (wall == quad){
    avoidance_heading_direction = 1.f;
  }else if (((wall-quad) == 1) || (wall==1 && quad==4)){
    avoidance_heading_direction = -1.f;
  }else{
    avoidance_heading_direction = 0.f;
  }

  // VERBOSE_PRINT("wall/quad/corner: %i/%i/%i\n", wall, quad, corner);

  if (fabs(x)>2.8 && fabs(y)>2.8){
    // VERBOSE_PRINT("In corner %i\n", corner);
    if(corner == quad){
      if (!(40 <= rest && rest <= 50)){
        if (rest <= 40){
          avoidance_heading_direction = -1.f;
        }else{
          avoidance_heading_direction = 1.f;
        }
      }
    }
  }
  return false;
}

/*
 * Checks what wall of the cyberzoo the drone would collide with 
 */

uint8_t CheckWall(struct EnuCoor_i new_coor)
{
  float x = POS_FLOAT_OF_BFP(new_coor.x);
  float y = POS_FLOAT_OF_BFP(new_coor.y);
  float psi = 0.0;

  RotationOperation(&x, &y, &psi);

  if (fabs(y)>fabs(x)){
    if (y>0){
      wall = 1;
    }else{
      wall = 3;
    }
  }else{
    if (x>0){
      wall = 2; 
    }else{
      wall = 4;
    }
  }
  return false;
}

/*
 * Transforms the coordinates to an inertial reference frame for ease of calculations
 */
uint8_t RotationOperation(float *x, float *y, float *psi){
  float rot_angle = RadOfDeg(35.18285188);

  float x_old = *x;
  float y_old = *y;

  *x = cosf(rot_angle)* x_old+sinf(rot_angle)* y_old;
  *y = (-sinf(rot_angle)* x_old+cosf(rot_angle)* y_old);
  *psi = *psi+DegOfRad(rot_angle);

  return false;
}


/*
 * Calculates coordinates of a distance of 'distanceMeters' forward w.r.t. current position and heading
 */
uint8_t calculateForwards(struct EnuCoor_i *new_coor, float distanceMeters)
{
  float heading  = stateGetNedToBodyEulers_f()->psi;

  // Now determine where to place the waypoint you want to go to
  new_coor->x = stateGetPositionEnu_i()->x + POS_BFP_OF_REAL(sinf(heading) * (distanceMeters));
  new_coor->y = stateGetPositionEnu_i()->y + POS_BFP_OF_REAL(cosf(heading) * (distanceMeters));
  // VERBOSE_PRINT("Calculated %f m forward position. x: %f  y: %f based on pos(%f, %f) and heading(%f)\n", distanceMeters,	
                // POS_FLOAT_OF_BFP(new_coor->x), POS_FLOAT_OF_BFP(new_coor->y),
                // stateGetPositionEnu_f()->x, stateGetPositionEnu_f()->y, DegOfRad(heading));
  return false;
}
