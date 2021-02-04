//
//  APQuery.h
// AppPlot
//
// Created by NC Design & Consulting Co., Ltd. on 8/20/13.
// Copyright (c) 2013 NC Design & Consulting Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "APService.h"
#import "APResponse.h"
#import "APObject.h"
#import "APGroupQuery.h"

/// 取得条件を指定し、AppPotで管理するモデルクラスのデータを取得するクラス。
@interface APQuery : NSObject

/// ソート基準
typedef NS_ENUM(NSInteger, SortType) {
    APSortDefault = 0, // QQQ Do we need Default?
	APSortAscending,
    ApSortDescending
};

/// 検索のタイプ。曖昧検索などの指定をする際に使う。
typedef NS_ENUM(NSInteger, SearchType) {
    APEqual = 1,
    APNotEqual,
    APLike,
    APGreaterThan,
    APGreaterThanOrEqual,
    APLessThan,
    APLessThanOrEqual,
    APContain,
    APNotContain
};

typedef NS_ENUM(NSInteger, WhereType) {
    APWhereAND = 0,
    APWhereOR = 1,
    APWhereGroup = 2
};

typedef NS_ENUM(NSInteger, JoinType) {
    APInnerJoinType = 1,
    APLeftOuterJoinType = 2,
    APRightOuterJoinType = 3
};

/*!
 *  @abstract このクエリが保持している条件。
 *  @property condition
 *
 *  @discussion
 *  このクエリに指定されたデータ取得用の条件
 */
@property (nonatomic, strong, nullable) NSMutableDictionary* condition;

/*!
 *  @abstract このクラスが結果としてのデータのリストをキャッシュするか否かを指定する。
 *  @property isCachingResult
 *
 *  @discussion
 *  この値がYESの場合は一回取得したデータはこのクラス内にそのまま保存され、２回目からのデータリクエストは内部のキャッシュから行われる。キャッシュされたデータを使うには以下に注意する。
 *  - データはメモリー上にキャッシュされるので多量のデータをキャッシュすることはメモリーを使いすぎることに繋がる。
 *  - 他の操作によってデータが変更されてもキャッシュされたデータには反映されない。反映するには再びデータを読み込む必要がある。
 */
@property (nonatomic) BOOL isCachingResult;

/*!
 *  @abstract 取得するデータの最大件数。
 *  @property maxResult
 *
 *  @discussion
 *  指定しない場合はSDKのデフォルトの値（１０００件）が使用される。
 */
@property (nonatomic) int maxResult;


/*!
 *  @abstract 同じ条件で何個目のデータセットを取得するかを指定。
 *  @property pageIndex
 *
 *  @discussion
 *  データの取得にページングを行う場合に指定する。指定しない場合はSDKのデフォルトの値（１）が使用される。
 */
@property (nonatomic) int pageIndex;

/*!
 *  @abstract Initalizes a query with class
 *  @property _class
 *
 *  @discussion
 *  User developer will initalize a query first for this class. 
 *  Then all queries (update/ delete/ insert/ getData) will be done for this class.
 *
 */
+ (nonnull APQuery *) queryWithClass:(nonnull Class ) _class;

/*!
 *  @property joinClass
 *  @property joinColumn
 *
 *  @discussion INNER JOIN joinClass ON BASE_CLASS.joinColumn = joinClass.joinColumn;
 */
+ (nonnull APQuery *) queryOnInnerJoinClass:(nonnull Class)joinClass withJoinColumn:(nonnull NSString *)joinColumn;

/*!
 *  @property joinClass
 *  @property joinColumn
 *  @property baseClass
 *  @property baseColumn
 *
 *  @discussion INNER JOIN joinClass ON baseClass.baseColumn = joinClass.joinColumn;
 */

+ (nonnull APQuery *) queryOnInnerJoinClass:(nonnull Class )joinClass withJoinColumn:(nonnull NSString *)joinColumn withBaseClass:(nonnull Class)baseClass withBaseColumn:(nonnull NSString *)baseColumn;

/*!
 *  @property joinClass
 *  @property joinColumn
 *
 *  @discussion LEFT OUTER JOIN joinClass ON BASE_CLASS.joinColumn = joinClass.joinColumn;
 */

+ (nonnull APQuery *) queryOnLeftOuterJoinClass:(nonnull Class)joinClass withJoinColumn:(nonnull NSString *)joinColumn;

