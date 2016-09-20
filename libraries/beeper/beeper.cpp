#include <Arduino.h>
#include <beeper.h>
#include <toneAC.h>

beeper::beeper(double lowerThreshold, double upperThreshold) {

  /* check threshold */
  beepLowerThreshold = lowerThreshold;
  if( upperThreshold - BEEP_VELOCITY_SENSITIVITY <= 0.0 ) {
    beepUpperThreshold = BEEP_VELOCITY_SENSITIVITY;
  } else {
    beepUpperThreshold = upperThreshold;
  }

  /* init private vars */
  beepStartTime = 0;
  beepVelocity = 0.0;
  beepFilteredVelocity = beepVelocity * BEEP_VELOCITY_FILTER_COEFF + BEEP_VELOCITY_FILTER_BASE;   
  beepFreq = BEEP_BASE_FREQ;
  beepPaternEnabled = false;
  beepPaternBasePosition = 0.0;
  beepPaternPosition = 0.0;
  beepStatus = LOW;
}

void beeper::setThresholds(double lowerThreshold, double upperThreshold) {

  /* check threshold */
  beepLowerThreshold = lowerThreshold;
  if( upperThreshold - BEEP_VELOCITY_SENSITIVITY <= 0.0 ) {
    beepUpperThreshold = BEEP_VELOCITY_SENSITIVITY;
  } else {
    beepUpperThreshold = upperThreshold;
  }
}

void beeper::setBeepParameters(double velocity) {
  beepVelocity = velocity;
  beepFilteredVelocity = beepVelocity * BEEP_VELOCITY_FILTER_COEFF + BEEP_VELOCITY_FILTER_BASE;
  if(velocity > 0.0) {
    beepFreq = BEEP_FREQ_COEFF * velocity + BEEP_BASE_FREQ;
    beepPaternEnabled = true;
  } else {
    beepFreq = BEEP_LOW_FREQ;
    beepPaternEnabled = false;
  }
}

void beeper::setVelocity(double velocity) {
  
  /**************/
  /* check beep */
  /**************/
  
  /* check if beep need to be started */
  if( beepStartTime == 0 &&
      (velocity < beepLowerThreshold || velocity > beepUpperThreshold)
      ) {
    beepStartTime = millis();
    beepPaternBasePosition = 0.0;
    beepPaternPosition = 0.0;
    setBeepParameters(velocity);
  }

  /* check if beep need to be stopped */
  if( beepStartTime != 0 &&
      velocity > beepLowerThreshold + BEEP_VELOCITY_SENSITIVITY &&
      velocity < beepUpperThreshold - BEEP_VELOCITY_SENSITIVITY ) {
    beepStartTime = 0;
  }

  /* check if beep parameters need to be changed */
  if( beepStartTime != 0 &&
      (velocity < beepVelocity - BEEP_VELOCITY_SENSITIVITY || velocity > beepVelocity + BEEP_VELOCITY_SENSITIVITY)
      ) {
    setBeepParameters(velocity);
  }
}


void beeper::update() {

  /***************/
  /* update beep */
  /***************/
  if( beepStartTime != 0 ) {
    if( beepPaternEnabled ) {
      
      /* update position */
      unsigned long currentTime = millis();
      double currentLength = (double)(currentTime - beepStartTime);
      currentLength = currentLength/1000.0 * beepFilteredVelocity;
      
      if( currentLength + beepPaternBasePosition > beepPaternPosition ) {
        beepPaternPosition = currentLength + beepPaternBasePosition;
      }
      
      while( beepPaternPosition > BEEP_HIGH_LENGTH + BEEP_LOW_LENGTH ) {
        beepPaternPosition -= BEEP_HIGH_LENGTH + BEEP_LOW_LENGTH;
        beepStartTime = millis();
        beepPaternBasePosition = beepPaternPosition;
      }

      /* update patern */
      if( beepPaternPosition < BEEP_HIGH_LENGTH ) {
        /* beep high */
        if( beepStatus == LOW ) {
          toneAC(beepFreq);
          beepStatus = HIGH;
        } 
      } else {
        /* beep low */ 
        toneAC(0);
        beepStatus = LOW;
      }
    } else {
      // no patern 
      if( beepStatus == LOW ) {
        toneAC(beepFreq);
        beepStatus = HIGH;
      } 
    }
  } else {
    // no beep 
    toneAC(0);
    beepStatus = LOW;
  }
}