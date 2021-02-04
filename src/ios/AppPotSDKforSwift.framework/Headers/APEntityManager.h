//
// APEntityManager.h
// AppPot
//
// Created by NC Design & Consulting Co., Ltd. on 8/20/13.
// Copyright (c) 2013 NC Design & Consulting Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "AppPot.h"
#import "APObject.h"
#import "APTypeDefinition.h"

/// モデルクラスで定義したデータの操作を総括するクラス。モデルクラスの設定内容に応じてアプリ内のデータベース操作とAppPotとのデータのやり取りを行う。
@interface APEntityManager : NSObject{
    
}

/*!
 *  @abstract Singleton object
 *
 *  @discussion
 *	This is a singleton object which keep objects alive until the end of app.
 */
+ (nonnull APEntityManager *)sharedInstance;

/*!
 *  @abstract アプリが使用するデータオブジェクトをSDKに登録する。このクラスはデータベースのバージョンを「0」に指定して「- (void) registerValueObjects:(NSArray*) classes isRecreate:(BOOL)isRecreateDatabase withBlock:(APErrorBlock) block;」メソッドを呼び出す。
 *  @param classes データオブジェクトののClass型の一覧。
 *  @param block データのスキーマ作成に失敗した場合に実行する処理を記述するブロック。初期起動時、開発時にはアプリの稼働を中止することを推奨する。
 *  @discussion アプリケーションのデベロッパーはアプリで使う全てのデータクラスをAPObjectの子クラスとして定義、このメソッドを通してSDKに登録する。
 *  この登録によってデバイス及びサーバー側のデータベースや各テーブルのスキーマが作成される。登録されなかったデータクラスはSDKによって管理されない。
 */
- (void) registerValueObjects:(nonnull NSArray*) classes isRecreate:(BOOL)isRecreateDatabase withBlock:(nullable APErrorBlock) block;

/*!
 *  @abstract アプリが使用するデータオブジェクトをSDKに登録する。Register Value Objects of App to AppPot SDK.
 *  @param classes データオブジェクトののClass型の一覧。
 *  @param block データのスキーマ作成に失敗した場合に実行する処理を記述するブロック。初期起動時、開発時にはアプリの稼働を中止することを推奨する。
 *  @discussion アプリケーションのデベロッパーはアプリで使う全てのデータクラスをAPObjectの子クラスとして定義、このメソッドを通してSDKに登録する。
 *  この登録によってデバイス及びサーバー側のデータベースや各テーブルのスキーマが作成される。登録されなかったデータクラスはSDKによって管理されない。
 */
- (void) registerValueObjects:(nonnull NSArray*) classes isRecreate:(BOOL)isRecreateDatabase withDbVersion:(int)dbVersion withBlock:(nullable APErrorBlock) block;

/*!
 *  @abstract アプリが使用するデータオブジェクトをSDKに登録する。Register Value Objects of App to AppPot SDK.
 *  @param classes データオブジェクトののClass型の一覧。
 *  @param successBlock データのスキーマ作成に成功した時に実行する処理を記述するブロック。
 *  @param errorBlock データのスキーマ作成に失敗した場合に実行する処理を記述するブロック。初期起動時、開発時にはアプリの稼働を中止することを推奨する。
 *  @discussion アプリケーションのデベロッパーはアプリで使う全てのデータクラスをAPObjectの子クラスとして定義、このメソッドを通してSDKに登録する。
 *  この登録によってデバイス及びサーバー側のデータベースや各テーブルのスキーマが作成される。登録されなかったデータクラスはSDKによって管理されない。
 */
- (void) registerValueObjects:(nonnull NSArray*) classes isRecreate:(BOOL)isRecreateDatabase withDbVersion:(int)dbVersion onSuccess:(nullable   APSuccessBlock) successBlock onError:(nullable APErrorBlock) errorBlock;

/**
 * 自動更新処理クラスを登録する
 */
- (void)registerAutoRefresh;

/*!
 *  @abstract アプリが使用するローカルDBを削除し、新しくする。
 *  @discussion 既存DBのデータはなくなる。主に、既存ユーザーと異なるユーザーでログインした際に呼ばれる。
 */
- (void) resetDatabase;

/*!
 *  @abstract 主キー（objectId）を指定して一件のデータクラスを取得する。このメソッドは非同期型で処理される。
 *  @param _class 取得するデータの種別をクラスで指定
 *  @param objectId 取得するデータの主キー
 *  @param block 取得したデータや発生したエラーを処理するブロック
 *
 *  @discussion TBD
 *
 */
- (void) objectForClass:(nonnull Class)_class withId:(nonnull NSString *) objectId withBlock:(nonnull APObjectResultBlock)block;  //__attribute__((deprecated));


/*!
 *  @abstract 主キー（objectId）を指定して一件のデータクラスを取得する。このメソッドは同期型で処理される。
 *  @param _class 取得するデータの種別をクラスで指定
 *  @param objectId 取得するデータの主キー
 *  @param error 発生したエラーを収納するエラーオブジェクトを渡す。
 *
 *  @discussion TBD
 *
 */
