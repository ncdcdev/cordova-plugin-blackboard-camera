//
//  APSearchResult.h
//  AppPotSDK
//
//  Created by NCDC on 2014/12/18.
//  Copyright (c) 2014年 TMA Solutions. All rights reserved.
//

#import <Foundation/Foundation.h>

/// APQueryクラスを使ったデータの検索結果を格納するクラス。
@interface APSearchResult : NSObject

/// データの数
@property (nonatomic, assign) int counter;
/// ページングする場合、全体のページ数
@property (nonatomic, assign) int totalPage;
/// ページングする場合、このクラスに格納されたデータが何ページに該当するかの情報
@property (nonatomic, assign) int pageIndex;
/// 実際のデータを格納した配列
@property (nonatomic, strong, nullable) NSArray* resultArray;
@end
