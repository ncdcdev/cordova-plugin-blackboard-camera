//
//  APUserRole.h
//  AppPotSDK
//
//  Created by Roy Kim on 2016/07/31.
//  Copyright © 2016年 TMA Solutions. All rights reserved.
//


/// グループ（組織）におけるユーザーの権限を表すクラス。

#import <Foundation/Foundation.h>

@interface APUserRole : NSObject
/// 企業ID
@property (nonatomic) long long comapnyId;
/// ロールのキー
@property (nonatomic) long long roleId;
/// ロールの名称
@property (nonatomic, strong, nonnull) NSString *roleName;
/// ロールの説明
@property (nonatomic, strong, nullable) NSString *descrioptionOfRole;

@end
