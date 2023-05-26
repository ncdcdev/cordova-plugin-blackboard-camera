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
         
        let now = Date()
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy:MM:dd HH:mm:ss"
        let formattedDate = formatter.string(from: now)

        let metaData = [
            kCGImagePropertyTIFFDictionary: [
                kCGImagePropertyTIFFImageDescription: imageDescription,
                kCGImagePropertyTIFFModel: model,
                kCGImagePropertyTIFFSoftware: software,
            ],
            kCGImagePropertyExifDictionary: [
                kCGImagePropertyExifDateTimeOriginal: formattedDate,
//                kCGImagePropertyExifUserComment:"fizzbuzz!"
            ]
        ] as CFDictionary
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

