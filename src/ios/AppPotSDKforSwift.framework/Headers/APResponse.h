//
//  APResponse.h
// AppPlot
//
// Created by NC Design & Consulting Co., Ltd. on 8/20/13.
// Copyright (c) 2013 NC Design & Consulting Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
/*!
 * @abstract AppPotからのレスポンスを格納するオブジェクト
 * @discussion A
 */
@interface APResponse : NSObject

/*!
 *  @abstract レスポンスのステータス　Status of response
 *  @property status
 *
 *  @discussion
 *  Status of response is "error" or "OK"
 */
@property (nonatomic, strong, nonnull) NSString* status;
/*!
 *  @abstract レスポンスの返り値を格納。　Result of response
 *  @property result
 *
 *  @discussion A
 */
@property (nonatomic, strong, nullable) NSString* result;
/*!
 *  @abstract レスポンスの追加情報を格納。　Description of response
 *  @property description
 *
 *  @discussion A
 */
@property (nonatomic, strong, nullable) NSString* description;

@end
