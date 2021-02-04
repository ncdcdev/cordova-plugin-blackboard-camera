//
//  APUserGroup.h
//  Stew
//
//  Created by Roy S. Kim on 1/29/14.
//  Copyright (c) 2014 TMA Solutions. All rights reserved.
//

#import <Foundation/Foundation.h>

/// ユーザーが属するグループの情報
@interface APUserGroup : NSObject

/// グループの識別子
@property (nonatomic) NSInteger groupId;
/// グループ名
@property (nonatomic, strong, nullable) NSString *groupName;
// Property name of Server is 'description'.
/// グループの説明
@property (nonatomic, strong, nullable) NSString *descriptionOfGroup;

// Below two properties are used to show roles in group for a user when this instance is refered by APUser instance.
// We don't need to define 'APRole' class because of bellow properties.
/// このグループでユーザーが持つロールの識別子
@property (nonatomic) NSInteger roleId;
/// このグループでユーザーが持つロールの名称
@property (nonatomic, strong, nullable) NSString *roleName;
@end
    
