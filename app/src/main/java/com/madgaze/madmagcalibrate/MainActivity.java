package com.madgaze.madmagcalibrate;

import java.util.List;
import java.util.Vector;

import android.app.Activity;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
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

	public List<sample> samples = new ArrayList<sample>();//用于记录采样点的值

	public static final int MAX_COORDINATE = 5;
	public List<sample> mag = new ArrayList<sample>();//用于计算单个点的均值
	private int mRecordStatus;
	public static final int RECORD_STARTED = 0;
	public static final int  RECORD_STOPPED = 1;
    public static final int MAX_SAMPLE_COUNT = 13;
	public native boolean addSample(double x, double y, double z);
	public native double[] getParams(double radius);

    Handler mHandler = new Handler();
    Runnable r = new Runnable() {

        @Override
        public void run() {
            //do something
            //每隔1s循环执行run方法
            synchronized (MainActivity.this) {
                mRecordStatus = RECORD_STARTED;
                if (mStartFlag == RECORDING) {
                    mHandler.postDelayed(this, 1000);
                }
            }
        }
    };

	private void initSensorService() {
        mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);
        mMagneticField = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
    }
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mStartFlag = INIT;
		mRecordStatus = RECORD_STOPPED;
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
                        mRecordStatus = RECORD_STARTED;
                        mHandler.postDelayed(r, 100);//延时100毫秒
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
                            //Log.d(TAG, "mag: ("+s.m_x+"," + s.m_y + "," + s.m_z +")");
                        }

                        double[] params = getParams(13.0);
                        if (params != null) {
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
    protected void onPause() {
        super.onPause();
        if (mSensorManager != null) {
            mSensorManager.unregisterListener(this);
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
            //mMagneticFieldValues = event.values;
            //addSample(mMagneticFieldValues[0],mMagneticFieldValues[1],mMagneticFieldValues[2]);
            synchronized (MainActivity.this) {
                if(mStartFlag == RECORDING && mRecordStatus == RECORD_STARTED) {
                    mRecordStatus = RECORD_STOPPED;
                    sample cord = new sample();
                    cord.m_x = event.values[0];
                    cord.m_y = event.values[1];
                    cord.m_z = event.values[2];

					//String coordinateTxt = "X: " + cord.m_x + ", Y: " + cord.m_y + ", Z: " + cord.m_z;
					//mTextViewParams.setText(coordinateTxt);
					samples.add(cord);
				}
			}
        } 
	}
}