- (nullable APObject*) objectForClass:(nonnull Class)_class withId:(nonnull NSString *) objectId error:(NSError * _Nullable * _Nullable)error;  //__attribute__((deprecated));


/*!
 *  @abstract 対象のデータオブジェクトを保存する。この処理は同期型で行われる。
 *  @param valueObject 保存するデータオブジェクト。
 *  @param error 発生したエラーを格納するためのエラーオブジェクト
 *
 *  @discussion 対象データオブジェクトが新規の場合は新規データの追加、既存のものである場合は既存データの更新が行われる。正常時は更新が終わったデータオブジェクトを返す。
 *
 */
- (nullable APObject*) saveInstance:(nonnull APObject*)valueObject error:(NSError * _Nullable * _Nullable)error;


/*!
 *  @abstract 対象のデータオブジェクトを保存する。この処理は非同期型で行われる。
 *  @param valueObject 保存するデータオブジェクト。
 *  @param block データの処理結果を取得しその後の処理を行うためのブロック
 *
 *  @discussion 対象データオブジェクトが新規の場合は新規データの追加、既存のものである場合は既存データの更新が行われる。正常時は更新が終わったデータオブジェクトを返す。
 *
 */
- (void) saveInstance:(nonnull APObject*)valueObject withBlock:(nullable APObjectResultBlock)block;

/*!
 *  @abstract 対象のデータオブジェクト一覧を保存する。この処理は同期型で行われる。
 *  @param list 保存するデータオブジェクトの一覧。
 *  @param error 発生したエラーを格納するためのエラーオブジェクト
 *
 *  @discussion 対象データオブジェクトが新規の場合は新規データの追加、既存のものである場合は既存データの更新が行われる。正常時は更新が終わったデータオブジェクトを返す。
 *
 */
- (nullable NSArray*) saveListOfInstances:(nonnull NSArray*) list error:(NSError * _Nullable * _Nullable)error;


- (void) saveListOfInstances:(nonnull NSArray*) list onSuccess:(nullable APArrayBlock)successBlock onError:(nullable APErrorBlock) errorBlock;
/*!
 *  @abstract 対象データオブジェクトを削除する。この処理は同期型で行われる。
 *  @param instance 削除するデータオブジェクト。
 *  @param error 発生したエラーを格納するためのエラーオブジェクト
 *  @return 削除処理の成敗
 *  @discussion TBD
 *
 */
- (BOOL) deleteInstance:(nonnull APObject*)instance error:(NSError * _Nullable * _Nullable) error;


/*!
 *  @abstract 対象データオブジェクトを削除する。この処理は非同期型で行われる。
 *  @property instance　削除するデータオブジェクト。
 *  @param block 発生したエラーを格納するためのエラーオブジェクト
 *
 *  @discussion TBD
 */
- (void) deleteInstance:(nonnull APObject*)instance withBlock:(nullable APObjectResultBlock) block;

/*!
 *  @abstract 対象データオブジェクトの一覧を削除する。この処理は同期型で行われる。
 *  @property instance　削除するデータオブジェクト。
 *  @param error 発生したエラーを格納するためのエラーオブジェクト
 *
 *  @discussion TBD
 *
 */
- (BOOL) deleteListOfInstance:(nonnull NSArray*)list error:(NSError * _Nullable * _Nullable) error;

- (void) deleteListOfInstance:(nonnull NSArray*)list withBlock:(nullable APObjectResultBlock) block;

/*!
 *  @abstract 指定したクラスの全てのデータを削除する。
 *  @param _class 削除対象のクラス
 *
 *  @discussion データストレージ内のデータが全て削除されるので使用には注意が必要。ローカルストレージでは物理削除、サーバー側では論理削除が行われる。
 *
 */
- (void) deleteAllDataFor:(nonnull Class)_class;


/*!
 *  @abstract 指定したテーブルのデータの件数を返す。Return number of rows of table in local database
 *  @param class 数える対象クラス
 *
 *  @discussion TBD
 *
 */
- (NSInteger) numberOfRecordsForClass:(nonnull Class) class;

#pragma mark V3 APIの追加
/*!
 *  @abstract 主キー（objectId）を指定して一件のデータクラスを取得する。このメソッドは非同期型で処理される。
 *  @param _class 取得するデータの種別をクラスで指定
 *  @param objectId 取得するデータの主キー
 *  @param block 取得したデータや発生したエラーを処理するブロック
 *
 *  @discussion TBD
 *
 */
- (void) objectForClass:(nonnull Class)_class withObjectId:(nonnull NSString *) objectId withBlock:(nonnull APObjectResultBlock)block;

- (void) objectsFromServerForClass:(nonnull Class)_class withCondition:(nonnull NSDictionary*) condition withBlock:(nonnull APSelectResultBlock) block;

- (nullable NSDictionary *) countFromServerForClass:(nonnull Class) _class withCondition:(nonnull NSDictionary*) condition withError:(NSError*_Nullable*_Nullable) theError;
- (nonnull NSDictionary *) objectsFromLocalBySQLQuery:(nonnull NSString *)sqlQuery withParams:(nullable NSArray *)params withAliasEntityDic:(nonnull NSDictionary *)aliasEntityDic;

@end
