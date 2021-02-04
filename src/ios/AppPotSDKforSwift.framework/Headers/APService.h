//
// APService.h
// AppPlot
//
// Created by NC Design & Consulting Co., Ltd. on 8/20/13.
// Copyright (c) 2013 NC Design & Consulting Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "APTypeDefinition.h"
#import "APObject.h"
#import "APUserGroup.h"
#import "APUser.h"
#import "APMailRequest.h"
#import "APUserRole.h"
#import "APGateway.h"
#import "APMessageOptions.h"

@class APAuthInfo;
@class APResponse;


#define APLogError(s, ...) [[APService sharedInstance] logError:[NSString stringWithFormat:@"APLogger(ERROR) %@ (Line %i): %@",[NSString stringWithFormat: @"%s", __PRETTY_FUNCTION__], __LINE__, [NSString stringWithFormat:(s), ##__VA_ARGS__]]]

#define APLogMonitor(s, ...) [[APService sharedInstance] logMonitor:[NSString stringWithFormat:@"APLogger(MONITOR) %@ (Line %i): %@",[NSString stringWithFormat: @"%s", __PRETTY_FUNCTION__], __LINE__, [NSString stringWithFormat:(s), ##__VA_ARGS__]]]

#define APServiceTimeOut -1

#define TARGET_GROUP @"groupId"
#define TARGET_ROLE @"roleId"
#define TARGET_USER @"userName"


@protocol APServiceDelegate <NSObject>

- (void)processInvalidToken:(nullable NSError*) error;

@end


/**
 * @abstract AppPotが提供する機能を公開するクラス。
 *
 * @discussion NA
 */
@interface APService : NSObject



/**
 *  @abstract Type of log
 *  @property typeOfLog
 *
 *  @discussion
 *	User developer can specify what is type of log.
 *  These logs fit with logLevel then store to database
 *  In there, we have 2 kinds of type of log (Monior and Error).
 *  This will receive Error value by default.
 */
typedef enum NCLogger_LEVEL : int{
    NCLogger_LEVEL_MONITOR ,
    NCLogger_LEVEL_ERROR,
    NCLogger_LEVEL_NONE ,
}   NCLogger_LEVEL;


/// 認証情報を返す。
typedef void (^APAuthInfoBlock)( APAuthInfo * _Nonnull  authInfo);

#pragma mark APServiceDelegate
@property (nonatomic, strong, nullable) id <APServiceDelegate> delegate;
#pragma mark APSeriveの設定情報
/**
 *  @abstract Reset database in server side
 *  @property isResetAppData
 *  @discussion
 *  If isResetDatabase = YES and "sync:" function is called then reset database in server side.
 */
@property (nonatomic, assign) BOOL isResetAppData ;

/**
 *  Device UDID object
 *
 *	The device UDID is an unique of device. This sends to sever then server will keep it and push messages to correct device.
 */
@property (nonatomic, copy, readonly, nullable) NSString* deviceUDID;
/**
 *  @abstract Gather form APNS.
 *  @property deviceToken
 *
 *  @discussion TBD
 *
 */
@property (nonatomic, copy, nullable) NSString* deviceToken;

/*!
 *  @abstract  TBD
 *
 *  @discussion  TBD
 *
 */
@property (nonatomic, copy, nonnull) NSString* appVersion;

/*!
 *
 */
@property (nonatomic, copy, nonnull) NSString* appKey;

/*!
 *  @abstract App ID
 *  @property appID
 *
 *  @discussion
 *  The first time, user developer register an application in Apple Portal. And getting AppID from this.
 *  AppID will identify apps in the same device.
 *	User developer must be implementation this function for the first time init Stew object.
 *  This keeps appID in Stew object and use for actions later.
 *
 */
@property (nonatomic, copy, nonnull) NSString* appID;
/*!
 *  @abstract Initializes a time out
 *  @property timeOut
 *
 *  @discussion
 *	If have problem in to timming response, user will define a time out number.
 *  If over the time out number then Stew will return a time out error.
 */
@property (nonatomic) CGFloat timeoutForRequest;
/*!
 *  @abstract Host name
 *  @property host
 *
 *  @discussion
 *	User developer can assign directly to hostName without using "- (void) stewWithHostName:(NSString*) hostName".
 */
