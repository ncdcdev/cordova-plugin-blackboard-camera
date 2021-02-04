//
//  APMessageOptions.h
//  AppPotSDKforSwift
//
//  Created by 成哲 on 2017/12/05.
//  Copyright © 2017年 NC Design. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface APMessageOptions : NSObject

@property (nonatomic, strong, nullable) NSString *icon;
@property (nonatomic, strong, nullable) NSString *sound;
@property (nonatomic, strong, nullable) NSNumber *badge;
@property (nonatomic, strong, nullable) NSString *messageLocKey;
@property (nonatomic, strong, nullable) NSArray *messageLocArgs;
@property (nonatomic, strong, nullable) NSString *titleLocKey;
@property (nonatomic, strong, nullable) NSArray *titleLocArgs;
@property (nonatomic, strong, nullable) NSDictionary *customData;

+ (nonnull instancetype)newInstance;

- (void)mergeData:(nonnull NSMutableDictionary *)parameter;
@end
