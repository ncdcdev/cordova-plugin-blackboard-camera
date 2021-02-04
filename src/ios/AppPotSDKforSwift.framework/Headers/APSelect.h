//
//  APSelect.h
//  AppPotSDKforSwift
//
//  Created by SCJ on 2017/08/15.
//  Copyright © 2017年 NC Design. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "APTypeDefinition.h"


@interface APSelect : NSObject

typedef NS_ENUM(NSInteger, APJoinType) {
    APJoinTypeInnerJoin = 1,
    APJoinTypeLeftOuterJoin = 2,
    APJoinTypeRightOuterJoin = 3
};

/// ソート基準
typedef NS_ENUM(NSInteger, APSortType) {
    APSortTypeAscending,
    APSortTypeDescending
};

/*!
 * Whereを指定する
 * @param where NSString Where文のクエリ
 */
- (nonnull APSelect *) where:(nonnull NSString *) where;

/*!
 * Whereを指定する
 * @param where NSString Where文のクエリ
 * @param param id クエリに指定する値
 */
- (nonnull APSelect *) where:(nonnull NSString *) where withParam:(nullable id) param;

/*!
 * Whereを指定する
 * @param where NSString Where文のクエリ
 * @param params NSArray クエリに指定する値の配列
 */
- (nonnull APSelect *) where:(nonnull NSString *) where withParams:(nullable NSArray *) params;

/*!
 * Joinを指定する
 * @param join NSString Joinクエリ
 * @param entityClass Class JoinするEntityのClass
 * @param entityAlias NSString JoinするEntityのAlias
 */
- (nonnull APSelect *) join:(nonnull NSString *) join withEntity:(nonnull Class) entityClass withEntityAlias:(nonnull NSString *) entityAlias;

/*!
 * Joinを指定する。パラメーター指定なし。
 * @param join NSString Joinクエリ
 * @param entityClass Class JoinするEntityのClass
 * @param entityAlias NSString JoinするEntityのAlias
 * @param joinType APJoinType Jointするタイプ
 */
- (nonnull APSelect *) join:(nonnull NSString *) join withEntity:(nonnull Class) entityClass withEntityAlias:(nonnull NSString *) entityAlias withType:(APJoinType) joinType;

/*!
 * Joinを指定する。全て「InnerJoin」になる
 * @param join NSString Joinクエリ
 * @param entityClass Class JoinするEntityのClass
 * @param entityAlias NSString JoinするEntityのAlias
 * @param params NSArray Joinクエリに指定するパラメータ値の配列
 */
- (nonnull APSelect *) join:(nonnull NSString *) join withEntity:(nonnull Class) entityClass withEntityAlias:(nonnull NSString *) entityAlias withParams:(nullable NSArray *) params;

/*!
 * Joinを指定する
 * @param join NSString Joinクエリ
 * @param entityClass Class JoinするEntityのClass
 * @param entityAlias NSString JoinするEntityのAlias
 * @param params NSArray Joinクエリに指定するパラメータ値の配列
 * @param joinType APJoinType Jointするタイプ
 */
- (nonnull APSelect *) join:(nonnull NSString *) join withEntity:(nonnull Class) entityClass withEntityAlias:(nonnull NSString *) entityAlias withParams:(nullable NSArray *) params withType:(APJoinType) joinType;

/*!
 * 複数のJoinを指定する
 * @param joins NSArray Joinクエリの配列
 * @param entityClasses NSArray JoinするEntityの配列
 * @param entityAliases NSArray JoinするEntityのAliasの配列
 * @param joinParams NSArray Joinクエリに指定するパラメータ値の配列（2重配列）
 * @param joinTypes NSArray JoinするTypeの配列
 */
- (nonnull APSelect *) joins:(nonnull NSArray *) joins withEntityClasses:(nonnull NSArray *) entityClasses withEntityAliases:(nonnull NSArray *) entityAliases withParams:(nullable NSArray *) joinParams withTypes:(nonnull NSArray *) joinTypes;

/*!
 * OrderByを指定する
 * @param column NSString
 * @param sortType APSortType (ASC, DESC)
 */
- (nonnull APSelect *) orderByColumn:(nonnull NSString *)column withType:(APSortType) sortType;

/*!
 * 複数のOrderByを指定する
 * @param columns NSArray
 * @param sortTypes NSArray(APSortType)
 */
- (nonnull APSelect *) orderByColumns:(nonnull NSArray *)columns withTypes:(nonnull NSArray *) sortTypes;

/*!
 * offsetとlimitを指定する
 * ※注意：listCountには値を指定しても無効になる
 * @param from NSInteger (offset)
 * @param to NSInteger (limit)
 */
- (nonnull APSelect *) rangeFrom:(NSInteger)from to:(NSInteger)to;

/*!
 * Selectの結果を取得する
 * @param result APSelectResultBlock
 */
- (void)findList:(nonnull APSelectResultBlock) result;

- (void)listCount:(nonnull APSelectCountBlock) result;

/*!
 * これを直接Callしない。APObject.selectを通してCallすること。
 */
+ (nonnull APSelect *) queryWithClass:(nonnull Class ) _class withAlias:(nullable NSString *)alias withPersistentType:(NSInteger) persistentType;

@end