@property (nonatomic, copy, nonnull) NSString* hostName;

/*!
 *  @abstract ContextRoot Name
 *  @property contextRoot
 *
 *  @discussion
 *	User developer should set contextRoot properly. Do not add '/api'. It will be added by the SDK.
 */
@property (nonatomic, copy, nonnull) NSString* contextRoot;

/*!
 *  @abstract Define to indentify application user SSL or not.
 *  @property isUseSSL
 *
 *  @discussion
 *	SSL to identify app use http or https method. It is NO by default
 */
@property (nonatomic) BOOL isUseSSL;
/*!
 *  @abstract Port.
 *  @property portNumber
 *
 *  @discussion
 *	Port of server
 */
@property (nonatomic) int portNumber;

/*!
 *  @abstract  TBD
 *  @property comapnyId
 *
 *  @discussion TBD
 *
 */

@property (nonatomic) int companyId;
/*!
 *  @abstract Headers
 *  @property headers
 *
 *  @discussion
 *	User developer can assign directly to headers without using functions to init headers.
 */
@property (nonatomic, copy, nullable) NSDictionary* headers;

/*!
 *  @abstract Indentify of app
 *  @property tokenID
 *
 *  @discussion
 *	This will be received after user developer authentication with server.
 *  Token ID stores to Keychain because of security.
 *  Token ID is separate apps together in same device.
 */


@property (nonatomic, strong, readonly, nullable) NSString* tokenID;

/**
 *  @abstract ログインユーザー、ユーザーの属したグループやロール、及び認証トークン等、認証・認可に必要なデータを格納するオブジェクト。
 *  @property authInfo
 */
@property (nonatomic, strong, nullable) APAuthInfo* authInfo;


/*!
 *  @abstract Define a number of logging in database.
 *  @property numberOfLogginMaximum
 *
 *  @discussion
 *	User developer can assign a maximum number of logging they can store to database
 */
@property (nonatomic, assign) CGFloat numberOfLoggingMaximum;
@property (nonatomic, assign) int intervalForMessageQueue;

/*!
 *  @abstract  TBD
 *
 *  @discussion
 *	Authentication with 2 options: isUsingPushMessage is YES and NO.
 *  If YES:
 *       User developer must be implement deviceRegistration API or authentication will response error
 *  If NO:
 *       User developer doesn't need implement deviceRegistration API.
 *  Response is NCResponse
 *
 */
@property (nonatomic, assign) BOOL isUsingPushMessage;

@property (nonatomic, assign) BOOL hasLimitedSessionTime;

/*!
 * サービース時間外のチェックを行うための文字列
 * サーバーからのReponseCodeが200かつ該当文字列が設定された場合
 */
@property (nonatomic, strong, nullable) NSString *checkServiceTimeOut;

#pragma mark - シングルトンインスタンス取得用のメソッド
/*!
 *  @abstract APServiceクラスのシングルトンインスタンスを取得する。　Getting a Singleton object
 *
 *  @discussion	このインスタンスはアプリが終了するまで存在する。 This is a singleton object which keep objects alive until the end of app.
 */
+ (nullable instancetype)sharedInstance;

#pragma mark - 初期化メソッド
/*!
 *  @abstract Initializes host name and header
 *
 *  @discussion
 *	If the request requires header, user developer must be use this function to create header.
 */
- (void) setHostName:(nonnull NSString *)hostName contextRoot:(nonnull NSString*) contextRoot customHeaderFields:(nullable NSDictionary*) headers;
/*!
 *  @abstract Initializes host name, header and port
 *
 *  @discussion  TBD
 */
- (void) setHostName:(nonnull NSString *)hostName
         contextRoot:(nonnull NSString*) contextRoot
  customHeaderFields:(nullable NSDictionary*) headers
          portNumber:(int) port;


/*!
 *  @abstract Initializes host name, header, port, app ID
 *
 *  @discussion  TBD
 */
- (void) setAppID:(nonnull NSString*) appID
       appVersion:(nonnull NSString*) version
         hostName:(nonnull NSString*) host
      contextRoot:(nonnull NSString*) contextRoot
