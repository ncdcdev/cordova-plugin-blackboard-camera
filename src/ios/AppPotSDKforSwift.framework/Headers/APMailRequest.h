//
//  APMailRequest.h
//  AppPotSDK
//
//  Created by Roy Kim on 2016/08/03.
//  Copyright © 2016年 TMA Solutions. All rights reserved.
//

#import <Foundation/Foundation.h>

/// メール送信をリクエストするための情報を格納するクラス。

@interface APMailRequest : NSObject

/// 送信元のメールアドレス。オプション項目
@property (nonatomic, strong, nullable) NSString *mailFrom;

/// 送信先（To）メールアドレスリスト。オプション項目
@property (nonatomic, strong, nullable) NSArray *mailTo;
/// 送信先（CC）メールアドレスリスト。オプション項目
@property (nonatomic, strong, nullable) NSArray *mailCc;
/// 送信先（BCC）メールアドレスリスト。オプション項目
@property (nonatomic, strong, nullable) NSArray *mailBcc;

/// 送信先経路。必須項目
@property (nonatomic, strong, nonnull) NSString *sendingRouteName;

///  メール件名。オプション項目
@property (nonatomic, strong, nullable) NSString *subject;
///  メール本文。オプション項目
@property (nonatomic, strong, nullable) NSString *body;


@end
