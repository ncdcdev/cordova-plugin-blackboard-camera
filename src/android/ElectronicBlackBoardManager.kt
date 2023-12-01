package jp.co.taisei.construction.fieldmanagement.plugin

import android.content.Context
import java.io.File
import java.util.*
import android.graphics.BitmapFactory
import android.graphics.Bitmap
import androidx.exifinterface.media.ExifInterfaceFix
import java.text.SimpleDateFormat

object ElectronicBlackBoardManager {

    fun createImageEmbeddedMetaData(path: String, photoInfo: PhotoInfo?, imageDescription: String, model: String, software: String) {

        val formatter = SimpleDateFormat("yyyy:MM:dd HH:mm:ss", Locale.US)
        val now = Date()
        val millisec = (now.time % 1000).toInt()
        val formattedDate = formatter.format(now)
        val exif = ExifInterfaceFix(path)
        exif.setAttribute(ExifInterfaceFix.TAG_IMAGE_DESCRIPTION, imageDescription)
        exif.setAttribute(ExifInterfaceFix.TAG_MODEL, model)
        exif.setAttribute(ExifInterfaceFix.TAG_SOFTWARE, software)
        exif.setAttribute(ExifInterfaceFix.TAG_DATETIME_ORIGINAL, formattedDate)
        exif.setAttribute(ExifInterfaceFix.TAG_SUBSEC_TIME_ORIGINAL, millisec.toString())
        exif.setAttribute(ExifInterfaceFix.TAG_FLASH, "1")
        if (photoInfo != null) {
            exif.setAttribute(ExifInterfaceFix.TAG_XMP, photoInfo.toXMP())
        }
        exif.saveAttributes()
    }
}