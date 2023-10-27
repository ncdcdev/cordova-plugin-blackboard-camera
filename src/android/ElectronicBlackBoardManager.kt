package jp.co.taisei.construction.fieldmanagement.plugin

import android.content.Context
import java.io.File
import java.util.*
import android.graphics.BitmapFactory
import android.graphics.Bitmap
import androidx.exifinterface.media.ExifInterface
import java.text.SimpleDateFormat

object ElectronicBlackBoardManager {

    fun createImageEmbeddedMetaData(path: String, photoInfo: PhotoInfo?, imageDescription: String, model: String, software: String) {

        val formatter = SimpleDateFormat("yyyy:MM:dd HH:mm:ss", Locale.US)
        val now = Date()
        val millisec = (now.time % 1000).toInt()
        val formattedDate = formatter.format(now)
        val exif = ExifInterface(path)
        exif.setAttribute(ExifInterface.TAG_IMAGE_DESCRIPTION, imageDescription)
        exif.setAttribute(ExifInterface.TAG_MODEL, model)
        exif.setAttribute(ExifInterface.TAG_SOFTWARE, software)
        exif.setAttribute(ExifInterface.TAG_DATETIME_ORIGINAL, formattedDate)
        exif.setAttribute(ExifInterface.TAG_SUBSEC_TIME_ORIGINAL, millisec.toString())
        exif.setAttribute(ExifInterface.TAG_FLASH, "1")
        if (photoInfo != null) {
            exif.setAttribute(ExifInterface.TAG_XMP, photoInfo.toXMP())
        }
        exif.saveAttributes()
    }
}