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
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import android.view.KeyEvent
//import jp.co.taisei.construction.fieldmanagement.R
import jp.co.taisei.construction.fieldmanagement.prod2.R
//import jp.co.taisei.construction.fieldmanagement.develop.R
//import kotlinx.android.synthetic.main.fragment_camera2.*
import org.apache.cordova.CordovaActivity

class CameraActivity : CordovaActivity(), SensorEventListener, ActivityCompat.OnRequestPermissionsResultCallback {
    //センサ用定数
    private val THRESHOLD_DEGREE = 60                                   //回転したと判定する際の閾値角度
    private val VERTICAL_TO_HORIZONTAL_DEGREE = THRESHOLD_DEGREE        //縦から横に変化したと判定する際の角度
    private val HORIZONTAL_TO_VERTICAL_DEGREE = 90 - THRESHOLD_DEGREE   //横から縦にに変化したと判定する際の角度
    private val ORIENTATION_VERTICAL = 0     //縦向きを表す定数
    private val ORIENTATION_HORIZONTAL = 1   //横向きを表す定
    private val PERMISSIONS_REQUEST_CODE = 100
    private val CAMERA_PERMISSION_REQUEST_CODE = 101
    private val STORAGE_PERMISSION_REQUEST_CODE = 102

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
    var photoInfo: PhotoInfo? = null
    lateinit var blackboardViewPriority: String
    lateinit var version: String

    private var mPreOrientation = -1

    private var sensorManager: SensorManager? = null

