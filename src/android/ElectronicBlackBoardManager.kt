package jp.co.taisei.construction.fieldmanagement.plugin

import androidx.exifinterface.media.ExifInterface
import java.util.*
import java.text.SimpleDateFormat

object ElectronicBlackBoardManager {

    fun createImageEmbeddedMetaData(path: String, photoInfo: PhotoInfo?, imageDescription: String, model: String, software: String) {

        // http://ns.adobe.com/xap/1.0/情報を先にセットする
        val xmpMeta = XmpUtil.extractOrCreateXMPMeta(path)
        XmpUtil.writeXMPMeta(path, xmpMeta)

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

        exif.saveAttributes()

        if (photoInfo != null) {
            KotlinXMP().embedXmp(path, photoInfo.toXMP())
        }
    }
}