package jp.co.taisei.construction.fieldmanagement.plugin

import android.content.Intent
import android.util.Base64
import android.util.Log
import org.apache.cordova.CallbackContext
import org.apache.cordova.CordovaPlugin
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject
import java.io.File

class BlackboardCamera : CordovaPlugin() {
    lateinit var context: CallbackContext

    @Throws(JSONException::class)
    override fun execute(action: String, data: JSONArray, callbackContext: CallbackContext): Boolean {
        Log.d(TAG, "BlackboardCamera:$action")
        context = callbackContext
        var result = true

        try {
            if (action == "capture") {

                var applicationContext = cordova.activity.applicationContext
                val intent = Intent(applicationContext, CameraActivity::class.java)
                val base64: String = data[0] as String
                Log.d("TAG", "data[1]=" + data[1])
                val isNeedBlackBoard: Boolean = try {
                    (data[1] as Boolean)
                } catch (e: ClassCastException) {
                    (data[1] as Int) == 1
                }
                val blackboardViewPriority: String = data[2]  as String
                var jcomsiaPhoto: String = data[3] as String
                val photoInfoJson = JSONObject(jcomsiaPhoto)
                val photoInfo = PhotoInfo(photoInfoJson)
                var version: String = data[4] as String
                val filePath = "${cordova.activity.applicationContext.filesDir}/board.png"
                decoder(base64, filePath)
                if (File(filePath).exists()) {
                    intent.putExtra("boardPath", filePath)
                    intent.putExtra("isNeedBlackBoard", isNeedBlackBoard)
                    intent.putExtra("blackboardViewPriority", blackboardViewPriority)
                    intent.putExtra("photoInfo", photoInfo)
                    intent.putExtra("version", version)
                }
                cordova.startActivityForResult(this, intent, 1)
            } else {
                handleError("Invalid action")
                result = false
            }
        } catch (e: Exception) {
            handleException(e)
            result = false
        }

        return result
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, intent: Intent?) {
        Log.i(TAG, "onActivityResult:requestCode=$requestCode, resultCode=$resultCode")
        if (requestCode == 1) {
            var result = JSONObject()
            var mode = intent!!.getStringExtra("mode")
            result.put("mode", mode)
            if (resultCode == 1) {
                var filePath = intent!!.getStringExtra("filePath")
                result.put("data", filePath)
                context.success(result)
            } else if (resultCode == 0) {
                // Close
                result.put("data", "Close")
                context.success(result)
            } else if (resultCode == 2) {
                // 黒板編集モード
                result.put("data", "Edit")
                context.success(result)
            }

        }
        return
    }

    private fun decoder(base64Str: String, filePath: String) {
        val imageByteArray = Base64.decode(base64Str.substring(base64Str.indexOf(",") + 1), Base64.DEFAULT)
        return File(filePath).writeBytes(imageByteArray)
    }

    /**
     * Handles an error while executing a plugin API method.
     * Calls the registered Javascript plugin error handler callback.
     *
     * @param errorMsg Error message to pass to the JS error handler
     */
    private fun handleError(errorMsg: String) {
        try {
            Log.e(TAG, errorMsg)
            context.error(errorMsg)
        } catch (e: Exception) {
            Log.e(TAG, e.toString())
        }

    }

    private fun handleException(exception: Exception) {
        handleError(exception.toString())
    }

    companion object {

        private const val TAG = "BlackboardCamera"
    }

}