    private var mSensorOrientation: SensorOrientation? = SensorOrientation.getInstance()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)

        //カメラフラグメント設定
        val instance = Camera2Fragment.newInstance()
        if (null == savedInstanceState) {
            fragmentManager.beginTransaction()
                    .replace(R.id.container, instance)
                    .commit()
        }
        this.boardPath = intent.extras?.get("boardPath") as String?
        this.isNeedBlackBoard = intent.extras?.get("isNeedBlackBoard") as Boolean
        this.blackboardViewPriority = intent.extras!!["blackboardViewPriority"] as String
        this.photoInfo = intent.extras?.get("photoInfo") as PhotoInfo?
        this.version = intent.extras?.get("version") as String

        checkPermissions()
    }


    //センサインスタンス生成
    private fun initSensor(){
        sensorManager = getSystemService(Context.SENSOR_SERVICE) as SensorManager
        if( sensorManager == null ) return
    }

    private fun checkPermissions() {
        if (Build.VERSION.SDK_INT >= 23) {
            val cameraPermission = ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
            val storagePermission = ContextCompat.checkSelfPermission(this, Manifest.permission.READ_MEDIA_IMAGES)

            if (cameraPermission != PackageManager.PERMISSION_GRANTED) {
                requestCameraPermission()
            } else if (storagePermission != PackageManager.PERMISSION_GRANTED) {
                requestStoragePermission()
            } else {
                initSensor()
            }
        } else {
            initSensor()
        }
    }

    private fun requestCameraPermission() {
        if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.CAMERA)) {
            // カメラ権限が必要な理由を説明するダイアログを表示
            showRationaleDialog("カメラ", CAMERA_PERMISSION_REQUEST_CODE)
        } else {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.CAMERA), CAMERA_PERMISSION_REQUEST_CODE)
        }
    }

    private fun showRationaleDialog(permissionName: String, requestCode: Int) {
        val builder = androidx.appcompat.app.AlertDialog.Builder(this)
        builder.setTitle("$permissionName パーミッションが必要です")
        builder.setMessage("$permissionName パーミッションを許可してください。")
        builder.setPositiveButton("OK") { _, _ ->
            ActivityCompat.requestPermissions(this, arrayOf(
                if (requestCode == CAMERA_PERMISSION_REQUEST_CODE) Manifest.permission.CAMERA else Manifest.permission.READ_EXTERNAL_STORAGE
            ), requestCode)
        }
        builder.setNegativeButton("キャンセル") { dialog, _ ->
            dialog.dismiss()
            showPermissionDeniedDialog(permissionName)
        }
        builder.show()
    }


    private fun requestStoragePermission() {
        if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.READ_EXTERNAL_STORAGE)) {
            // ストレージ権限が必要な理由を説明するダイアログを表示
            showRationaleDialog("ストレージ", STORAGE_PERMISSION_REQUEST_CODE)
        } else {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE), STORAGE_PERMISSION_REQUEST_CODE)
        }
    }

    private fun showPermissionDeniedDialog(permissionName: String) {
        val builder = androidx.appcompat.app.AlertDialog.Builder(this)
        builder.setTitle("$permissionName パーミッションが必要です")
        builder.setMessage("$permissionName パーミッションを許可してください。設定画面で許可できます。")
        builder.setPositiveButton("設定") { _, _ ->
            val intent = Intent(android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS)
            val uri = android.net.Uri.fromParts("package", packageName, null)
            intent.data = uri
            startActivity(intent)
        }
        builder.setNegativeButton("キャンセル") { dialog, _ ->
            dialog.dismiss()
            val intent = Intent()
            intent.putExtra("mode", blackboardViewPriority)
            setResult(0, intent)
            finish()
        }
        builder.show()
    }


    override fun onResume() {
        super.onResume()
        registerSensorListeners()
    }

    private fun registerSensorListeners() {
        sensorManager?.registerListener(
            this,
            sensorManager?.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
            SensorManager.SENSOR_DELAY_UI
        )
        sensorManager?.registerListener(
            this,
            sensorManager?.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),
            SensorManager.SENSOR_DELAY_UI
        )
    }

    override fun onPause() {
        super.onPause()
        unregisterSensorListeners()
    }

    override fun onDestroy() {
        super.onDestroy()
        sensorManager = null
    }

    private fun unregisterSensorListeners() {
        sensorManager?.unregisterListener(this)
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
                mSensorOrientation!!.orientation = 0;
                setFragmentOrientation(90)
            }else {
                if( roll > 60 ) {
                    mSensorOrientation!!.orientation = 90;
                    setFragmentOrientation(180)
                }else if ( roll < -60 ){
                    mSensorOrientation!!.orientation = 270;
                    setFragmentOrientation(0)
                }
            }
        }

    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            PERMISSIONS_REQUEST_CODE -> {
                if (grantResults.isNotEmpty() && grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
                    // すべてのパーミッションが許可された
                    initSensor()
                } else {
                    // パーミッションが拒否された
                    showPermissionDeniedDialog()
                }
            }
            CAMERA_PERMISSION_REQUEST_CODE -> {
                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // カメラ権限が許可された
                    val storagePermission = ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
                    if (storagePermission != PackageManager.PERMISSION_GRANTED) {
                        requestStoragePermission()
                    } else {
                        initSensor()
                    }
                } else {
                    // カメラ権限が拒否された
                    showPermissionDeniedDialog("カメラ")
                }
            }
            STORAGE_PERMISSION_REQUEST_CODE -> {
                if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // ストレージ権限が許可された
                    val cameraPermission = ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                    if (cameraPermission != PackageManager.PERMISSION_GRANTED) {
                        requestCameraPermission()
                    } else {
                        initSensor()
                    }
                } else {
                    // ストレージ権限が拒否された
                    showPermissionDeniedDialog("ストレージ")
                }
            }
        }
    }

    private fun showPermissionDeniedDialog() {
        val builder = androidx.appcompat.app.AlertDialog.Builder(this)
        builder.setTitle("パーミッションが必要です")
        builder.setMessage("カメラ機能を使用するには、カメラとストレージのパーミッションが必要です。設定画面でパーミッションを許可してください。")
        builder.setPositiveButton("設定") { _, _ ->
            val intent = Intent(android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS)
            val uri = android.net.Uri.fromParts("package", packageName, null)
            intent.data = uri
            startActivity(intent)
        }
        builder.setNegativeButton("キャンセル") { dialog, _ ->
            dialog.dismiss()
            val intent = Intent()
            intent.putExtra("mode", blackboardViewPriority)
            setResult(0, intent)
            finish()
        }
        builder.show()
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