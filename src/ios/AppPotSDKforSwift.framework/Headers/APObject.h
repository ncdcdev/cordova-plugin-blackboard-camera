//
// APObject.h
// AppPot
//
// Created by NC Design & Consulting Co., Ltd. on 8/20/13.
// Copyright (c) 2013 NC Design & Consulting Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "APTypeDefinition.h"
#import "APSelect.h"

/*!
 @abstract AppPot SDKによって管理されるデータを表すクラス(Value Object)の共通の親クラス。
 @discussion AppPotによって永続化が管理するクラスは全てこのクラスを親クラスとして指定する必要がある。
 　　　　　　　SDKを利用する開発車はこのクラスが定義している属性を適切に設定することでAppPot SKDによるデータの管理方式やデータ保存場所等を指定する。
 */
@interface APObject : NSObject <NSCopying> 


/*!
 * @abstract このデータの識別子となるキーシステム全体でユニークである。
 * @property objectId
 */
@property (nonatomic, strong, nullable) NSString* objectId;

/*!
 *  @abstract Type of data access
 *  @property peristentType
 *
 *  @discussion
 *  Persistent type will indicate a way to access data. Pre-defined values in PERSISTENT_TYPE can be used. The default value is 'APLocalOnly'.
 *  PERSISTENT_TYPE has 3 values:
 *      APServerOnly        :   Only process data at server side.
 *      APLocalOnly         :   Only process data at local side
 *      APLocalAndServer    :   Process data both server side and local side
 */
@property (nonatomic) NSInteger persistentType;

/*!
 *  @abstract データを更新する前に他の人によって変更されたかを確認するか否かを指定する。
 *  @property isUsingLockForUpdate
 *
 *  @discussion
 *  この属性のデフォルト値はNO。つまり、データの変更を伴うリクエストは常に成功し、最後に変更した情報が残される。途中の変更データは後のデータによって上書きされることになる。
 *  この属性値をYESに設定した場合、データをストレージに書き込む前に更新日時を確認し、他のユーザーやシステムによって該当データが変更されてないかを確認する。もし変更されている場合は操作は取り消される。
 *  Default value is NO and it means the last request of update should be reflected to server. If this value is set YES, updateTime in server will be compared with updateTime of request data and it cancels the request if the two time is not same.
 */
@property (nonatomic, strong, nonnull) NSNumber *isUsingLockForUpdate;


/*!
 *  @abstract Created time
 *  @property createTime
 *
 *  @discussion
 *  createTime is obtained at the time when object is created
 */
@property (nonatomic, strong, nullable) NSDate*  createTime;
/*!
 *  @abstract Updated object
 *  @property updateTime
 *
 *  @discussion
 *  updateTime is obtained at the time when object is updated.
 */
@property (nonatomic, strong, nullable) NSDate*  updateTime;

/*!
 *  @abstract サーバーでの更新時刻。
 *  @property updateTime
 *
 *  @discussion
 *  このプロパティはサーバー側での楽観的ロックのために存在する。APObjectの永続化タイプによってオブジェクトの以下の状況を示す。
 *  APPersistentLocalOnly : 常にnil。値は無視される。
 *  APPersistentServer : nilの場合は新規のデータを作成し、そのインスタンスを使っている場合。サーバーにデータを保持した時点でここの値が入る。ただし、サーバーから取得したデータにはこの値が最初から入っているため、この値ををサーバーに保存された情報か否かを判断するためには使えない。
 *  APPersistentLocalAndServer : nilの場合は新規のデータを作成し、そのインスタンスを使っている場合。サーバーにデータを保持した時点でここの値が入る。ただし、サーバーから取得したデータにはこの値が最初から入っているため、この値ををサーバーに保存された情報か否かを判断するためには使えない。
 */
@property (nonatomic, strong, nullable) NSString*  serverUpdateTime;


#pragma mark データの所有者及び権限の管理

/*!
 * @abstract このデータを作成したユーザーのID
 * @property createUserID
 */
@property (nonatomic) long long createUserId;


/*!
 * @abstract このデータへのアクセスができるグループのID
 * @property groupIDs
 */
@property (nonatomic, strong, nullable) NSString* groupIds;

/*!
 *  @abstract このクラスに対するアクセス権限（readwrite）Access right for the class
 *  @property scopeType
 *
 *  @discussion
 *  このクラスが表すデータに対して参照及び編集の権限を指定する。デフォルトではデータ作成者の属するグループがアクセス権限を持つが、必要に応じて全員、作成者のみと限定することができる。
 */
