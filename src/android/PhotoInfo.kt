package jp.co.taisei.construction.fieldmanagement.plugin

import android.os.Parcelable
import kotlinx.android.parcel.Parcelize
import org.json.JSONObject
import java.util.*

// PhotoInfo Class

@Parcelize
class PhotoInfo(private val constructionName: String, private val contractor: String, private val largeClassification: String, private val photoClassification: String, private val constructionType: String, private val middleClassification: String, private val smallClassification: String, private val title: String, private val classificationRemarks: ArrayList<String>, private val shootingSpot: String, private val isRepresentative: Boolean, private val isFrequencyOfSubmission: Boolean, private val measurements: Measurement, private val contractorRemarks: String) : Parcelable {

    constructor(json: JSONObject): this(
        json.optString("constructionName", ""),
        json.optString("contractor", ""),
        json.optString("largeClassification", ""),
        json.optString("photoClassification", ""),
        json.optString("constructionType", ""),
        json.optString("middleClassification", ""),
        json.optString("smallClassification", ""),
        json.optString("title", ""),
        ArrayList(json.optJSONArray("classificationRemarks").let { 0.until(it.length()).map { i -> it.optString(i) } }),
        json.optString("shootingSpot", ""),
        json.optBoolean("isRepresentative", false),
        json.optBoolean("isFrequencyOfSubmission", false),
        Measurement(json.optJSONObject("measurements")),
        json.optString("contractorRemarks", ""))


    fun toXMP() : String {
        val classificationRemarksXMP = classificationRemarks.withIndex().joinToString("") { "<rdf:li>${it.index}</rdf:li>" }
        val measurementsXMP = measurements.toXMP()

        val isRepresentative = if (this.isRepresentative) "True" else "False"
        val isFrequencyOfSubmission = if (this.isFrequencyOfSubmission) "True" else "False"

        return """
        <x:xmpmeta xmlns:x="adobe:ns:meta/">
        <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
        <rdf:Description rdf:about="" xmlns:photo="http://dcpadv.org/schema/3.0/photoinfo"
        xmlns:measurement="http://dcpadv.org/schema/3.0/measurement"
        xmlns:item="http://dcpadv.org/schema/3.0/measurementitem">
        <photo:ConstructionName>${constructionName}</photo:ConstructionName>
        <photo:Contructor>${contractor}</photo:Contructor>
        <photo:LargeClassification>${largeClassification}</photo:LargeClassification>
        <photo:PhotoClassification>${photoClassification}</photo:PhotoClassification>
        <photo:ConstructionType>${constructionType}</photo:ConstructionType>
        <photo:MiddleClassification>${middleClassification}</photo:MiddleClassification>
        <photo:SmallClassification>${smallClassification}</photo:SmallClassification>
        <photo:Title>${title}</photo:Title>
        <photo:ClassificationRemarks>
        <rdf:Seq>
        ${classificationRemarksXMP}
        </rdf:Seq>
        </photo:ClassificationRemarks>
        <photo:ShootingSpot>${shootingSpot}</photo:ShootingSpot>
        <photo:IsRepresentative>${isRepresentative}</photo:IsRepresentative>
        <photo:IsFrequencyOfSubmission>${isFrequencyOfSubmission}</photo:IsFrequencyOfSubmission>
        <photo:Measurements>
        ${measurementsXMP}
        </photo:Measurements>
        <photo:ContractorRemarks>${contractorRemarks}</photo:ContractorRemarks>
        </rdf:Description>
        </rdf:RDF>
        </x:xmpmeta>
        """
    }
}

// Measurement Class
@Parcelize
class Measurement(private val classification: MeasurementClassification, private val measurementItems: ArrayList<MeasurementItem>) : Parcelable {

    constructor(json: JSONObject) : this(MeasurementClassification.values()[json.optInt("classification", 0)], ArrayList(json.optJSONArray("measurementItems").let { 0.until(it.length()).map { i -> MeasurementItem(it.optJSONObject(i)) } }))

    fun toXMP() : String {
        val measurementItemsXMP = measurementItems.joinToString("") { it.toXMP() }
        return """
        <rdf:Description>
        <measurement:Classification>${classification.ordinal}</measurement:Classification>
        <measurement:MeasurementItems>
        <rdf:Seq>
        $measurementItemsXMP
        </rdf:Seq>
        </measurement:MeasurementItems>
        </rdf:Description>
        """.trimIndent()
    }
}

enum class MeasurementClassification {
    CONSTRUCTION_MANAGEMENT_VALUE,
    QUALITY_CERTIFICATION_VALUE,
    SUPERVISION_VALUE,
    INSPECTION_VALUE,
    RESERVED4,
    RESERVED5,
    RESERVED6,
    RESERVED7,
    RESERVED8,
    OTHERS;

    companion object {
        fun fromRawJapanese(rawJapanese: String): MeasurementClassification {
            return when (rawJapanese) {
                "施工管理値" -> CONSTRUCTION_MANAGEMENT_VALUE
                "品質証明値" -> QUALITY_CERTIFICATION_VALUE
                "監督値" -> SUPERVISION_VALUE
                "検査値" -> INSPECTION_VALUE
                "予約1" -> RESERVED4
                "予約2" -> RESERVED5
                "予約3" -> RESERVED6
                "予約4" -> RESERVED7
                "予約5" -> RESERVED8
                "その他" -> OTHERS
                else -> CONSTRUCTION_MANAGEMENT_VALUE
            }
        }
    }
}


// MeasurementItem Class
@Parcelize
class MeasurementItem(private val name: String, private val mark: String, private val designedValue: String, private val measuredValue: String, private val unitName: String, private val remarks: ArrayList<String>) : Parcelable {

    constructor(dic: JSONObject): this(
        dic["name"] as? String ?: "",
        dic["mark"] as? String ?: "",
        dic["designedValue"] as? String ?: "",
        dic["measuredValue"] as? String ?: "",
        dic["unitName"] as? String ?: "",
        dic["remarks"] as? ArrayList<String> ?: ArrayList<String>()
    )

    fun toXMP() : String {
        val remarksXMP = remarks.withIndex().joinToString("") { "<rdf:li>${it.value}</rdf:li>" }
        return """
        <rdf:li>
        <rdf:Description>
        <item:Name>$name</item:Name>
        <item:Mark>$mark</item:Mark>
        <item:DesignedValue>$designedValue</item:DesignedValue>
        <item:MeasuredValue>$measuredValue</item:MeasuredValue>
        <item:UnitName>$unitName</item:UnitName>
        <item:Remarks>
        <rdf:Seq>
        $remarksXMP
        </rdf:Seq>
        </item:Remarks>
        </rdf:Description>
        </rdf:li>
        """.trimIndent()
    }
}

