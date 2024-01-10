package jp.co.taisei.construction.fieldmanagement.plugin

import java.util.*
import androidx.exifinterface.media.ExifInterfaceFix
import com.adobe.xmp.XMPConst
import java.text.SimpleDateFormat

object ElectronicBlackBoardManager {

    fun createImageEmbeddedMetaData(path: String, photoInfo: PhotoInfo?, imageDescription: String, model: String, software: String) {

        // http://ns.adobe.com/xap/1.0/情報を先にセットする
        val xmpMeta = XmpUtil.extractOrCreateXMPMeta(path)
        xmpMeta.setProperty(XMPConst.NS_XMP, "Label", "TPR2");
        XmpUtil.writeXMPMeta(path, xmpMeta)

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