@property (nonatomic) NSInteger scopeType;




/*!
 *  @abstract データの同期状態
 *  @property syncStatus
 *
 *  @discussion
 *  データがサーバーに保存される場合（persistentTypeがAPPersistentServerOnlyかAPPersistentLocalAndServerの場合）、該当するデータがサーバーと同期されているかを示す。ローカルのストレージのみに保存されるデータオブジェクトに対してはこのプロパティは無視される。
 */
@property (nonatomic, assign) NSInteger syncStatus;



#pragma mark データの操作

/*!
 *  @abstract サーバー側に保存する目的でデータを生成する。
 *  @param scopeType データへのアクセス権限の範囲を指定
 *
 *  @discussion 端末にはデータを保存せず、サーバーだけに保存するためのデータを生成する。
 *  このメソッドで生成されたデータはこのクラスの指定された永続化場所とは関係なくサーバーにだけ保存される。
 *
 */
- (nonnull id) initForServerOnlyWithScopeType:(SCOPE_TYPE) scopeType;

/**
 * このモデルの基本的な属性を指定するために使われる。実装としては先ずはsetDefaultPropertiesWithScope:presistentType:useLockForUpdate:isAutoRefresh:autoRefreshInterval:lifeSpan:scopeForAutoRefresh:hasEncryptedProperties:encryptedPropertiesを呼ぶ。これはデータの属性を設定するためには必須。他に追加で設定したいプロパティに対してデフォルト値を設定できる。
 */
- (void) configureModel;


/**
   @abstract このクラスの基本的な設定を行う。このメソッドは-configureModelメソッドの中でだけ呼び出すようにする。
 
   @discussion 以下の属性を設定することでこのクラスの基本的な操作方式を指定する。この指定はこのクラスの全てのインスタンスに適用される。
    このメソッドを呼んでなかった場合は各パラメーターのデフォルト値が適用される。
 
    - scope                           = APScopeGroup; // データを参照・編集できる範囲を指定する。デフォルト値はデータ作成者のグループ(APScopeGroup) かSDKの初期化にて指定されたタイプが使用される。
    - peristentType                   = APPersistentServerOnly; // データの保存場所を指定する。
    - useLockForUpdate                = NO; // データの更新の際にロックをかけるかの設定。
    - isAutoRefresh                   = NO; // データの保存場所が端末とサーバー（APPersistentServerAndLocal)で、端末のデータを自動更新する場合のみYESにする。
    - autoRefreshInterval             = 600; // 自動更新を行う場合、自動更新の時間間隔を秒単位で指定する。デフォルト値は１０分か、SDKの初期化にて指定された時間が使用される。
    - lifeSpan                        = 2,592,000; // 永続化場所が端末とサーバー（APPersistentServerAndLocal）である場合、端末内のデータの保存期間を秒で指定する。デフォルト値は30日か、SDKの初期化にて指定された時間が使用される。
    - scopeForAutoRefresh             = APScopeGroup; // 端末にあるデータの自動更新対象の権限範囲を指定する。デフォルト値はデータの権限（_scopeType)が使用される。
    - hasEncryptedProperties          = NO // 暗号化対象のプロパティーの有無を設定
    - encryptedProperties             = nil // 暗号化対象のプロパティーがある場合はそのプロパティー名を配列に入れて渡す。ない場合はnilを渡す。
 */
- (void) setDefaultPropertiesWithScope:(SCOPE_TYPE) scope
                        persistentType:(PERSISTENT_TYPE) persistentType
                      useLockForUpdate:(BOOL) useLockForUpdate
                         isAutoRefresh:(BOOL) isAutoRefresh
                   autoRefreshInterval:(long) interval
                              lifeSpan:(long) lifeSpan
                   scopeForAutoRefresh:(SCOPE_TYPE) scopeTypeForAutoRefresh
                hasEncryptedProperties:(BOOL) hasEncryptedProperties
                   encryptedProperties:(nullable NSArray*) encryptedProperties;

/*!
 *  @abstract このオブジェクトの情報をデータストーアに同期型で反映する。保存先のデータストーアはpersistentTypeによって制御される。
 *  @param error オブジェクトの更新中に発生したエラーを格
 *  @return object. 保存されたオブジェクトを返す。
 *  @discussion
 *  このメソッドはオブジェクトの状態によってデータストーアに新しいデータを追加、あるいは既存のデータを更新する。新しいデータであるか否かはobjectIdの有無で判断する。objectIdがない場合は新しいデータとして追加する。
 */