customHeaderFields:(nullable NSDictionary*) headers
       portNumber:(int) port
        companyId:(int) companyId;

/**
 *
 */
- (void) setAppID:(nonnull NSString*) appID
       appVersion:(nonnull NSString*) version
           appKey:(nonnull NSString*) appKey
         hostName:(nonnull NSString*) host
      contextRoot:(nonnull NSString*) contextRoot
customHeaderFields:(nullable NSDictionary *)headers
       portNumber:(int) port
        companyId:(int) companyId;


#pragma mark - ログ出力用メソッド
/*!
 *  @abstract ログ送信を始める。
 *  @param logLevel 送信するログのレベル
 *  @discussion
 *  これを呼ばないとログの収集と送信は始まらない。
 */
- (void) startLoggerAtLogLevel:(NCLogger_LEVEL) logLevel;


/*!
 *  @abstract ログ送信を止める。
 *  @discussion
 *  APLoggerインスタンスが削除され、ログの送信が止まる。このメソッドを呼び出した後のログ出力用のメソッド呼び出しは無視されるようになる。
 */
- (void) stopLogger;


/**
 * 送信するログのレベルを変えたい場合に呼び出す。
 * @param level 新しく設定するログのレベル
 * @discussion
 *  この設定はログ収集を開始する際には有効であるが、サーバーの設定によって定期的に上書きされる。
 */
- (void) setLoglevel:(NCLogger_LEVEL) level;

/**
 * ログを送信する時間間隔を指定する。
 * @param seconds ログ送信のインタバルを指定する秒数
 * @discussion
 * 指定しないとデフォルトで600秒が使われる。この時間間隔でサーバー側のログ収集レベルを取得しアプリの設定を更新する。また、そのログレベルに合うログだけを送信する。
 */
- (void) setLogSendingInterval:(NSTimeInterval) seconds;

/*!
 *  @abstract エラーログをサーバーに送信する。
 *  @param message ログ出力用文字列
 *  @discussion
 *  エラー発生時のログをこのメッソドで記述する。ログは端末内のデータベースに保管され、適時にサーバーに送信される。ただし、サーバーからのアプリに対するログレベルの指定がNoneの場合は何も送信されず、ログは削除される。
 */
- (void) logError:(nonnull NSString*) message;

/*!
 *  @abstract モニタリングログをサーバーに送信する。
 *  @param message ログ出力用文字列
 *  @discussion
 *  モニタリングが必要な内容をログとしてこのメッソドで記述する。ログは端末内のデータベースに保管され、適時にサーバーに送信される。ただし、サーバーからのアプリに対するログレベルの指定がNoneあるいはErrorの場合は何も送信されず、ログは削除される。
 */
- (void) logMonitor:(nonnull NSString*) message;


/*!
 *  @abstract ログキューに溜まっているすべてのログを直ちにサーバーに送る。
 *  @discussion
 *  サーバーへのログ送信は最初はアプリによってい指定され、後はサーバーによって設定されたログ送信間隔で行われる。このメソッドを呼び出すことで設定された送信間隔とは関係なく即時にすべてのログを送信できる。
 */
- (void) flushLogs;

#pragma mark 認証関連メソッド
/**
 *  ログイン処理を行う。常に同じユーザーでのログインだけを許す。前回と異なるユーザーでログインしようとするとエラーを返す。
 *
 *  @param username        ユーザーアカウント
 *  @param password        パスワード
 *  @param completionBlock ログインに成功した場合の処理を記述
 *  @param errorBlock      ログインに失敗した場合の処理を記述
 */
- (void) loginByUsername:(nonnull NSString *)username
                password:(nonnull NSString *)password
            onCompletion:(nonnull APAuthInfoBlock)completionBlock
                 onError:(nonnull APErrorBlock)errorBlock;
/**
 *  前回のログイン時と異なるユーザーでログインすることを許すか否かを指定してログイン処理を行う。
 *
 *  @param username        ユーザーアカウント
 *  @param password        パスワード
 *  @param allowDifferentUser 異なるユーザーでのログインを許可するか否かのフラグ
 *  @param completionBlock ログインに成功した場合の処理を記述
 *  @param errorBlock      ログインに失敗した場合の処理を記述
 */
