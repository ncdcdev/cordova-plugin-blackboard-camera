//
//  APGroupQuery.h
//  AppPotSDK
//
//  Created by NC Design & Consulting Co., Ltd. on 2014/11/26.
//  Copyright (c) 2014年 TMA Solutions. All rights reserved.
//

#import "Foundation/NSObject.h"
@class APQuery;

NS_ASSUME_NONNULL_BEGIN

/// APQueryクラスに対してデータ取得のための多様な条件を指定するクラス
@interface APGroupQuery : NSObject



/// 初期化クラス。
- (id)initWithAPQuery:(APQuery *)query;

/// 同じ値のデータを条件として指定する
- (void) whereKey:(NSString*) columnKey equalTo:(id) value;
/// 異なる値のデータを条件として指定する
- (void) whereKey:(NSString*) columnKey notEqualTo:(id) value;
/// もっと大きな値のデータを条件として指定する
- (void) whereKey:(NSString*) columnKey greaterThan:(id) value;
/// もっと大きいか同じ値のデータを条件として指定する
- (void) whereKey:(NSString*) columnKey greaterThanOrEqual:(id) value;
/// もっと小さい値のデータを条件として指定する
- (void) whereKey:(NSString*) columnKey lessThan:(id) value;
/// もっと小さいか同じ値のデータを条件として指定する
- (void) whereKey:(NSString*) columnKey lessThanOrEqual:(id) value;
/// 一部が同じ値のデータを条件として指定する
- (void) whereKey:(NSString*) columnKey like:(NSString*) value;
/// 指定した値を含むデータを条件として指定する
- (void) whereKey:(NSString*) columnKey contain:(NSArray*) values;
/// 指定した値を含まないデータを条件として指定する
- (void) whereKey:(NSString*) columnKey notContain:(NSArray*) values;

/// 他にクラスに対して指定した値をデータを条件として指定する。
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey equalTo:(id) value;
/// 他にクラスに対して指定した値と異なるデータを条件として指定する。
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey notEqualTo:(id) value;
/// 他にクラスに対して指定した値よりもっと大きい値のデータを条件として指定する
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey greaterThan:(id) value;
/// 他にクラスに対して指定した値よりもっと大きいか同じ値のデータを条件として指定する
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey greaterThanOrEqual:(id) value;
/// 他にクラスに対して指定した値より小さい値のデータを条件として指定する
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey lessThan:(id) value;
/// 他にクラスに対して指定した値より小さいか同じ値のデータを条件として指定する
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey lessThanOrEqual:(id) value;
/// 他にクラスに対して指定した値と一部が同じ値のデータを条件として指定する
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey like:(NSString*) value;
/// 他にクラスに対して指定した値を含む値のデータを条件として指定する
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey contain:(NSArray*) values;
/// 他にクラスに対して指定した値を含まない値のデータを条件として指定する
- (void) whereClass:(Class)columnClass key:(NSString*) columnKey notContain:(NSArray*) values;

@end

NS_ASSUME_NONNULL_END
