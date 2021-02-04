//
//  APGateway.h
//  AppPotSDKforSwift
//
//  Created by 成哲 on 2017/07/21.
//  Copyright © 2017年 NC Design. All rights reserved.
//

#import <Foundation/Foundation.h>

@class APResponse;

@interface APGateway : NSObject

/// 認証情報を返す。
typedef void (^ResultBlock)( NSDictionary * _Nullable response);
typedef void (^OriginResultBlock)( NSHTTPURLResponse * _Nullable response);


+ (instancetype _Nullable )initWithServiceName:(NSString *_Nonnull)serviceName
                                           url:(NSString *_Nonnull)url;

- (void)getQueryParam:(nullable id)queryparam body:(nullable NSArray *)body result:(nonnull ResultBlock) resultBlock;
- (void)getQueryParam:(nullable id)queryparam body:(nullable NSArray *)body resultOrigin:(nonnull OriginResultBlock) originResultBlock;

- (void)postQueryParam:(nullable id)queryparam body:(nullable NSArray *)body result:(nonnull ResultBlock) jsonResultBlock;
- (void)postQueryParam:(nullable id)queryparam body:(nullable NSArray *)body resultOrigin:(nonnull OriginResultBlock) originResultBlock;

- (void)putQueryParam:(nullable id)queryparam body:(nullable NSArray *)body result:(nonnull ResultBlock) jsonResultBlock;
- (void)putQueryParam:(nullable id)queryparam body:(nullable NSArray *)body resultOrigin:(nonnull OriginResultBlock) originResultBlock;

- (void)removeQueryParam:(nullable id)queryparam body:(nullable NSArray *)body result:(nonnull ResultBlock) jsonResultBlock;
- (void)removeQueryParam:(nullable id)queryparam body:(nullable NSArray *)body resultOrigin:(nonnull OriginResultBlock) originResultBlock;


@end
