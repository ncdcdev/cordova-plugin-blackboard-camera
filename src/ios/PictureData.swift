//
//  PictureData.swift
//  TPR2
//
//  Created by 張成哲 on 2021/02/01.
//

import Foundation
import AppPotSDKforSwift

@objcMembers
open class PictureData: APObject {
    var siteId: String
    var kuiId: String
    var itemId: String
    var title: String
    var comment: String
    var kuisBreakdownId: String
    var fileName: String = ""

    open override func configureModel() {
        // 設定内容はパラメーターの順で下記の通り。
        // データの捜査権限はユーザーが属するグループ
        // データのデータの保持場所を端末内とサーバーの両方に指定する。
        // データ更新時の排他制御をオンにする。
        // データをサーバーのデータで自動更新を行う。
        // 自動更新の間隔を5分に指定.
        // データの端末内での保持期間を24時間に指定.
        // 自動同期するデータの範囲をユーザーが属するグループのデータに指定する。
        // 暗号化対象のプロパティーはないと指定する
        // 暗号化対象のプロパティーがないので、nilを渡す。
        super.setDefaultPropertiesWithScope(SCOPE_TYPE.APScopeGroup,
         persistentType: PERSISTENT_TYPE.APPersistentServerOnly,
         useLockForUpdate: true,
         isAutoRefresh: false,
         autoRefreshInterval: 300,
         lifeSpan: 86400,
         scopeForAutoRefresh: SCOPE_TYPE.APScopeGroup,
         hasEncryptedProperties: false ,
         encryptedProperties: nil)
    }

    override init() {
        self.siteId = ""
        self.kuiId = ""
        self.itemId = ""
        self.title = ""
        self.comment = ""
        self.kuisBreakdownId = ""
        self.fileName = ""
        super.init()
    }
    
    init(data: Dictionary<String, Any>) {
        self.siteId = data["siteId"] as! String
        self.kuiId = data["kuiId"] as! String
        self.itemId = data["itemId"] as! String
        if let kuisBreakdownId = data["kuisBreakdownId"] {
            self.kuisBreakdownId = (kuisBreakdownId as! String)
        } else {
            self.kuisBreakdownId = ""
        }
        if let title = data["title"] {
            self.title = title as! String
        } else {
            self.title = ""
        }
        if let comment = data["comment"] {
            self.comment = comment as! String
        } else {
            self.comment = ""
        }
        super.init()
    }

}