- (void) loginByUsername:(nonnull NSString *)username
                password:(nonnull NSString *)password
      allowDifferentUser:(BOOL) allowDifferentUser
            onCompletion:(nonnull APAuthInfoBlock)completionBlock
                 onError:(nonnull APErrorBlock)errorBlock;
/**
 *  ログアウトする。
 *
 *  @param mustSyncWithServer サーバー側のログアウト処理が必須か否かの指定。
 *  @param completionBlock    ログアウトに成功した場合の処理を記述
 *  @param errorBlock         ログアウトに失敗した場合の処理を記述
 *
 *  mustSyncWithServerがFALSEの場合、オフライン、エラー等によってサーバー側のログアウト処理が済まなくても端末はログアウト状態になる。TRUEの場合はサーバー側のログアウト処理の成功が必須となる。
 */
- (void) logoutWithServerSync:(BOOL)mustSyncWithServer OnCompletion:(nullable APResponseBlock)completionBlock
                    onError:(nullable APErrorBlock)errorBlock;

#pragma mark - トークン操作
/**
 *  現在持っているトークンを削除する。ログイン中であるか否かは関係ない。
 * */
- (void) deleteCurrentToken;

/**
 *  アプリが有限なセッションタイムを持っている場合のみ、現在持っているトークンを削除する。
 *  西武の案件でデバイス登録・ログイン・データベース作成などの操作で既存トークンを削除するとログアウト処理に失敗するので、これを新しく追加した。
 * */
- (void) deleteCurrentTokenIfAppHasLimitedSessionTime;

/**
 *  サーバーからTokenが無効と判定されたとこに呼ばれるメソッド
 *  エラーコード：120 = Tokenの有効期間超過、121: 存在しないToken
 *  @param error 判定内容を含むエラー
 */
- (void) processInvalidToken:(nonnull NSError*) error;

#pragma mark - グループ
// Group -----------------------------------------------------------------
/**
 *  グループ情報を取得（非同期型）
 *  @param resultArrayBlock 処理に成功した場合の処理を記述
 *  @param errorBlock         ログアウトに失敗した場合の処理を記述
 */
- (void) getGroupListOnCompletion:(nonnull APArrayBlock) resultArrayBlock onError:(nonnull APErrorBlock) errorBlock;

/**
 *  グループ情報を取得（同期型）
 *  @param error 失敗した場合のエラー
 */
- (nullable NSArray<APUserGroup *>*) getGroupList:(NSError* _Nullable * _Nullable) error;

/**
 *  新しいグループを追加する
 *  @param newGroup 新規追加するグループの情報
 *  @param addAppToTheGroup このアプリを新規で作成するグループから使用できるようにするか否かを指定する。
 *  @param error 失敗した場合のエラー。
 */
- (nullable APUserGroup*) addNewUserGroup:(nonnull APUserGroup*) newGroup addingCurrentAppToTheGroup:(BOOL) addAppToTheGroup error:(NSError* _Nullable * _Nullable) error;

/**
 *  グループを変更する
 *  @param group 変更するグループの情報
 *  @param error 失敗した場合のエラー。
 */
- (nullable APUserGroup*) updateUserGroup:(nonnull APUserGroup*)group error:(NSError* _Nullable * _Nullable) error;

/**
 *  グループを削除する
 *  @param group 削除するグループの情報
 *  @param error 失敗した場合のエラー。
 */
- (void) deleteUserGroup:(nonnull APUserGroup*)group error:(NSError* _Nullable * _Nullable) error;

#pragma mark - ユーザー
// User -------------------------------------------------------------------
/**
 *  ユーザ情報を取得
 *  @param groupId グループID、[self getGroupListOnCompletion:onError]から取得する
 *  @param resultArrayBlock 処理に成功した場合の処理を記述。APUserクラスのインスタンスを配列に格納して返している。
 *  @param errorBlock         ログアウトに失敗した場合の処理を記述
 */
