package jp.co.taisei.construction.fieldmanagement.plugin

import android.annotation.SuppressLint
import android.annotation.TargetApi
import android.app.Fragment
import android.content.ContentValues
import android.content.Context
import android.content.Context.CAMERA_SERVICE
import android.content.Intent
import android.content.SharedPreferences
import android.content.res.Configuration
import android.graphics.*
import android.graphics.drawable.BitmapDrawable
import android.hardware.camera2.*
import android.media.Image
import android.media.ImageReader
import android.media.MediaActionSound
import android.os.*
import android.provider.MediaStore
import android.support.media.ExifInterface
import android.util.Log
import android.util.Size
import android.util.SparseIntArray
import android.view.*
import android.widget.Button
import android.widget.ImageButton
import android.widget.ImageView
import android.widget.Toast
//import jp.co.taisei.construction.fieldmanagement.R
// import jp.co.taisei.construction.fieldmanagement.prod2.R
import jp.co.taisei.construction.fieldmanagement.develop.R
import java.io.ByteArrayInputStream
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.lang.Exception
import java.util.*
import java.util.concurrent.Semaphore
import java.util.concurrent.TimeUnit
import kotlin.math.max
import kotlin.math.min

@TargetApi(Build.VERSION_CODES.LOLLIPOP)
class Camera2Fragment : Fragment(), View.OnClickListener, View.OnTouchListener {

    /**
     * [TextureView.SurfaceTextureListener] handles several lifecycle events on a
     * [TextureView].
     */
    private val mSurfaceTextureListener = object : TextureView.SurfaceTextureListener {

        override fun onSurfaceTextureAvailable(texture: SurfaceTexture, width: Int, height: Int) {
            openCamera(width, height)
        }

        override fun onSurfaceTextureSizeChanged(texture: SurfaceTexture, width: Int, height: Int) {
            Log.d(TAG, "test:SurfaceTextureListener:call configureTransform:w=${width}, h=${height}")
            configureTransform(width, height)
        }

        override fun onSurfaceTextureDestroyed(texture: SurfaceTexture): Boolean {
            return true
        }

        override fun onSurfaceTextureUpdated(texture: SurfaceTexture) {}

    }

    /**
     * ID of the current [CameraDevice].
     */
    private var mCameraId: String? = null

    /**
     * An [AutoFitTextureView] for camera preview.
     */
    private var mTextureView: AutoFitTextureView? = null

    /**
     * Camera Flash : Auto -> Off -> On
     */
    private var mFlashButton: Button? = null

    private var mCameraButton: ImageButton? = null

    private var mToggleButton: Button? = null

    private var mBoardImage: ImageView? = null

    /**
     * 黒板拡大用マーカー
     */
    private var mLtCircle: ImageView? = null // leftTop
    private var mLbCircle: ImageView? = null // leftBottom
    private var mRtCircle: ImageView? = null // rightTop
    private var mRbCircle: ImageView? = null // rightBottom

    /**
     * A [CameraCaptureSession] for camera preview.
     */
    private var mCaptureSession: CameraCaptureSession? = null

    /**
     * A reference to the opened [CameraDevice].
     */
    private var mCameraDevice: CameraDevice? = null

    /**
     * The [android.util.Size] of camera preview.
     */
    private var mPreviewSize: Size? = null

    /**
     * [CameraDevice.StateCallback] is called when [CameraDevice] changes its state.
     */
    private val mStateCallback = object : CameraDevice.StateCallback() {

        override fun onOpened(cameraDevice: CameraDevice) {
            // This method is called when the camera is opened.  We start camera preview here.
            Log.d(TAG, "test:StateCallback:onOpened")
            mCameraOpenCloseLock.release()
            mCameraDevice = cameraDevice
            createCameraPreviewSession()
        }

        override fun onDisconnected(cameraDevice: CameraDevice) {
            Log.d(TAG, "test:StateCallback:onDisconnected")
            mCameraOpenCloseLock.release()
            cameraDevice.close()
            mCameraDevice = null
        }

        override fun onError(cameraDevice: CameraDevice, error: Int) {
            mCameraOpenCloseLock.release()
            cameraDevice.close()
            mCameraDevice = null
            val activity = activity
            activity?.finish()
        }

    }

    /**
     * An additional thread for running tasks that shouldn't block the UI.
     */
    private var mBackgroundThread: HandlerThread? = null

    /**
     * A [Handler] for running tasks in the background.
     */
    private var mBackgroundHandler: Handler? = null

    /**
     * An [ImageReader] that handles still image capture.
     */
    private var mImageReader: ImageReader? = null

    /**
     * 出力した写真ファイル
     */
    private lateinit var mFile: File

    /**
     * This a callback object for the [ImageReader]. "onImageAvailable" will be called when a
     * still image is ready to be saved.
     */
    private val mOnImageAvailableListener = ImageReader.OnImageAvailableListener { reader ->
        Log.d(TAG, "test:ImageReader:OnImageAvailableListener:($screenOffset)")
        val boardBitmapDrawable = if (mBoardImage!!.isVisible()) (mBoardImage?.drawable) as BitmapDrawable else null
        var boardRect = Rect()
        if (mBoardImage!!.isVisible()) {
            boardRect = mBoardImage!!.toRect() // screen基準座標(x, y)
            // TextureView上の座標に変換(0, 0)
            boardRect.offsetTo(boardRect.left - screenOffset.x, boardRect.top - screenOffset.y)
        }
        // 黒板の座標をカメラビューをベースにした座標に変換
        mBackgroundHandler!!.post(ImageSaver(reader.acquireNextImage(), mFile, boardBitmapDrawable, boardRect, mTextureView!!.toRect().min().toFloat(), activity as CameraActivity?))
    }

    /**
     * [CaptureRequest.Builder] for the camera preview
     */
    private var mPreviewRequestBuilder: CaptureRequest.Builder? = null

    /**
     * [CaptureRequest] generated by [.mPreviewRequestBuilder]
     */
    private var mPreviewRequest: CaptureRequest? = null

    /**
     * The current state of camera state for taking pictures.
     *
     * @see .mCaptureCallback
     */
    private var mState = STATE_PREVIEW

    /**
     * A [Semaphore] to prevent the app from exiting before closing the camera.
     */
    private val mCameraOpenCloseLock = Semaphore(1)

    /**
     * Whether the current camera device supports Flash or not.
     */
    private var mFlashSupported: Boolean = false

    /**
     * Orientation of the camera sensor
     */
    private var mSensorOrientation: Int = 0

    private var mFlashModeIndex: Int = 0 // 0:AUTO 1:OFF 2:ON

    private var mCameraStartFlag = false // カメラ２重処理防止