- (nullable APObject*) saveObject:(NSError * _Nullable * _Nullable)error;


/*!
 *  @abstract このオブジェクトの情報をデータストーアに非同期型で反映する。保存先のデータストーアはpersistentTypeによって制御される。
 *  @param block オブジェクトの更新が終わった後に実行されるコードを渡す。
 *  @discussion
 *  このメソッドはオブジェクトの状態によってデータストーアに新しいデータを追加、あるいは既存のデータを更新する。新しいデータであるか否かはobjectIdの有無で判断する。objectIdがない場合は新しいデータとして追加する。
 */
- (void) saveWithBlock:(nullable APObjectResultBlock)block;

/*!
 *  @abstract データを同期型で削除する。
 *  @param   error データの削除の際に発生したエラーを格納する
 *
 *  @discussion TBD
 *  
 */

- (BOOL) deleteObject:(NSError * _Nullable * _Nullable) error;

/*!
 *  @abstract データを非同期型で削除する。
 *  @param   successBlock データの削除後の処理を格納する
 *  @param   errorBlock データの削除の際に発生したエラーを格納する
 *
 *  @discussion TBD
 *
 */
- (void) deleteObjectWithBlockOnSuccess:(nullable APResponseBlock) successBlock onError:(nullable APErrorBlock) errorBlock;

/*!
 *  @abstract メモリー上のデータを最新の状態に更新する。
 *  @param block データ更新後の処理
 *
 *  @discussion
 *  ブロックのパラメータ :
 *           object  : 成功時に更新されたデータを格納
 *           error   : 失敗時にエラーが格納される。
 */
- (void) refreshObjectWithBlock:(nonnull APObjectResultBlock) block;

/*!
 *  @abstract オブジェクトのユニークなキーであるobjectIdを指定して一件のデータを取得する。
 *  @param objectId 取得するデータのキー
 *  @param error エラーが発生した場合にエラーを格納するためのエラーインスタンスのポインター
 *
 *  @discussion TBD
 */
+ (nullable instancetype) objectForClass:(nonnull Class) class withId:(nonnull NSString *) objectId error:(NSError * _Nullable * _Nullable)error;

/*!
 *  @abstract オブジェクトのユニークなキーであるobjectIdを指定して一件のデータを取得する。この処理は非同期で行われる。
 *  @param class 取得するデータの型
 *  @param objectId 取得するデータのキー
 *  @param block データ取得後の処理
 *
 *  @discussion TDB
 *
 *  ブロックのパラメータ :
 *           object  : 成功時に更新されたデータを格納
 *           error   : 失敗時にエラーが格納される。
 */
+ (void) objectForClass:(nonnull Class) class withId:(nonnull NSString *) objectId withBlock:(nonnull APObjectResultBlock)block;


/**
 *  自動更新のための条件を返す。
 *  @param lastRefreshTime 前回自動更新した時刻
 *  @return 自動更新ための条件が設定されたAPQueryオブジェクト
 *  @discussion
 *  この条件はオブジェクトの自動更新がオンになっている場合（PersistentTypeがAPPersistentLocalAndServerで、かつisAutoRefreshがTRUE）に使われるサーバーへの最新情報取得のために使われる。
 *  APObjectのサブクラスでこのメソッドをオーバーライドすることで各オブジェクトに独自の自動更新条件を指定できる。
 *  このメソッドをオーバーライドしなかった場合はデフォルトで下記の条件が使われることとなる。
 *  (updateTime >= lastRefreshTime) and ((currentTime – updateTime) < lifeSpan) and (scopeTypeForAutoRefresh = scopeType)
 */
- (nullable NSDictionary*) conditionForAutoRefreshSince:(nonnull NSDate*) lastRefreshTime;

- (nonnull NSNumber*) isAutoRefresh;
- (long) autoRefreshInterval;
- (long) lifeSpan;
- (nonnull NSArray *) encryptedProperties;
- (nonnull NSNumber*) hasEncryptedProperties;

#pragma mark V3 API対応
/*!
 * エイリアス指定なしのSelect。エイリアスはClass名がそのまま使われる。
 */
- (nonnull APSelect*) select;

/*!
 * @param alias Entityのエイリアス
 */
- (nonnull APSelect*) selectAS:(nullable NSString *)alias;

@end

