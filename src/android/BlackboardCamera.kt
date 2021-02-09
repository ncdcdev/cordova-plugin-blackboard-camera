package jp.co.taisei.construction.fieldmanagement.plugin

import android.content.Intent
import android.util.Base64
import android.util.Log
import android.widget.Toast
import org.apache.cordova.CallbackContext
import org.apache.cordova.CordovaPlugin
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject
import java.io.File
import jp.co.ncdc.apppot.stew.APService
import jp.co.ncdc.apppot.stew.APUserInfo
import jp.co.ncdc.apppot.stew.utils.JsonUtils

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
                // 黒板イメージ
                val base64: String = data[0] as String
                val filePath = "${cordova.activity.applicationContext.filesDir}/board.png"
                decoder(base64, filePath)
                if (File(filePath).exists()) {
                    intent.putExtra("boardPath", filePath)
                }

                // 黒板を表示/非表示フラグ
                val isNeedBlackBoard: Boolean = try {
                    (data[1] as Boolean)
                } catch (e: ClassCastException) {
                    (data[1] as Int) == 1
                }
                intent.putExtra("isNeedBlackBoard", isNeedBlackBoard)

                // 黒板表示する場合、WebとAppのどちらの設定値を優先するか(Web or App)
                val blackboardViewPriority: String = data[2]  as String
                intent.putExtra("blackboardViewPriority", blackboardViewPriority)

                val authInfo = data[3] as String
                initAppPotSdk(authInfo)
                val jsonString = data[4] as String
                intent.putExtra("pictureJson", jsonString)

                Log.d(TAG, "jsonString=${jsonString}")
                cordova.startActivityForResult(this, intent, 1)
            } else {
                handleError("Invalid action")
                Toast.makeText(cordova.context, "[Error] Invalid action", Toast.LENGTH_LONG).show()
                result = false
            }
        } catch (e: Exception) {
            handleException(e)
            Log.e(TAG, e.message)
            Toast.makeText(cordova.context, "[Error] ${e.message}", Toast.LENGTH_LONG).show()
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

    private fun initAppPotSdk(authInfo: String) {
        val service = APService.getInstance()
        service.setServiceInfo(
                cordova.activity.applicationContext,
                63,
                "taisei_kui_develop",
                "b40b0497304642008a9fe808a7bbd9a3",
                "1.0",
                "trial.apppot.net",
                "apppot",
                80,
                true,
                false
        )
        val authInfoJson = JsonUtils.convertToJSONObject(authInfo)
        val userInfoJson = authInfoJson["userInfo"] as JSONObject
        val userInfo = APUserInfo()
        userInfo.account = userInfoJson["account"] as String
        userInfo.firstName = try {
            userInfoJson["firstName"] as String
        } catch (e: JSONException) {
            ""
        }
        userInfo.lastName = try {
            userInfoJson["lastName"] as String
        } catch (e: JSONException) {
            ""
        }
        val userId = try {
            authInfoJson["userId"] as Long
        } catch (e: java.lang.ClassCastException) {
            (authInfoJson["userId"] as Int).toLong()
        }
        userInfo.userId = userId
        userInfo.userTokens = authInfoJson["userTokens"] as String
        service.userInfo = userInfo
        Log.d(TAG, "BlackboardCamera:initAppPotSdk=success")
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