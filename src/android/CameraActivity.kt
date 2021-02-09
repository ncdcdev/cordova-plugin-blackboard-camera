package jp.co.taisei.construction.fieldmanagement.plugin

import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.Build
import android.os.Bundle
import android.support.v4.app.ActivityCompat
import android.support.v4.content.ContextCompat
import android.util.Log
import android.view.KeyEvent
import jp.co.ncdc.apppot.stew.utils.JsonUtils
//import jp.co.taisei.construction.fieldmanagement.R
//import jp.co.taisei.construction.fieldmanagement.prod2.R
import jp.co.taisei.construction.fieldmanagement.develop.R
import kotlinx.android.synthetic.main.fragment_camera2.*
import org.apache.cordova.CordovaActivity
import org.json.JSONObject

class CameraActivity : CordovaActivity(), SensorEventListener, ActivityCompat.OnRequestPermissionsResultCallback {
    //センサ用定数
    private val THRESHOLD_DEGREE = 60                                   //回転したと判定する際の閾値角度
    private val VERTICAL_TO_HORIZONTAL_DEGREE = THRESHOLD_DEGREE        //縦から横に変化したと判定する際の角度
    private val HORIZONTAL_TO_VERTICAL_DEGREE = 90 - THRESHOLD_DEGREE   //横から縦にに変化したと判定する際の角度
    private val ORIENTATION_VERTICAL = 0     //縦向きを表す定数
    private val ORIENTATION_HORIZONTAL = 1   //横向きを表す定
    private val PERMISSIONS_REQUEST_CODE = 100

    val RAD2DEG = 180 / Math.PI  //ラジアンを度に変換する際の定数
    val MATRIX_SIZE = 16         //回転行列の要素数

    var inR = FloatArray(MATRIX_SIZE)
    var outR = FloatArray(MATRIX_SIZE)
    var I = FloatArray(MATRIX_SIZE)

    var orientationValues = FloatArray(3)
    var magneticValues = FloatArray(3)
    var accelerometerValues = FloatArray(3)

    var boardPath: String? = null
    var isNeedBlackBoard: Boolean = false
    lateinit var blackboardViewPriority: String
    lateinit var pictureJson: JSONObject

    private var mPreOrientation = -1

    private var sensorManager: SensorManager? = null

