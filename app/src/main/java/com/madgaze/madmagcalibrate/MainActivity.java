package com.madgaze.madmagcalibrate;

import java.util.List;
import java.util.Vector;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import java.util.ArrayList;

public class MainActivity extends Activity implements SensorEventListener {
	public static final String TAG = "MagActivity";
	private Button mCaliStart;
	private TextView mTextViewParams;
	
	private SensorManager mSensorManager;
    private Sensor mMagneticField;
    private float[] mMagneticFieldValues = new float[3];
    private int mStartFlag;
    
    public static final int INIT = 0;
	public static final int  RECORDING = 1;
    public static final int STOPPED = 2;
    public class sample{
    	double m_x;
    	double m_y;
    	double m_z;
    }

	static {
        System.loadLibrary("jni_MadMagCalibrate");
    }
    private IMagCalibrate messageCenter = null;
    private boolean mBound = false;
	public List<sample> samples = new ArrayList<sample>();//用于记录采样点的值
	public native boolean addSample(double x, double y, double z);
	public native double[] getParams(double radius);
    //public native boolean setBias(double[] bias);
	private void initSensorService() {
        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mMagneticField = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
    }
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mStartFlag = INIT;
        mTextViewParams = (TextView)findViewById(R.id.textViewParams);
        mCaliStart = (Button)findViewById(R.id.cali_start);
        mCaliStart.setText(R.string.cali_start);
        mCaliStart.setOnClickListener(new View.OnClickListener(){

			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
                synchronized (MainActivity.this) {
                    if (mStartFlag == INIT) {
                        mStartFlag = RECORDING;
                        samples.clear();

                        mCaliStart.setEnabled(false);
                        mCaliStart.setText(R.string.cali_stop);
                        mCaliStart.setEnabled(true);

                    } else if (mStartFlag == RECORDING) {
                        mStartFlag = STOPPED;
                        mCaliStart.setEnabled(false);
                        int size = samples.size();

                        Log.d(TAG, "array size : (" + size + ")");
                        for (int i = 0; i < size; i++) {
                            sample s;
                            s = samples.get(i);
                            addSample(s.m_x, s.m_y, s.m_z);
                        }

                        double[] params = getParams(13.0);
                        if (params != null) {
                            if (messageCenter != null) {
                                try {
                                    messageCenter.setBias(params[0], params[1], params[2], params[3], params[4], params[5]);
                                } catch (RemoteException e) {
                                    e.printStackTrace();
                                }
                            }
                            mTextViewParams.setText("param : (" + params[0] + "," + params[1] + "," + params[2] + "," + params[3] + "," + params[4] + "," + params[5] + ")");
                            Log.d(TAG, "param : (" + params[0] + "," + params[1] + "," + params[2] + ","
                                    + params[3] + "," + params[4] + "," + params[5] + ")");
                        }

                        mCaliStart.setText(R.string.cali_start);
                        mCaliStart.setEnabled(true);
                        mStartFlag = INIT;

                        Toast.makeText(MainActivity.this, "vector size :" + size, Toast.LENGTH_SHORT).show();
                    }
                }
			}
        	
        });

        initSensorService();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    @Override
    protected void onResume() {
        super.onResume();
        mSensorManager.registerListener(this, mMagneticField, SensorManager.SENSOR_DELAY_NORMAL);
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (!mBound) {
            attemptToBindService();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mSensorManager != null) {
            mSensorManager.unregisterListener(this);
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mBound) {
            unbindService(mServiceConnection);
            mBound = false;
        }
    }

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void onSensorChanged(SensorEvent event) {
		// TODO Auto-generated method stub
        if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
            synchronized (MainActivity.this) {
                if(mStartFlag == RECORDING) {
                    sample cord = new sample();
                    cord.m_x = event.values[0];
                    cord.m_y = event.values[1];
                    cord.m_z = event.values[2];

					samples.add(cord);
				}
			}
        } 
	}

    /**
     * 尝试与服务端建立连接
     */
    private void attemptToBindService() {
        Intent intent = new Intent();
        //intent.setAction("com.vvvv.aidl");
        //intent.setPackage("com.iiiv.aidlserver");
        bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
    }


    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.e(getLocalClassName(), "service connected");
            messageCenter = IMagCalibrate.Stub.asInterface(service);
            mBound = true;

            if (messageCenter != null) {
                try {
                    double[] p = messageCenter.getBias();
                    Log.e(getLocalClassName(), "bias = ("+p[0]+","+p[1]+","+p[2]+"),scaler =("+p[3]+","+p[4]+","+p[5]+")");
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.e(getLocalClassName(), "service disconnected");
            mBound = false;
        }
    };

}
