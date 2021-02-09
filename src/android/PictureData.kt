package jp.co.taisei.construction.fieldmanagement.plugin

import jp.co.ncdc.apppot.stew.dto.APObject
import org.json.JSONException
import org.json.JSONObject

class PictureData() : APObject() {
    var siteId: String = ""
    var kuisBreakdownId: String = ""
    var kuiId: String = ""
    var itemId: String = ""
    var title: String = ""
    var comment: String = ""
    var fileName: String = ""

    constructor(json: JSONObject) : this() {
        siteId = try { json["siteId"] as String } catch (e: JSONException) { "" }
        kuisBreakdownId = try { json["kuisBreakdownId"] as String } catch (e: JSONException) { "" }
        kuiId = try { json["kuiId"] as String } catch (e: JSONException) { "" }
        itemId = try { json["itemId"] as String } catch (e: JSONException) { "" }
        title = try { json["title"] as String } catch (e: JSONException) { "" }
        comment = try { json["comment"] as String } catch (e: JSONException) { "" }
    }

    init {
        persistentType = PersistentType.APServerOnly
        isAutoRefresh = false
    }
}