    private var mSensorOrientation: SensorOrientation? = SensorOrientation.getInstance()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)

        //カメラフラグメント設定
        if (null == savedInstanceState) {
            val instance = Camera2Fragment.newInstance()
            fragmentManager.beginTransaction()
                    .replace(R.id.container, instance)
                    .commit()
        }
        this.boardPath = intent.extras["boardPath"] as String?
        this.isNeedBlackBoard = intent.extras["isNeedBlackBoard"] as Boolean
        this.blackboardViewPriority = intent.extras["blackboardViewPriority"] as String
        val jsonString = intent.extras["pictureJson"] as String?
        this.pictureJson = JsonUtils.convertToJSONObject(jsonString) ?: JSONObject()
        Log.d(TAG, "pictureJson=${pictureJson}")
        checkPermission()
    }


    //センサインスタンス生成
    private fun initSensor(){
        sensorManager = getSystemService(Context.SENSOR_SERVICE) as SensorManager
        if( sensorManager == null ) return
    }

    private fun checkPermission() {
        if (Build.VERSION.SDK_INT >= 23) {
            if (ContextCompat.checkSelfPermission(this!!, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE), PERMISSIONS_REQUEST_CODE)
            } else {
                initSensor()
            }
        } else {
            initSensor()
        }
    }

    override fun onResume() {
        super.onResume()

        sensorManager!!.registerListener(
                this,
                sensorManager!!.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                SensorManager.SENSOR_DELAY_UI)
        sensorManager!!.registerListener(
                this,
                sensorManager!!.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),
                SensorManager.SENSOR_DELAY_UI)
    }

    override fun onAccuracyChanged(sensor: Sensor, accuracy: Int) {}

    override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            val intent = Intent()
            intent.putExtra("mode", blackboardViewPriority)
            setResult(0, intent)
            finish()
        } else if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN || keyCode == KeyEvent.KEYCODE_CAMERA) {
            val fragment = fragmentManager.findFragmentById(R.id.container)
            if (fragment != null && fragment is Camera2Fragment) {
                fragment.takePicture()
                return false
            }

        }
        return super.onKeyDown(keyCode, event)
    }

    //傾き変化時処理
    override fun onSensorChanged(event: SensorEvent){

        when (event.sensor.type) {
            Sensor.TYPE_MAGNETIC_FIELD -> magneticValues = event.values.clone()
            Sensor.TYPE_ACCELEROMETER  -> accelerometerValues = event.values.clone()
        }

        if (magneticValues != null && accelerometerValues != null) {

            SensorManager.getRotationMatrix(inR, I, accelerometerValues, magneticValues);

            SensorManager.remapCoordinateSystem(inR, SensorManager.AXIS_X, SensorManager.AXIS_Z, outR);
            SensorManager.getOrientation(outR, orientationValues);

            var roll = (orientationValues[2] * RAD2DEG).toInt()

            //画面の縦横判定処理
            var absRoll = Math.abs(roll)
            if (mPreOrientation == -1) {
                if( absRoll < VERTICAL_TO_HORIZONTAL_DEGREE ){
                    mPreOrientation = ORIENTATION_VERTICAL
                }else{
                    mPreOrientation = ORIENTATION_HORIZONTAL
                }
            } else if (absRoll < 90) {
                mPreOrientation = getOrientation(mPreOrientation, roll);
            } else {
                // プラマイ90度を超える場合は90度未満に置き換えて向きを反転させる。
                var plusMinus:Int
                if( roll >= 0 ){
                    plusMinus = 1
                }else{
                    plusMinus = -1
                }
                roll = (absRoll - 90) * plusMinus
                var preOrientation: Int = invertOrientation(mPreOrientation)
                mPreOrientation = invertOrientation(getOrientation(preOrientation, roll))
            }

            // 表示の更新
            if( mPreOrientation == ORIENTATION_VERTICAL ){
                picture.rotation = 0f
                mSensorOrientation!!.orientation = 0;
                setFragmentOrientation(90)
            }else {
                if( roll > 60 ) {
                    picture.rotation = -90f
                    mSensorOrientation!!.orientation = 90;
                    setFragmentOrientation(180)
                }else if ( roll < -60 ){
                    picture.rotation = 90f
                    mSensorOrientation!!.orientation = 270;
                    setFragmentOrientation(0)
                }
            }
        }

    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        when (requestCode) {
            PERMISSIONS_REQUEST_CODE -> {
                if ((grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED)) {
                    // 継続
                    initSensor()
                } else {
                    val intent = Intent()
                    intent.putExtra("mode", blackboardViewPriority)
                    setResult(0, intent)
                    finish()
                }
                return
            }
        }
    }

    private fun setFragmentOrientation(orientation: Int) {
        val fragment = fragmentManager.findFragmentById(R.id.container)
        if (fragment != null && fragment is Camera2Fragment) {
            fragment.setOrientation(orientation)
        }
    }

    private fun getOrientation(preOrientation: Int, roll: Int): Int {
        val absRoll = Math.abs(roll)
        return if (preOrientation == ORIENTATION_VERTICAL) {
            if (absRoll < VERTICAL_TO_HORIZONTAL_DEGREE)
                ORIENTATION_VERTICAL
            else
                ORIENTATION_HORIZONTAL
        } else if (preOrientation == ORIENTATION_HORIZONTAL) {
            if (absRoll < HORIZONTAL_TO_VERTICAL_DEGREE)
                ORIENTATION_VERTICAL
            else
                ORIENTATION_HORIZONTAL
        } else {
            -1
        }
    }

    private fun invertOrientation(orientation: Int): Int {
        return if (orientation == ORIENTATION_HORIZONTAL) {
            ORIENTATION_VERTICAL
        } else if (orientation == ORIENTATION_VERTICAL) {
            ORIENTATION_HORIZONTAL
        } else {
            -1
        }
    }

    companion object {

        private const val TAG = "CameraActivity"
    }
}