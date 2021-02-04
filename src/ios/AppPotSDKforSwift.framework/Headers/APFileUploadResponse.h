//
//  APUploadFileResponse.h
//  AppPotSDK
//
//  Created by Roy S. Kim on 6/23/15.
//  Copyright (c) 2015 TMA Solutions. All rights reserved.
//

#import <Foundation/Foundation.h>

/// ファイルをAppPotを利用してアップロードした場合のレスポンス
@interface APFileUploadResponse : NSObject


/// ファイルをアップロードしてサーバーに保存されたファイル名
@property (nonatomic, strong, nonnull) NSString *fileName;
/// ファイルのURL
@property (nonatomic, strong, nonnull) NSString *fileURL;
@end