- (void) getUserListWithGroupId:(NSInteger)groupId onCompletion:(nonnull APArrayBlock) resultArrayBlock onError:(nullable APErrorBlock) errorBlock;
- (void) getUserListWithGroupId:(NSInteger)groupId account:(nullable NSString *)account firstName:(nullable NSString *)firstName lastName:(nullable NSString *)lastName onCompletion:(nonnull APArrayBlock) resultArrayBlock onError:(nullable APErrorBlock) errorBlock;

- (nullable NSArray *)getUserListForGroup:(NSInteger) groupId error:(NSError* _Nullable * _Nullable) error;
- (nullable NSArray *)getUserListForGroup:(NSInteger) groupId account:(nullable NSString *)account firstName:(nullable NSString *)firstName lastName:(nullable NSString *)lastName error:(NSError* _Nullable * _Nullable) error;
- (nullable APUser *)getUserById:(NSInteger)userId error:(NSError*_Nullable * _Nullable) error;
- (nullable APUser*) addNewUser:(nonnull APUser*)user toGroup:(NSInteger) groupId withRole:(nonnull NSString*)roleName error:(NSError* _Nullable * _Nullable) error;
- (nullable APUser*) updateUser: (nonnull APUser*)user error:(NSError* _Nullable * _Nullable) error;
- (void) deleteUser:(nonnull APUser*) user error:(NSError* _Nullable * _Nullable) error;

#pragma mark - ロール
// Role --------------------------------------------------------------------
- (nullable NSArray<APUserRole *>*)getAllRoles:(NSError* _Nullable * _Nullable) error;

//#pragma mark - ファイル
//// File --------------------------------------------------------------------
///**
// *  AppPotサーバーを経由してサーバー側にファイルをアップロードする。
// *  @param fileName アップロードするファイルの名前
// *  @param filePath アップロードするファイルへのパス。ファイル名も含む。
// *  @param completionBlock 画像のアップロードに成功した場合の処理を記述。ファイル情報を使って必要な記録を行うことを想定している。
// *  @param errorBlock ファイルのアップロードに失敗した場合の処理を記述
// */
//- (void) uploadFile:(NSString*) fileName
//                            atPath:(NSString*) filePath
//                      onCompletion:(APFileUploadResponseBlock)completionBlock
//                           onError:(APErrorBlock)errorBlock __attribute__((deprecated));
//
///**
// *  注意：このメソッドの実装は完璧ではない。テストは行っているが、使い手を考えて実装を見直す必要がある。
// *  指定したURLからファイルをダウンロードして指定した場所に保存する。（AppPotサーバーの提供する機能とは関係ない）
// *
// *  @param urlOfFile ダウンロードするファイルの位置をURLで指定。
// *  @param outputPath ダウンロードしたファイルを保存する場所を指定
// *  @param completionBlock ダウンロードに成功した場合の処理を記述
// *  @param errorBlock ダウンロードに失敗した場合の処理を記述
// */
//- (void)downloadImage: (NSString*)urlOfFile toPath:(NSString*)outputPath onCompletion:(APResponseBlock)completionBlock onError:(APErrorBlock)errorBlock;

/**
 *  指定したfileIdからファイルを削除する。
 *
 *  @param fileId 削除するファイルのID
 *  @param block 処理が終わった場合の処理を記述
 */
//- (void) deleteFile:(NSString*)fileId onCompletion:(APResponseResultBlock) completionBlock  __attribute__((deprecated));

#pragma mark - ファイル操作
/**
 *  指定したfilePathにあるファイルをサーバーにアップロードする。
 *
 *  @param filePath アップロードするファイルの名前を含むパスD
 *  @param completionBlock 処理が成功した場合の処理を記述
 *  @param errorBlock 処理が失敗した場合の処理を記述
 */
- (void) uploadFile:(nonnull NSString*)filePath onCompletion:(nonnull APFileUploadResponseBlock)completionBlock onError:(nonnull APErrorBlock)errorBlock;

/**
 *  指定したfilePathにあるファイルでサーバーのファイルを更新する。
 *
 *  @param filePath アップロードするファイルの名前を含むパスD
 *  @param oldFileName 更新対象ファイルのサーバー上の名称
 *  @param completionBlock 処理が成功した場合の処理を記述
 *  @param errorBlock 処理が失敗した場合の処理を記述
 */
