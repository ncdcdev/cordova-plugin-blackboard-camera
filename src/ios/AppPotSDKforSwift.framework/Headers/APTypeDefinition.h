//
//  APTypeDefine.h
//  AppPotSDK
//
//  Created by NCDC on 2015/01/22.
//  Copyright (c) 2015年 TMA Solutions. All rights reserved.
//

/// AppPotで使われているブロックの定義

#import "APResponse.h"
#import "APSearchResult.h"
#import "APFileUploadResponse.h"

@class APObject;

/// パラメーターと返り値を持たない成功時の実行ブロック
typedef void (^APSuccessBlock)(void);
/// エラー時の実行ブロック
typedef void (^APErrorBlock)(NSError * _Nonnull error);
/// APResponseクラスをパラメーターに持つブロック
typedef void (^APResponseBlock)(APResponse * _Nonnull response);
/// NSDictionary型のパラメーターに持つブロック
typedef void (^APDictionaryBlock)(NSDictionary * _Nonnull result);
/// NSHTTPURLResponse型のパラメーターに持つブロック
typedef void (^ResponseBlock)(NSHTTPURLResponse * _Nonnull result);

/// 配列をパラメーターに持つブロック
typedef void (^APArrayBlock)(NSArray * _Nonnull result);
/// ファイルアップロード後のアップロードしたファイルのサーバーにおける情報をパラメーターに持つブロック
typedef void (^APFileUploadResponseBlock)(APFileUploadResponse * _Nonnull fileInfo);
/// ファイルをダウンロードする場合使われる、ファイルのデータをパラメーターに持つブロック
typedef void (^APFileResponseBlock)(NSData * _Nullable fileData);
/// APQueryを使ってデータを検索した結果を処理するブロック
typedef void (^APSearchResultBlock)(APSearchResult * _Nullable result, NSError * _Nullable error);
/// １件のデータを取得して処理するブロック
typedef void (^APObjectResultBlock)(APObject * _Nullable object, NSError * _Nullable error);
// APSelectを使って複数件のデータを取得するブロック
typedef void (^APSelectResultBlock)(NSDictionary * _Nullable result, NSError * _Nullable error);
// APSelectを使ったデータを件数を取得するブロック
typedef void (^APSelectCountBlock)(NSInteger count, NSError * _Nullable error);

/*!
 @abstract データの保持場所を指定するための定数定義。
 @discussion 定義順に、サーバーだけにデータを保持、端末内だけに保持、そして端末とサーバーの両方にデータを保持することを示す。SDKのデフォルト値はAPPersistentServerOnlyである。
 */
typedef NS_ENUM(NSInteger, PERSISTENT_TYPE) {
    APPersistentServerOnly = 0,
    APPersistentLocalOnly,
    APPersistentLocalAndServer
};

/*!
 * @abstract データの権限を指定するための定数定義。
 * @discussion 定義順に、全使用者が使用可能、同じグループの人が使用可能、データの作成者だけが使用可能であることを示す。SDKのデフォルト値はAPScopeGroupである。
 */
typedef NS_ENUM(NSInteger, SCOPE_TYPE) {
    APScopeAll = 3,
    APScopeGroup = 2,
    APScopeUser = 1,
    
};

/*!
 * @abstract データの権限を指定するための定数定義。
 * @discussion 定義順に、全使用者が使用可能、同じグループの人が使用可能、データの作成者だけが使用可能であることを示す。
 */
typedef NS_ENUM(NSInteger, SYNC_STATUS) {
    APSyncStatusNotInSync = 0,
    APSyncStatusInSync
};

