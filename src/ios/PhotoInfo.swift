//
//  PhotoInfo.swift
//  TPR2
//
//  Created by 張成哲 on 2023/05/24.
//

import Foundation

// PhotoInfo Class
class PhotoInfo {
    var constructionName: String
    var contractor: String
    var largeClassification: String
    var photoClassification: String
    var constructionType: String
    var middleClassification: String
    var smallClassification: String
    var title: String
    var classificationRemarks: [String]
    var shootingSpot: String
    var isRepresentative: Bool
    var isFrequencyOfSubmission: Bool
    var measurements: Measurement
    var contractorRemarks: String
    
    init(constructionName: String, contractor: String, largeClassification: String, photoClassification: String, constructionType: String, middleClassification: String, smallClassification: String, title: String, classificationRemarks: [String], shootingSpot: String, isRepresentative: Bool, isFrequencyOfSubmission: Bool, measurements: Measurement, contractorRemarks: String) {
        self.constructionName = constructionName
        self.contractor = contractor
        self.largeClassification = largeClassification
        self.photoClassification = photoClassification
        self.constructionType = constructionType
        self.middleClassification = middleClassification
        self.smallClassification = smallClassification
        self.title = title
        self.classificationRemarks = classificationRemarks
        self.shootingSpot = shootingSpot
        self.isRepresentative = isRepresentative
        self.isFrequencyOfSubmission = isFrequencyOfSubmission
        self.measurements = measurements
        self.contractorRemarks = contractorRemarks
    }

    init(dic: Dictionary<String, Any>) {
        self.constructionName = dic["constructionName"] as? String ?? ""
        self.contractor = dic["contractor"] as? String ?? ""
        self.largeClassification = dic["largeClassification"] as? String ?? ""
        self.photoClassification = dic["photoClassification"] as? String ?? ""
        self.constructionType = dic["constructionType"] as? String ?? ""
        self.middleClassification = dic["middleClassification"] as? String ?? ""
        self.smallClassification = dic["smallClassification"] as? String ?? ""
        self.title = dic["title"] as? String ?? ""
        self.classificationRemarks = dic["classificationRemarks"] as? [String]  ?? [""]
        self.shootingSpot = dic["shootingSpot"] as? String ?? ""
        self.isRepresentative = dic["isRepresentative"] as? Bool ?? false
        self.isFrequencyOfSubmission = dic["isFrequencyOfSubmission"] as? Bool ?? false
        self.contractorRemarks = dic["contractorRemarks"] as? String ?? ""
        self.measurements = Measurement(dic: dic["measurements"] as! Dictionary<String, Any>)
    }
    
    func toXMP() -> String {
        let classificationRemarksXMP = classificationRemarks.enumerated().map { "<rdf:li>\($0)</rdf:li>"}.joined()
        let measurementsXMP = measurements.toXMP()
        
        return """
        <x:xmpmeta xmlns:x="adobe:ns:meta/">
        <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
        <rdf:Description rdf:about="" xmlns:photo="http://dcpadv.org/schema/3.0/photoinfo"
        xmlns:measurement="http://dcpadv.org/schema/3.0/measurement"
        xmlns:item="http://dcpadv.org/schema/3.0/measurementitem">
        <photo:LargeClassification>\(largeClassification)</photo:LargeClassification>
        <photo:PhotoClassification>\(photoClassification)</photo:PhotoClassification>
        <photo:ConstructionType>\(constructionType)</photo:ConstructionType>
        <photo:MiddleClassification>\(middleClassification)</photo:MiddleClassification>
        <photo:SmallClassification>\(smallClassification)</photo:SmallClassification>
        <photo:Title>\(title)</photo:Title>
        <photo:ClassificationRemarks>
        <rdf:Seq>
        \(classificationRemarksXMP)
        </rdf:Seq>
        </photo:ClassificationRemarks>
        <photo:ShootingSpot>\(shootingSpot)</photo:ShootingSpot>
        <photo:IsRepresentative>\(isRepresentative)</photo:IsRepresentative>
        <photo:IsFrequencyOfSubmission>\(isFrequencyOfSubmission)</photo:IsFrequencyOfSubmission>
        \(measurementsXMP)
        <photo:ContractorRemarks>\(contractorRemarks)</photo:ContractorRemarks>
        </rdf:Description>
        </rdf:RDF>
        </x:xmpmeta>
        """
    }
}

// Measurement Class
class Measurement {
    var classification: MeasurementClassification
    var measurementItems: [MeasurementItem]
    