/*!
 *  @property joinClass
 *  @property joinClass
 *  @property baseClass
 *  @property baseColumn
 *
 *  @discussion LEFT OUTER JOIN joinClass ON baseClass.baseColumn = joinClass.joinColumn;
 */

+ (nonnull APQuery *) queryOnLeftOuterJoinClass:(nonnull Class )joinClass withJoinColumn:(nonnull NSString *)joinColumn withBaseClass:(nonnull Class)baseClass withBaseColumn:(nonnull NSString *)baseColumn;

/*!
 *  @abstract 指定した条件でデータを取得する。データのキャッシュが有効で、キャッシュが存在する場合はそれを返す。
 *
 *  @discussion
 *  このメソッドは非同期型で処理されるので呼び出し元のスレッドはこのメソッドを読んだ後結果を待たずにその後の処理を続行する。処理結果はブロックから後で取得できる。
 */
- (void) resultSetWithBlock:(nonnull APSearchResultBlock)block;


/*!
 *  @abstract 指定した条件でデータを取得する。データのキャッシュが有効で、キャッシュが存在する場合はそれを返す。
 *
 *  @discussion
 *  このメソッドは同期型で処理されるので呼び出し元の処理が待機することになる。画面が固まらないようにする等、待機時間を考慮する必要がある。
 */
- (nullable APSearchResult*) resultSet:(NSError * _Nullable * _Nullable ) error;


/*!
 *  @abstract このクエリオブジェクトが保持しているキャッシュデータを最新のものにする。この処理は非同期で実行される。
 *  @param errorBlock エラー発生時の実行処理
 *  @return BOOL 処理結果を返す。キャッシュがなかった場合はTRUEを返す。
 *
 *  @discussion
 *  キャッシュを使用しない設定の場合はこのメソッドは何もしない。
 */
- (BOOL) refreshCacheWithBlock:(nonnull APErrorBlock) errorBlock;


/*!
 *  @abstract このクエリオブジェクトが保持しているキャッシュデータを最新のものにする。この処理は同期で実行されるので時間がかかる可能性がある。
 *  @param error 実行時に発生したエラー
 *  @return BOOL 処理結果を返す。キャッシュがなかった場合はTRUEを返す。
 *
 *  @discussion
 *  キャッシュを使用しない設定の場合はこのメソッドは何もしない。
 */
- (BOOL) refreshCache:(NSError * _Nullable * _Nullable) error;


/*!
 *  @abstract 取得するデータに対する条件を記述
 *  @param value 条件と比較する値
 *
 *  @discussion
 *  各メソッドが指定する条件はメソッド名を参照する。
 *
 */
- (void) whereKey:(nonnull NSString*) columnKey equalTo:(nonnull id) value;
- (void) whereKey:(nonnull NSString*) columnKey notEqualTo:(nonnull id) value;
- (void) whereKey:(nonnull NSString*) columnKey greaterThan:(nonnull id) value;
- (void) whereKey:(nonnull NSString*) columnKey greaterThanOrEqual:(nonnull id) value;
- (void) whereKey:(nonnull NSString*) columnKey lessThan:(nonnull id) value;
- (void) whereKey:(nonnull NSString*) columnKey lessThanOrEqual:(nonnull id) value;

/*!
 *  @abstract Refresh with condition like
 *  @property
 *
 *  @discussion
 *  Requires a particular key's object must be contain the value.
 *
 */
- (void) whereKey:(nonnull NSString*) columnKey like:(nonnull NSString*) value;

- (void) whereKey:(nonnull NSString*) columnKey contain:(nonnull NSArray*) values;
- (void) whereKey:(nonnull NSString*) columnKey notContain:(nonnull NSArray*) values;

// where Class.column = value
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey equalTo:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey notEqualTo:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey greaterThan:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey greaterThanOrEqual:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey lessThan:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey lessThanOrEqual:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey like:(nonnull NSString*) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey contain:(nonnull NSArray*) values;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey notContain:(nonnull NSArray*) values;