- (void) updateFile:(nonnull NSString*)filePath withOldFileName:(nonnull NSString*)oldFileName onCompletion:(nonnull APFileUploadResponseBlock)completionBlock onError:(nonnull APErrorBlock)errorBlock;
/**
 *  ファイル名を指定してサーバーのファリルをダウンロードする。
 *
 *  @param fileName ダウンロードするファイルの名前
 *  @param completionBlock 処理が成功した場合の処理を記述
 *  @param errorBlock 処理が失敗した場合の処理を記述
 */
- (void) getFile:(nonnull NSString*)fileName onCompletion:(nonnull APFileResponseBlock)completionBlock onError:(nonnull APErrorBlock)errorBlock;

/**
 *  サーバーのファイルを削除する。
 *
 *  @param fileName 削除するファイル名
 *  @param completionBlock 処理が成功した場合の処理を記述
 *  @param errorBlock 処理が失敗した場合の処理を記述
 */
- (void) deleteFile:(nonnull NSString*)fileName onCompletion:(nonnull APResponseBlock)completionBlock onError:(nonnull APErrorBlock)errorBlock;

#pragma mark - プッシュメッセージ送信のためのデバイス登録
/*!
 *  @abstract プッシュメッセージの受信のために、AppPotサーバーにデバイストークンを登録する
 *  @param deviceToken 登録するデバイストークン
 *  @param completionBlock 処理に成功した場合の処理を記述
 *  @param errorBlock エラー発生時の処理を記述する
 *  @discussion
 *  AppPotサーバーからのプッシュメッセージを受信するためにはデバイストークンをAppPotサーバーに登録する。
 *  After user developer call authentication API then user developer must be call deviceRegistration API in case if authentication have push notification parameter is YES.
 *  User developer must be implemnent this function in register push notification of Apple function.
 *  Paramenter: deviceToken which's got from Apple.
 */
- (void) deviceRegistration:(nonnull NSString*) deviceToken
               onCompletion:(nonnull APResponseBlock) completionBlock
                    onError:(nonnull APErrorBlock) errorBlock;

#pragma mark - プッシュメッセージ送信
/*!
 * PushMessageを送る。
 * @param message 送信するメッセージ
 * @param title 送信するメッセージのタイトル
 * @param targetArray 送信対象のリスト
 * @param resultBlock 成功した場合の処理
 * @param errorBlock 失敗した場合の処理
 */
- (void)sendPushMessage:(nonnull NSString *)message
              withTitle:(nonnull NSString *)title
          toTargetArray:(nonnull NSArray *)targetArray
           onCompletion:(nonnull APDictionaryBlock)resultBlock
                onError:(nonnull APErrorBlock)errorBlock;

/*!
 * 他のアプリにPushMessageを送る。
 * @param message 送信するメッセージ
 * @param title 送信するメッセージのタイトル
 * @param targetAppID 送信対象のアプリID
 * @param targetAppVersion 送信対象のアプリバージョン
 * @param targetArray 送信対象のリスト {TARGET_GROUP,TARGET_ROLE, TARGET_USER}
 * @param resultBlock 成功した場合の処理
 * @param errorBlock 失敗した場合の処理
 */

- (void)sendPushMessage:(nonnull NSString *)message
              withTitle:(nonnull NSString *)title
            targetAppID:(nonnull NSString *)targetAppID
     toTargetAppVersion:(nonnull NSString *)targetAppVersion
            targetArray:(nonnull NSArray *)targetArray
           onCompletion:(nonnull APDictionaryBlock)resultBlock
                onError:(nonnull APErrorBlock)errorBlock;

/*!
 * PushMessageを送る。
 * @param message 送信するメッセージ
 * @param title 送信するメッセージのタイトル
 * @param targetArray 送信対象のリスト
 * @param sendDate 送信日時を指定(省略した場合、すぐに送信)
 * @param resultBlock 成功した場合の処理
 * @param errorBlock 失敗した場合の処理
 */
