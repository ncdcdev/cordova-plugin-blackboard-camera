//
//  APUser.h
//  AppPotSDK
//
//  Created by Roy S. Kim on 6/4/14.
//  Copyright (c) 2014 TMA Solutions. All rights reserved.
//

#import <Foundation/Foundation.h>
/// ユーザー情報
@interface APUser : NSObject

/// アカウント名。一回決まると変更は不可。
@property (nonatomic, strong, nullable) NSString *account;
/// パスワード
@property (nonatomic, strong, nullable) NSString *password;
/// ユーザーの識別子
@property (nonatomic) long long userId;
/// ユーザーの名前：名
@property (nonatomic, strong, nullable) NSString* firstName;
/// ユーザーの名前：性
@property (nonatomic, strong, nullable) NSString* lastName;

// List of Groups this user is in in. (Role for each group is included in APUserGroup class.)
/// ユーザーが属するグループの一覧。グループと各グループでのロールの情報を格納している。
@property (nonatomic, strong, nullable) NSArray* groupsWithRole;

@end
