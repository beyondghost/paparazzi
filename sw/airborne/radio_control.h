/*
 * Paparazzi $Id$
 *  
 * Copyright (C) 2006 Pascal Brisset, Antoine Drouin
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 *
 */

#ifndef RADIO_CONTROL_H
#define RADIO_CONTROL_H

#if defined RADIO_CONTROL

#define RadioControlEventCheckAndHandle(_user_callback) { \
    if (ppm_valid) {					  \
      ppm_valid = FALSE;				  \
      radio_control_event_task();			  \
      _user_callback();					  \
    }							  \
  }



#include "led.h"
#include "sys_time.h"
#include "ppm.h"
#include "radio.h"
#include "airframe.h"
#include "paparazzi.h"

#define RC_AVG_PERIOD 8
#define RC_LOST_TIME 30  /* 500ms with a 60Hz timer */
#define RC_REALLY_LOST_TIME 60 /* ~1s */
// Number of valid ppm frame to go back to RC OK
#define RC_OK_CPT 15

#define RC_OK          0
#define RC_LOST        1
#define RC_REALLY_LOST 2

extern pprz_t rc_values[PPM_NB_PULSES];
extern uint8_t rc_status;
extern int32_t avg_rc_values[PPM_NB_PULSES];
extern uint8_t rc_values_contains_avg_channels;
extern uint8_t time_since_last_ppm;
extern uint8_t ppm_cpt, last_ppm_cpt, radio_ok_cpt;

/* 
 * On tiny (and booz) the ppm counter is running at the same speed as
 * the systic counter. There is no reason for this to be true.
 * Let's add a pair of macros to make it possible for them to be different.
 *
 */
#define RC_PPM_TICS_OF_USEC        SYS_TICS_OF_USEC
#define RC_PPM_SIGNED_TICS_OF_USEC SIGNED_SYS_TICS_OF_USEC


/************* INIT ******************************************************/
static inline void radio_control_init ( void ) {
  rc_status = RC_REALLY_LOST; 
  time_since_last_ppm = RC_REALLY_LOST_TIME;
  radio_ok_cpt = 0;
}

/************* PERIODIC ******************************************************/
static inline void radio_control_periodic_task ( void ) {
  static uint8_t _1Hz;
  _1Hz++;

  if (_1Hz >= 60) {
    _1Hz = 0;
    last_ppm_cpt = ppm_cpt;
    ppm_cpt = 0;
  }

  if (time_since_last_ppm >= RC_REALLY_LOST_TIME) {
    rc_status = RC_REALLY_LOST;
  } else {
    if (time_since_last_ppm >= RC_LOST_TIME) {
      rc_status = RC_LOST;
      radio_ok_cpt = RC_OK_CPT;
    }
    time_since_last_ppm++;
  }

#if defined RC_LED
  if (rc_status == RC_OK) {
    LED_ON(RC_LED);
  }
  else {
    LED_OFF(RC_LED);
  }
#endif
 
}

/********** EVENT ************************************************************/
static inline void radio_control_event_task ( void ) {
  ppm_cpt++;
  time_since_last_ppm = 0;

  /* Wait for enough valid frame to switch back to RC_OK */
  if (radio_ok_cpt > 0) radio_ok_cpt--;
  else {
    rc_status = RC_OK;
    /** From ppm values to normalised rc_values */
    NormalizePpm();
  }
}

#endif /* RADIO_CONTROL */

#endif /* RADIO_CONTROL_H */