- (void)sendPushMessage:(nonnull NSString *)message
              withTitle:(nullable NSString *)title
          toTargetArray:(nonnull NSArray *)targetArray
             atSendDate:(nullable NSDate *)sendDate
           onCompletion:(nonnull APDictionaryBlock)resultBlock
                onError:(nonnull APErrorBlock)errorBlock;

/*!
 * 他のアプリにPushMessageを送る。
 * @param message 送信するメッセージ
 * @param title 送信するメッセージのタイトル
 * @param targetAppID 送信対象のアプリID
 * @param targetAppVersion 送信対象のアプリバージョン
 * @param targetArray 送信対象のリスト {TARGET_GROUP,TARGET_ROLE, TARGET_USER}
 * @param sendDate 送信日時を指定(省略した場合、すぐに送信)
 * @param resultBlock 成功した場合の処理
 * @param errorBlock 失敗した場合の処理
 */

- (void)sendPushMessage:(nonnull NSString *)message
              withTitle:(nullable NSString *)title
            targetAppID:(nonnull NSString *)targetAppID
     toTargetAppVersion:(nonnull NSString *)targetAppVersion
            targetArray:(nonnull NSArray *)targetArray
             atSendDate:(nullable NSDate *)sendDate
           onCompletion:(nonnull APDictionaryBlock)resultBlock
                onError:(nonnull APErrorBlock)errorBlock;

/*!
 * 同じアプリのユーザー全体へPushMessageを送る。
 * @param message 送信するメッセージ
 * @param title 送信するメッセージのタイトル
 * @param resultBlock 成功した場合の処理
 * @param errorBlock 失敗した場合の処理
 */

- (void)sendToAllPushMessage:(nonnull NSString *)message
                   withTitle:(nonnull NSString *)title
                onCompletion:(nonnull APDictionaryBlock)resultBlock
                     onError:(nonnull APErrorBlock)errorBlock;

/*!
 * 他のアプリの全体ユーザーへPushMessageを送る。（企業単位）
 * @param message 送信するメッセージ
 * @param title 送信するメッセージのタイトル
 * @param targetAppID 送信対象のアプリID
 * @param targetAppVersion 送信対象のアプリのバージョン
 * @param resultBlock 成功した場合の処理
 * @param errorBlock 失敗した場合の処理
 */

- (void)sendToAllPushMessage:(nonnull NSString *)message
                   withTitle:(nonnull NSString *)title
                 targetAppID:(nonnull NSString *)targetAppID
            targetAppVersion:(nonnull NSString *)targetAppVersion
                onCompletion:(nonnull APDictionaryBlock)resultBlock
                     onError:(nonnull APErrorBlock)errorBlock;


/*!
 * 他のアプリにPushMessageを送る。
 * @param message 送信するメッセージ
 * @param title 送信するメッセージのタイトル
 * @param targetAppID 送信対象のアプリID
 * @param targetAppVersion 送信対象のアプリバージョン
 * @param targetArray 送信対象のリスト {TARGET_GROUP,TARGET_ROLE, TARGET_USER}
 * @param sendDate 送信日時を指定(省略した場合、すぐに送信)
 * @param options Push送信のOptionsデータ
 * @param resultBlock 成功した場合の処理
 * @param errorBlock 失敗した場合の処理
 */
- (void)sendPushMessage:(nonnull NSString *)message
              withTitle:(nullable NSString *)title
            targetAppID:(nonnull NSString *)targetAppID
     toTargetAppVersion:(nonnull NSString *)targetAppVersion
            targetArray:(nullable NSArray *)targetArray
             atSendDate:(nullable NSDate *)sendDate
             optionData:(nullable APMessageOptions *)options
           onCompletion:(nullable APDictionaryBlock)resultBlock
                onError:(nullable APErrorBlock)errorBlock;


#pragma mark - メール送信
/**
 *  メールを送信する。
 *  @param mailRequest 送信するメールの情報
 *  @param error 送信時のエラー情報を格納
 */
- (void) sendMail:(nonnull APMailRequest*) mailRequest error:(NSError* _Nullable * _Nullable) error;

#pragma mark - Gateway API
- (nonnull APGateway *)gatewayWithServiceName:(NSString *_Nonnull)serviceName
                                          url:(NSString *_Nonnull)url;

@end