    private lateinit var preferences: SharedPreferences

    /**
     * 全画面から実際のカメラビューサイズの差分ポイント
     */
    private lateinit var screenOffset: Point

    /**
     * A [CameraCaptureSession.CaptureCallback] that handles events related to JPEG capture.
     */
    private val mCaptureCallback = object : CameraCaptureSession.CaptureCallback() {

        private fun process(result: CaptureResult) {
            when (mState) {
                STATE_PREVIEW -> {
                }// We have nothing to do when the camera preview is working normally.
                STATE_WAITING_LOCK -> {
//                    Log.d(TAG, "test:CaptureCallback:STATE_WAITING_LOCK")
                    val afState = result.get(CaptureResult.CONTROL_AF_STATE)
                    if (afState == null) {
                        captureStillPicture()
                    } else if (CaptureResult.CONTROL_AF_STATE_FOCUSED_LOCKED == afState || CaptureResult.CONTROL_AF_STATE_NOT_FOCUSED_LOCKED == afState) {
                        // CONTROL_AE_STATE can be null on some devices
                        val aeState = result.get(CaptureResult.CONTROL_AE_STATE)
                        if (aeState == null || aeState == CaptureResult.CONTROL_AE_STATE_CONVERGED) {
                            mState = STATE_PICTURE_TAKEN
                            Log.d(TAG, "test:CaptureCallback:STATE_WAITING_LOCK:2")
                            captureStillPicture()
                        } else {
                            Log.d(TAG, "test:CaptureCallback:STATE_WAITING_LOCK:3")
                            runPrecaptureSequence()
                        }
                    }
                }
                STATE_WAITING_PRECAPTURE -> {
                    Log.d(TAG, "test:CaptureCallback:STATE_WAITING_PRECAPTURE")
                    // CONTROL_AE_STATE can be null on some devices
                    val aeState = result.get(CaptureResult.CONTROL_AE_STATE)
                    if (aeState == null ||
                            aeState == CaptureResult.CONTROL_AE_STATE_PRECAPTURE ||
                            aeState == CaptureRequest.CONTROL_AE_STATE_FLASH_REQUIRED) {
                        mState = STATE_WAITING_NON_PRECAPTURE
                    }
                }
                STATE_WAITING_NON_PRECAPTURE -> {
                    Log.d(TAG, "test:CaptureCallback:STATE_WAITING_NON_PRECAPTURE")
                    // CONTROL_AE_STATE can be null on some devices


                    val aeState = result.get(CaptureResult.CONTROL_AE_STATE)
                    if (aeState == null || aeState != CaptureResult.CONTROL_AE_STATE_PRECAPTURE) {
                        mState = STATE_PICTURE_TAKEN
                        captureStillPicture()
                    }
                }
            }
        }

        override fun onCaptureProgressed(session: CameraCaptureSession,
                                         request: CaptureRequest,
                                         partialResult: CaptureResult) {
            process(partialResult)
        }

        override fun onCaptureCompleted(session: CameraCaptureSession,
                                        request: CaptureRequest,
                                        result: TotalCaptureResult) {
            process(result)
        }

    }

