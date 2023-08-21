//
//  ElectronicBlackBoardManager.swift
//  TPR2
//
//  Created by 張成哲 on 2023/05/24.
//


import Foundation
import UIKit
import CoreImage
import ImageIO
import SwiftXMP
import MobileCoreServices
import UniformTypeIdentifiers

protocol ElectronicBlackBoardManagerProtocol {
    // PhotoInfoと画像を受け取り、XMPを生成して、メタ情報に埋め込み新しい画像を作成する
    static func createImageEmbeddedMetaData(from image: Data, photoInfo: PhotoInfo , imageDescription: String,model:String,software:String) -> Data?
}

final class ElectronicBlackBoardManager: ElectronicBlackBoardManagerProtocol {

    static func createImageEmbeddedMetaData(from image:Data , photoInfo: PhotoInfo,imageDescription: String,model:String,software:String) -> Data? {
        var path = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0]
        path = path + "/sample.jpg"
        let url = URL(fileURLWithPath: path)
        try? image.write(to: url)
        let cgImg = CGImageSourceCreateWithURL(url as CFURL, nil)
        let orgMeta = CGImageSourceCopyPropertiesAtIndex(cgImg!, 0, nil) as! [String: Any]
        // // デバッグ出力
        // print("orgMeta: \(orgMeta)")
        // // exifを出力
        // print("exif: \(orgMeta[kCGImagePropertyExifDictionary as String]!)")
        
        let now = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy:MM:dd HH:mm:ss"
        let millisec = Int(now.timeIntervalSince1970 * 1000) % 1000
        let formattedDate = formatter.string(from: now)

        // ExifVersion	Exifバージョン	0x9000	Undefined	4	◎
        // ComponentsConfiguration	各コンポーネントの意味	0x9101	Undefined	4	◎
        // FlashpixVersion	対応フラッシュピックスバージョン	0xA000	Undefined	4	◎
        // ColorSpace	色空間情報	0xA001	Short	1	◎
        // PixelXDimension	実効画像幅	0xA002	Short/Long	1	◎
        // PixelYDimension	実効画像高さ	0xA003	Short/Long	1	◎

        // SubSecTime	DateTimeのサブセック	0x9290	Ascii	Any	△
        // SubSecTimeOriginal	DateTimeOriginalのサブセック	0x9291	Ascii	Any	△
        // SubSecTimeDigitized	DateTimeDigitizedのサブセック	0x9292	Ascii	Any	△
        // DateTime	ファイル変更日時	0x0132	Ascii	20	○
        // DateTimeOriginal	原画像データの生成日時	0x9003	Ascii	20	△
        // DateTimeDigitized	デジタルデータの作成日時	0x9004	Ascii	20	△

        // tiffヘッダ？の情報も書き換える
        var tiff = orgMeta[kCGImagePropertyTIFFDictionary as String] as! [String: Any]
        tiff[kCGImagePropertyTIFFImageDescription as String] = imageDescription
        tiff[kCGImagePropertyTIFFModel as String] = model
        tiff[kCGImagePropertyTIFFSoftware as String] = software
        // exifを書き換える
        var exif = orgMeta[kCGImagePropertyExifDictionary as String] as! [String: Any]
        exif[kCGImagePropertyExifDateTimeOriginal as String] = formattedDate
        exif[kCGImagePropertyExifSubsecTimeOriginal as String] = millisec
        exif[kCGImagePropertyExifVersion as String] = [2, 3, 0]
        exif[kCGImagePropertyExifComponentsConfiguration as String] = [1, 2, 3, 0]
        exif[kCGImagePropertyExifFlashPixVersion as String] = [1, 0, 0]

        // メタ情報を作成
        let metaData = [
            kCGImagePropertyTIFFDictionary: tiff,
            kCGImagePropertyExifDictionary: exif
        ] as CFDictionary

        // let metaData = [
        //     kCGImagePropertyTIFFDictionary: [
        //         kCGImagePropertyTIFFImageDescription: imageDescription,
        //         kCGImagePropertyTIFFModel: model,
        //         kCGImagePropertyTIFFSoftware: software,
        //     ],
        //     kCGImagePropertyExifDictionary: [
        //         kCGImagePropertyExifDateTimeOriginal: formattedDate,
        //         kCGImagePropertyExifSubsecTimeOriginal: millisec,
        //         kCGImagePropertyExifVersion: "0230",
        //     ]
        // ] as CFDictionary

        // 埋め込むメタデータを表示
        print("metaData: \(metaData)")

        let dest: CGImageDestination? = CGImageDestinationCreateWithURL(url as CFURL, kUTTypeJPEG, 1, nil)
        CGImageDestinationAddImageFromSource(dest!, cgImg!, 0, metaData)
        CGImageDestinationFinalize(dest!)

        // xmpの書き込み
        let xmp = photoInfo.toXMP()
        let swiftXmp = SwiftXMP()

        switch swiftXmp.embedXmp(contens: url, xml: xmp) {
        case .success(let data):
            print("add xmp successfully")
            return  data
        case .failed(let e):
            print("add xmp error : \(e.localizedDescription)")
            return nil
        }
    }
}