// WhereType対応
- (void) whereOrKey:(nonnull NSString*) columnKey equalTo:(nonnull id) value;
- (void) whereOrKey:(nonnull NSString*) columnKey notEqualTo:(nonnull id) value;
- (void) whereOrKey:(nonnull NSString*) columnKey greaterThan:(nonnull id) value;
- (void) whereOrKey:(nonnull NSString*) columnKey greaterThanOrEqual:(nonnull id) value;
- (void) whereOrKey:(nonnull NSString*) columnKey lessThan:(nonnull id) value;
- (void) whereOrKey:(nonnull NSString*) columnKey lessThanOrEqual:(nonnull id) value;
- (void) whereOrKey:(nonnull NSString*) columnKey like:(nonnull NSString*) value;
- (void) whereOrKey:(nonnull NSString*) columnKey contain:(nonnull NSArray*) values;
- (void) whereOrKey:(nonnull NSString*) columnKey notContain:(nonnull NSArray*) values;

- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey equalTo:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey notEqualTo:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey greaterThan:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey greaterThanOrEqual:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey lessThan:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey lessThanOrEqual:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey like:(nonnull NSString*) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey contain:(nonnull NSArray*) values;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey notContain:(nonnull NSArray*) values;

// ignoreCase
- (void) whereKey:(nonnull NSString*) columnKey equalToIgnoreCase:(nonnull id) value;
- (void) whereKey:(nonnull NSString*) columnKey notEqualToIgnoreCase:(nonnull id) value;
- (void) whereKey:(nonnull NSString*) columnKey likeToIgnoreCase:(nonnull NSString*) value;

- (void) whereOrKey:(nonnull NSString*) columnKey equalToIgnoreCase: (nonnull id) value;
- (void) whereOrKey:(nonnull NSString*) columnKey notEqualToIgnoreCase:(nonnull id) value;
- (void) whereOrKey:(nonnull NSString*) columnKey likeIgnoreCase:(nonnull id) value;

- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey equalToIgnoreCase:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey notEqualToIgnoreCase:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass key:(nonnull NSString*) columnKey likeToIgnoreCase:(nonnull NSString*) value;

- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey equalToIgnoreCase:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey notEqualToIgnoreCase:(nonnull id) value;
- (void) whereClass:(nonnull Class)columnClass orKey:(nonnull NSString*) columnKey likeIgnoreCase:(nonnull id) value;

/*!
 *  block文の引数APGroupQueryから条件を追加するとグループ条件化される
 *  グループ条件の中はすべてAND条件になる
 *  ANDは次の条件があった場合に後ろに付く
 *  [ex]
 *  [query whereGroupAnd:^(APQuery* groupQuery) {
 *      [groupQuery whereKey:@"column1" equalTo:@"abc"];
 *      [groupQuery whereKey:@"column2" like:@"abc"];
 *  }];
 *  [sql]
 *  ... (column1 = "abc" AND column2 like '%abc%') AND ...
 */
- (void) whereGroupAnd:(nonnull void(^)(APGroupQuery* _Nonnull groupQuery))groupBlock;

/*!
 *  block文の引数APGroupQueryから条件を追加するとグループ条件化される
 *  グループ条件の中はすべてAND条件になる
 *  ORは次の条件があった場合に後ろに付く
 *  [ex]
 *  [query whereGroupOr:^(APQuery* groupQuery) {
 *      [groupQuery whereKey:@"column1" equalTo:@"abc"];
 *      [groupQuery whereKey:@"column2" like:@"abc"];
 *  }];
 *  [sql]
 *  ... (column1 = "abc" AND column2 like '%abc%') OR ...
 */
- (void) whereGroupOr:(nonnull void(^)(APGroupQuery* _Nonnull groupQuery))groupBlock;

/*!
 *  @deprecated Use ascSortByKey: or descSortByKey:
 *  @abstract Filter response data
 *  @property
 *
 *  @discussion
 *  We can sort response data by ASCE and DESCE
 *
 */
- (void) sortByKey:(nonnull NSString*) columnKey sortType:(SortType) sortType __attribute__ ((deprecated));
- (void) ascSortByKey:(nonnull NSString*) columnKey;
- (void) descSortByKey:(nonnull NSString*) columnKey;

- (void) ascSortByClass:(nonnull Class)sortClass key:(nonnull NSString*) columnKey;
- (void) descSortByClass:(nonnull Class)sortClass key:(nonnull NSString*) columnKey;


/*!
 *  @abstract Provide a way to other people can access this object
 *  @property
 *
 *  @discussion
 *  Scope tpe i.e USER and GROUP
 *
 */
- (void) setScopeType:(NSInteger) scopeType;

- (void)addJoinQuery:(nonnull APQuery *)joinQuery;

+ (nonnull NSString *)toStringJoinType:(JoinType)joinType;
@end