    /**
     * Shows a [Toast] on the UI thread.
     *
     * @param text The message to show
     */
    private fun showToast(text: String) {
        val activity = activity
        activity?.runOnUiThread { Toast.makeText(activity, text, Toast.LENGTH_SHORT).show() }
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?,
                              savedInstanceState: Bundle?): View? {
        return inflater!!.inflate(R.layout.fragment_camera2, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        view!!.findViewById<View>(R.id.picture).setOnClickListener(this)
        view!!.findViewById<View>(R.id.done).setOnClickListener(this)
        view!!.findViewById<View>(R.id.flash).setOnClickListener(this)
        view!!.findViewById<View>(R.id.edit).setOnClickListener(this)
        view!!.findViewById<View>(R.id.toggle).setOnClickListener(this)

        mTextureView = view.findViewById<View>(R.id.texture) as AutoFitTextureView
        mCameraButton = view.findViewById(R.id.picture)
        mFlashButton = view.findViewById(R.id.flash)
        mToggleButton = view.findViewById(R.id.toggle)
        mBoardImage = view.findViewById(R.id.board)
        mLtCircle = view.findViewById(R.id.lt)
        mRtCircle = view.findViewById(R.id.rt)
        mLbCircle = view.findViewById(R.id.lb)
        mRbCircle = view.findViewById(R.id.rb)

        // タッチイベントの登録
        mTextureView!!.setOnTouchListener(this)

        val boardPath = (activity as CameraActivity).boardPath
        Log.w(TAG, "boardPath = $boardPath")
        val filePath = File(boardPath).absolutePath
        var bitmap = BitmapFactory.decodeFile(filePath)
        mBoardImage?.setImageBitmap(bitmap)

        initBoardSize(bitmap)

        preferences = activity.getSharedPreferences("Camera", Context.MODE_PRIVATE)
//        preferences.edit().clear().commit() // Testコード

        mFlashModeIndex = preferences.getInt("flashMode", 0)
        var isVisible = preferences.getBoolean("boardMode", true)
        var priority = (activity as CameraActivity).blackboardViewPriority
        var isNeedBlackBoard = (activity as CameraActivity).isNeedBlackBoard
        // 黒板表示優先モードが「Web」で、Webから取得した「isNeedBlackBoard」がfalaeの場合、黒板は非表示とする
        if (priority == "Web") {
            if (!isNeedBlackBoard) {
                isVisible = false
            }
        }
        toggleBlackBoardMode(isVisible)

        changeFlashMode()
    }

    override fun onActivityCreated(savedInstanceState: Bundle?) {
        super.onActivityCreated(savedInstanceState)
        val fileName = "${UUID.randomUUID()}.jpeg"
//        val fullPath = "${activity.applicationContext.filesDir}/$fileName"
        val fullPath = "${activity.getExternalFilesDir(null)}/$fileName"
        Log.d(TAG, "[onActivityCreated] fullPath=${fullPath}")
//        mFile = File(activity.getExternalFilesDir(null), fileName)
        mFile = File(fullPath)
        Log.d(TAG, "[onActivityCreated] ${mFile.absolutePath}")
    }

    fun setOrientation(orientation: Int) {
        mOrientation = orientation
    }

    override fun onResume() {
        super.onResume()
        startBackgroundThread()

        Handler().postDelayed({
            renderBoardFromSaved()
            renderBoardMarker()
        }, 500)

        // When the screen is turned off and turned back on, the SurfaceTexture is already
        // available, and "onSurfaceTextureAvailable" will not be called. In that case, we can open
        // a camera and start preview from here (otherwise, we wait until the surface is ready in
        // the SurfaceTextureListener).
        if (mTextureView!!.isAvailable) {
            openCamera(mTextureView!!.width, mTextureView!!.height)
        } else {
            mTextureView!!.surfaceTextureListener = mSurfaceTextureListener
        }
    }

    override fun onPause() {
        closeCamera()
        stopBackgroundThread()
        super.onPause()
    }

//    private fun requestCameraPermission() {
//        if (this.shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
//            ConfirmationDialog().show(childFragmentManager, FRAGMENT_DIALOG)
//        } else {
//            requestPermissions(arrayOf(Manifest.permission.CAMERA), REQUEST_CAMERA_PERMISSION)
//        }
//    }

    private fun initBoardSize(bitmap: Bitmap) {
        val w1 = bitmap.width
        val h1 = bitmap.height
        val displaySize = Point()
        activity.windowManager.defaultDisplay.getSize(displaySize)
        val w2: Float = min(displaySize.x, displaySize.y) * BOARD_INIT_SCALE
        val h2 = h1.toFloat() / w1.toFloat() * w2

        mBoardImage!!.layoutParams.width = w2.toInt()
        mBoardImage!!.layoutParams.height = h2.toInt()
        mBoardImage!!.requestLayout()
    }

    /**
     * Sets up member variables related to camera.
     *
     * @param width  The width of available size for camera preview
     * @param height The height of available size for camera preview
     */
    private fun setUpCameraOutputs(width: Int, height: Int) {
        val activity = activity
        val manager = activity!!.getSystemService(CAMERA_SERVICE) as CameraManager
        try {
            for (cameraId in manager.cameraIdList) {
                val characteristics = manager.getCameraCharacteristics(cameraId)

                // We don't use a front facing camera in this sample.
                val facing = characteristics.get(CameraCharacteristics.LENS_FACING)
                if (facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT) {
                    continue
                }

                val map = characteristics.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP) ?: continue

                // For still image captures, we use the largest available size.
                val sizeList = listOf(*map.getOutputSizes(ImageFormat.JPEG)).sortedByDescending { it.width }
                val largest = sizeList[sizeList.indexOfFirst { it.width <= 1280 }]
                Log.d(TAG, "test:${largest.width} x ${largest.height}")
                mImageReader = ImageReader.newInstance(largest.width, largest.height,
                        ImageFormat.JPEG, 2)
                mImageReader!!.setOnImageAvailableListener(
                        mOnImageAvailableListener, mBackgroundHandler)

                // Find out if we need to swap dimension to get the preview size relative to sensor
                // coordinate.
                val displayRotation = activity.windowManager.defaultDisplay.rotation

                mSensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION)!!
                Log.i(TAG, "test::Call mSensorOrientation=$mSensorOrientation")
                var swappedDimensions = areDimensionsSwapped(displayRotation)

                val displaySize = Point()
                activity.windowManager.defaultDisplay.getSize(displaySize)
                var rotatedPreviewWidth = width
                var rotatedPreviewHeight = height
                var maxPreviewWidth = displaySize.x
                var maxPreviewHeight = displaySize.y

                if (swappedDimensions) {
                    rotatedPreviewWidth = height
                    rotatedPreviewHeight = width
                    maxPreviewWidth = displaySize.y
                    maxPreviewHeight = displaySize.x
                }

                if (maxPreviewWidth > MAX_PREVIEW_WIDTH) {
                    maxPreviewWidth = MAX_PREVIEW_WIDTH
                }

                if (maxPreviewHeight > MAX_PREVIEW_HEIGHT) {
                    maxPreviewHeight = MAX_PREVIEW_HEIGHT
                }

                // Danger, W.R.! Attempting to use too large a preview size could  exceed the camera
                // bus' bandwidth limitation, resulting in gorgeous previews but the storage of
                // garbage capture data.
                mPreviewSize = chooseOptimalSize(map.getOutputSizes(SurfaceTexture::class.java),
                        rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
                        maxPreviewHeight, largest)


                // We fit the aspect ratio of TextureView to the size of preview we picked.
                if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                    mTextureView!!.setAspectRatio(
                            mPreviewSize!!.width, mPreviewSize!!.height)
                } else {
                    mTextureView!!.setAspectRatio(
                            mPreviewSize!!.height, mPreviewSize!!.width)
                }

                // Check if the flash is supported.
                val available = characteristics.get(CameraCharacteristics.FLASH_INFO_AVAILABLE)
                mFlashSupported = available ?: false

                mCameraId = cameraId
                Log.d(TAG, "test:setUpCameraOutputs:cameraId=${mCameraId}")
                return
            }
        } catch (e: CameraAccessException) {
            e.printStackTrace()
        } catch (e: NullPointerException) {
            // Currently an NPE is thrown when the Camera2API is used but not supported on the
            e.printStackTrace()
            // device this code runs.
        }

    }

    private var mediaActionSound = MediaActionSound()

    /**
     * Determines if the dimensions are swapped given the phone's current rotation.
     *
     * @param displayRotation The current rotation of the display
     *
     * @return true if the dimensions are swapped, false otherwise.
     */
    private fun areDimensionsSwapped(displayRotation: Int): Boolean {
        var swappedDimensions = false
        when (displayRotation) {
            Surface.ROTATION_0, Surface.ROTATION_180 -> {
                if (mSensorOrientation == 90 || mSensorOrientation == 270) {
                    swappedDimensions = true
                }
            }
            Surface.ROTATION_90, Surface.ROTATION_270 -> {
                if (mSensorOrientation == 0 || mSensorOrientation == 180) {
                    swappedDimensions = true
                }
            }
            else -> {
                Log.e(TAG, "Display rotation is invalid: $displayRotation")
            }
        }
        return swappedDimensions
    }

    /**
     * Opens the camera specified by [Camera2Fragment.mCameraId].
     */
    @SuppressLint("MissingPermission")
    private fun openCamera(width: Int, height: Int) {
        // JS側で対応するため、ここではSkip
//        if (ContextCompat.checkSelfPermission(activity!!, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
//            requestCameraPermission()
//            return
//        }
        mediaActionSound = MediaActionSound()
        mediaActionSound.load(MediaActionSound.SHUTTER_CLICK)

        setUpCameraOutputs(width, height)
        Log.d(TAG, "test:openCamera:configureTransform:w=${width}, h=${height}")
        configureTransform(width, height)
        val activity = activity
        val manager = activity!!.getSystemService(CAMERA_SERVICE) as CameraManager
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw RuntimeException("Time out waiting to lock camera opening.")
            }
            manager.openCamera(mCameraId!!, mStateCallback, mBackgroundHandler)
        } catch (e: CameraAccessException) {
            e.printStackTrace()
        } catch (e: InterruptedException) {
            throw RuntimeException("Interrupted while trying to lock camera opening.", e)
        }

    }

    /**
     * Closes the current [CameraDevice].
     */
    private fun closeCamera() {
        try {
            mCameraOpenCloseLock.acquire()
            if (null != mCaptureSession) {
                mCaptureSession!!.close()
                mCaptureSession = null
            }
            if (null != mCameraDevice) {
                mCameraDevice!!.close()
                mCameraDevice = null
            }
            if (null != mImageReader) {
                mImageReader!!.close()
                mImageReader = null
            }
        } catch (e: InterruptedException) {
            throw RuntimeException("Interrupted while trying to lock camera closing.", e)
        } finally {
            mCameraOpenCloseLock.release()
        }
    }

    /**
     * Starts a background thread and its [Handler].
     */
    private fun startBackgroundThread() {
        Log.d(TAG, "test:startBackgroundThread")
        mBackgroundThread = HandlerThread("CameraBackground")
        mBackgroundThread!!.start()
        mBackgroundHandler = Handler(mBackgroundThread!!.looper)
    }

    /**
     * Stops the background thread and its [Handler].
     */
    private fun stopBackgroundThread() {
        Log.d(TAG, "test:stopBackgroundThread")
        mBackgroundThread!!.quitSafely()
        try {
            mBackgroundThread!!.join()
            mBackgroundThread = null
            mBackgroundHandler = null
        } catch (e: InterruptedException) {
            e.printStackTrace()
        }

    }

    /**
     * Creates a new [CameraCaptureSession] for camera preview.
     */
    private fun createCameraPreviewSession() {
        try {
            val texture = mTextureView!!.surfaceTexture!!

            // We configure the size of default buffer to be the size of camera preview we want.
            texture.setDefaultBufferSize(mPreviewSize!!.width, mPreviewSize!!.height)

            // This is the output Surface we need to start preview.
            val surface = Surface(texture)

            // We set up a CaptureRequest.Builder with the output Surface.
            mPreviewRequestBuilder = mCameraDevice!!.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW)
            mPreviewRequestBuilder!!.addTarget(surface)

            // Here, we create a CameraCaptureSession for camera preview.
            mCameraDevice!!.createCaptureSession(listOf(surface, mImageReader!!.surface),
                    object : CameraCaptureSession.StateCallback() {

                        override fun onConfigured(cameraCaptureSession: CameraCaptureSession) {
                            // The camera is already closed
                            if (null == mCameraDevice) {
                                return
                            }

                            // When the session is ready, we start displaying the preview.
                            mCaptureSession = cameraCaptureSession
                            try {
                                // Auto focus should be continuous for camera preview.
                                mPreviewRequestBuilder!!.set(CaptureRequest.CONTROL_AF_MODE,
                                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE)
                                // Flash is automatically enabled when necessary.
                                setAutoFlash(mPreviewRequestBuilder!!)

                                // Finally, we start displaying the camera preview.
                                mPreviewRequest = mPreviewRequestBuilder!!.build()
                                mCaptureSession!!.setRepeatingRequest(mPreviewRequest!!,
                                        mCaptureCallback, mBackgroundHandler)
                            } catch (e: CameraAccessException) {
                                e.printStackTrace()
                            }

                        }

                        override fun onConfigureFailed(
                                cameraCaptureSession: CameraCaptureSession) {
                            showToast("Failed")
                        }
                    }, null
            )
        } catch (e: CameraAccessException) {
            e.printStackTrace()
        }
    }

    /**
     * Configures the necessary [android.graphics.Matrix] transformation to `mTextureView`.
     * This method should be called after the camera preview size is determined in
     * setUpCameraOutputs and also the size of `mTextureView` is fixed.
     *
     * @param viewWidth  The width of `mTextureView`
     * @param viewHeight The height of `mTextureView`
     */
    private fun configureTransform(viewWidth: Int, viewHeight: Int) {
        activity ?: return
        val rotation = activity.windowManager.defaultDisplay.rotation
        val matrix = Matrix()
        val viewRect = RectF(0f, 0f, viewWidth.toFloat(), viewHeight.toFloat())
        val bufferRect = RectF(0f, 0f, mPreviewSize!!.height.toFloat(), mPreviewSize!!.width.toFloat())
        val centerX = viewRect.centerX()
        val centerY = viewRect.centerY()
        if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
            bufferRect.offset(centerX - bufferRect.centerX(), centerY - bufferRect.centerY())
            matrix.setRectToRect(viewRect, bufferRect, Matrix.ScaleToFit.FILL)
            val scale = max(
                    viewHeight.toFloat() / mPreviewSize!!.height,
                    viewWidth.toFloat() / mPreviewSize!!.width)
            matrix.postScale(scale, scale, centerX, centerY)
            matrix.postRotate((90 * (rotation - 2)).toFloat(), centerX, centerY)
        } else if (Surface.ROTATION_180 == rotation) {
            matrix.postRotate(180f, centerX, centerY)
        }
        Log.d(TAG, "test:configureTransform:${matrix}, viewWidth:${viewWidth}, viewHeight:${viewHeight}")
        mTextureView!!.setTransform(matrix)
        val rect = mTextureView!!.toRect()
        screenOffset = Point(rect.left, rect.top)
        Log.d(TAG, "Test:mTextureView:${rect}")
    }

    /**
     * Initiate a still image capture.
     */
    fun takePicture() {

        lockFocus()

    }

    /**
     * Lock the focus as the first step for a still image capture.
     */
    private fun lockFocus() {
        try {
            Log.d(TAG, "test:lockFocus")
            // This is how to tell the camera to lock focus.
            mPreviewRequestBuilder!!.set(CaptureRequest.CONTROL_AF_TRIGGER,
                    CameraMetadata.CONTROL_AF_TRIGGER_START)
            // Tell #mCaptureCallback to wait for the lock.
            mState = STATE_WAITING_LOCK
            mCaptureSession!!.capture(mPreviewRequestBuilder!!.build(), mCaptureCallback,
                    mBackgroundHandler)
        } catch (e: CameraAccessException) {
            e.printStackTrace()
        }

    }

    /**
     * Run the precapture sequence for capturing a still image. This method should be called when
     * we get a response in [.mCaptureCallback] from [.lockFocus].
     */
    private fun runPrecaptureSequence() {
        try {
            Log.d(TAG, "test:runPrecaptureSequence")
            // This is how to tell the camera to trigger.
            mPreviewRequestBuilder!!.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                    CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START)
            // Tell #mCaptureCallback to wait for the precapture sequence to be set.
            mState = STATE_WAITING_PRECAPTURE
            mCaptureSession!!.capture(mPreviewRequestBuilder!!.build(), mCaptureCallback,
                    mBackgroundHandler)
        } catch (e: CameraAccessException) {
            e.printStackTrace()
        }

    }

    /**
     * Capture a still picture. This method should be called when we get a response in
     * [.mCaptureCallback] from both [.lockFocus].
     */
    private fun captureStillPicture() {
        try {
            Log.d(TAG, "test:captureStillPicture")
            val activity = activity
            if (null == activity || null == mCameraDevice) {
                return
            }
            if (mCameraStartFlag) {
                return
            }
            mCameraStartFlag = true
            val rotation = activity.windowManager.defaultDisplay.rotation
            // This is the CaptureRequest.Builder that we use to take a picture.
            val captureBuilder = mCameraDevice!!.createCaptureRequest(
                    CameraDevice.TEMPLATE_STILL_CAPTURE)?.apply {
                addTarget(mImageReader!!.surface)

                // Sensor orientation is 90 for most devices, or 270 for some devices (eg. Nexus 5X)
                // We have to take that into account and rotate JPEG properly.
                // For devices with orientation of 90, we return our mapping from ORIENTATIONS.
                // For devices with orientation of 270, we need to rotate the JPEG 180 degrees.
                val fixedRotation = (ORIENTATIONS.get(rotation) + mSensorOrientation + 270) % 360
                val jpegRotation = getJpegOrientation(rotation)
                Log.i(TAG, "test:captureStillPicture.mSensorOrientation=${mSensorOrientation}, rotation:${rotation}, fixedRotation:${fixedRotation}, jpegRotation=${jpegRotation}")
                set(CaptureRequest.JPEG_ORIENTATION, fixedRotation)

                // Use the same AE and AF modes as the preview.
                set(CaptureRequest.CONTROL_AF_MODE,
                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE)
            }?.also { setAutoFlash(it) }

            // Use the same AE and AF modes as the preview.
//            captureBuilder!!.set(CaptureRequest.CONTROL_AE_MODE, CameraMetadata.CONTROL_AE_MODE_ON)
//            captureBuilder!!.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE)

            val CaptureCallback = object : CameraCaptureSession.CaptureCallback() {

                override fun onCaptureCompleted(session: CameraCaptureSession,
                                                request: CaptureRequest,
                                                result: TotalCaptureResult) {

                    unlockFocus()

                }
            }

            // シャッター音を鳴らす.
            mediaActionSound.play(MediaActionSound.SHUTTER_CLICK)

            mCaptureSession?.apply {
                stopRepeating()
                abortCaptures()
                capture(captureBuilder?.build(), CaptureCallback, null)
            }
        } catch (e: CameraAccessException) {
            Log.e(TAG, e.toString())
        }

    }


    private fun getJpegOrientation(deviceOrientation: Int): Int {
        var cameraManager = activity.getSystemService(CAMERA_SERVICE) as CameraManager

        val characteristics = cameraManager.getCameraCharacteristics(mCameraId)
        val sensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION)

        // Round device orientation to a multiple of 90
        val newDeviceOrientation = (deviceOrientation + 45) / 90 * 90

        return (sensorOrientation + newDeviceOrientation + 360) % 360

    }

    /**
     * Unlock the focus. This method should be called when still image capture sequence is
     * finished.
     */
    private fun unlockFocus() {
        try {
            // Reset the auto-focus trigger
            mPreviewRequestBuilder!!.set(CaptureRequest.CONTROL_AF_TRIGGER,
                    CameraMetadata.CONTROL_AF_TRIGGER_CANCEL)
            setAutoFlash(mPreviewRequestBuilder!!)
            mCaptureSession!!.capture(mPreviewRequestBuilder!!.build(), mCaptureCallback,
                    mBackgroundHandler)
            // After this, the camera will go back to the normal state of preview.
            mState = STATE_PREVIEW
            mCaptureSession!!.setRepeatingRequest(mPreviewRequest!!, mCaptureCallback,
                    mBackgroundHandler)
        } catch (e: CameraAccessException) {
            e.printStackTrace()
        } catch (e: java.lang.NullPointerException) {
            e.printStackTrace()
        }

    }

    /**
     * 黒板描画関連 <start---------------------------------------
     */
    private lateinit var currentPoint: PointF
    private lateinit var currentBoardRect: Rect

    private fun renderBoardMarker() {
        val board = mBoardImage ?: return

        mLtCircle!!.renderCenter(board.left, board.top)
        mLbCircle!!.renderCenter(board.left, board.bottom)
        mRtCircle!!.renderCenter(board.right, board.top)
        mRbCircle!!.renderCenter(board.right, board.bottom)
    }

    private fun moveBoard(id: Int, dx: Float, dy: Float): Boolean {
        val board = mBoardImage ?: return false
        var rect = board.toRect()
//        Log.d(TAG, "moveBoard:(dx:$dx, dy:$dy, rect:$rect)")
        when (id) {
            R.id.lt -> {
                rect.offset(dx.toInt(), dy.toInt())// left-top移動、(right・bottomは不変)
                val size = checkSize(currentBoardRect.right - rect.left)
                val width = size.width
                val height = size.height
                rect = Rect(currentBoardRect.right - width, currentBoardRect.bottom - height, currentBoardRect.right, currentBoardRect.bottom)
//                Log.d(TAG, "moveBoard:lt:offset:$rect")
            }
            R.id.rt -> {
                rect.offset(dx.toInt(), dy.toInt())// right-top移動、(left・bottomは不変)
                val size = checkSize(rect.right - currentBoardRect.left)
                val width = size.width
                val height = size.height
                rect = Rect(currentBoardRect.left, currentBoardRect.bottom - height, currentBoardRect.left + width, currentBoardRect.bottom)
//                Log.d(TAG, "moveBoard:rt:offset:$rect")
            }
            R.id.lb -> {
                rect.offset(dx.toInt(), dy.toInt())// left-bottom移動、(right・topは不変)
                val size = checkSize(currentBoardRect.right - rect.left)
                val width = size.width
                val height = size.height
                rect = Rect(currentBoardRect.right - width, currentBoardRect.top, currentBoardRect.right, currentBoardRect.top + height)
//                Log.d(TAG, "moveBoard:lb:offset:$rect")
            }
            R.id.rb -> {
                rect.offset(dx.toInt(), dy.toInt())// right-bottom移動、(left・topは不変)
                val size = checkSize(rect.right - currentBoardRect.left)
                val width = size.width
                val height = size.height
                rect = Rect(currentBoardRect.left, currentBoardRect.top, currentBoardRect.left + width, currentBoardRect.top + height)
//                Log.d(TAG, "moveBoard:rb:offset:$rect")
            }
            R.id.board -> {
//                Log.d(TAG, "moveBoard:inside:offset:$rect")
                rect.offset(dx.toInt(), dy.toInt())
            }
            else -> {
//                Log.d(TAG, "moveBoard:Ignore:$rect")
                return false
            }
        }
        correctBoardRect(rect)
//        Log.d(TAG, "moveBoard:after:offset:$rect")
        board.layout(rect.left, rect.top, rect.right, rect.bottom)
        return true

    }

    private fun correctBoardRect(offset: Rect) {
        val preview = mTextureView!!.toRect()
        if (offset.left < preview.left) {
            offset.offsetTo(preview.left, offset.top)
        }
        if (offset.right > preview.right) {
            offset.offsetTo(preview.right - offset.width(), offset.top)
        }
        if (offset.top < preview.top) {
            offset.offsetTo(offset.left, preview.top)
        }
        if (offset.bottom > preview.bottom) {
            offset.offsetTo(offset.left, preview.bottom - offset.height())
        }
    }

    private fun checkSize(widthSize: Int): Size {
        val wScale = currentBoardRect.height().toFloat() / currentBoardRect.width().toFloat()
        val hScale = currentBoardRect.width().toFloat() / currentBoardRect.height().toFloat()
        val minSize = min(mTextureView!!.width, mTextureView!!.height)
        // 最小のサイズチェック
        var width = max(widthSize, (minSize.toFloat() * BOARD_MIN_SCALE).toInt())
        var height = (wScale * widthSize.toFloat()).toInt()
        // 最大サイズチェック
        if (mTextureView!!.width > mTextureView!!.height) {
            // Landscape : 黒板の縦幅は画面の縦幅の9割までとする
            height = min(height, (minSize * BOARD_MAX_SCALE).toInt())
            width = (hScale * height.toFloat()).toInt()
        } else {
            width = min(width, (minSize * BOARD_MAX_SCALE).toInt())
        }
        return Size(width, height)

    }

    private fun findTargetView(x:Int, y:Int): Int {
        if (mLtCircle!!.toRect(50).contains(x, y)) {
            return mLtCircle!!.id
        }
        if (mLbCircle!!.toRect(50).contains(x, y)) {
            return mLbCircle!!.id
        }
        if (mRtCircle!!.toRect(50).contains(x, y)) {
            return mRtCircle!!.id
        }
        if (mRbCircle!!.toRect(50).contains(x, y)) {
            return mRbCircle!!.id
        }
        if (mBoardImage!!.toRect().contains(x, y)) {
            return mBoardImage!!.id
        }
        return 0

    }

    override fun onTouch(v: View?, event: MotionEvent?): Boolean {
        val ev = event ?: return false
        val board = mBoardImage ?: return false
        val x = ev.x + v!!.left
        val y = ev.y + v!!.top
        val id = findTargetView(x.toInt(), y.toInt())
        when (ev.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                Log.d(TAG, "ACTION_DOWN:$x, $y")
                this.currentPoint = PointF(x, y)
                this.currentBoardRect = board.toRect()
            }
            MotionEvent.ACTION_MOVE -> {
                val flag = moveBoard(id, x - currentPoint.x, y - currentPoint.y)
                if (flag) {
//                Log.d(TAG, "ACTION_MOVE:OK:x:${x}, y:${y}, dx=${x - currentPoint.x}, dy=${y - currentPoint.y}, board.rect=${board.toRect()}")
                    renderBoardMarker()
                } else {
//                    Log.d(TAG, "ACTION_MOVE:NG:x:${x}, y:${y}, dx=${x - currentPoint.x}, dy=${y - currentPoint.y}, board.rect=${board.toRect()}")
                }
                currentPoint = PointF(x, y)
            }
            MotionEvent.ACTION_UP -> {
                Log.d(TAG, "ACTION_UP:${x}, ${y}, board.left=${board.left}, top=${board.top}")
                if (id > 0) {
                    val edit = preferences.edit()
                    edit.putInt("board.left", board.left)
                    edit.putInt("board.top", board.top)
                    edit.putInt("board.right", board.right)
                    edit.putInt("board.bottom", board.bottom)
                    edit.apply()
                }
            }
        }
        return true
    }

    private fun renderBoardFromSaved() {
        val board = mBoardImage ?: return
        val left = preferences.getInt("board.left", board.left)
        val top  = preferences.getInt("board.top", board.top)
        val right = preferences.getInt("board.right", board.right)
        val bottom = preferences.getInt("board.bottom", board.bottom)
        val rect = Rect(left, top, right, bottom)
        correctBoardRect(rect)
        board.layout(rect)
    }

    /**
     * 黒板描画関連 ---------------------------------------end>
     */

    override fun onClick(view: View) {
        when (view.id) {
            R.id.picture -> {
                takePicture()
            }
            R.id.done -> {
                closeCamera()
                val intent = Intent()
                intent.putExtra("mode", (activity as CameraActivity).blackboardViewPriority)
                activity.setResult(0, intent)
                activity.finish()
            }
            R.id.flash -> {
                mFlashModeIndex++
                if (mFlashModeIndex > 2) mFlashModeIndex = 0
                preferences.edit().putInt("flashMode", mFlashModeIndex).apply()
                changeFlashMode()
                setAutoFlash(mPreviewRequestBuilder!!)
            }
            R.id.edit -> {
                closeCamera()
                val intent = Intent()
                intent.putExtra("mode", (activity as CameraActivity).blackboardViewPriority)
                activity.setResult(2, intent)
                activity.finish()
            }
            R.id.toggle -> {
                var flag = mBoardImage?.visibility == View.VISIBLE
                preferences.edit().putBoolean("boardMode", !flag).apply()
                toggleBlackBoardMode(!flag)
                (activity as CameraActivity).blackboardViewPriority = "App"
            }
        }
    }

    private fun toggleBlackBoardMode(flag: Boolean) {
        val views = arrayListOf(mBoardImage, mLbCircle, mLtCircle, mRbCircle, mRtCircle)
        views.map {
            // MEMO : 再表示時に一瞬移動する動作が見えるため、一旦、INVISIBLEにした後、座標を差指定した後にVisibleにする
            it!!.visibility = if (flag) View.INVISIBLE else View.GONE
        }
        this.mToggleButton!!.text = if (flag)  "黒板OFF" else "黒板ON"
        if (flag) {
            Handler().postDelayed({
                renderBoardFromSaved()
                renderBoardMarker()
                views.map {
                    it!!.visibility = View.VISIBLE
                }
            }, 1)
        }
    }

    private fun changeFlashMode() {
        when (mFlashModeIndex) {
            0 -> {
                mFlashButton!!.text = "自動"
                mFlashButton!!.setCompoundDrawablesWithIntrinsicBounds(R.drawable.icn_flash_auto, 0, 0, 0)
            }
            1 -> {
                mFlashButton!!.text = "オフ"
                mFlashButton!!.setCompoundDrawablesWithIntrinsicBounds(R.drawable.icn_flash_off, 0, 0, 0)
            }
            2 -> {
                mFlashButton!!.text = "オン"
                mFlashButton!!.setCompoundDrawablesWithIntrinsicBounds(R.drawable.icn_flash_on, 0, 0, 0)
            }
        }
    }

    private fun setAutoFlash(requestBuilder: CaptureRequest.Builder) {
        if (mFlashSupported) {
            Log.d(TAG, "setAutoFlash：mFlashModeIndex=${mFlashModeIndex}")
            when (mFlashModeIndex) {
                0 -> {
                    // AUTO
                    requestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO)
                    requestBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER, CameraMetadata.CONTROL_AE_PRECAPTURE_TRIGGER_START)
                    requestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CameraMetadata.CONTROL_AE_MODE_ON_AUTO_FLASH)
                    requestBuilder.set(CaptureRequest.FLASH_MODE, CameraMetadata.FLASH_MODE_OFF)
                    Log.d(TAG, "testLog:AUTO")
                }
                1 -> {
                    // OFF
                    requestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CameraMetadata.CONTROL_AE_MODE_ON)
                    requestBuilder.set(CaptureRequest.FLASH_MODE, CameraMetadata.FLASH_MODE_OFF)
                    Log.d(TAG, "testLog:OFF")
                }
                2 -> {
                    // ON
                    requestBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO)
                    requestBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER, CameraMetadata.CONTROL_AE_PRECAPTURE_TRIGGER_START)
                    requestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CameraMetadata.CONTROL_AE_MODE_ON_ALWAYS_FLASH)
                    requestBuilder.set(CaptureRequest.FLASH_MODE, CameraMetadata.FLASH_MODE_SINGLE)
                    Log.d(TAG, "testLog:ON")
                }
            }
            // 画面がリロードされるため、黒板を再描画する
            Handler().postDelayed({
                activity.runOnUiThread{
                    renderBoardFromSaved()
                    renderBoardMarker()
                }
            }, 1)

        } else {
            Log.w(TAG, "カメラのフラッシュがサポートされてない！！！")
        }
    }


    /**
     * Saves a JPEG [Image] into the specified [File].
     */
    private class ImageSaver internal constructor(
            /**
             * The JPEG image
             */
            private val image: Image,

            private val file: File,

            private val board: BitmapDrawable?,

            private val boardRect: Rect,

            private val minSize: Float,

            private val activity: CameraActivity?

    ) : Runnable {

        override fun run() {
            Log.d(TAG, "test:ImageSaver")
            var buffer = image.planes[0].buffer
            var bytes: ByteArray? = ByteArray(buffer.remaining())
            buffer.get(bytes)
            var output: FileOutputStream? = null
            try {

                if (bytes != null) {

                    output = FileOutputStream(file)

                    val exif = ExifInterface(ByteArrayInputStream(bytes))
                    val exifOrientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL)
                    var rotation: Int = activity!!.windowManager.defaultDisplay.rotation
                    val options = BitmapFactory.Options().apply {
                        this.inMutable = true
                    }
                    var original: Bitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes!!.size, options)
                    var m = Matrix()
                    var orientation = 0
                    if (exifOrientation != ExifInterface.ORIENTATION_UNDEFINED) {
                        Log.d(TAG, "degree:${ORIENTATIONS.get(rotation)}")
                        //画像回転
                        when (exifOrientation) {
                            ExifInterface.ORIENTATION_ROTATE_90 -> {
                                orientation = 90
                            }
                            ExifInterface.ORIENTATION_ROTATE_180 -> {
                                orientation = 180
                            }
                        }

                        m.setRotate((orientation).toFloat())
                    } else {
//                        output.write(bytes)
                        Log.d(TAG, "ExifInterface.ORIENTATION_UNDEFINED")
                    }
                    val rect = cropRectByAspect(original.width, original.height)
                    var rotated: Bitmap = Bitmap.createBitmap(original, rect.left, rect.top, rect.width(), rect.height(), m, true)
                    if (board != null) {
                        boardRect.convert(minSize / min(rect.width(), rect.height()))
                        rotated.append(board.bitmap, boardRect)
                    }
                    // 100万画素のサイズに変更
                    var sWidth: Int
                    var sHeight: Int
                    if (rotated.width > rotated.height) {
                        sWidth = 1156
                        sHeight = 867
                    } else {
                        sWidth = 867
                        sHeight = 1156
                    }
                    val scaled: Bitmap = Bitmap.createScaledBitmap(rotated, sWidth, sHeight, true)
                    scaled.compress(Bitmap.CompressFormat.JPEG, 100, output)

                    Log.i(TAG, "exifOrientation=$exifOrientation, rotation=$rotation")
//                    activity.runOnUiThread {
//                        Toast.makeText(activity, "exifOrientation=$exifOrientation, rotation=$rotation", Toast.LENGTH_LONG).show()
//                    }

                    addImageToGallery(file.absolutePath)
                    val intent = Intent()
                    intent.putExtra("filePath", file.absolutePath)
                    intent.putExtra("mode", activity.blackboardViewPriority)
                    activity.setResult(1, intent)
                    activity.finish()
                }
            } catch (e: IOException) {
                e.printStackTrace()
            } catch (e: Exception) {
                e.printStackTrace()
            } finally {
                image.close()
                if (null != output) {
                    try {
                        output.close()
                    } catch (e: IOException) {
                        e.printStackTrace()
                    }

                }
            }
        }

        private fun addImageToGallery(filePath: String) {
            try {
                val values = ContentValues().apply {
                    put(MediaStore.Images.Media.DATE_TAKEN, System.currentTimeMillis())
                    put(MediaStore.Images.Media.MIME_TYPE, "image/jpeg")
                    put(MediaStore.Images.Media.DATA, filePath)
                }
                activity!!.contentResolver.insert(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values)
                Log.i(TAG, "addImageToGallery:success:$filePath")
            } catch (e: Exception) {
                Log.e(TAG, e.localizedMessage)
            }
        }

        // 4:3に調整したRectを求める
        private fun cropRectByAspect(width: Int, height: Int): Rect {
            val minSize = min(width, height)
            val maxSize = max(width, height)
            val aspectSize = (minSize * (4.0F/3.0F)).toInt()
            val distance = (maxSize - aspectSize) / 2
            var rect: Rect
            rect = if (width > height) {
                Rect(distance, 0, maxSize - distance, minSize)
            } else {
                Rect(0, distance, minSize, maxSize - distance)
            }
            Log.d(TAG, "[cropRectByAspect]:($width, $height), Rect($rect) rect.width=${rect.width()}, rect.height=${rect.height()}")
            return rect
        }

        private fun Bitmap.append(bitmap: Bitmap, rect: Rect) {
            try {
                val canvas = Canvas(this)
                val src = Rect(0, 0, bitmap.width, bitmap.height)
                canvas.drawBitmap(bitmap, src, rect, Paint())
            } catch (e: Exception) {
                Log.w(TAG, e.localizedMessage);
            }
        }
        private fun Rect.convert(per: Float) {
            this.left = (this.left.toFloat() / per).toInt()
            this.top = (this.top.toFloat() / per).toInt()
            this.right = (this.right.toFloat() / per).toInt()
            this.bottom = (this.bottom.toFloat() / per).toInt()
        }
    }

    /**
     * Compares two `Size`s based on their areas.
     */
    internal class CompareSizesByArea : Comparator<Size> {

        override fun compare(lhs: Size, rhs: Size): Int {
            // We cast here to ensure the multiplications won't overflow
            return java.lang.Long.signum(lhs.width.toLong() * lhs.height - rhs.width.toLong() * rhs.height)
        }

    }


    companion object {

        /**
         * Conversion from screen rotation to JPEG orientation.
         */
        private val ORIENTATIONS = SparseIntArray()

        init {
            ORIENTATIONS.append(Surface.ROTATION_0, 90)
            ORIENTATIONS.append(Surface.ROTATION_90, 0)
            ORIENTATIONS.append(Surface.ROTATION_180, 270)
            ORIENTATIONS.append(Surface.ROTATION_270, 180)
        }

        /**
         * Tag for the [Log].
         */
        private const val TAG = "Camera2Fragment"

        /**
         * Camera state: Showing camera preview.
         */
        private const val STATE_PREVIEW = 0

        /**
         * Camera state: Waiting for the focus to be locked.
         */
        private const val STATE_WAITING_LOCK = 1

        /**
         * Camera state: Waiting for the exposure to be precapture state.
         */
        private const val STATE_WAITING_PRECAPTURE = 2

        /**
         * Camera state: Waiting for the exposure state to be something other than precapture.
         */
        private const val STATE_WAITING_NON_PRECAPTURE = 3

        /**
         * Camera state: Picture was taken.
         */
        private const val STATE_PICTURE_TAKEN = 4

        /**
         * Max preview width that is guaranteed by Camera2 API
         */
        private const val MAX_PREVIEW_WIDTH = 1920

        /**
         * Max preview height that is guaranteed by Camera2 API
         */
        private const val MAX_PREVIEW_HEIGHT = 1080

        private const val BOARD_MIN_SCALE = 0.4F
        private const val BOARD_INIT_SCALE = 0.6F
        private const val BOARD_MAX_SCALE = 0.9F

        private var mOrientation: Int = 0

        /**
         * Given `choices` of `Size`s supported by a camera, choose the smallest one that
         * is at least as large as the respective texture view size, and that is at most as large as the
         * respective max size, and whose aspect ratio matches with the specified value. If such size
         * doesn't exist, choose the largest one that is at most as large as the respective max size,
         * and whose aspect ratio matches with the specified value.
         *
         * @param choices           The list of sizes that the camera supports for the intended output
         * class
         * @param textureViewWidth  The width of the texture view relative to sensor coordinate
         * @param textureViewHeight The height of the texture view relative to sensor coordinate
         * @param maxWidth          The maximum width that can be chosen
         * @param maxHeight         The maximum height that can be chosen
         * @param aspectRatio       The aspect ratio
         * @return The optimal `Size`, or an arbitrary one if none were big enough
         */
        private fun chooseOptimalSize(choices: Array<Size>, textureViewWidth: Int,
                                      textureViewHeight: Int, maxWidth: Int, maxHeight: Int, aspectRatio: Size): Size {

            // Collect the supported resolutions that are at least as big as the preview Surface
            val bigEnough = ArrayList<Size>()
            // Collect the supported resolutions that are smaller than the preview Surface
            val notBigEnough = ArrayList<Size>()
            val w = aspectRatio.width
            val h = aspectRatio.height
            for (option in choices) {
                if (option.width <= maxWidth && option.height <= maxHeight &&
                        option.height == option.width * h / w) {
                    if (option.width >= textureViewWidth && option.height >= textureViewHeight) {
                        bigEnough.add(option)
                    } else {
                        notBigEnough.add(option)
                    }
                }
            }

            // Pick the smallest of those big enough. If there is no one big enough, pick the
            // largest of those not big enough.
            if (bigEnough.size > 0) {
                return Collections.min(bigEnough, CompareSizesByArea())
            } else if (notBigEnough.size > 0) {
                return Collections.max(notBigEnough, CompareSizesByArea())
            } else {
                Log.e(TAG, "Couldn't find any suitable preview size")
                return choices[0]
            }
        }

        fun newInstance(): Camera2Fragment {
            return Camera2Fragment()
        }
    }

    /**
     * Extensions Method
     */
    private fun ImageView.renderCenter(x: Int, y: Int) {
        val width = this.width
        val height = this.height
        val left = x - (width / 2)
        val top = y - (height / 2)
        this.layout(left, top, (left + width), (top + height))
    }

    private fun View.layout(rect: Rect) {
        layout(rect.left, rect.top, rect.right, rect.bottom)
    }

    private fun View.toRect(): Rect {
        return Rect(this.left, this.top, this.right, this.bottom)
    }

    // Rectサイズを指定area分、拡大する
    private fun View.toRect(per: Int): Rect {
        val area = (right - left) * per / 2 / 100
        return Rect(this.left - area, this.top - area, this.right + area, this.bottom + area)
    }

    private fun View.toRectF(): RectF {
        return RectF(this.left.toFloat(), this.top.toFloat(), this.right.toFloat(), this.bottom.toFloat())
    }

    private fun View.center(): Point {
        val rect = toRect()
        return Point(rect.centerX(), rect.centerY())
    }

    private fun View.isVisible(): Boolean {
        return this.visibility == View.VISIBLE
    }

    private fun Rect.min(): Int {
        return min(width(), height())
    }

    private fun Size.min(): Int {
        return min(width, height)
    }

    private fun View.setCenter(point: Point) {
        val dx = point.x - center().x
        val dy = point.y - center().y
        var rect = toRect()
        rect.offset(dx, dy)
        layout(rect)
    }



}