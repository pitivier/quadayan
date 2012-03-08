

#include "Device_i2C.h"
#include "AQMath.h"

#define ITG3200_ADDRESS 0x68

#define ITG3200_MEMORY_ADDRESS                  0x1D
#define ITG3200_BUFFER_SIZE                     6
#define ITG3200_RESET_ADDRESS                   0x3E
#define ITG3200_RESET_VALUE                     0x80
#define ITG3200_LOW_PASS_FILTER_ADDR            0x16
#define ITG3200_LOW_PASS_FILTER_VALUE           0x1D    // 10Hz low pass filter
#define ITG3200_OSCILLATOR_ADDR                 0x3E
#define ITG3200_OSCILLATOR_VALUE                0x01    // use X gyro oscillator
#define ITG3200_SCALE_TO_RADIANS                823.626831 // 14.375 LSBs per °/sec, / Pi / 18

#define FINDZERO 49

#define XAXIS 0
#define YAXIS 1
#define ZAXIS 2



const int SDA_PIN = 4;
const int SCL_PIN = 5;

int gyroAddress = ITG3200_ADDRESS;

float gyroRate[3] = {0.0,0.0,0.0};
int   gyroZero[3] = {0,0,0};
long  gyroSample[3] = {0,0,0};
float gyroSmoothFactor = 1.0;
float gyroScaleFactor = 0.0;
float gyroHeading = 0.0;
unsigned long gyroLastMesuredTime = 0;

void initializeGyro() {

  /*if (readWhoI2C(gyroAddress) == ITG3200_ADDRESS+1) {
          vehicleState |= GYRO_DETECTED;
  }*/
        
  gyroScaleFactor = radians(1.0 / 14.375);  //  ITG3200 14.375 LSBs per °/sec
  updateRegisterI2C(gyroAddress, ITG3200_RESET_ADDRESS, ITG3200_RESET_VALUE); // send a reset to the device
  updateRegisterI2C(gyroAddress, ITG3200_LOW_PASS_FILTER_ADDR, ITG3200_LOW_PASS_FILTER_VALUE); // 10Hz low pass filter
  updateRegisterI2C(gyroAddress, ITG3200_RESET_ADDRESS, ITG3200_OSCILLATOR_VALUE); // use internal oscillator 
}

void calibrateGyro() {
  int findZero[FINDZERO];
    for (byte calAxis = XAXIS; calAxis <= ZAXIS; calAxis++) {
      for (int i=0; i<FINDZERO; i++) {
        sendByteI2C(gyroAddress, (calAxis * 2) + 0x1D);
        findZero[i] = readShortI2C(gyroAddress);
        delay(10);
      }
      if (calAxis == XAXIS) {
        gyroZero[YAXIS] = findMedianInt(findZero, FINDZERO);
      }
      else if (calAxis == YAXIS) {
        gyroZero[XAXIS] = findMedianInt(findZero, FINDZERO);
      }
      else {
        gyroZero[ZAXIS] = findMedianInt(findZero, FINDZERO);
      }
    }
}

void measureGyro() {
  sendByteI2C(gyroAddress, ITG3200_MEMORY_ADDRESS);
  Wire.requestFrom(gyroAddress, ITG3200_BUFFER_SIZE);
    
  // The following 3 lines read the gyro and assign it's data to gyroADC
  // in the correct order and phase to suit the standard shield installation
  // orientation.  See TBD for details.  If your shield is not installed in this
  // orientation, this is where you make the changes.
  int gyroADC[3];
  gyroADC[YAXIS] = readShortI2C() - gyroZero[YAXIS];
  gyroADC[XAXIS] = readShortI2C() - gyroZero[XAXIS];
  gyroADC[ZAXIS] = gyroZero[ZAXIS] - readShortI2C();

  for (byte axis = 0; axis <= ZAXIS; axis++) {
    gyroRate[axis] = filterSmooth(gyroADC[axis] * gyroScaleFactor, gyroRate[axis], gyroSmoothFactor);
  }
 
  // Measure gyro heading
  long int currentTime = micros();
  if (gyroRate[ZAXIS] > radians(1.0) || gyroRate[ZAXIS] < radians(-1.0)) {
    gyroHeading += gyroRate[ZAXIS] * ((currentTime - gyroLastMesuredTime) / 1000000.0);
  }
  gyroLastMesuredTime = currentTime;
}


