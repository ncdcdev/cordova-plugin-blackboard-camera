//
//  APAuthInfo.h
//  Stew
//
//  Created by Roy S. Kim on 1/28/14.
//  Copyright (c) 2014 TMA Solutions. All rights reserved.
//

#import <Foundation/Foundation.h>

/// AppPotへのアクセスやログインによって取得されたに認証情報
@interface APAuthInfo : NSObject

/// ログン中か否かの情報
@property (nonatomic, readonly) BOOL isLogInUser;

/// AppPotにアクセスするためのトークン。非ログイン時のトークン。
@property (nonatomic, strong, nullable) NSString *anonymousAuthToken;

/// AppPotにアクセスするためのトークン。非ログイン時のトークン。
@property (nonatomic, strong, nullable) NSString *authToken;
/// ユーザーID
@property (nonatomic) long userId;
/// ユーザーのアカウント名
@property (nonatomic, strong, nullable) NSString *userAccount;
/// ユーザーの名前：名
@property (nonatomic, strong, nullable) NSString *firstName;
/// ユーザーの名前：性
@property (nonatomic, strong, nullable) NSString *lastName;
/// ユーザーが属するグループの一覧
@property (nonatomic, strong, nullable) NSArray *userGroups;

//TODO: Add Comment 初期化に失敗するとnilを返す。
+ (nullable instancetype) authInfoWithDictionary:(nonnull NSDictionary*) dictionary;
//TODO: Add Comment 初期化に失敗するとnilを返す。
+ (nullable instancetype) authInfoWithJSON:(nonnull NSString *) jsonData;

@end