    init(classification: MeasurementClassification, measurementItems: [MeasurementItem]) {
        self.classification = classification
        self.measurementItems = measurementItems
    }
    init(dic: Dictionary<String, Any>) {
        self.classification = MeasurementClassification(rawValue: dic["classification"] as! Int ) ?? MeasurementClassification.constructionManagementValue
        self.measurementItems = (dic["measurementItems"] as! Array<Dictionary<String, Any>>).map { MeasurementItem(dic: $0 ) }
    }
    
    func toXMP() -> String {
        let measurementItemsXMP = measurementItems.map { $0.toXMP() }.joined()
        return """
        <rdf:Description>
        <measurement:Classification>\(classification.rawValue)</measurement:Classification>
        <measurement:MeasurementItems>
        <rdf:Seq>
        \(measurementItemsXMP)
        </rdf:Seq>
        </measurement:MeasurementItems>
        </rdf:Description>
        """
    }
}

enum MeasurementClassification: Int {
    case constructionManagementValue = 0
    case qualityCertificationValue = 1
    case supervisionValue = 2
    case inspectionValue = 3
    case reserved4 = 4
    case reserved5 = 5
    case reserved6 = 6
    case reserved7 = 7
    case reserved8 = 8
    case others = 9

    init?(rawJapanese: String) {
        switch rawJapanese {
        case "施工管理値":
            self = .constructionManagementValue
        case "品質証明値":
            self = .qualityCertificationValue
        case "監督値":
            self = .supervisionValue
        case "検査値":
            self = .inspectionValue
        case "予約1":
            self = .reserved4
        case "予約2":
            self = .reserved5
        case "予約3":
            self = .reserved6
        case "予約4":
            self = .reserved7
        case "予約5":
            self = .reserved8
        case "その他":
            self = .others
        default:
            self = .constructionManagementValue
        }
    }
}


// MeasurementItem Class
class MeasurementItem {
    var name: String
    var mark: String
    var designedValue: String
    var measuredValue: String
    var unitName: UnitName
    var remarks: [String]
    
    init(name: String, mark: String, designedValue: String, measuredValue: String, unitName: UnitName, remarks: [String]) {
        self.name = name
        self.mark = mark
        self.designedValue = designedValue
        self.measuredValue = measuredValue
        self.unitName = unitName
        self.remarks = remarks
    }
    init(dic: Dictionary<String, Any>) {
        self.name = dic["name"] as! String
        self.mark = dic["mark"] as! String
        self.designedValue = dic["designedValue"] as! String
        self.measuredValue = dic["measuredValue"] as! String
        self.unitName = UnitName(rawValue: dic["unitName"] as! String) ?? UnitName.um // TODO
        self.remarks = dic["remarks"] as! [String]
    }
    
    func toXMP() -> String {
        let remarksXMP = remarks.map { "<rdf:li>\($0)</rdf:li>" }.joined()
        return """
        <rdf:li>
        <rdf:Description>
        <item:Name>\(name)</item:Name>
        <item:Mark>\(mark)</item:Mark>
        <item:DesignedValue>\(designedValue)</item:DesignedValue>
        <item:MeasuredValue>\(measuredValue)</item:MeasuredValue>
        <item:Unit>\(unitName.rawValue)</item:Unit>
        <item:Remarks>
        <rdf:Seq>
        \(remarksXMP)
        </rdf:Seq>
        </item:Remarks>
        </rdf:Description>
        </rdf:li>
        """
    }
}


enum UnitName: String {
    case um = "μm"
    case mm = "mm"
    case cm = "cm"
    case m = "m"
    case km = "km"
    case g = "g"
    case kg = "kg"
    case t = "t"
    case lb = "lb"
    case m2 = "m2"
    case a = "a"
    case ha = "ha"
    case tatami = "畳"
    case m3 = "m3"
    case l = "l"
    case rad = "rad"
    case degree = "°"
    case minute = "′"
    case second = "″"
    case percent = "%"
    case permille = "‰"
    case ratio = "1:n"
    case celsius = "°C"
    case newton = "N"
    case kilonewton = "kN"
    case pascal = "Pa"
    case kilopascal = "kPa"
    case hectopascal = "hPa"
    case megapascal = "MPa"
    case gf_cm2 = "gf/cm2"
    case gf_m2 = "gf/m2"
    case kgf_m2 = "kgf/m2"
    case kgf_cm2 = "kgf/cm2"
    case kg_m3 = "kg/m3"
    case kg_cm3 = "kg/cm3"
    case g_cm3 = "g/cm3"
    case ohm = "Ω"
    case volt = "V"
    case millivolt = "mV"
    case db = "dB"
    case secondTime = "s"
    case hour = "h"
    case piece = "個"
    case stick = "本"
    case sheet = "枚"
    case stage = "段"
    case part = "部"
    case pot = "鉢"
    case bag = "袋"
    case stand = "台"
    case style = "式"
    case place = "箇所"
    case set = "組"
    case eye = "目"
}
