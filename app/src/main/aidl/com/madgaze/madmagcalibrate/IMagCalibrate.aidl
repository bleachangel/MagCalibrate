// IMagCalibrate.aidl
package com.madgaze.madmagcalibrate;

// Declare any non-default types here with import statements

interface IMagCalibrate {
    boolean setBias(double x_bias, double y_bias, double z_bias, double x_scaler, double y_scaler, double z_scaler);
    double [] getBias();
}
