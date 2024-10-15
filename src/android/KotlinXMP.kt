package jp.co.taisei.construction.fieldmanagement.plugin

import java.io.File
import java.io.IOException
import java.nio.charset.StandardCharsets

class KotlinXMP {

    @Throws(IOException::class)
    fun embedXmp(contents: File, xml: String) {
        try {
            val data = contents.readBytes()
            val indexes = findXmp(data)
            val newData = writeXmp(data, indexes.head, indexes.end, indexes.exifHead, xml)
            contents.outputStream().use { it.write(newData) }
        } catch (e: IOException) {
            println("error: ${e.localizedMessage}")
        }
    }

    fun embedXmp(path: String, xml: String) {
        try {
            val inputStream = File(path).inputStream()
            val data = inputStream.readBytes()
            val indexes = findXmp(data)
            val newData = writeXmp(data, indexes.head, indexes.end, indexes.exifHead, xml)
            val outputStream = File(path).outputStream()
            outputStream.use { it.write(newData) }
        } catch (e: IOException) {
            println("error: ${e.localizedMessage}")
        }
    }

    fun embedXmp(contents: ByteArray, xml: String): ByteArray {
        val bytes = convertDataToBytes(contents)
        val indexes = findXmp(bytes)
        return writeXmp(bytes, indexes.head, indexes.end, indexes.exifHead, xml)
    }

    private fun findXmp(bytes: ByteArray): Indexes {
        //XMPの始まりのindex(0xFFE1のFFにあたる部分)
        var xmpHeadIndex: Int? = null
        //XMPの終わりのindex
        //XMPのセグメントの次のセグメント(0xFF~)の1つ前のindex
        var xmpEndIndex: Int? = null
        //Exifの始まりのindex
        var exifHeadIndex: Int? = null

        bytes.forEachIndexed { index, value ->
            if (value == 0xFF.toByte()) {
                //APP1 segment : 0xFFE1
                if (index + 1 < bytes.size && bytes[index + 1] == 0xE1.toByte()) {
                    val start = index + 4
                    val bytesStr = getBytesString(bytes, start, 28)
                    if (bytesStr == "http://ns.adobe.com/xap/1.0/") {
                        xmpHeadIndex = index
                        xmpEndIndex = index + convertBytesToInt(bytes, index + 2) + 2 // findEndXmpIndex(start, bytes)
                    } else {
                        exifHeadIndex = index
                    }
                }
            }
        }

        return Indexes(xmpHeadIndex, xmpEndIndex, exifHeadIndex)
    }

    // byteのarrayから2byte分をintに変換
    private fun convertBytesToInt(bytes: ByteArray, start: Int): Int {
        return ((bytes[start].toInt() and 0xFF) shl 8) + (bytes[start + 1].toInt() and 0xFF)
    }

    private fun writeXmp(
        bytes: ByteArray, xmpHeadIndex: Int?, xmpEndIndex: Int?, exifHeadIndex: Int?, xml: String
    ): ByteArray {
        val xmp = createXmp(xml)

        return if (xmpHeadIndex != null && xmpEndIndex != null) {
            val removedBytes = removeBytes(bytes, xmpHeadIndex, xmpEndIndex)
            return insertBytes(removedBytes, xmpHeadIndex, xmp)
        } else {
            val exifIndex = exifHeadIndex ?: bytes.size
            return insertBytes(bytes, exifIndex, xmp)
        }
    }

    private fun convertDataToBytes(data: ByteArray): ByteArray {
        return data
    }

    private fun createXmp(xml: String): ByteArray {
        val adobe = "http://ns.adobe.com/xap/1.0/"
        val header = "<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>"
        val footer = "<?xpacket end=\"w\"?>"

        var adobeBytes = convertStringToBytes(adobe)
        val headerBytes = convertStringToBytes(header)
        val footerBytes = convertStringToBytes(footer)
        var xmlBytes = convertStringToBytes(xml)

        adobeBytes += 0x00.toByte()
        xmlBytes = adobeBytes + headerBytes + xmlBytes + footerBytes

        val count = xmlBytes.size + 2
        val lengthBytes = byteArrayOf((count shr 8).toByte(), (count and 0xFF).toByte())

        return byteArrayOf(0xFF.toByte(), 0xE1.toByte()) + lengthBytes + xmlBytes
    }

    private fun convertStringToBytes(value: String): ByteArray {
        return value.toByteArray(StandardCharsets.UTF_8)
    }

    private fun removeBytes(bytes: ByteArray, start: Int, end: Int): ByteArray {
        val result = bytes.toMutableList()
        var counter = start
        while (counter < end) {
            result.removeAt(start)
            counter++
        }
        return result.toByteArray()
    }

    private fun insertBytes(bytes: ByteArray, start: Int, insertBytes: ByteArray): ByteArray {
        val result = bytes.toMutableList()
        insertBytes.forEachIndexed { index, byte ->
            result.add(start + index, byte)
        }
        return result.toByteArray()
    }

    private fun findEndXmpIndex(start: Int, bytes: ByteArray): Int {
        var offset = start + 29
        while (offset < bytes.size) {
            if (getBytesString(bytes, offset, 19) == "<?xpacket end=\"w\"?>") {
                return offset + 19
            }
            offset++
        }
        return bytes.size
    }

    private fun getBytesString(bytes: ByteArray, start: Int, length: Int): String {
        val selectedBytes = bytes.sliceArray(start until (start + length).coerceAtMost(bytes.size))
        return String(selectedBytes, StandardCharsets.UTF_8)
    }

    data class Indexes(val head: Int?, val end: Int?, val exifHead: Int?)
}