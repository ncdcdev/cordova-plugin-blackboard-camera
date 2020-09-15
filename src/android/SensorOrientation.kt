package jp.co.taisei.construction.fieldmanagement.plugin

import java.io.Serializable

class SensorOrientation : Serializable {
    var orientation: Int = 0

    companion object {
        private val sensorOrientation = SensorOrientation()
        fun getInstance(): SensorOrientation {
            return sensorOrientation
        }
    }
}